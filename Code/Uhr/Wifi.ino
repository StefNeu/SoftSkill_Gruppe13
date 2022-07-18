// Variables
// WiFi
const char *ssid = "ONEPLUS";
const char *password = "31121212";

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
