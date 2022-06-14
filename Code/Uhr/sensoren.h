/*
*/
#ifndef sensoren_h
#define sensoren_h

#include <Arduino.h>
#include <DHTesp.h>
#include <Adafruit_BMP085.h>
#include <hp_BH1750.h>
#include <SparkFunCCS811.h>
#define CCS811_ADDR 0x5B

class sensoren{

  public:
  sensoren();
  double temperature();
  double volume();
  double pressure();
  double co2();
  double light();
  double humidity();
  double tvoc();
  bool button();
  
  

  private:
  DHTesp dht;
  Adafruit_BMP085 bmp;
  hp_BH1750 hp;
  

};
#endif