#Version = 0.0.1
#Autor = Steffen Neumann
#Programfunktion =

#importe
#Nicht ändern so lassen der code funktionirt so auf den Computer und auf den Rassperry
try:
    import RPi.GPIO
except (RuntimeError, ModuleNotFoundError):
    import fake_rpigpio.utils
    fake_rpigpio.utils.install()
    #GPIO.install()

# GPIO zeigt auf RPi.GPIO damit man weniger schreiben muss
GPIO = RPi.GPIO
#Einstellung wie die Pins aufgerufen werden
GPIO.setmode(GPIO.BOARD)
#Main
if __name__=="__main__":
    print("Hello world")






