from machine import Pin
import time
class schritmotor():                                #Schritmotor classe
    def __init__(self, pinA, pinB, pinC, pinD):     #construktor
        self.coil_A_1_pin = pinA  # pink            #
        self.coil_A_2_pin = pinB  # orange          #
        self.coil_B_1_pin = pinC  # blau            #
        self.coil_B_2_pin = pinD  # gelb            #
        # anpassen, falls andere Sequenz

        #
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

        self.pinA = Pin(self.coil_A_1_pin,Pin.OUT)
        self.pinB = Pin(self.coil_A_2_pin, Pin.OUT)
        self.pinC = Pin(self.coil_B_1_pin, Pin.OUT)
        self.pinD = Pin(self.coil_B_2_pin, Pin.OUT)


        # GPIO.output(enable_pin, 1)

    #funktion um die schrite zu setzen wird von den anderen funktionen genutzt
    def setStep(self , w1, w2, w3, w4):
        self.pinA.value(w1)
        self.pinB.value(w2)
        self.pinC.value(w3)
        self.pinD.value(w4)

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


