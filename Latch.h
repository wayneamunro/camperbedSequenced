#ifndef LATCH_H
#define LATCH_H
#include <Arduino.h>

class Latch {

  public:
    Latch(int releasePin, int inputPin);
    void release();
    void releaseEnd();
    long runningTime();
    void ping();
    int getStatus();
    int isClosed();

    int releasePin, inputPin, status=0;
    long startTime=0;

  private:

};

#endif




