
#include "Latch.h"
#include <Arduino.h>

  Latch::Latch(int releasePin, int inputPin){
    this->releasePin = releasePin;
    this->inputPin = inputPin;

    pinMode(releasePin, OUTPUT);
    digitalWrite(releasePin, HIGH);
    pinMode(inputPin, INPUT_PULLUP);

  }


  void Latch::release(){
    // release the latch, set a timer
    digitalWrite(releasePin,LOW);
    startTime=millis();
    status=1;
  }

  void Latch::releaseEnd(){
    // enable the latch
    digitalWrite(releasePin,HIGH);
    status=0;
  }


  long Latch::runningTime(){
    if (status==1){
      return millis()-startTime;
    }
    return 0;
  }

  void Latch::ping(){
  }

  int Latch::getStatus(){
    // return a 1 if it is active
    return status;
  }

  int Latch::isClosed(){
    // check to see if the latch is closed
    int inputState = digitalRead(this->inputPin);
    if (inputState==0){
      return 1;
    }
    return 0;
  }




