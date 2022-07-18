#include <FastLED.h>

#define FASTLED_ALLOW_INTERRUPTS 0

#define STRIP 14 // D5    // Strip1: äußerer Ringe mit 24 LEDs
#define NUM 24

#define STRIP2 12 // D6   // Strip2: innerer Ring mit 8 LEDs
#define NUM2 8  

CRGB strip[NUM];
CRGB strip2[NUM2];
unsigned long startTime = 0;
int timerWork = 1;
int timerBreak = 1;

void setup() {
  FastLED.addLeds<WS2812B, STRIP, GRB>(strip, NUM).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B, STRIP2, GRB>(strip2, NUM2).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 300);
  FastLED.clear(true);

  strip2[0] = CHSV(21, 255, 255);
  strip2[2] = CHSV(110, 255, 255);
  strip2[4] = CHSV(40, 255, 255);
  strip2[6] = CHSV(120, 255, 255);
}

void loop() {
  updateTimer();
}
void startTimer() {
  startTime = millis();
}
void updateTimer() {
  if (millis() - startTime > 1000 * 60 * timerWork) { // Work time beendet
    if (millis() - startTime > 1000 * 60 * (timerWork + timerBreak)) { // Timer beendet: Arbeitszeit + Pausenzeit abgelaufen
      return startTimer(); // Starte neuen Timer
    }
      // Pause läuft:
      //  mapt die Restpausenzeit und zeigt in einem Kreisbogen am Ende des Rings in grün die Restpause an
      int remaining = map(1000 * 60 * timerBreak - (millis() - (startTime + 1000 * 60 * timerWork)), 0, 1000 * 60 * timerWork, NUM - 1, -1);
      for (int i = 0; i < NUM; i++) {
        strip[i] = CHSV(0, 0, 0);
      }
      if (remaining >= 0) {
        for (int i = 0; i <= remaining; i++) {
           strip[NUM - 1 - i] = CRGB(0, 255, 0); // Grün
        }
      }
      FastLED.show();
  } else {
       // Arbeitsphase laeuft:
       //  mapt verbleibende Arbeitszeit in diesem Timer-Zyklus relativ zur Laenge des Arbeitszyklus
       //  auf eine LED-Nummer auf dem Strip
       int past = map(1000 * 60 * timerWork - (millis() - startTime), 0, 1000 * 60 * timerWork, -1, NUM - 1);
       for (int i = 0; i < NUM; i++) {
         strip[i] = CHSV(0, 0, 0);
       }
       if (past >= 0) {
         for (int i = 0; i <= past; i++) {
            strip[i] = CHSV(0, 0, 255); // zeigt in einem Kreisbogen Restzeit in weiß an
         }
       }
       FastLED.show();
    }
}

