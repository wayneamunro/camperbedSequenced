#ifndef ACTUATOR_H
#define ACTUATOR_H
#include <Arduino.h>

class Actuator {

  public:
    Actuator(int openPin, int closePin);
    void stop();
    void close();
    void open();
    long runningTime();
    void ping();
    int getStatus();
    String getState();
    void setState(String S);

    int openPin, closePin, status=0;
    long startTime=0;
    String State; 

  private:

};

#endif




