
void output( String lastAction, String runState, int status, float a, float bbase, float b, float c){
  Serial.print("lastAction=");
  Serial.print(lastAction);
  Serial.print(",timeStamp=");
  Serial.print(millis());
  Serial.print(", runState=");
  Serial.print(runState);
  Serial.print(", status=");
  Serial.print(status);
  Serial.print(", a=");
  Serial.print(a);
  Serial.print(", bbase=");
  Serial.print(bbase);
  Serial.print(", b=");
  Serial.print(b);
  Serial.print(", c=");
  Serial.println(c);
}

void printCurrents(float currents[5]){
  Serial.print("currents: ");
  for (int i=0; i<5; i++){
    Serial.print(currents[i],2);
    if (i<4){
      Serial.print(", ");
    }
  }
  Serial.println();
}

float getCurrent(){
  int readNum=10;
  int sensorValue = 0;
  for (int i=0; i<readNum; i++){
    sensorValue += analogRead(A0);
  }
  float current = (sensorValue/(1.0*readNum)/1024.0 - 0.5)*15.0;
  return current;
}

// pass in a pin, such as A0 -> A10
float getCurrentByPin(int pin){
  int readNum=10;
  int sensorValue = 0;
  for (int i=0; i<readNum; i++){
    sensorValue += analogRead(pin);
  }
  float current = (sensorValue/(1.0*readNum)/1024.0 - 0.5)*15.0;
  return current;
}

void getAllCurrents(float currents[5]){
  currents[0] = getCurrentByPin(A0);
  currents[1] = getCurrentByPin(A1);
  currents[2] = getCurrentByPin(A2);
  currents[3] = getCurrentByPin(A3);
  currents[4] = getCurrentByPin(A4);
}

float getAngle(int ADXL345, String type){
  // === Read acceleromter data === //

  int numRead = 5;
  float X=0.0;
  float Y=0.0;
  float Z=0.0;

  for (int i=0; i<numRead; i++){
//    Serial.println("reading adxl");
    delay(5);
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    X_out = ( Wire.read()| Wire.read() << 8); // X-axis value
    X_out = X_out/256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
    X += X_out;
    Y_out = ( Wire.read()| Wire.read() << 8); // Y-axis value
    Y_out = Y_out/256;
    Y += Y_out;
    Z_out = ( Wire.read()| Wire.read() << 8); // Z-axis value
    Z_out = Z_out/256;
    Z += Z_out;

  }
  X = X/(1.0 * numRead);
  Y = Y/(1.0 * numRead);
  Z = Z/(1.0 * numRead);

  // calculate the y/z angle
  float angle=0.0;
  
  if (type=="yz"){
    angle = atan(Y/Z) * 57.2957795131;
  }
  

  if (type=="yx") {
//    if (X>Y){ 
      angle = atan(Y/X) * 57.2957795131;  
//    } else {
//      angle = 90.0- (atan(X/Y) * 57.2957795131);  

//    }
  }

  return angle;

}

float getAngle2(int ADXL345, String type){
  // === Read acceleromter data === //

  int numRead = 5;
  float X=0.0;
  float Y=0.0;
  float Z=0.0;

  for (int i=0; i<numRead; i++){
//    Serial.println("reading adxl");
    delay(5);
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    X_out = ( Wire.read()| Wire.read() << 8); // X-axis value
    X_out = X_out/256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
    X += X_out;
    Y_out = ( Wire.read()| Wire.read() << 8); // Y-axis value
    Y_out = Y_out/256;
    Y += Y_out;
    Z_out = ( Wire.read()| Wire.read() << 8); // Z-axis value
    Z_out = Z_out/256;
    Z += Z_out;

  }
  X = X/(1.0 * numRead);
  Y = Y/(1.0 * numRead);
  Z = Z/(1.0 * numRead);

  // calculate the y/z angle
  float angle=0.0;
  
  if (type=="yz"){
    angle = atan(Y/Z) * 57.2957795131;
  }
  

  if (type=="yx") {
    float AX = abs(X);
    float AY = abs(Y);

    if (X>=0.0 && Y>=0.0){
      if (AX>AY){
        angle = atan(AY/AX);
      } else {
        angle = 90.0-atan(AX/AY);
      }
    }
    if (X<0.0 && Y>=0.0){
      if (AX>AY){
        angle = 180.0-atan(AY/AX);
      } else {
        angle = 90.0+atan(AX/AY);
      }      
    }
    if (X>=0.0 && Y<0.0){
      if (AX>AY){
        angle = 0.0-atan(AY/AX);
      } else {
        angle = -90.0+atan(AX/AY);
      }      

    }
    if (X<0.0 && Y<0.0){
      if (AX>AY){
        angle = -180.0+atan(AY/AX);
      } else {
        angle = -90.0-atan(AX/AY);
      }      
      
    }

  }

  return angle;

}

void displayLCD(float a,float b,float c,String displayState){
  //===============================================
  // output the current state to the LCD

  char buffer[20]; 
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A=");
  if (a>=0){
    lcd.print(" ");
  }
  dtostrf(a, -4, 1, buffer);
  lcd.print(buffer);
  lcd.print(",");

  lcd.setCursor(9,0);
  lcd.print("B=");
  if (b>=0){
    lcd.print(" ");
  }
  dtostrf(b, -4, 1, buffer);
  lcd.print(buffer);

  lcd.setCursor(15,1);
  lcd.print(random(9));



  lcd.setCursor(0,1);
  lcd.print("C= ");
  lcd.print(c);

//  lcd.print (" ");
  lcd.setCursor(9,1);
  lcd.print (displayState);
  //===============================================
}

