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
    
    
    
    
}

double sensoren::humidity(){
    return dht.getHumidity();
}

double sensoren::temperature(){

    return ((dht.getTemperature()+bmp.readTemperature())/2.0);
}

double sensoren::volume(){
    return analogRead(0);
    }

double sensoren::pressure(){return bmp.readPressure();}

double sensoren::co2(){
    CCS811 co_voc(CCS811_ADDR);
    co_voc.readAlgorithmResults();
    return co_voc.getCO2();}

double sensoren::tvoc(){
    CCS811 co_voc(CCS811_ADDR);
    co_voc.readAlgorithmResults();
    return co_voc.getTVOC();
}

double sensoren::light(){
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