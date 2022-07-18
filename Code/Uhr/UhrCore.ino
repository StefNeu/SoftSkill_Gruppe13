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
#include <avr/power.h>         // Power-reduction-management (Regulation Stromverbrauch)
#endif
// #define TIMER_INTERVAL_MS       10000

// ----------------------------------------------------------------------------

// ID
#define ID 1

// ----------------------------------------------------------------------------

// Zeitzone
const int timezone = 2; // Differenz zu UTC

// PINOUT: Pins an denen die Peripherie angeschlossen ist
#define STRIP 12 // D6    // Strip1: äußerer Ringe mit 24 LEDs
int NUM = 24;

#define STRIP2 14 // D5   // Strip2: innerer Ring mit 8 LEDs
int NUM2 = 8;    

int button = 13; // D7    // Druckknopf zur Timersteuerung

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

//
// Button
unsigned int lastState = 0;       // Zustand des Druckknopfes am Gehäuse
//

//
// Pomodoro
boolean pomodoro = false;         // Timer inaktiv
//

//
// Glitch Prevent
int checkLux = 1000; //Lux nur alle 1000 Loops prüfen, sonst Strobopop
//

//
// Setup
//
void setup() {
  Serial.begin(115200);
  delay(300);
  
  Wire.begin(); // Starte I2C
  
  pinMode(button, INPUT);          // schaltet den Pin, an den der Button angeschlossen ist, auf Eingang
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Schalte build-in LED aus

  initLED();

  // Initialisiere BME680
  if (BH1750.begin(BH1750_TO_GROUND)) { // check BME680
    Serial.println("BH1750 FOUND");
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
  initMQTT();
  
  getTime();          // Aktualisierung der Systemzeit
}

//
// Loop
//
void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  checkButton();
  
  if(checkLux < 0) {
    checkLux = 1000;
    bh1750();
  } else {
    checkLux--;
  }
  
  bme680();
  
  if (pomodoro) { // Timer
    updateTimer();
  } else {
    getTime();
  }
}
