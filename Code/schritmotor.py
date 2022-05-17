#ULN2003
#Code idee von https://tutorials-raspberrypi.de/raspberry-pi-schrittmotor-steuerung-l293d-uln2003a/
#geändert von Steffen Neumann am 17.Mai.2022
#für ULM2003A mit dem Schritmotor L293D
#der Motor hat 512 schritte.
#importe nicht ändern!
import time
try:
    import RPi.GPIO
except (RuntimeError, ModuleNotFoundError):
    import fake_rpigpio.utils
    fake_rpigpio.utils.install()
GPIO = RPi.GPIO
#setzen der GPIO pin belegung
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)                             #ausschaltend er Wanungsausgabe


class schritmotor():                                #Schritmotor classe
    def __init__(self, pinA, pinB, pinC, pinD):     #construktor
        self.coil_A_1_pin = pinA  # pink            #
        self.coil_A_2_pin = pinB  # orange          #
        self.coil_B_1_pin = pinC  # blau            #
        self.coil_B_2_pin = pinD  # gelb            #
        # anpassen, falls andere Sequenz
        self.StepCount = 8                          #anzahl der Steps
        self.Seq = list(range(0, self.StepCount))   #anlegen des Arrays
        self.Seq[0] = [0, 1, 0, 0]                  #
        self.Seq[1] = [0, 1, 0, 1]                  #
        self.Seq[2] = [0, 0, 0, 1]                  #
        self.Seq[3] = [1, 0, 0, 1]                  #
        self.Seq[4] = [1, 0, 0, 0]                  #
        self.Seq[5] = [1, 0, 1, 0]                  #
        self.Seq[6] = [0, 0, 1, 0]                  #
        self.Seq[7] = [0, 1, 1, 0]                  #


        # enable_pin   = 7 # Nur bei bestimmten Motoren benoetigt (+Zeile 24 und 30)

        # anpassen, falls andere Sequenz


        # GPIO.setup(enable_pin, GPIO.OUT)
        #setzen der GPIO Pins
        GPIO.setup(self.coil_A_1_pin, GPIO.OUT)     #
        GPIO.setup(self.coil_A_2_pin, GPIO.OUT)     #
        GPIO.setup(self.coil_B_1_pin, GPIO.OUT)     #
        GPIO.setup(self.coil_B_2_pin, GPIO.OUT)     #


        # GPIO.output(enable_pin, 1)

    #funktion um die schrite zu setzen wird von den anderen funktionen genutzt
    def setStep(self , w1, w2, w3, w4):
        GPIO.output(self.coil_A_1_pin, w1)
        GPIO.output(self.coil_A_2_pin, w2)
        GPIO.output(self.coil_B_1_pin, w3)
        GPIO.output(self.coil_B_2_pin, w4)

    #funktion um den schritmotor nach rechtz zu drehen das Delay wird hinter jeden Step hinzugefügt
    def forward(self , delay, steps):
        for i in range(steps):
            for j in range(self.StepCount):
                self.setStep(self.Seq[j][0], self.Seq[j][1], self.Seq[j][2], self.Seq[j][3])
                time.sleep(delay)

    #funktion um den schritmotor nach links zu drehen, das Delay wird hinter jeden Step hinzugefügt
    def backwards(self, delay, steps):
        for i in range(self.steps):
            for j in reversed(range(self.StepCount)):
                self.setStep(self.Seq[j][0], self.Seq[j][1], self.Seq[j][2], self.Seq[j][3])
                time.sleep(delay)


#if __name__ == '__main__':
 #   while True:
 #       delay = raw_input("Zeitverzoegerung (ms)?")
 #       steps = raw_input("Wie viele Schritte vorwaerts? ")
 #       forward(int(delay) / 1000.0, int(steps))
 #       steps = raw_input("Wie viele Schritte rueckwaerts? ")
 #       backwards(int(delay) / 1000.0, int(steps))