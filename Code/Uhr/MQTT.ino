// Variables
// MQTT
const char *mqtt_server = "mqtt.iot.informatik.uni-oldenburg.de";
const int mqtt_port = 2883;
const char *mqtt_user = "sutk";
const char *mqtt_pw = "SoftSkills";
String id = String("cloc-") + String(ID); // individueller Name dieser Connection zu unserem mqtt-Server Account
                                          // falls sich mehrere Geräte gleichzeitig mit unserem Account verbinden,
                                          // braucht jedes seinen eigenen Namen
String topic;                             // Topic String zum Konvertieren in ein Char Array
char msg[8];                              // Char Array zum Versenden von MQTT Nachrichten

//Externs
extern unsigned int sinceBoot;
extern unsigned int inRow;
extern unsigned int timerWork;
extern unsigned int timerBreak;  

//init
void initMQTT() {
  client.setServer(mqtt_server, mqtt_port); // MQTT
  client.setCallback(callback);             // Teilt client mit, dass jedes Mal wenn Daten vom MQTT-Server kommen,
                                            //  die Rountine "callback" aufzurufen ist
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
