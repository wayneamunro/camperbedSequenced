#include <Wire.h>  // Wire library - used for I2C communication
// include the library code:
#include <LiquidCrystal.h>
#include <avr/wdt.h>  // AVR watchdog functions

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 41, en = 31, d4 = 33, d5 = 35, d6 = 37, d7 = 39;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int ADXL345A = 0x53; // The ADXL345 sensor I2C address
int ADXL345B = 0x1D; // The ADXL345 sensor I2C address

float X_out, Y_out, Z_out;  // Outputs


int openButtonState=0;
int closeButtonState=0;
int goButtonState=0;
int cancelButtonState=0;
int buttonPushed = 0;

float currentZero = 0.0;

const int openButton=29;
const int closeButton=27;
const int goButton=25;
const int cancelButton=23;

// the pins that will be used to control the relays
int relayPins[] = {22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52};

String lastAction="";

String runState="stopped";
int closingStatus=0;
int openingStatus=0;
int isLooping=0;
int endPhase = 0;
int legPhase = 0;
int topPhase = 0;
int bedPhase = 0;

String displayState="****";


//================================================================
// setup actuators
#include "Actuator.h"
struct NamedActuator {
  String name;
  Actuator actuator;
};

NamedActuator actuators[4] = {
    { "bed",  Actuator(relayPins[0], relayPins[1]) },
    { "leg",  Actuator(relayPins[2], relayPins[3]) },
    { "end", Actuator(relayPins[4], relayPins[5]) },
    { "top", Actuator(relayPins[6], relayPins[7]) }
};

// Lookup function by name
Actuator* getActuatorByName(String target) {
    for (int i = 0; i < 4; i++) {
        if (actuators[i].name == target) {
            return &actuators[i].actuator;
        }
    }
    return nullptr;  // not found
}

Actuator* ac_bed = getActuatorByName("bed");
Actuator* ac_leg = getActuatorByName("leg");
Actuator* ac_end = getActuatorByName("end");
Actuator* ac_top = getActuatorByName("top");
//================================================================



// setup some controller actions
#include "controller.h"
// open the bed
//controller c_open_bed(ac_bed,"a",0.2,NAN, NAN,NAN, 0 , NAN, 60000);
controller c_open_bed(ac_bed);




void setup() {
  pinMode(openButton, INPUT);
  pinMode(closeButton, INPUT);
  pinMode(goButton, INPUT);
  pinMode(cancelButton, INPUT);

  Serial.begin(9600); // Initiate serial communication for printing the results on the Serial monitor
  delay(100);

  // Enable the watchdog timer with a 1-second timeout
  //wdt_enable(WDTO_2S);  // Options: WDTO_15MS, WDTO_30MS, ..., WDTO_8S
  //wdt_enable(WDTO_4S);  // Options: WDTO_15MS, WDTO_30MS, ..., WDTO_8S

  //================================================================
  // set up the coms to the angle sensors
  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345A); // Start communicating with the device 
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
  Wire.endTransmission();

  delay(10);
  Wire.beginTransmission(ADXL345B); // Start communicating with the device 
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
  Wire.endTransmission();
  delay(10);

  //================================================================
  // start the lcd
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("System Starting.");

  //Serial.println("system starting");
  //lastAction = "system starting";

  //================================================================
  // set the baseline as the current sensor floats a bit, this also gets reset at the beginning of each major operation (open or close)
  currentZero = getCurrent();


  //================================================================
  // setup the actuator pins
  for (int i=0; i<16; i++){
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
  }


}

void loop() {

  //================================================================
  // Reset the watchdog timer to prevent a watchdog timeout reset
  wdt_reset();

  //================================================================
  // get the control parameters and display them
  // apply modifications to the angles based on the spreadsheet mods

  //---------- a is the angle of the bed
  float a = getAngle(ADXL345A,"yx");
//  float a = getAngle2(ADXL345A,"yx");
  // add calibration factor
  a = a + 4.0;
/*
  if (a>45){
    a=a-180;
  }
*/
  //----------- bbase is the angle of the end
  float bbase = getAngle(ADXL345B,"yx");
  // calibrate

  //----------- b is the angle between the bed and the end
  float b = bbase+a;
  //b = b*-1.0;

  //----------- c is the current going through all actuators except for the bed ones
  float c = getCurrent() - currentZero;


  displayLCD(a,b,c,displayState);

  delay (100);


  //================================================================
  // read in the buttons
  openButtonState = digitalRead(openButton);
  closeButtonState = digitalRead(closeButton);
  goButtonState = digitalRead(goButton);
  cancelButtonState = digitalRead(cancelButton);

  if (openButtonState == LOW && closeButtonState==LOW && goButtonState==LOW && cancelButtonState==LOW){
    // if none of the buttons are pushed, then clear the flag saying that a button has been pushed
    buttonPushed=0;
  }

  //----------------------------------------------------------------
  // based on the button presses and the current state, set the current state
  //----------------------------------------------------------------

  // cancel
  if (cancelButtonState == HIGH){
    // need to stop everything
    runState="stopping";
    isLooping=0;
  }


  // open
  if (runState != "stopping" && buttonPushed==0 && openButtonState == HIGH){
    buttonPushed=1;

    if (runState=="opening"){
      // if it is already opening, then stop opening
      runState="stopping";
      lastAction = "stopping";
    } else {
      // else then set it to opening
      runState = "opening";
      openingStatus=0;
      //Serial.println("activating opening");
      lastAction  = "activating opening";
    }
  }

  // close
  if (runState != "stopping" && buttonPushed==0 && closeButtonState == HIGH){
    buttonPushed=1;

    if (runState=="closing"){
      runState="stopping";
      lastAction = "stopping";
    } else {
      runState = "closing";
      closingStatus=0;
      lastAction = "activating closing";
    }
  }

  // start looping
  if (runState=="stopped" && buttonPushed==0 && goButtonState==HIGH){
    // it has been requested to set the isLooping flag, to run the thing continuously

    // check that we are sufficiently closed

    // set the flag and start it off (note this 'flag' is also used as a counter)
    // start with a 'close' as that is the safest
    isLooping=1;
    runState = "opening";
    closingStatus=0;
    openingStatus=0;
    lastAction = "starting looping";

  }




  //----------------------------------------------------------------
  // based on the current state, perform the operations
  //----------------------------------------------------------------

  //----------------------------------------------------------------
  // stopping
  if (runState == "stopping"){
    // a stop has been requested, clear all actuator flags

      ac_bed->stop();
      ac_leg->stop();
      ac_end->stop();
      ac_top->stop();

      runState="stopped";
      displayState="----";
      openingStatus=0;
      closingStatus=0;

  }


  //----------------------------------------------------------------
  // opening
  if (runState == "opening"){

    // reset the current zero point at the start of a new opening, it shifts a bit with time
    if (openingStatus==0){
      openingStatus=1;
      delay(100); // small delay to let any currents settle before zeroing
      currentZero = getCurrent();
      endPhase=0;
      legPhase=0;
      topPhase=0;
      bedPhase=0;
      displayState="open";
    }

    // count the number of actuators that are open
    int openCount = 0;
    if (ac_bed->getState()=="open"){ openCount = openCount + 1; }
    if (ac_top->getState()=="open"){ openCount = openCount + 1; }
    if (ac_end->getState()=="open"){ openCount = openCount + 1; }
    if (ac_leg->getState()=="open"){ openCount = openCount + 1; }

    displayState = "open" + String(openCount) + "*";

    // check current for general over limit
    if (c>3.0){
      // make this an error
        openingStatus = 99;  // error condition
    }

    // when the bed opening goes above -70 (and under 45), then start opening the end
    // stop opening when end opening is at 30 degrees
    float endOpenATrigger1 = -70;
    float endOpenBTrigger2 = -30;

    if (openingStatus==1){
      
      // -------------------------------------------------------------
      // bed opening
      // start opening the bed if not opening
      if (bedPhase==0 && ac_bed->getStatus()==0 && ac_bed->getState()!="open"){
        ac_bed->open();
        bedPhase=1;
      }

      // stop opening the bed when it is open enough
      if (bedPhase==1 && (a>0.2 && a<45) ){
        ac_bed->stop();
        ac_bed->setState("open");
        bedPhase=2;
      }

      // check if the bed has taken too long
      if (ac_bed->runningTime() > 60000 ){
        openingStatus = 99;  // error condition
      }

      // -------------------------------------------------------------
      // end opening

      // initial open, to do while hanging, angle based
      if (endPhase==0 && ac_end->getStatus()==0 && a>endOpenATrigger1 && a<45 ){
        ac_end->open();
        endPhase=1;
      }

      if (endPhase==1 && b<endOpenBTrigger2  ){
        ac_end->stop();
        endPhase=2;
      }

      // secondary open, further on, takes to nearly full, angle based
      if (endPhase==2  && a> -30 && a<45 ){
        ac_end->open();
        endPhase=3;
      }

      if (endPhase==3 && b<-70  ){
        ac_end->stop();
        endPhase=4;
      }

      // tertiary open, open fully based on time
      if (endPhase==4 && a>-20 && a<45){
        // run it for 3 seconds to fully open
        ac_end->open();
        endPhase=5;
      }
      if (endPhase==5 && ac_end->runningTime() > 3000){
        endPhase=6;
        ac_end->stop();
        ac_end->setState("open");
      }

      // -------------------------------------------------------------
      // leg opening
      // once bed is open 30 degrees, run leg open for 23 seconds
      // might need to be adjusted to allow retraction if it hits before bed is open
      if (legPhase==0 && ac_leg->getStatus()==0 && a>-60 && a<45 ){
        ac_leg->open();
        legPhase=1;
      }

      if (legPhase==1 && ac_leg->runningTime() > 23000){
        ac_leg->stop();
        legPhase=2;
      }


      // second part of opening goes until it hits, activates after bed is open
      if (legPhase==2 && ac_bed->getState()=="open" && ac_end->getState()=="open" && ac_top->getStatus()==0){
        // once bed is open and the end and top are not moving, finish off the leg
        delay(100); // small delay to let any currents settle before zeroing
        currentZero = getCurrent();
        ac_leg->open();
        legPhase=3;
      }

      if (legPhase==3 && c>=0.5 && ac_leg->runningTime()>500){
        // then the leg has opened fully
        ac_leg->stop();
        ac_leg->setState("open");
        legPhase=5;
      }
      
      // -------------------------------------------------------------
      // top opening
      if (topPhase==0 && a<45 && a>-35){
        // run the top for 10 seconds
        ac_top->open();
        topPhase=1;
      }

      if (topPhase==1 && ac_top->runningTime()>25000){
        ac_top->stop();
        topPhase=2;
      }

      if (topPhase==2 && ac_bed->getState()=="open" && ac_end->getState()=="open" && ac_leg->getState()=="open"){
        // very last thing, tighten the roof
        delay(100); // small delay to let any currents settle before zeroing
        currentZero = getCurrent();
        ac_top->open();
        topPhase=3;
      }

      if (topPhase==3 && ((c > 0.4 && ac_top->runningTime()>200) || (c<0.05 && ac_top->runningTime()>20000))){
        // move until tight enough (based on current), then stop
        ac_top->stop();
        topPhase=4;
        ac_top->setState("open");

      }



      if (ac_bed->getState()=="open"  && ac_leg->getState()=="open"  && ac_end->getState()=="open"  && ac_top->getState()=="open"){
        // then opening is complete
        openingStatus=2;
      }
    }

    if (openingStatus==2){
      // opening has been successuly completed
      displayState="----";
      openingStatus=0;
      if (isLooping){
        runState = "closing";
        closingStatus=0;
      } else {
        runState = "stopping";
      }

    }


    if (openingStatus==99){
      // this is an error condition somehow, at the least stop operations.
      ac_bed->stop();
      ac_end->stop();
      ac_leg->stop();
      ac_top->stop();
      runState="stopped";
      displayState="err";
    }


  }

  //----------------------------------------------------------------
  // closing
  if (runState == "closing"){

    // reset the current zero point at the start of a new opening, it shifts a bit with time
    if (closingStatus==0){
      closingStatus=1;
      delay(100);
      currentZero = getCurrent();
      endPhase=0;
      legPhase=0;
      topPhase=0;
      bedPhase=0;
    }

    // check current for general over limit
    if (c>3.0){
      // make this an error
        closingStatus = 99;  // error condition
    }

    if (closingStatus==1){
      // do the closing, initially keep this simple, but easy to move to a sequenced operation


      // count the number of actuators that are open
      int closeCount = 0;
      if (ac_bed->getState()=="closed"){ closeCount = closeCount + 1; }
      if (ac_top->getState()=="closed"){ closeCount = closeCount + 1; }
      if (ac_end->getState()=="closed"){ closeCount = closeCount + 1; }
      if (ac_leg->getState()=="closed"){ closeCount = closeCount + 1; }
      displayState = "cls" + String(closeCount) + "-";
/*
      Serial.println(closeCount);
      Serial.println(ac_top->getState());
      Serial.println(displayState);
*/


      // -------------------------------------------------------------
      // top closing
      if (topPhase==0 && ac_top->getStatus()==0 && ac_top->getStatus()!="closed"){
        ac_top->close();
        topPhase=1;
//        delay(100); // small delay to let any currents settle before zeroing
//        currentZero = getCurrent(); // ******* need to be careful with this when sequenced
      }

//      if (topPhase==1 && (ac_top->runningTime() > 60000 || (c>0.5) )){
      if (topPhase==1 && (ac_top->runningTime() > 60000 )){
        // taken too long or too much current, error
        closingStatus = 99;
      }

      if (topPhase==1 && ac_top->runningTime() > 1000 && c < 0.05 ){
        // consider it closed
        ac_top->stop();
        ac_top->setState("closed");
        Serial.println("Top Closed");
        topPhase=2;
      }

      // -------------------------------------------------------------
      // end closing
      // close end after a pause
      if (endPhase==0 && ac_top->getState()=="closed" && ac_end->getStatus()==0 && ac_end->getState()!="closed"){
//      if (endPhase==0 && ac_top->runningTime()>7000 && ac_end->getStatus()==0 && ac_end->getState()!="closed"){
        ac_end->close();
        endPhase=1;
      }

      if (endPhase==1 && ac_end->runningTime() > 25000 ){
        // taken too long, error
        closingStatus = 99;
      }

      if (endPhase==1 && ( (b>4 && b<45) || (c>0.6 && ac_end->runningTime()>1000) ) ){
//      if (endPhase==1 && ( (b>-5 && b<45) || (c>0.6 && ac_end->runningTime()>1000) ) ){
//      if (endPhase==1 && ( (b>6 && b<45) || (c>2.0 && ac_end->runningTime()>1000) ) ){
//      if (endPhase==1 && ( (b>6 && b<45) ) ){
        // call it opened
        ac_end->stop();
        ac_end->setState("closed");
        endPhase=2;
        Serial.println("End Closed");
        Serial.println(ac_end->getState() );
      }

      // -------------------------------------------------------------
      // leg closing
      // close leg after the top and the end are closed
//      if (legPhase==0 && ac_top->getState()=="closed" && ac_end->getState()=="closed" && ac_leg->getStatus()==0 && ac_leg->getState()!="closed"){
      if (legPhase==0 && ac_top->getState()=="closed" && ac_end->getState()=="closed" ){
//      if (legPhase==0 && ac_end->runningTime()>1000){
        delay(100);
        currentZero = getCurrent(); // **** adjust this when multiple things are happening
        ac_leg->close();
        legPhase=1;
      }


      if (legPhase==1 && ac_leg->runningTime() > 60000 ){
        // taken too long, error
        closingStatus = 99;
        delay(100);
        currentZero = getCurrent(); // **** adjust this when multiple things are happening

        // reset the lcd, think there is a hardware bug on leg use which causes it to go funny now and again
        LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
        lcd.begin(16, 2);
      }
/*
      // run for a time at the same time as everything else
      if (legPhase==1 && ac_leg->runningTime() > 25000 ){
        ac_leg->stop();
        legPhase=2;
      }

      if (legPhase==2 && ac_top->getState()=="closed" && ac_end->getState()=="closed"){
        legPhase=3;
        ac_leg->close();
      }
*/
      if (legPhase==1 && ( (ac_leg->runningTime() >500 && c<0.1) || (ac_leg->runningTime()>500 && c>0.7)  )){
        ac_leg->stop();
        ac_leg->setState("closed");
//        delay(100);
//        currentZero = getCurrent();
        legPhase=2;  // make sure to change this when going two stage

        // reset the lcd, think there is a hardware bug on leg use which causes it to go funny now and again
        LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
        lcd.begin(16, 2);
      }


      // -------------------------------------------------------------
      // bed closing
      // start closing the bed if not closing already, do after all the others are shut
      if (ac_top->getState()=="closed" && ac_end->getState()=="closed" && ac_leg->getState()=="closed" && ac_bed->getStatus()==0 && ac_bed->getState()!="closed"){
        ac_bed->close();
        bedPhase=1;
      }

      // check if the bed has taken too long
      if (bedPhase==1 && ac_bed->runningTime() > 60000 ){
        openingStatus = 99;  // error condition
      }

      if (bedPhase==1 && a<-70){
        ac_bed->stop();
        bedPhase=2;
      }

      if (bedPhase==2){
        ac_bed->close();
        bedPhase=3;
      }

      if (bedPhase==3 && ac_bed->runningTime()>8000){
        // then must be fully closed
        bedPhase=4;
        ac_bed->stop();
        ac_bed->setState("closed");
      }

      if (ac_bed->getState()=="closed"  && ac_leg->getState()=="closed"  && ac_end->getState()=="closed"  && ac_top->getState()=="closed"){
        // then closing is complete
        closingStatus=2;
      }

    }

    if (closingStatus==2){
      // opening has been successuly completed
      displayState="----";
      closingStatus=0;
      if (isLooping){
        runState = "opening";
        openingStatus=0;
      } else {
        runState = "stopping";
      }

    }

    if (closingStatus==99){
      // this is an error condition somehow, at the least stop operations.
      ac_bed->stop();
      ac_end->stop();
      ac_leg->stop();
      ac_top->stop();
      runState="stopped";
      displayState = "err99";
    }
  }


}
