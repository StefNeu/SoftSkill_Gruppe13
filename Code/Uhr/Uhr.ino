
#include <DHTesp.h>

DHTesp dht;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.setup(4, DHTesp::DHT11);

}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(dht.getTemperature());
}
