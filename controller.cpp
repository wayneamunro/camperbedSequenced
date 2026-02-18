/*

class for control blocks to control the flow of the system

blocks will have actions which are called after they complete, these actions will mostly be other control blocks

types:
  - actuator, control the actions of an actuator, it will run until it reaches an end condition of ann error condition
  - wait, wait until this has been called 'n' times before moving on to the next action

the check function should be called every cycle, or ideally only on active blocks

actuators can legitimately end because of:
  - angle reached
  - current over limit
  - current under limit
  - time reached

error conditions can be:
  - time exceeded
  - current exceeded

*/


#include "controller.h"
#include <Arduino.h>
#include "Actuator.h"

/*
  controller::controller(Actuator* ac, String angleLabel, float upperAngle, float lowerAngle, float upperCurrent, float lowerCurrent, long timeLimit, float errorCurrent, long errorTime){
    this->type = "actuator";
    this->ac = ac;


  }
*/
  controller::controller(Actuator* ac){
    this->type = "actuator";
    this->ac = ac;


  }


  // start the action of the controller
  void controller::start(){

  }

  // cancel any actions happening
  // put into a stopped status
  void controller::stop(){

  }

  // called each cycle, will return stopped, running, done, error - with other info, such as when 'done' will have name of next controller(s) to be triggered 
  // the control angles and current are passed in to allow checks to be made
  void controller::check(float a, float b, float c){

  }

