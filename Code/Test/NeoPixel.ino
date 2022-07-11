//Dear Programmer:
//When we wrote this code, only god and we knew how it worked.
//Now, only god knows!

//Therefore, if you are trying to optimize it and you fail (most surely),
//please increase this counter as a warning for the next person:

//total_hours_wasted_time = 42

// Libraries
#include <ESP8266WiFi.h>        // Bibliothek, um ESP8266 mit einem Wifi-Netzwerk zu verbinden
#include <NTPClient.h>          // Ein NTPClient für die Verbindung zu einem Zeit Server: Zeitsynchronisation
#include <WiFiUdp.h>            // wird für UDP-Kommunikation über Wifi benötigt
#include <PubSubClient.h>       // Bibliothek zum Senden und Empfangen von MQTT Nachrichten
#include <FastLED.h>            // zum Steuern der LED Ringe
#include <bsec.h>               // gehoert zu BME680, erhält und verarbeitet BME680 Signale; erzeugt benötigte Sensordatenausgaben
                                // BME680 : Sensor für Luftqualität, Temparatur, Luftdruck, Luftfeuchtigkeit,...
#include <hp_BH1750.h>          // High perfomance non-blocking library zur Benutzung des BH1750 Lichtsensors
// #include <ESP8266TimerInterrupt.h>

#ifdef __AVR__
  #include <avr/power.h>        // Power-reduction-management (Regulation Stromverbrauch)
#endif
// #define TIMER_INTERVAL_MS       10000

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

// Zeitzone
const int timezone = 2; // Differenz zu UTC

// ----------------------------------------------------------------------------

// PINOUT: Pins an denen die Peripherie angeschlossen ist
#define STRIP 14 // D5    // Strip1: äußerer Ringe mit 24 LEDs
#define NUM 24

#define STRIP2 12 // D6   // Strip2: innerer Ring mit 8 LEDs
#define NUM2 8            

int button = 13; // D7    // Druckknopf zur Timersteuerung

// FastLED
CRGB strip[NUM];
CRGB strip2[NUM2];

CRGB lastLED[NUM2];       // Zweites Array um den letzten Zustand zu speichern

// BH1750
hp_BH1750 BH1750;         // Erstellt ein Objekt zur Ansteuerung des BH1750 Sensors

// BME680
Bsec BME680;              // Erstellt ein Objekt zur Ansteuerung des BME680 Sensors

// Network
WiFiUDP ntpUDP;           // Objekt zur Kommunikation über WiFiUDP erzeugen
NTPClient timeClient(ntpUDP, "pool.ntp.org");   // Teilt dem NTPClient die Domain des NTP-Servers mit
                                                //  und dass er zur Kommunikation ntpUDP nutzen soll
WiFiClient espClient;     // Objekt zur Kommunikation über WiFi erzeugen
PubSubClient client(espClient);     // Teilt dem PubSubClient mit, dass er zur Kommunikation 
                                    //  espClient nutzen soll
// ESP8266Timer ITimer;

// Variables
// Timer
unsigned int timerWork = 25;      // Zeit in Minuten: Arbeitsphase
unsigned int timerBreak = 5;      // Zeit in Minuten: Pause danach
boolean pomodoro = false;         // Timer inaktiv
unsigned long startTime = 0;
unsigned int sinceBoot = 0;       // Abgeschlossene Timer seit Boot
unsigned int inRow = 0;           // Abgeschlossene Timer in Reihe
// Light
boolean light = true;             // LEDs an
unsigned int brightness = 125;    // aktuelle Helligkeitseinstellung für die LEDs
unsigned int num = 0;             // globale Variable zum bewegen eines rotes Lichtes während der Einrichtung
                                  // hält den aktuellen Index
boolean automatic = true;         // Steuert ob die Helligkeit über der Helligkeitssensor oder über MQTT gesteuert wird
// Clock
unsigned int minute;              // Aktuelle Zeit
unsigned int minuteLED;           // Index der korrespondierenden LED zur Minute
unsigned int hour;
unsigned int hourLED;             // Index der korrespondierenden LED zur Minute
unsigned long last = 0;           // Timestamp der letzen Zeitabfrage
// MQTT
String id = String("cloc-") + String(ID); // individueller Name dieser Connection zu unserem mqtt-Server Account
                                          // falls sich mehrere Geräte gleichzeitig mit unserem Account verbinden,
                                          // braucht jedes seinen eigenen Namen
String topic;                     // Topic String zum Konvertieren in ein Char Array
char msg[8];                      // Char Array zum Versenden von MQTT Nachrichten
//
// Button
unsigned int lastState = 0;       // Zustand des Druckknopfes am Gehäuse
//
// Lux
float lastLux = 0;                  // vorheriger vom BH1750 erhaltene LUX-Wert oder 0 nach Neustart
float lastLuxSend = 0;              // LUX-Wert der letzten Uebermittlung an MQTT-Server oder 0 nach Neustart
unsigned long lastBrightness = 0;   // Zeitpunkt der letzten LUX-Übermittlung an MQTT-Server oder 0 nach Neustart

boolean sens = false;

// void IRAM_ATTR TimerHandler() {
//  sens = true;
// }

//
// Setup
//
void setup() {
  Serial.begin(115200);
  delay(300);
  
  Wire.begin(); // Starte I2C

  // Interval in microsecs
  // if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, TimerHandler)) {
  //  Serial.print("Starting ITimer OK");
  // } else {
  //   Serial.println("Can't set ITimer correctly. Select another freq. or interval");
  // }
  
  pinMode(button, INPUT);          // schaltet den Pin, an den der Button angeschlossen ist, auf Eingang
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Schalte build-in LED aus

  FastLED.addLeds<WS2812B, STRIP, GRB>(strip, NUM).setCorrection(TypicalLEDStrip); // Initalisierung der LEDs und Farbkorrektur
  FastLED.addLeds<WS2812B, STRIP2, GRB>(strip2, NUM2).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 300); // Maximalleistung begrenzen
  FastLED.clear(); // Initialize all pixels to 'off'
  FastLED.show();

  // Initialisiere BME680
  if (BH1750.begin(BH1750_TO_GROUND)) { // check BME680
    BH1750.start();
  } else {
    Serial.println("BH1750 NOT FOUND");
  }
  BME680.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  // Teilt dem BME680-Objekt mit, dass er über Wire(I2C) mit der ID (BME680_I2C_ADDR_PRIMARY)
  //  kommunizieren soll/kann
  if (BME680.status != BSEC_OK || BME680.bme680Status != BME680_OK) {
    Serial.print("FAIL ");
    Serial.print(BME680.status);
    Serial.print(" : ");
    Serial.println(BME680.bme680Status);
  }
  bsec_virtual_sensor_t sensorList[10] =    // definiert Liste der gewünschten Sensordaten
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
      // meldet Liste der gewünschten Sensordaten und die gewünschte Probenrate an den BME680

  setup_wifi();
  timeClient.begin(); // NTP Time
  timeClient.setTimeOffset(60*60*timezone);           // legt aktuelle Zeitzone fest
  client.setServer(mqtt_server, mqtt_port); // MQTT
  client.setCallback(callback);             // Teilt client mit, dass jedes Mal wenn Daten vom MQTT-Server kommen,
                                            //  die Rountine "callback" aufzurufen ist
  getTime();          // Aktualisierung der Systemzeit
}

// verbindet das Gerät mit dem WLAN
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // während des Wartens auf die Wifi-Connection läuft ein Punkt über den Pixel-Ring
    if (num >= NUM2) {
      num = 0;
    }
    FastLED.clear();
    strip2[num] = CRGB(brightness, 0, 0);
    FastLED.show();
    num++;
  }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  FastLED.clear();
}

// verbindet das Gerät (über das WLAN) mit dem MQTT-Server der Uni
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

      // Veröffentliche default Werte beim Booten
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
      // Warten auf die MQTT Verbindung
      delay(5000);
    }
  }
}

// Callback: verarbeitet eine Nachricht vom MQTT-Server
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
      brightness = map(constrain(intPayload, 8, 255), 0, 100, 8, 255); // Bei 8 geht der LED Ring aus, daher muss dies die untere Grenze sein
      sprintf(msg, "%d", intPayload);     // TODO: Hier sollte constrain(intPayload, 8, 255) zurueckgegeben werden; KJELL: nein der Wert soll in Prozent gesteuert werden (nutzerfreundlicher)
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

// Clock:     timeClient holt die aktuelle Zeit vom NTP-Server
void getTime() {
  if (millis() - last > 30000 || last == 0) { // Letzte Aktualisierung > 3000 order gerade gestartet
    timeClient.update();
    last = millis();

    hour = timeClient.getHours(); // Stunde
    Serial.print("Time: ");
    Serial.print(hour);
    if (hour > 12) { // 12h Format
      hour -= 12;
    }
    minute = timeClient.getMinutes(); // Minute
    Serial.print(":");
    Serial.print(minute);
    Serial.println();

    hourLED = map(hour, 0, 12, 0, NUM - 1); // Map Stunde zu LED
    minuteLED = map(minute, 0, 60, 0, NUM - 1); // Map Minute zu LED
  }
  updateClock();
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
    strip[hourLED] = CHSV(0, 0, brightness); // Erste LED (Stunde)
    FastLED.show();
  }
}

// Timer
void checkButton() {
  int state = digitalRead(button);
  if (state != lastState) { // Zustand verändert
    lastState = state;
    if (state == HIGH) { // Button gedrückt
      if (pomodoro) { // Timer an
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
  // übermittelt Anzahl der bereits abgeschlossenen Timer-Zyklen an MQTT-Server
  if (sinceBoot > 0) {
    topic = id + "/timer/counter/sinceBoot";
    sprintf(msg, "%d", sinceBoot);
    client.publish(topic.c_str(), msg);
  }
  if (inRow >= 0) {
    topic = id + "/timer/counter/inRow";
    sprintf(msg, "%d", inRow);
    client.publish(topic.c_str() , msg);
  }
}

void stopTimer() {  // TODO: warum werden nicht die letzten aktuellen Daten an den mqtt-Server uebermittelt ?
  pomodoro = false;
  inRow = 0;
  Serial.println("TIMER: OFF");
  topic = id + "/timer/status";
  client.publish(topic.c_str() , "OFF");
}

void updateTimer() {
  if (millis() - startTime > 1000 * 60 * timerWork) { // Work time beendet
    if (millis() - startTime > 1000 * 60 * (timerWork + timerBreak)) { // Timer beendet: Arbeitszeit + Pausenzeit abgelaufen
      sinceBoot++;
      inRow++; // erhöhe counter
      return startTimer(); // Starte neuen Timer
    }
    if (light) {
      // Pause läuft:
      //  mapt die Restpausenzeit und zeigt in einem Kreisbogen am Ende des Rings in grün die Restpause an
      int remaining = map(1000 * 60 * timerBreak - (millis() - (startTime + 1000 * 60 * timerWork)), 0, 1000 * 60 * timerWork, NUM - 1, -1);
      for (int i = 0; i < NUM; i++) {
        strip[i] = CHSV(0, 0, 0);
      }
      if (remaining >= 0) {
        for (int i = 0; i <= remaining; i++) {
           strip[NUM - 1 - i] = CRGB(0, brightness, 0); // Grün
        }
      }
      FastLED.show();
    }
  } else {
    if (light) {
       // Arbeitsphase laeuft:
       //  mapt verbleibende Arbeitszeit in diesem Timer-Zyklus relativ zur Laenge des Arbeitszyklus
       //  auf eine LED-Nummer auf dem Strip
       int past = map(1000 * 60 * timerWork - (millis() - startTime), 0, 1000 * 60 * timerWork, -1, NUM - 1);
       for (int i = 0; i < NUM; i++) {
         strip[i] = CHSV(0, 0, 0);
       }
       if (past >= 0) {
         for (int i = 0; i <= past; i++) {
            strip[i] = CHSV(0, 0, brightness); // zeigt in einem Kreisbogen Restzeit in weiß an
         }
       }
       FastLED.show();
    }
  }
}

// Sensoren
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

// BME680
void bme680() {
  if (BME680.run()) { // fragt die Bsec-Library, ob am bme680 neue Messwerte vorliegen
    // new value arrived
    String output = String(BME680.iaqAccuracy);
    output += ", " + String(BME680.temperature);
    output += ", " + String(BME680.humidity);
    output += ", " + String(BME680.staticIaq);
    output += ", " + String(BME680.co2Equivalent);
    Serial.println(output);

    // Temperatur
    float temp = BME680.temperature;
    int h = map(constrain(temp, 15, 27), 15, 27, 120, 0); // Map temperature to HSV scale between 240, 0
    if ((80 < h && h < 90) || !light) { // if light off or h green don't show
          //  unterdrückt Normalwerte, zeigt nur oberes und unteres Drittel der Temperatur-Range auf strip2 an
      strip2[0] = CRGB::Black;
    } else {
      Serial.println(h);
      strip2[0] = CHSV(h, 255, brightness);
      FastLED.show();
      lastLED[0] = CHSV(h, 255, brightness); // Make copy
    }
    // Humidity
    float hum = BME680.humidity;
    h = map(constrain(hum, 40, 60), 40, 60, 0, 120); // Map humidity to HSV scale between 0, 240
    if ((80 < h && h < 90) || !light) { // if light off or h green don't show
      strip2[6] = CRGB::Black;
    } else {
      Serial.println(h);
      strip2[6] = CHSV(h, 255, brightness);
      FastLED.show();
      lastLED[6] = CHSV(h, 255, brightness);
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
  sens = false;
}

// Loop
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  checkButton();
  //if (sens) {
   // bh1750();
    bme680();
  //}
  if (pomodoro) { // Timer
    updateTimer();
  } else {
    getTime();
  }
}
