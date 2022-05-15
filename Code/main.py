# Version = 0.0.1
# Autor = Steffen Neumann
# Programmfunktion =

# Importe
# Nicht ändern, so lassen der Code funktioniert so auf dem Computer und auf dem Raspberry
try:
    import RPi.GPIO
except (RuntimeError, ModuleNotFoundError):
    import fake_rpigpio.utils
    fake_rpigpio.utils.install()
    #GPIO.install()

# GPIO zeigt auf RPi.GPIO damit man weniger schreiben muss
GPIO = RPi.GPIO
# Einstellung wie die Pins aufgerufen werden
GPIO.setmode(GPIO.BOARD)
# Main
if __name__=="__main__":
    print("Hello world")






