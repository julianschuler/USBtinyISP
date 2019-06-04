// ======================================================================
// main.c
//
// This file is a part of the ATtiny24A based USBtinyISP project
// by Julian Schuler (https://github.com/julianschuler/usbtinyisp).
//
// It is based on the USBtiny project by Dick Streefland
// (https://dicks.home.xs4all.nl/avr/usbtiny/index.html)
//
// Copyright (C) 2006-2010 Dick Streefland
// Copyright (C) 2018 Julian Schuler
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// ======================================================================


#include <avr/io.h>
#include <avr/interrupt.h>
#include <def.h>
#include <usb.h>

#define USE_OLD_PCB		1	//necessary due to an unintended pin switch between MOSI and SCK, will be removed after a new PCB has been created
#define USE_USI_SPI		1	//set to 1 to use USI for SPI, limited to DO, DI and USCK pins for MOSI, MISO and SCK


// ----------------------------------------------------------------------
// I/O pins:
// ----------------------------------------------------------------------
#define	PORT		PORTA
#define	DDR			DDRA
#define	PIN			PINA

#define	LED			PA2		// output
#define	RESET		PA3		// output
#define	MISO		PA6		// input

#if USE_OLD_PCB
#define	MOSI		PA4		// output
#define	SCK			PA5		// output
#else
#define	MOSI		PA5		// output
#define	SCK			PA4		// output
#endif

#define	LED_MASK	_BV(LED)
#define	RESET_MASK	_BV(RESET)
#define	MOSI_MASK	_BV(MOSI)
#define	MISO_MASK	_BV(MISO)
#define	SCK_MASK	_BV(SCK)


#if USE_OLD_PCB
#undef USE_USI_SPI
#define USE_USI_SPI	 	0	//USI SPI only usable with new PCB
#endif


enum {
	// Generic requests
	USBTINY_ECHO,			// echo test
	USBTINY_READ,			// read byte
	USBTINY_WRITE,			// write byte
	USBTINY_CLR,			// clear bit 
	USBTINY_SET,			// set bit
	
	// Programming requests
	USBTINY_POWERUP,		// apply power (wValue:SCK-period, wIndex:RESET)
	USBTINY_POWERDOWN,		// remove power from chip
	USBTINY_SPI,			// issue SPI command (wValue:c1c0, wIndex:c3c2)
	USBTINY_POLL_BYTES,		// set poll bytes for write (wValue:p1p2)
	USBTINY_FLASH_READ,		// read flash (wIndex:address)
	USBTINY_FLASH_WRITE,	// write flash (wIndex:address, wValue:timeout)
	USBTINY_EEPROM_READ,	// read eeprom (wIndex:address)
	USBTINY_EEPROM_WRITE,	// write eeprom (wIndex:address, wValue:timeout)
	USBTINY_DDRWRITE,		// set port direction
	USBTINY_SPI1			// a single SPI command
};


// ----------------------------------------------------------------------
// Local data
// ----------------------------------------------------------------------
static uint8_t		sck_period;	// SCK period in microseconds (1..250)
static uint8_t		poll1;		// first poll byte for write
static uint8_t		poll2;		// second poll byte for write
static uint16_t		address;	// read/write address
static uint16_t		timeout;	// write timeout in usec
static uint8_t		cmd0;		// current read/write command byte
static uint8_t		cmd[4];		// SPI command buffer
static uint8_t		res[4];		// SPI result buffer

// ----------------------------------------------------------------------
// Delay exactly <sck_period> times 0.5 microseconds (6 cycles).
// ----------------------------------------------------------------------
__attribute__((always_inline))
static inline void delay(void) {
	asm volatile(
		"	mov	__tmp_reg__,%0	\n"
		"0:	rjmp	1f		\n"
		"1:	dec	__tmp_reg__	\n"
		"	brne	0b		\n"
		: : "r" (sck_period) );
}


// ----------------------------------------------------------------------
// Issue one SPI command.
// ----------------------------------------------------------------------
static void spi(uint8_t* cmd, uint8_t* res, uint8_t n) {
	uint8_t	c;
	#if !USE_USI_SPI
	uint8_t	r;
	uint8_t	mask;
	#endif
	
	while (n != 0) {
		n--;
		c = *cmd++;
		
		#if USE_USI_SPI
		USIDR = c;
		USISR = 1 << USIOIF;

		while ((USISR & (1<<USIOIF)) == 0) {
			USICR = (1<<USIWM0) | (1<<USICS1) | (1<<USICLK) | (1<<USITC);
			delay();
		}

		*res++ = USIDR;
		#else
		r = 0;
		//cli();
		for (mask = 0x80; mask; mask >>= 1) {
			if (c & mask) {
				PORT |= MOSI_MASK;
			}
			else {
				PORT &= ~MOSI_MASK;
			}
			delay();
			PORT |= SCK_MASK;
			//PORT |= LED_MASK;
			delay();
			r <<= 1;
			if (PIN & MISO_MASK) {
				r++;
			}
			PORT &= ~SCK_MASK;
			//PORT &= ~LED_MASK;
		}
		*res++ = r;
		//sei();
		#endif
	}
}

// ----------------------------------------------------------------------
// Create and issue a read or write SPI command.
// ----------------------------------------------------------------------
static void spi_rw (void) {
	uint16_t a;
	
	a = address++;
	if (cmd0 & 0x80) {	// eeprom
		a <<= 1;
	}
	cmd[0] = cmd0;
	if (a & 1) {
		cmd[0] |= 0x08;
	}
	cmd[1] = a >> 9;
	cmd[2] = a >> 1;
	spi(cmd, res, 4);
}

// ----------------------------------------------------------------------
// Handle a non-standard SETUP packet.
// ----------------------------------------------------------------------
extern uint8_t usb_setup (uint8_t data[8]) {
	uint8_t req = data[1];
	uint8_t mask;
	
	// Generic requests
	if (req == USBTINY_ECHO) {
		return 8;
	}
	if (req == USBTINY_READ) {
		data[0] = PIN;
		return 1;
	}
	if (req == USBTINY_WRITE || req == USBTINY_CLR || req == USBTINY_SET || req == USBTINY_DDRWRITE) {
		return 0;
	}
	
	// Programming requests
	if (req == USBTINY_POWERUP) {
		sck_period = data[2];
		mask = 0;
		mask = LED_MASK;
		if (data[4]) {
			mask |= RESET_MASK;
		}
		DDR  |= LED_MASK | RESET_MASK | SCK_MASK | MOSI_MASK;
		PORT |= mask;
		return 0;
	}
	
	if (req == USBTINY_POWERDOWN) {
		DDR  &= ~(LED_MASK | RESET_MASK | SCK_MASK | MOSI_MASK);
		PORT &= ~(LED_MASK | RESET_MASK | SCK_MASK | MOSI_MASK);
		return 0;
	}
	if (!PORT) {
		return 0;
	}
	if (req == USBTINY_SPI) {
		spi(data + 2, data + 0, 4);
		return 4;
	}
	if (req == USBTINY_SPI1) {
		spi(data + 2, data + 0, 1);
		return 1;
	}
	if (req == USBTINY_POLL_BYTES) {
		poll1 = data[2];
		poll2 = data[3];
		return 0;
	}
	address = * (uint16_t*) & data[4];
	if (req == USBTINY_FLASH_READ) {
		cmd0 = 0x20;
		return 0xff;	// usb_in() will be called to get the data
	}
	if (req == USBTINY_EEPROM_READ) {
		cmd0 = 0xa0;
		return 0xff;	// usb_in() will be called to get the data
	}
	timeout = * (uint16_t*) & data[2];
	if (req == USBTINY_FLASH_WRITE) {
		cmd0 = 0x40;
		return 0;	// data will be received by usb_out()
	}
	if (req == USBTINY_EEPROM_WRITE) {
		cmd0 = 0xc0;
		return 0;	// data will be received by usb_out()
	}
	return 0;
}

// ----------------------------------------------------------------------
// Handle an IN packet.
// ----------------------------------------------------------------------
extern	uint8_t	usb_in(uint8_t* data, uint8_t len) {
	uint8_t	i;
	
	for (i = 0; i < len; i++) {
		spi_rw();
		data[i] = res[3];
	}
	return len;
}

// ----------------------------------------------------------------------
// Handle an OUT packet.
// ----------------------------------------------------------------------
extern void usb_out(uint8_t* data, uint8_t len) {
	uint8_t i;
	uint16_t usec;
	uint8_t r;
	
	for	(i = 0; i < len; i++) {
		cmd[3] = data[i];
		spi_rw();
		cmd[0] ^= 0x60;	// turn write into read
		for	(usec = 0; usec < timeout; usec += 32 * sck_period) {	// when timeout > 0, poll until byte is written
			spi(cmd, res, 4);
			r = res[3];
			if (r == cmd[3] && r != poll1 && r != poll2) {
				break;
			}
		}
	}
}

// ----------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------
extern int main ( void ) {
	usb_init();
	while (1) {
		usb_poll();
	}
}
