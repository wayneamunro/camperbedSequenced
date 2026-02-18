#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <Arduino.h>
#include "Actuator.h"

class controller {

  public:
    String type;
    Actuator* ac;
    String status;

    float upperAngle;
    float lowerAngle;
    float upperCurrent;
    float lowerCurrent;

    float errorCurrent;
    float errorTime;

    controller::controller(Actuator*);
    void controller::start();
    void controller::stop();
    void controller::check(float a, float b, float c);

};

#endif
