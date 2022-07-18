// Variables
// Temp
float lastTemp = 0;
unsigned long lastTempTime = 0;
// Hum
float lastHum = 0;
unsigned long lastHumTime = 0;
// CO2
float lastCo2 = 0;
unsigned long lastCo2Time = 0;

//Externs
extern unsigned int brightness;
extern String topic;
extern String id;
extern char msg[8];
extern boolean light;
extern CRGB strip[];
extern CRGB strip2[];
extern CRGB lastLED[];

// BME680
void bme680() {
  if (BME680.run()) { // fragt die Bsec-Library, ob am bme680 neue Messwerte vorliegen
    // Temperatur
    float temp = BME680.temperature;
    int h = map(constrain(temp, 15, 27), 15, 27, 120, 0); // Map temperature to HSV scale between 240, 0
    if ((80 < h && h < 90) || !light) { // if light off or h green don't show
          //  unterdrückt Normalwerte, zeigt nur oberes und unteres Drittel der Temperatur-Range auf strip2 an
      strip2[0] = CRGB::Black;
    } else {
      strip2[0] = CHSV(h, 255, brightness);
      lastLED[0] = CHSV(h, 255, brightness); // Make copy
    }
    FastLED.show();
    if (abs(temp - lastTemp) > 0.1 || lastTempTime == 0) {
      if (millis() - lastTempTime > 5000 || lastTempTime == 0) {
            // um den MQTT-Server nicht mit Nachrichten zu überschwemmen, 
            // müssen mindestens 5 Sekunden zwischen zwei Übermittlungen liegen
        lastTemp = temp;
        topic = id + "/sens/temp";
        dtostrf(temp, 3, 1, msg); // Konvertiere float zu char*
        client.publish(topic.c_str(), msg);
        lastTempTime = millis();
      }
    }
    // Humidity
    float hum = BME680.humidity;
    h = map(constrain(hum, 40, 60), 40, 60, 0, 120); // Map humidity to HSV scale between 0, 240
    if ((80 < h && h < 90) || !light) { // if light off or h green don't show
      strip2[6] = CRGB::Black;
    } else {
      strip2[6] = CHSV(h, 255, brightness);
      lastLED[6] = CHSV(h, 255, brightness);
    }
        FastLED.show();
    if (abs(hum - lastHum) > 0.5 || lastHumTime == 0) {
      if (millis() - lastHumTime > 5000 || lastHumTime == 0) {
            // um den MQTT-Server nicht mit Nachrichten zu überschwemmen, 
            // müssen mindestens 5 Sekunden zwischen zwei Übermittlungen liegen
        lastHum = hum;
        topic = id + "/sens/hum";
        dtostrf(hum, 3, 1, msg); // Konvertiere float zu char*
        client.publish(topic.c_str(), msg);
        lastHumTime = millis();
      }
    }
    // IAQ
    if (BME680.iaqAccuracy > 0) { // IAQ Accuracy above 0 to get safe values
      float co2 = BME680.co2Equivalent;
      if (co2 < 600 || !light) {
        strip2[2] = CRGB::Black;
      } else {
        h = map(co2, 600, 750, 80, 0);
        strip2[2] = CHSV(h, 255, brightness);
        lastLED[2] = CHSV(h, 255, brightness);
      }
      FastLED.show();
      if (abs(co2 - lastCo2) > 3 || lastCo2Time == 0) {
        if (millis() - lastCo2Time > 5000 || lastCo2Time == 0) {
              // um den MQTT-Server nicht mit Nachrichten zu überschwemmen, 
              // müssen mindestens 5 Sekunden zwischen zwei Übermittlungen liegen
          lastCo2 = co2;
          topic = id + "/sens/co2";
          dtostrf(co2, 3, 1, msg); // Konvertiere float zu char*
          client.publish(topic.c_str(), msg);
          lastCo2Time = millis();
        }
      }
    } else {
      if (!light) {
        strip2[2] = CRGB::Black;
      } else {
        strip2[2] = CRGB(brightness,brightness/2,0);
        lastLED[2] = CRGB(brightness,brightness/2,0);
      }
      FastLED.show();
    }
  } else if (BME680.status != BSEC_OK || BME680.bme680Status != BME680_OK) { // prueft ob Fehlersituation vorliegt
    Serial.print("FAIL ");
    Serial.print(BME680.status);
    Serial.print(" : ");
    Serial.println(BME680.bme680Status);
  }
}
