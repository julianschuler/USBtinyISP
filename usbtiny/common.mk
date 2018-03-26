# ======================================================================
# Common Makefile for USBtiny applications
#
# Macros to be defined before including this file:
#
# USBTINY		- the location of this directory
# TARGET_ARCH	- gcc -mmcu= option with AVR device type
# OBJECTS		- the objects in addition to the USBtiny objects
# FLASH_CMD		- command to upload main.hex to flash
# FUSES_CMD		- command to program the fuse bytes
# STACK			- maximum stack size (optional)
# FLASH			- flash size (optional)
# SRAM			- SRAM size (optional)
#
# This file is a part of the ATtiny24A based USBtinyISP project
# by Julian Schuler (https://github.com/julianschuler/usbtinyisp).
#
# It is based on the USBtiny project by Dick Streefland
# (https://dicks.home.xs4all.nl/avr/usbtiny/index.html)
#
# Copyright (C) 2006-2010 Dick Streefland
# Copyright (C) 2018 Julian Schuler
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# ======================================================================


check	= $(shell $(CC) $1 -c -xc /dev/null -o/dev/null 2>/dev/null && echo $1)

CC		= avr-gcc
OPTIM	= -Os -ffunction-sections $(call check,-fno-split-wide-types)
CFLAGS	= -g -Wall -I. -I$(USBTINY) $(OPTIM)
LDFLAGS	= -g -Wl,--relax,--gc-sections
MODULES = crc.o int.o usb.o $(OBJECTS)

main.hex:

all:		main.hex

clean:
			rm -f main.elf *.o tags *.sch~ gschem.log

clobber:	clean
			rm -f main.hex

main.elf:	$(MODULES)
			$(LINK.o) -o $@ $(MODULES)

main.hex:	main.elf $(USBTINY)/check.py
			#@python $(USBTINY)/check.py main.elf $(STACK) $(FLASH) $(SRAM)
			avr-objcopy -j .text -j .data -O ihex main.elf main.hex

check:		main.elf $(USBTINY)/check.py
			@python $(USBTINY)/check.py main.elf $(STACK) $(FLASH) $(SRAM)

disasm:		main.elf
			avr-objdump -S main.elf

flash:		main.hex
			$(FLASH_CMD)

fuses:
			$(FUSES_CMD)

crc.o:		$(USBTINY)/crc.S $(USBTINY)/def.h usbtiny.h
			$(COMPILE.c) $(USBTINY)/crc.S

int.o:		$(USBTINY)/int.S $(USBTINY)/def.h usbtiny.h
			$(COMPILE.c) $(USBTINY)/int.S

usb.o:		$(USBTINY)/usb.c $(USBTINY)/def.h $(USBTINY)/usb.h usbtiny.h
			$(COMPILE.c) $(USBTINY)/usb.c

main.o:		$(USBTINY)/usb.h
