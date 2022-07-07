// Libraries
#include <ESP8266WiFi.h>        // Bibliothek, um ESP8266 mit einem Wifi-Netzwerk zu verbinden
#include <NTPClient.h>          // Ein NTPClient für die Verbindung zu einem Zeit Server: Zeitsynchronisation
#include <WiFiUdp.h>            // wird fuer UDP-Kommunikation ueber Wifi benoetigt
#include <PubSubClient.h>       // Client Library for sending and receiving MQTT messages
#include <Adafruit_NeoPixel.h>  // For controlling single-wire-based LED pixels and strip (strip1: grosser Ring)
#include <FastLED.h>            // for controlling dozens of different types of LEDs + optimized math, effect, noise functions (strip2: kleiner Ring)
#include <bsec.h>               // gehoert zu BME680, erhaelt und verarbeitet BME680 Signale; erzeugt benoetigte Sensordatenausgaben
                                // BME680 : Sensor fuer Luftqualitaet, Temparatur, Luftdruck, Luftfeuchtigkeit,...
#include <hp_BH1750.h>          // High perfomance non-blocking library zur Benutzung des BH1750 Lichtsensors

#ifdef __AVR__
  #include <avr/power.h>        // Power-reduction-management (Regulation Stromverbrauch)
#endif

// ----------------------------------------------------------------------------

// ID
#define ID 1

// WiFi
const char *ssid = "HTC U11";
const char *password = "a28139348";

// MQTT
const char *mqtt_server = "mqtt.iot.informatik.uni-oldenburg.de";
const int mqtt_port = 2883;
const char *mqtt_user = "sutk";
const char *mqtt_pw = "SoftSkills";

// ----------------------------------------------------------------------------

// PINOUT: Pins an denen die Peripherie angeschlossen ist
#define STRIP1 14 // D5   // Strip1: aeusserer Ringe mit 24 LEDs
#define NUM1 24

#define STRIP2 12 // D6   // Strip2: innerer Ring mit 8 LEDs
#define NUM2 8

int button = 13; // D7    // Druckknopf zur Timersteuerung

// NeoPixel
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM1, STRIP1, NEO_GRBW + NEO_KHZ800);
// FastLED
CRGB strip2[NUM2];
CRGB lastLED[NUM2];       // Second Array to copy last LED state

// BH1750
hp_BH1750 BH1750;         // Erstellt ein Objekt zur Ansteuerung des BH1750 Sensors

// BME680
Bsec BME680;              // Erstellt ein Objekt zur Ansteuerung des BME680 Sensors

// Network
WiFiUDP ntpUDP;           // Objekt zur Kommunikation über WiFiUDP erzeugen
NTPClient timeClient(ntpUDP, "pool.ntp.org");   // Teilt dem NTPClient die domain des NTP-Servers mit
												                        //	und dass er zur Kommunikation ntpUDP nutzen soll
WiFiClient espClient;     // Objekt zur Kommunikation über WiFi erzeugen
PubSubClient client(espClient);     // Teilt dem PubSubClient mit, dass er zur Kommunikation 
									                  //  espClient nutzen soll

// Variables
// Timer
unsigned int timerWork = 25;      // Zeit in Minuten: Arbeitsphase
unsigned int timerBreak = 5;      // Zeit in Minuten: Pause danach
boolean pomodoro = false;         // Timer inactive
unsigned long startTime = 0;
unsigned int sinceBoot = 0;       // Completed timer since boot
unsigned int inRow = 0;           // Completed timer in a Row
// Light
boolean light = true;             // LEDs on
unsigned int brightness = 125;    // aktuelle Helligkeitseinstellung fuer die LEDs
unsigned int num = 0;             // global variable to move red light in circle through setup
                                  // holds the current position of the light on strip1
boolean automatic = true;         // Brightness automatic instead of MQTT
// Clock
unsigned int minute;              // Current minute
unsigned int minuteLED;           // Number of LED corresponding to the current minute
unsigned int hour;                // Current hour
unsigned int hourLED;             // Number of LED corresponding to the current hour
unsigned long last = 0;           // Timestamp of last calculation of minute and hour
// MQTT
String id = String("cloc-") + String(ID); // individueller Name dieser Connection zu unserem mqtt-Server Account
											                    // falls sich mehrere Geräte gleichzeitig mit unserem Account verbinden,
											                    // braucht jedes seinen eigenen Namen
String topic;       // Topic String to convert to char*
char msg[8];        // Char Array to hold mqtt messages
//
// Button
unsigned int lastState = 0;       // Zustand des Druckknopfes am Gehaeuse
//
// Lux
float lastLux = 0;                  // vorheriger vom BH1750 erhaltene LUX-Wert oder 0 nach Neustart
float lastLuxSend = 0;              // LUX-Wert der letzten Uebermittlung an mqtt-Server oder 0 nach Neustart
unsigned long lastBrightness = 0;   // Zeitpunkt der letzten LUX-Uebermittlung an mqtt-Server oder 0 nach Neustart

//
// Setup
//
void setup() {
  Serial.begin(115200);
  Wire.begin(); // Enable i2c, i.e. join i2c bus for BME680 communication
  
  pinMode(button, INPUT);          // schaltet den Pin, an den der Button angeschlossen ist, auf Eingang
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn build-in LED off
  
  // aeusserer Ring fuer Uhrzeit
  strip1.begin();
  strip1.clear();    // Initialize all pixels to 'off'
  strip1.show();

  // innerer Ringe fuer Timer
  FastLED.addLeds<WS2812B, STRIP2, GRB>(strip2, NUM2);
  FastLED.clear(); // Initialize all pixels to 'off'
  FastLED.show();

  // Initialize BME680
  if (BH1750.begin(BH1750_TO_GROUND)) { // check for BME680
    BH1750.start();
  } else {
    Serial.println("BH1750 NOT FOUND");
  }
  BME680.begin(BME680_I2C_ADDR_PRIMARY, Wire);
      // BME680_I2C_ADDR_PRIMARY: Device identifier parameter for the read/write interface functions
      // Wire: Physical communication interface
      // Teilt dem BME680-Object mit, dass er ueber Wire(I2C) mit der ID (BME680_I2C_ADDR_PRIMARY)
      //  kommunizieren soll/kann
  if (BME680.status != BSEC_OK || BME680.bme680Status != BME680_OK) {
    Serial.print("FAIL ");
    Serial.print(BME680.status);
    Serial.print(" : ");
    Serial.println(BME680.bme680Status);
  }
  bsec_virtual_sensor_t sensorList[10] =    // definiert Liste der gewuenschten Sensordaten
  {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };
  BME680.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
      // meldet Liste der gewuenschten Sensordaten und die gewuenschte Probenrate an den BME680

  setup_wifi();
  timeClient.begin(); // NTP Time
  timeClient.setTimeOffset(7200);           // legt aktuelle Zeitzone fest; TODO: ueber Konstante im Kopfbereich (aenderbar)
  client.setServer(mqtt_server, mqtt_port); // MQTT
  client.setCallback(callback);             // Teilt client mit, dass jedes Mal wenn Daten vom mqtt-Server kommen,
                                            //  die Rountine "callback" aufzurufen ist
  getTime();          // Aktualisierung der Systemzeit
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (num >= strip1.numPixels()) {
      num = 0;
    }
    strip1.clear();
    strip1.setPixelColor(num, strip1.Color(brightness, 0, 0, 0));
    strip1.show();
    num++;
  }
  randomSeed(micros());
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
      Serial.print("Connecting to ");
    Serial.println(mqtt_server);
  while (!client.connected()) {
    if (client.connect(id.c_str(), mqtt_user, mqtt_pw)) {
      Serial.print("connected as ");
      Serial.println(id);
      // Subscribe MQTT topics
      topic = id + "/light";
      client.subscribe(topic.c_str());
      topic = id + "/timer";
      client.subscribe(topic.c_str());
      topic = id + "/light/brightness";
      client.subscribe(topic.c_str());
      topic = id + "/timer/duration/work";
      client.subscribe(topic.c_str());
      topic = id + "/timer/duration/break";
      client.subscribe(topic.c_str());

      // Publish default values on reconnect
      topic = id + "/light/status";
      client.publish(topic.c_str(), "ON");
      topic = id + "/timer/status";
      client.publish(topic.c_str(), "OFF");
      sprintf(msg, "%d", sinceBoot);
      topic = id + "/timer/counter/sinceBoot";
      client.publish(topic.c_str(), msg);
      topic = id + "/timer/counter/inRow";
      sprintf(msg, "%d", inRow);
      client.publish(topic.c_str(), msg);
      topic = id + "/timer/duration/work/status";
      sprintf(msg, "%d", timerWork);
      client.publish(topic.c_str(), msg);
      topic = id + "/timer/duration/break/status";
      sprintf(msg, "%d", timerBreak);
      client.publish(topic.c_str(), msg);
      topic = id + "/light/brightness/status";
      client.publish(topic.c_str(), "A");
    } else {
      Serial.print(".");
      if (num >= strip1.numPixels()) {
        num = 0;
      }
      strip1.clear();
      strip1.setPixelColor(num, strip1.Color(brightness, 0, 0, 0));
      strip1.show();
      num++;
      delay(500);
    }
  }
}

// Callback
void callback(char *inTopic, byte *payload, unsigned int length) {
  String newTopic = inTopic;
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  int intPayload = newPayload.toInt();
  Serial.print("Message arrived: ");
  Serial.print(newTopic);
  Serial.print(" ");
  Serial.println(newPayload);
  if (newTopic == id + "/light") {
    if (newPayload == "ON") {
      startLight();
    } else if (newPayload == "OFF") {
      stopLight();
    }
  } else if (newTopic == id + "/timer") {
    if (newPayload == "ON") {
      startTimer();
    } else if (newPayload == "OFF") {
      stopTimer();
    }
  } else if (newTopic == id + "/light/brightness") {
    if (newPayload == "A") {
      automatic = true;
      topic = id + "/light/brightness/status";
      client.publish(topic.c_str(), "A");
    } else {
      automatic = false;
      brightness = map(constrain(intPayload, 8, 255), 0, 100, 8, 255);
      sprintf(msg, "%d", intPayload);
      topic = id + "/light/brightness/status";
      client.publish(topic.c_str(), msg);
    }
  } else if (newTopic == id +"/timer/duration/work") {
    if (intPayload > 0) {
      timerWork = intPayload;
      topic = id + "/timer/duration/work/status";
      sprintf(msg, "%d", timerWork);
      client.publish(topic.c_str(), msg);
    }
  } else if (newTopic == id +"/timer/duration/break") {
    if (intPayload > 0) {
      timerBreak = intPayload;
      topic = id + "/timer/duration/break/status";
      sprintf(msg, "%d", timerBreak);
      client.publish(topic.c_str(), msg);
    }
  }
}

// Light
void startLight() {
  memcpy(strip2, lastLED, sizeof(lastLED)); // Copy last state in current state
  FastLED.show(); 
  light = true;
  Serial.println("LIGHT: ON");
  topic = id + "/light/status";
  client.publish(topic.c_str(), "ON");
}
void stopLight() {
  strip1.clear();
  strip1.show();
  FastLED.clear(true);
  light = false;
  Serial.println("LIGHT: OFF");
  topic = id + "/light/status";
  client.publish(topic.c_str(), "OFF");
}

// Clock:     timeClient holt die aktuelle Zeit uebers Internet vom NTP-Server
void getTime() {
  if (millis() - last > 30000 || last == 0) { // Last update > 3000 or just booted
    timeClient.update();
    last = millis();

    hour = timeClient.getHours(); // Get current hour
    Serial.print("Time: ");
    Serial.print(hour);
    if (hour > 12) { // get 12h format
      hour -= 12;
    }
    minute = timeClient.getMinutes(); // Get current minute
    Serial.print(":");
    Serial.print(minute);
    Serial.println();

    hourLED = map(hour, 0, 12, 0, strip1.numPixels() - 1); // Map hour to LED number
    minuteLED = map(minute, 0, 60, 0, strip1.numPixels() - 1); // Map minute to LED number
  }
  updateClock();
}

void updateClock() {
  if (light) {
    strip1.clear(); // All LEDs off
    if (hourLED != minuteLED) { // Second LED (minute)
      strip1.setPixelColor(minuteLED, strip1.Color(brightness, 0, 0, 0));
    }
    strip1.setPixelColor(hourLED, strip1.Color(0, 0, 0, brightness)); // First LED
    strip1.show();
  }
}

// Timer
void checkButton() {
  int state = digitalRead(button);
  if (state != lastState) { // State changed
    lastState = state;
    if (state == HIGH) { // Button pressed
      if (pomodoro) { // Timer on
        stopTimer();
      } else {
        startTimer();
      }
    }
  }
}

void startTimer() {
  pomodoro = true;
  startTime = millis();
  Serial.println("TIMER: ON");
  topic = id + "/timer/status";
  client.publish(topic.c_str(), "ON");
  if (sinceBoot > 0) {
    topic = id + "/timer/counter/sinceBoot";
    sprintf(msg, "%d", sinceBoot);
    client.publish(topic.c_str(), msg);
  }
  if (inRow == 0) {
    topic = id + "/timer/counter/inRow";
    sprintf(msg, "%d", inRow);
    client.publish(topic.c_str() , msg);
  } else if (inRow > 0) {
    topic = id + "/timer/counter/inRow";
    sprintf(msg, "%d", inRow);
    client.publish(topic.c_str() , msg);
  }
}

void stopTimer() {
  pomodoro = false;
  inRow = 0;
  Serial.println("TIMER: OFF");
  topic = id + "/timer/status";
  client.publish(topic.c_str() , "OFF");
}

void updateTimer() {
  if (millis() - startTime > 1000 * 60 * timerWork) { // Work time finished
    if (millis() - startTime > 1000 * 60 * (timerWork + timerBreak)) { // Timer finished
      sinceBoot++;
      inRow++; // increment counter
      return startTimer(); // Start new timer
    }
    if (light) {
      // Pause
      int past = map(1000 * 60 * timerBreak - (millis() - (startTime + 1000 * 60 * timerWork)), 0, 1000 * 60 * timerWork, strip1.numPixels() - 1, -1);
      strip1.clear();
      if (past >= 0) {
        for (int i = 0; i <= past; i++) {
           strip1.setPixelColor(strip1.numPixels() - 1 - i, strip1.Color(0, brightness, 0, 0)); // Green
        }
      }
      strip1.show();
    }
  } else {
    if (light) {
       // Arbeiten
       int past = map(1000 * 60 * timerWork - (millis() - startTime), 0, 1000 * 60 * timerWork, -1, strip1.numPixels() - 1);
       strip1.clear();
       if (past >= 0) {
         for (int i = 0; i <= past; i++) {
            strip1.setPixelColor(i, strip1.Color(0, 0, 0, brightness)); // White
         }
       }
       strip1.show();
    }
  }
}

// Sensoren
// BH1750
void bh1750() {
  if (BH1750.hasValue() == true) {
    float lux = BH1750.getLux();
    BH1750.start();
    if (abs(lux - lastLux) > 3 || lastLux == 0) { // Value changed more than 3 or just booted
      lastLux = lux;    // lux als neuer Referenzwert gemerkt
      if (automatic) { // falls Lichtsteuerung automatisch: brightness fuer die Anzeigen neu berechnet
        brightness = map(constrain(lux, 8, 255), 0, 150, 8, 255); // Map Lux to brightness scale
      }
    }
    if (abs(lux - lastLuxSend) > 3 || lastLuxSend == 0) {
          // an den mqtt-Server werden nur Aenderungen des LUX-Wertes von mehr als 3 uebermittelt
          // oder der neue Wert nach einem Neustart des Geraetes
      if (millis() - lastBrightness > 5000 || lastBrightness == 0) { // Send lux through MQTT every 5 seconds if value changed more than 3
            // um den mqtt-Server nicht mit Nachrichten zu ueberschwemmen, 
            // muessen mindestens 5 Sekunden zwischen zwei Uebermittlungen liegen
        lastLuxSend = lux;
        topic = id + "/sens/lux";
        dtostrf(lux, 3, 1, msg); // Convert float to char*
        client.publish(topic.c_str(), msg);
        lastBrightness = millis();
      }
    }
  }
}

// BME680
void bme680() {
  if (BME680.run()) { // fragt die Bsec-library, ob am bme680 neue Messwerte vorliegen
    // new value arrived
    String output = String(BME680.iaq);
    output += ", " + String(BME680.iaqAccuracy);
    output += ", " + String(BME680.temperature);
    output += ", " + String(BME680.humidity);
    output += ", " + String(BME680.staticIaq);
    output += ", " + String(BME680.co2Equivalent);
    output += ", " + String(BME680.breathVocEquivalent);
    Serial.println(output);

    // Temperature
    float temp = BME680.temperature;
    int h = map(constrain(temp, 15, 25), 15, 25, 240, 0); // Map temperature to HSV scale between 240, 0
    if ((80 < h && h < 160) || !light) { // if light off or h green don't show
          //	unterdrueckt Normalwerte, zeigt nur oberes und unteres Drittel der Temperatur-Range auf strip2 an
      strip2[0] = CRGB::Black;
    } else {
      Serial.println(h);
      strip2[0] = CHSV(h, 255, brightness);
      FastLED.show();
      lastLED[0] = CHSV(h, 255, brightness); // Make copy
    }
    // Humidity
    float hum = BME680.humidity;
    h = map(constrain(hum, 40, 60), 40, 60, 0, 240); // Map humidity to HSV scale between 0, 240
    if ((80 < h && h < 160) || !light) { // if light off or h green don't show
      strip2[1] = CRGB::Black;
    } else {
      Serial.println(h);
      strip2[1] = CHSV(h, 255, brightness);
      FastLED.show();
      lastLED[1] = CHSV(h, 255, brightness);
    }
    // IAQ
    if (BME680.iaqAccuracy > 0) { // IAQ Accuracy above 0 to get safe values
      
    }
  } else if (BME680.status != BSEC_OK || BME680.bme680Status != BME680_OK) { // prueft ob Fehlersituation vorliegt
    Serial.print("FAIL ");
    Serial.print(BME680.status);
    Serial.print(" : ");
    Serial.println(BME680.bme680Status);
  }
}

// Loop
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  checkButton();
  bh1750();
  bme680();
  if (pomodoro) { // Timer
    updateTimer();
  } else {
    getTime();
  }
}
