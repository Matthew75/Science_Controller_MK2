#include <Adafruit_MAX31856.h>

// Defining pins
const float sensor = A1;
const float moistsensor = A2;
const int carouselMotor = 2;
const int carouselDirection = 3;
const int carouselStep = 4;
const int chopperMotor = 5;
const int emitter = 6;
const int zeroSwitch = 7;
const int zeroSwitchInput = 8;

// Thermocouple pins
// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 max = Adafruit_MAX31856(9, 10, 11, 12);
// use hardware SPI, just pass in the CS pin
//Adafruit_MAX31856 max = Adafruit_MAX31856(10);

// Drill snap action switch pins 
const int drillDown = 13;
const int drillUP = 14;
const int sampleLoading = 15;

// Other Stuff
int i; // iteration variable 
const int TotalSteps = 400; // Steps for one full rotation 

// Sets Pins and Sets everything so that initially nothing happens
void setup() {
  Serial.begin(115200);
  pinMode(emitter, OUTPUT);
  pinMode(chopperMotor, OUTPUT);
  pinMode(carouselMotor, OUTPUT);
  pinMode(carouselStep, OUTPUT);
  pinMode(carouselDirection, OUTPUT);
  pinMode(zeroSwitch, INPUT);
  pinMode(zeroSwitchInput, OUTPUT);
  digitalWrite(carouselStep,LOW);
  digitalWrite(carouselDirection, LOW);
  digitalWrite(carouselMotor, HIGH);
  digitalWrite(chopperMotor, LOW);
  digitalWrite(emitter, LOW);
  digitalWrite(zeroSwitchInput,HIGH);
}

// Carousel
// Runs the Carousel, choose direction, speed, # of steps
void runCarousel(boolean direction, double speed, int stepCount){
  digitalWrite(carouselDirection, direction);
  digitalWrite(carouselMotor, HIGH);
  for (int i = 0; i < stepCount; i++){
    digitalWrite(carouselStep, HIGH);
    delayer(speed);
    digitalWrite(carouselStep, LOW);
    delayer(speed);
    Serial.println(carouselStep);
  }
}

// Carousel 
// Delay function based on input speed
void delayer(double speed){
    long holdTime_us = (long)(1.0 / (double) TotalSteps / speed / 2.0 * 1E6);
  int overflowCount = holdTime_us / 65535;
  for (int i = 0; i < overflowCount; i++) {
    delayMicroseconds(65535);
  }
  delayMicroseconds((unsigned int) holdTime_us);
}

// Carousel
// Zeroing function, moves blocker in front of science module
void zeroer(){
  for (i = 0; i < 1;){
     if (digitalRead(zeroSwitch) == HIGH){
        runCarousel(true, 10, 1);
        delay(10);
     }
     if (digitalRead(zeroSwitch) == LOW){
      i++;
     }
  }
  delay(1000);
}

// Thermocouple
void thermocouple(){
  Serial.print("Cold Junction Temp: "); Serial.println(max.readCJTemperature());
  //Serial.println(max.readThermocoupleTemperature()); // It always wants to print out 0 before itll print out the temp so this just does that instead of having the "cold junction temp: " bit equal to 0 which is more confusing
  

  Serial.print("Thermocouple Temp: "); Serial.println(max.readThermocoupleTemperature());
  // Check and print any faults
  uint8_t fault = max.readFault();
  if (fault) {
    if (fault & MAX31856_FAULT_CJRANGE) Serial.println("Cold Junction Range Fault");
    if (fault & MAX31856_FAULT_TCRANGE) Serial.println("Thermocouple Range Fault");
    if (fault & MAX31856_FAULT_CJHIGH)  Serial.println("Cold Junction High Fault");
    if (fault & MAX31856_FAULT_CJLOW)   Serial.println("Cold Junction Low Fault");
    if (fault & MAX31856_FAULT_TCHIGH)  Serial.println("Thermocouple High Fault");
    if (fault & MAX31856_FAULT_TCLOW)   Serial.println("Thermocouple Low Fault");
    if (fault & MAX31856_FAULT_OVUV)    Serial.println("Over/Under Voltage Fault");
    if (fault & MAX31856_FAULT_OPEN)    Serial.println("Thermocouple Open Fault");
  }
  delay(1000);
}

// Moisture Sensor
void moistureSensor(){
  //Equation 4 from 10HS manual
  float moistsensorVal = analogRead(moistsensor);  
  //float output = pow(sensorVal/1000,3)*(2.97*pow(10,-9))-7.37*pow(10,-6)*pow(sensorVal/1000,2)-6.69*pow(10,-3)*(sensorVal/1000)-1.92;

  //Equation 2 from 10HS manual
  float dielectricConstant = abs(pow(moistsensorVal,4)*(2.589*pow(10,-10))-5.010*pow(10,-7)*pow(moistsensorVal,3)-(3.523*pow(10,-4))*pow(moistsensorVal,2)-(9.135*pow(10,-2))*moistsensorVal+7.457);
  // this is a measure of the dieletric permittivity of the soil 

  // paper: https://ntrs.nasa.gov/archive/nasa/casi.ntrs.nasa.gov/19750018483.pdf
  // relates dielectric constant and moisture content
  float moisture = dielectricConstant/80 - 0.26/(dielectricConstant - 1) + 0.11+0.48; //units g/cm^3
  
  Serial.print("Soil Moisture Content (g/cm^3): "); Serial.println(moisture);
}


// Detector
void detectorLoop(){
  for (i = 0; i < 100; i++){
    Serial.print("Infrared Intensity: "); Serial.println(sensor);
    delay(100);
  }
}

// Drill down or up 
void Drill(){
  for (i = 0; i<1;){
    if (digitalRead(drillDown) == HIGH){
      delay(100);
    }
    if (digitalRead(drillUP) == LOW){
      delay(100);
    }
    if (digitalRead(drillUP) == HIGH){
      i = 1;
    }
    if (digitalRead(sampleLoading) == HIGH){
      delay(100);
    }
  }
} 

void loop() {
  // 124 does a half rotation, for the motor at least have to check on how it affects the gears and stuff
  // Working well at 120-140 mA 10.8 V while active and 500 mA 10.8 V while inactive
  // Working on the other power supply draw 0.04A while active 7.2 volts while active
  // 0.18 A while inactive, 6.1 Volts while inactive
  // 624 steps for full rotation 
 
  digitalWrite(chopperMotor, LOW);
  digitalWrite(emitter, LOW);
  Drill();

  // Rotates to zero, in this case will be at the science module
  zeroer();

  // Rotates 1 to drill
  Drill();
  runCarousel(true, 10, 486);
  delay(1000);
  Drill();
  delay(1000);

  //Rotates 1 to science module
  Drill();
  runCarousel(true, 10, 324);
  delay(1000);

  // Turns on Science Module for sample 1
  digitalWrite(chopperMotor, HIGH);
  digitalWrite(emitter, HIGH);
  detectorLoop();
  digitalWrite(chopperMotor, LOW);
  digitalWrite(emitter, LOW);
  Drill();

  //Zeros again
  zeroer();

  // Rotates dump to drill
  Drill();
  runCarousel(true, 10, 162);
  delay(1000);
  Drill();
  delay(1000);

  // Zeros again
  zeroer();

  // Rotates 2 to drill 
  Drill();
  runCarousel(true, 10, 81+324);
  delay(1000);
  Drill();
  delay(1000);

  // Rotates 2 to science module
  Drill();
  runCarousel(true, 10, 324);
  delay(1000);

  // Turns on Science Module for sample 2
  digitalWrite(chopperMotor, HIGH);
  digitalWrite(emitter, HIGH);
  detectorLoop();
  digitalWrite(chopperMotor, LOW);
  digitalWrite(emitter, LOW);
  Drill();

  //Zeros again
  zeroer();

  // Rotates dump to drill
  Drill();
  runCarousel(true, 10, 162);
  delay(1000);
  Drill();
  delay(1000);

  //Zeros again
  zeroer();

  // Rotates 3 to drill 
  Drill();
  runCarousel(true, 10, 324-81);
  delay(1000);
  Drill();
  delay(1000);

  // Rotates 3 to science module
  Drill();
  runCarousel(true, 10, 324);
  delay(1000);
  Drill();
  delay(1000);

  // Turns on Science Module for sample 3
  digitalWrite(chopperMotor, HIGH);
  digitalWrite(emitter, HIGH);
  detectorLoop();
  digitalWrite(chopperMotor, LOW);
  digitalWrite(emitter, LOW);
  Drill();

  // Rotates block back into place
  zeroer();
  delay(1000);
 
  
  // Can exit the loop with this exit function
  //exit(0);
}






  

