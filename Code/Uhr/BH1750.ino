// Variables
// Lux
float lastLux = 0;                  // vorheriger vom BH1750 erhaltene LUX-Wert oder 0 nach Neustart
float lastLuxSend = 0;              // LUX-Wert der letzten Uebermittlung an MQTT-Server oder 0 nach Neustart
unsigned long lastBrightness = 0;   // Zeitpunkt der letzten LUX-Übermittlung an MQTT-Server oder 0 nach Neustart
boolean automatic = true;         // Steuert ob die Helligkeit über der Helligkeitssensor oder über MQTT gesteuert wird

//Externs
extern unsigned int brightness;
extern String topic;
extern String id;
extern char msg[8];

// BH1750
void bh1750() {
  if (BH1750.hasValue()) {
    // wenn neue Werte anliegen, neuen LUX-Wert lesen
    float lux = BH1750.getLux();
    BH1750.start();
    if (abs(lux - lastLux) > 3 || lastLux == 0) { // Value changed more than 3 since last measurement or just booted
      lastLux = lux;    // lux als neuer Referenzwert gemerkt
      if (automatic) { // falls Lichtsteuerung automatisch: brightness fuer die Anzeigen neu berechnet
        brightness = map(constrain(lux, 8, 255), 0, 150, 8, 255); // Map Lux zu Brightness
      }
    }
    if (abs(lux - lastLuxSend) > 3 || lastLuxSend == 0) {
          // an den MQTT-Server werden nur Änderungen des LUX-Wertes von mehr als 3 übermittelt
          // oder der neue Wert nach einem Neustart des Geraetes
      if (millis() - lastBrightness > 5000 || lastBrightness == 0) {
            // um den MQTT-Server nicht mit Nachrichten zu überschwemmen, 
            // müssen mindestens 5 Sekunden zwischen zwei Übermittlungen liegen
        lastLuxSend = lux;
        topic = id + "/sens/lux";
        dtostrf(lux, 3, 1, msg); // Konvertiere float zu char*
        client.publish(topic.c_str(), msg);
        lastBrightness = millis();
      }
    }
  }
}
