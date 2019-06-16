mpconsole: MicroPython port to ESP8266 with PS/2 input and LCD output
==========================================================

This supports a console with PS/2 input and LCD output.

Controller:
- supports ESP-WROOM-02 module.

PS/2 input:
- only Clock and Data pin used.
- does not supports Computer to Keyboard communication.

LCD output:
- supports ili9341 Lcd module (320x240).

# Install and Build
see README.orig.md carefully. Then do followings.

    cd micropython/ports/
    git clone https://github.com/EiichiroIto/mpconsole-esp8266.git
    make
    make erase
    make deploy

# Input Special Character
- ESC ,(comma) for '['
- ESC .(period) for ']'

# Schematic

![schematic](https://raw.githubusercontent.com/EiichiroIto/mpconsole-esp8266/master/images/mpconsole.png)

# Reference

## MicroPython port to ESP8266
https://github.com/micropython/micropython

## MicroPython library for RGB pixel displays.
modules/ili9341.py and modules/rgb.py from https://github.com/adafruit/micropython-adafruit-rgb-display

## frame buffer console class for MicroPython
modules/fbconsole.py from https://github.com/boochow/FBConsole

## PS/2 Keyboard Library for Arduino
ps2.c from https://github.com/PaulStoffregen/PS2Keyboard

