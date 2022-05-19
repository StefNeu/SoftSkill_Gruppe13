from machine import Pin
import time




class schritmotorPico:  # Schritmotor classe
    """classe für den Schritmotoren mit z.B. den Treiber ULN2003a

    """
    version = 0.1
    def __init__(self, pinAi, pinBi, pinCi, pinDi):  # construktor
        """classe für den Schritmotoren mit z.B. den Treiber ULN2003a

        :param pinAi: pink
        :param pinBi: orange
        :param pinCi: blau
        :param pinDi: gelb
        """
        self._coil_A_1_pin = pinAi  # pink            #
        self._coil_A_2_pin = pinBi  # orange          #
        self._coil_B_1_pin = pinCi  # blau            #
        self._coil_B_2_pin = pinDi  # gelb            #
        # anpassen, falls andere Sequenz
        self._StepCount = 8  # anzahl der Steps
        self._Seq = list(range(0, self.StepCount))  # anlegen des Arrays
        self._Seq[0] = [0, 1, 0, 0]  #
        self._Seq[1] = [0, 1, 0, 1]  #
        self._Seq[2] = [0, 0, 0, 1]  #
        self._Seq[3] = [1, 0, 0, 1]  #
        self._Seq[4] = [1, 0, 0, 0]  #
        self._Seq[5] = [1, 0, 1, 0]  #
        self._Seq[6] = [0, 0, 1, 0]  #
        self._Seq[7] = [0, 1, 1, 0]  #
        #

        # enable_pin   = 7 # Nur bei bestimmten Motoren benoetigt (+Zeile 24 und 30)

        # anpassen, falls andere Sequenz

        # GPIO.setup(enable_pin, GPIO.OUT)
        # setzen der GPIO Pins

        self._pinA = Pin(self._coil_A_1_pin, Pin.OUT)
        self._pinB = Pin(self._coil_A_2_pin, Pin.OUT)
        self._pinC = Pin(self._coil_B_1_pin, Pin.OUT)
        self._pinD = Pin(self._coil_B_2_pin, Pin.OUT)

        # GPIO.output(enable_pin, 1)

    # funktion um die schrite zu setzen wird von den anderen funktionen genutzt
    def setStep(self, w1, w2, w3, w4):
        """ funktion zum direkten ansprechen der Pins
        !Diese funktion bitte nur in als ausnahme nutzen!

        :param w1: pin 1 pink
        :param w2: pin 2 orange
        :param w3: pin 3 blau
        :param w4: pin 4 gelb

        """
        self.pinA.value(w1)
        self.pinB.value(w2)
        self.pinC.value(w3)
        self.pinD.value(w4)

    # funktion um den schritmotor nach rechtz zu drehen das Delay wird hinter jeden Step hinzugefügt
    def forward(self, delay, steps):
        """Den Schritmotor nach rechts drehen lassen

        :param delay: die Zeit zwischen den einzelnen Schritten
        :param steps: anzahl der Schrite die gemacht werden sollen

        """
        for i in range(steps):
            for j in range(self.StepCount):
                self.setStep(self.Seq[j][0], self.Seq[j][1], self.Seq[j][2], self.Seq[j][3])
                time.sleep(delay)

    # funktion um den schritmotor nach links zu drehen, das Delay wird hinter jeden Step hinzugefügt
    def backwards(self, delay, steps):
        """Den Schritmotor nach links laufen lassen

        :param delay: die Zeit zwischen den einzelnen Schritten
        :param steps: anzahl der Schrite die gemacht werden sollen

        """
        for i in range(self.steps):
            for j in reversed(range(self.StepCount)):
                self.setStep(self.Seq[j][0], self.Seq[j][1], self.Seq[j][2], self.Seq[j][3])
                time.sleep(delay)
