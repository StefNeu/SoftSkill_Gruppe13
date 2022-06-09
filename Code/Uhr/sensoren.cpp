#include "sensoren.h"
#include "Arduino.h"
#include <DHTesp.h>
#include <Adafruit_BMP085.h>
#include <hp_BH1750.h>
#include <SparkFunCCS811.h>
#define CCS811_ADDR 0x5B



sensoren::sensoren(){
    dht.setup(4,DHTesp::DHT11);
    hp.begin(BH1750_TO_GROUND);
    hp.calibrateTiming();
    hp.start();
    CCS811 co_voc_(CCS811_ADDR);
    co_voc = co_voc_;
    
    
}
String sensoren::test(){
    String out;
    if (!bmp.begin())
    {
        out = "bmp180 Fehler";
    }
    return out;
    
}
double sensoren::humidity(){
    return dht.getHumidity();
}

double sensoren::tmp(){

    return ((dht.getTemperature()+bmp.readTemperature())/2.0);
}

double sensoren::lautsterke(){return 0.0;}

double sensoren::druck(){return bmp.readPressure();}

double sensoren::co(){
    co_voc.readAlgorithmResults();
    return co_voc.getCO2();}
double sensoren::tvoc(){
    co_voc.readAlgorithmResults();
    return co_voc.getTVOC();
}

double sensoren::licht(){
    double out = hp.getLux();
    hp.start();

    return out;}

bool sensoren::button(){return false;

if (digitalRead(3) == HIGH)
{
    return true;
}else{
    return false;
}

}