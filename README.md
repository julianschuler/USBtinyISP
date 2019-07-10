# USBtinyISP
## Introduction
The USBtinyISP is an USB AVR ISP programmer, capable of writing and reading flash, EEPROM and fuse bits of Atmel AVRs. It is based on the [USBtiny project](https://dicks.home.xs4all.nl/avr/usbtiny/index.html) by Dick Streefland and can be used as the [USBtinyISP](https://www.adafruit.com/product/46) by Adafruit, therefore it works great with avrdude und AVRStudio!

## Features
The USBtinyISP is due to the usage of the ATtiny24A as main controller extremely small and low cost, it has the form factor and size of a general USB Stick and offers a standard 6 pin ISP connector.
This programmer is able to power the target board with up to 300mA @ 3.3V or 500mA @ 5V, selectable via DIP switches (see [How to use](#how-to-use) for further information). If power distribution is deactivated, the target AVR will be programmed with with a voltage level equal to the voltage applied by the target board to the VCC pin of the ISP. This means, e. g. a battery powered project running at a voltage anywhere around 3.7V will be programmed with the same voltage level, this way you don't have to worry about destroying your AVR by programming it with a voltage higher than the supply voltage!

## How to use
With the first DIP switch the target power distribution can be activated (ON ≙ activated, OFF ≙ deactivated), the second one selects the supply voltage (ON ≙ 5V, OFF ≙ 3.3V).The USBtinyISP runs under Windows, Linux and MacOS X, for Windows a [modified LibUSB driver](http://www.adafruit.com/downloads/usbtiny_signed_8.zip) has to be installed. It can be used with avrdude by including the option `-c usbtiny`, within the Arduino IDE `Tools > Programmer: "USBtinyISP"` has to be selected. 
Other than that the USBtinyISP just has to plugged into an USB port and connected with an standard 6 pin ISP cable to the target board!

## License
This project is licensed under GNU GPLv3, see [`LICENSE.txt`](LICENSE.txt) for further information.

## Todo
- Add build instructions
- Add pictures
