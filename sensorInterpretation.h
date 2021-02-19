/* ---Start header file--- */
#ifndef sensorInterpretation_h
#define sensorInterpretation_h

#include "Arduino.h"//inlude basic functions and syntax from the arduino library

class sensorInterpretation{
  //declare all the public functions
  public:
    sensorInterpretation(byte ,byte ,byte ,byte ,byte ,byte);
    sensorTester();
    int moistAverage();
    String timeOfDay();
    boolean waterReady();
    //declare all the private variables and functions only used within the library
  private:
    byte _moist1;
    byte _moist2;
    byte _switch1;
    byte _switch2;
    byte _switch3;
    byte _light;
    boolean moistReady(byte);
    boolean lightReady();
};


#endif //end header file
