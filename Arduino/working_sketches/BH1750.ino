#include <Wire.h>
#include <BH1750_WE.h>
#define BH1750_ADDRESS 0x23

BH1750_WE myBH1750 = BH1750_WE(&Wire1, BH1750_ADDRESS); 
// You may also pass a TwoWire object: 
//BH1750_WE myBH1750 = BH1750_WE(&Wire, BH1750_ADDRESS);
// If you don't pass any parameter, Wire and 0x23 will be applied

void setup(){
  Serial.begin(9600);
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();
  while(!myBH1750.init()){ // sets default values: mode = CHM, measuring time factor = 1.0
    Serial.println("Connection to the BH1750 failed");
    Serial.println("Check wiring and I2C address");
    //while(1){}
  }
    Serial.println("BH1750 is connected");
  // myBH1750.setMode(CLM);  // uncomment if you want to change the default values
  // myBH1750.setMeasuringTimeFactor(0.45); // uncomment for selection of value between 0.45 and 3.68
}

void loop(){ 
  float lightIntensity = myBH1750.getLux();
  Serial.print(F("Light intensity: "));
  Serial.print(lightIntensity);
  Serial.println(F(" Lux"));
  delay(1000);
}