# Version = 0.0.1
# Autor = Steffen Neumann
# Programmfunktion = Main Program für den Pico
from machine import Pin
import time

led = Pin(25, Pin.OUT, Pin.value(1))

while True:
    led(1)
    time.sleep(2)
    led(0)
    time.sleep(1)
