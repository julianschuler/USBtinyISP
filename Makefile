# ========================================================================
# Makefile for USBtinyISP
#
# This file is a part of the ATtiny24 based USBtinyISP by Julian Schuler
# (https://github.com/julianschuler/usbtinyisp).
#
# It is based on the USBtiny project by Dick Streefland
# (https://dicks.home.xs4all.nl/avr/usbtiny/index.html) and
# published under the same license (GNU GPLv3, see LICENSE.txt)
# ========================================================================


USBTINY		= ./usbtiny
TARGET_ARCH	= -mmcu=attiny84
OBJECTS		= main.o
FLASH_CMD	= avrdude -p t84 -U flash:w:main.hex
FUSES_CMD	= avrdude -p t84 -U hfuse:w:0xdf:m -U lfuse:w:0xef:m
STACK		= 32
FLASH		= 8192
SRAM		= 512

include $(USBTINY)/common.mk
