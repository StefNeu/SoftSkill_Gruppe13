/*
Diese Biblithek rechnet die gegeben Zeit in die ansteuerung der Led an 
*/
#ifdef Zeiger_h
#define Zeiger_h

#include <Arduino.h>

class Zeiger{

  public:
    Zeiger(int anzahlLEDs, long zeitzohnenUmrechnung, int pin);
    update(long time);


  private:
  
};

#endif
