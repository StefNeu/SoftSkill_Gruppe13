/*
*/
#ifndef sensoren_h
#define sensoren_h

#include <Arduino.h>

class sensoren{

  public:
  sensoren();
  double tmp();
  double lautsterke();
  double druck();
  double co();
  double licht();
  bool button();

  private:
};
#endif