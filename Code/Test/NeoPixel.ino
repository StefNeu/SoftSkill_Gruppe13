#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN1 5 // D1
#define NUM1 24

#define PIN2 4 // D2
#define NUM2 8

#define ID 1

const char *ssid = "HTC U11";
const char *password = "a28139348";

const char *mqtt_server = "mqtt.iot.informatik.uni-oldenburg.de";
const int mqtt_port = 2883;
const char *mqtt_user = "sutk";
const char *mqtt_pw = "SoftSkills";

const int timerWork = 1;
const int timerBreak = 1;

// ----------------------------------------------------------------------------

//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM1, PIN1, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM2, PIN2, NEO_GRB + NEO_KHZ800);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
WiFiClient espClient;
PubSubClient client(espClient);

boolean light = true;
boolean pomodoro = false;
long last = 0;
long startTime = 0;
int minute;
int minuteLED;
int hour;
int hourLED;
int brightness = 255;
int num = 0;
int sinceBoot = 0;
int inRow = 0;
String id = String("cloc_") + String(ID);
String topic;
char msg[8];


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

void callback(char *inTopic, byte *payload, unsigned int length) {
  String newTopic = inTopic;
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  int intPayload = newPayload.toInt();
  Serial.print("Message arrived: ");
  Serial.print(newTopic);
  Serial.print(" ");
  Serial.println(newPayload);
  Serial.println();
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
  } else if (newTopic == id + "/brightness") {
    brightness = map(intPayload, 0, 100, 0, 255);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to ");
    Serial.println(mqtt_server);
    if (client.connect(id.c_str(), mqtt_user, mqtt_pw)) {
      Serial.print("connected as ");
      Serial.println(id);
      topic = id + "/light";
      client.subscribe(topic.c_str());
      topic = id + "/timer";
      client.subscribe(topic.c_str());
      topic = id + "/light/status";
      client.publish(topic.c_str(), "ON");
      topic = id + "/timer/status";
      Serial.println(client.publish(topic.c_str(), "OFF"));
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

void getTime() {
   timeClient.update();

  if (millis() - last > 1000) {
    last = millis();

    hour = timeClient.getHours();
    Serial.print("Time: ");
    Serial.print(hour);
    if (hour > 12) {
      hour -= 12;
    }
  
    minute = timeClient.getMinutes();
    Serial.print(":");
    Serial.print(minute);

    Serial.println();

    hourLED = map(hour, 0, 12, 0, strip1.numPixels() - 1);
    Serial.print("LEDS: ");
    Serial.print(hourLED + 1);

    minuteLED = map(minute, 0, 60, 0, strip1.numPixels() - 1);
    Serial.print(":");
    Serial.print(minuteLED + 1);
    Serial.println();
    Serial.println();
     
    /* int second = timeClient.getSeconds();
    Serial.print(":");
    Serial.print(second);
    Serial.println();
    */
  }
}

// Light
void startLight() {
  light = true;
  topic = id + "/light/status";
  client.publish(topic.c_str(), "ON");
}
void stopLight() {
  strip1.clear();
  strip1.show();
  light = false;
  topic = id + "/light/status";
  client.publish(topic.c_str(), "OFF");
}

void updateClock() {
  for (int i = 0; i < 500; i++) {
    strip1.clear();
    if (hourLED != minuteLED) {
      strip1.setPixelColor(hourLED, strip1.Color(0, 0, 0, brightness));
    }
    strip1.setPixelColor(minuteLED, strip1.Color(0, 0, 0, map(i, 0, 500, 0, brightness)));
    strip1.show();
    delay(1);
  }
  for (int i = 500; i > 0; i--) {
    strip1.clear();
    if (hourLED != minuteLED) {
      strip1.setPixelColor(hourLED, strip1.Color(0, 0, 0, brightness));
    }
    strip1.setPixelColor(minuteLED, strip1.Color(0, 0, 0, map(i, 0, 500, 0, brightness)));
    strip1.show();
    delay(1);
  }
}

// Timer
void startTimer() {
  pomodoro = true;
  startTime = millis();
  topic = id + "/timer/status";
  client.publish(topic.c_str(), "ON");
  if (sinceBoot > 0) {
    topic = id + "/timer/sinceBoot";
    sprintf(msg, "%d", sinceBoot);
    client.publish(topic.c_str(), msg);
  }
  if (inRow > 0) {
    topic = id + "/timer/inRow";
    sprintf(msg, "%d", inRow);
    client.publish(topic.c_str() , msg);
  }
}
void stopTimer() {
  pomodoro = false;
  inRow = 0;
  topic = id + "/timer/status";
  client.publish(topic.c_str() , "OFF");
  topic = id + "/timer/inRow";
  sprintf(msg, "%d", inRow);
  client.publish(topic.c_str() , msg);
}

void updateTimer() {
  if (millis() - startTime > 1000 * 60 * timerWork) {
    if (millis() - startTime > 1000 * 60 * (timerWork + timerBreak)) {
      sinceBoot++;
      inRow++;
      return startTimer();
    }
    // Pause
    int past = map(1000 * 60 * timerBreak - (millis() - (startTime + 1000 * 60 * timerWork)), 0, 1000 * 60 * timerWork, strip1.numPixels() - 1, -1);
    strip1.clear();
    if (past >= 0) {
      for (int i = 0; i <= past; i++) {
         strip1.setPixelColor(strip1.numPixels() - 1 - i, strip1.Color(0, brightness, 0, 0)); // Green
      }
    }
    strip1.show();
  } else {
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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.begin(115200);
  
  strip1.begin();
  strip1.setBrightness(50);
  strip1.clear(); // Initialize all pixels to 'off'
  strip1.show();
  
  strip2.begin();
  strip2.setBrightness(50);
  strip2.clear(); // Initialize all pixels to 'off'
  strip2.show();
  
  setup_wifi();
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (light) {
    if (pomodoro) {
      updateTimer();
    } else {
      getTime();
      updateClock();
    }
  }
}
