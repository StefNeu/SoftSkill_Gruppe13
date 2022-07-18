// Variables
// LightB
boolean light = true;             // LEDs an
unsigned int brightness = 125;    // aktuelle Helligkeitseinstellung für die LEDs
unsigned int num = 0;             // globale Variable zum bewegen eines rotes Lichtes während der Einrichtung
                                  // hält den aktuellen Index

//Externs
extern String topic;
extern String id;
extern char msg[8];
extern int NUM;
extern int NUM2;

// FastLED
CRGB strip[24];
CRGB strip2[8];

CRGB lastLED[8];       // Zweites Array um den letzten Zustand zu speichern


//init
void initLED() {
  FastLED.addLeds<WS2812B, STRIP, GRB>(strip, NUM).setCorrection(TypicalLEDStrip); // Initalisierung der LEDs und Farbkorrektur
  FastLED.addLeds<WS2812B, STRIP2, GRB>(strip2, NUM2).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 300); // Maximalleistung begrenzen
  FastLED.clear(); // Initialize all pixels to 'off'
  FastLED.show();
}

// Light
void startLight() {
  memcpy(strip, lastLED, sizeof(lastLED)); // Kopiere den letzten Status in das aktuelle Array
  FastLED.show(); 
  light = true;
  Serial.println("LIGHT: ON");
  topic = id + "/light/status";
  client.publish(topic.c_str(), "ON");
}
void stopLight() {
  FastLED.clear(true);
  light = false;
  Serial.println("LIGHT: OFF");
  topic = id + "/light/status";
  client.publish(topic.c_str(), "OFF");
}


void updateClock() {
  if (light) {
    // aalle LEDs aus
    for (int i = 0; i < NUM; i++) {
      strip[i] = CHSV(0, 0, 0);
    }
    // zeigt Stunden- und Minuten-LED an: Stunden-LED hat Vorrang bei Gleichstand
    if (hourLED != minuteLED) { // Zweite LED (Minute)
      strip[minuteLED] = CRGB(brightness, 0, 0);
    }
    strip[hourLED] = CRGB(brightness, brightness, brightness); // Erste LED (Stunde)
    FastLED.show();
  }
}
