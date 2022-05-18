# Version = 0.0.1
# Autor = Steffen Neumann
# Programmfunktion = Main Program für den Pico

# Importe
# Nicht ändern, so lassen der Code funktioniert so auf dem Computer und auf dem Raspberry
#importe

try:

    from machine import Pin                                                             #zur nutzung der Pins auf dem Picos
    import time                                                                         #zur nutzung der Zeit
except(RuntimeError, ModuleNotFoundError):                                              #abfangen der Fehler wenn jemand die Anleitung nicht richtig gelsen hat
    print("Hast du auch die Anleitung gelesen? anleitungPython.txt")                    #ausgabe das er ein Fehler gemacht hat

# main
if __name__=="__main__":
    print("Hello world")
