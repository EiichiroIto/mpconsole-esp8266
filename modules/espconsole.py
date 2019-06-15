import ili9341
from machine import Pin, SPI
spi = SPI(1, baudrate=40000000, polarity=0, phase=0)
display = ili9341.ILI9341(spi, cs=Pin(15), dc=Pin(2), rst=Pin(0), width=320, height=240)
import ili9341_fb
fb = ili9341_fb.Ili9341Framebuffer(display)
import ps2
ps2.init(clock=5, data=4)
from fbconsole import FBConsole
console = FBConsole(fb)
import os
os.dupterm(console)

def cls():
    console.cls()

def on():
    console.on()

def off():
    console.off()
