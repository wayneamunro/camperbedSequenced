
#include "Actuator.h"
#include <Arduino.h>

  Actuator::Actuator(int openPin, int closePin){
    this->openPin = openPin;
    this->closePin = closePin;

    pinMode(openPin, OUTPUT);
    digitalWrite(openPin, HIGH);
    pinMode(closePin, OUTPUT);
    digitalWrite(closePin, HIGH);

    State='unknown'; // when everything starts there is no way for the system to know the state of the actuators, needs a close sequence to be run
  }

  void Actuator::stop(){
    digitalWrite(closePin,HIGH);
    digitalWrite(openPin,HIGH);
    status=0;
  }

  void Actuator::close(){
    // close the actuator, it will be defined open based on a range of criteria
    // activate the open pin, set a timer
    digitalWrite(closePin,LOW);
    digitalWrite(openPin,HIGH);
    startTime=millis();
    status=1;
  }

  void Actuator::open(){
    // open the actuator, it will be defined open based on a range of criteria
    // activate the open pin, set a timer
    digitalWrite(openPin,LOW);
    digitalWrite(closePin,HIGH);
    startTime=millis();
    status=1;
  }

  long Actuator::runningTime(){
    if (status==1){
      return millis()-startTime;
    }
    return 0;
  }

  void Actuator::ping(){
    // check if this actuator should have something done to it, ie, if opening, then has it met the open criteria, etc.
  }

  int Actuator::getStatus(){
    // return a 1 if it is active
    return status;
  }

  String Actuator::getState(){
    // return state
    return State;
  }

  void Actuator::setState(String S){
    // return state
    State=S;
  }



