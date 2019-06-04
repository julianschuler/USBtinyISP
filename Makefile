# ========================================================================
# Makefile for USBtinyISP
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


PROGRAMMER	= arduino
PORT		= /dev/ttyACM0

#PROGRAMMER	= usbtiny
#PORT		= usb


USBTINY		= ./usbtiny
TARGET_ARCH	= -mmcu=attiny24
OBJECTS		= main.o
FLASH_CMD	= avrdude -p t24 -b 19200 -c $(PROGRAMMER) -P $(PORT) -U flash:w:main.hex
FUSES_CMD	= avrdude -p t24 -b 19200 -c $(PROGRAMMER) -P $(PORT) -U hfuse:w:0xd7:m -U lfuse:w:0xef:m
STACK		= 32
FLASH		= 2048
SRAM		= 128

include $(USBTINY)/common.mk
