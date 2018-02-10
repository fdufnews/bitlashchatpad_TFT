### The project

It is a small mobile platform based on an ATmega 1284p running Bitlash.
An Xbox Chatpad is used as an input device.
A TFT screen is used as a display.
Schematic of the board is in the [Doc directory](../Doc/1284_portable_V2.pdf)

I would like to use this platform as an engineering assistant tool when doing electronics.
* To generate logic patterns
* To test SPI and I²C components and or peripherals
* Measure pulses
* Test servo
* ....

### Components

#### The screen
It is a TFT screen from OPENSMART with an ILI9326 driver chip.
Library for the screen is on [my GitHub](https://github.com/fdufnews/OPENSMART_TFT)

#### The keyboard
The chatpad is reprogrammed with the firmware from the OpenKeyboard Project. I have no link to this project as they are all dead links.

#### The software
The application is based on [Bitlash by Bill Roy](https://github.com/billroy/bitlash/issues).
 Bitlash is a scripting language with an easy to learn syntax.
I have made some modifications to Bill Roy work in order to make the best use of the ATmega 1284p. The library is on [my GitHub](https://github.com/fdufnews/bitlash)

#### Todo list
##### General
- [x] Adding schematic
- [x] Loading modified Arduino library for the screen on Github
- [x] Loading modified Arduino Bitlash library on Github
- [ ] Loading a copy of the OpenKeyboard Project on Github

##### Application
- [x] Adding chatpad input to Bitlash
- [x] Adding TFT support to Bitlash (see note))
- [ ] EEPROM cleaner. Utility to compact EEPROM content when there are too many holes in the file system
- [ ] Support for SPI in Bitlash
- [ ] Support for I²C in Bitlash
- [ ] jointly with the development of SPI and I²C, adding support for an array in order to transfert pack of data to the peripheral

Note: Concerning the screen, a scroll is very time consuming as the hardware support is only possible
 in portrait mode. So it was decided to make a rollover instead of a scroll. When bottom of screen
 is reached the cursor returns to top of screen. To highlight the cursor position the line following
 the cursor is deleted too.
