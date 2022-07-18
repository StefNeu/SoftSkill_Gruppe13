// Variables

// Clock
unsigned int minute;              // Aktuelle Zeit
unsigned int minuteLED;           // Index der korrespondierenden LED zur Minute
unsigned int hour;
unsigned int hourLED;             // Index der korrespondierenden LED zur Minute
unsigned long last = 0;           // Timestamp der letzen Zeitabfrage

//Externs
extern int NUM;

// Clock:     timeClient holt die aktuelle Zeit vom NTP-Server
void getTime() {
  if (millis() - last > 30000 || last == 0) { // Letzte Aktualisierung > 3000 order gerade gestartet
    timeClient.update();
    last = millis();

    hour = timeClient.getHours(); // Stunde
    Serial.print("Time: ");
    Serial.print(hour);
    if (hour >= 12) { // 12h Format
      hour -= 12;
    }
    if (minute == 60) {
      minute = 0;
    }
    minute = timeClient.getMinutes(); // Minute
    Serial.print(":");
    Serial.print(minute);
    Serial.println();

    hourLED = map(hour, 0, 11, 0, NUM - 1); // Map Stunde zu LED
    minuteLED = map(minute, 0, 59, 0, NUM - 1); // Map Minute zu LED
  }
  updateClock();
}
