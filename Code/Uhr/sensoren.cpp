#include "sensoren.h"
#include "Arduino.h"

sensoren::sensoren(){}

double sensoren::tmp(){return 0.0;}

double sensoren::lautsterke(){return 0.0;}

double sensoren::druck(){return 0.0;}

double sensoren::co(){return 0.0;}

double sensoren::licht(){return 0.0;}

bool sensoren::button(){return false;}