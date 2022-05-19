from machine import Pin
import utime

def blink(delay, pin):
    """Test Program zum Blinken eines Pins

    :param delay: die Zeit in ms zwischen den Umschalten
    :param pin: der Pin auf den Geblinkt werden soll
    """
    LED = Pin(pin, Pin.OUT)                                 #der Pin pin wird als Output geschaltet
    while True:                                           #dauerschleife
        LED.on()                                            #der Pin pin wird eingeschaltet
        utime.sleep(delay)                                   #Warten
        LED.off()                                           #der Pin pin wird ausgeschaltet
        utime.sleep(delay)                                   #Warten