// Variables
unsigned int timerWork = 25;      // Zeit in Minuten: Arbeitsphase
unsigned int timerBreak = 5;      // Zeit in Minuten: Pause danach
unsigned long startTime = 0;
unsigned int sinceBoot = 0;       // Abgeschlossene Timer seit Boot
unsigned int inRow = 0;           // Abgeschlossene Timer in Reihe

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
            strip[i] = CRGB(brightness, brightness, brightness); // zeigt in einem Kreisbogen Restzeit in weiß an
         }
       }
       FastLED.show();
    }
  }
}
