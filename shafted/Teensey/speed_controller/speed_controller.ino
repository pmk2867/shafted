/* Speed Teensy Code
 *  Action Items to be completed related to this code by priority
 *  
 */

//Libraries
#include <TimeLib.h>
#include <i2c_t3.h> //i2c Library

//Defining Pins
#define A1_phase 22 //encoder data inputs
#define Z1_phase 21
#define A2_phase 3
#define Z2_phase 2
#define I2C_SCL 19 //i2c clock and data pins
#define I2C_SDA 18

uint8_t first_message = 0; //UART Address

// Time and Position Counters
volatile int A1_cnt = 0;
volatile double prev_timeA1 = 0;
volatile double prev_timeZ1 = 0;
volatile int A2_cnt = 0;
volatile double prev_timeA2 = 0;
volatile double prev_timeZ2 = 0;
volatile int Called = 0;

//Global Variables
uint16_t A1_rpm_int = 0;
uint16_t Z1_rpm_int = 0;
uint8_t A1_rpm_LSB = 0;
uint8_t A1_rpm_MSB = 0;
uint8_t Z1_rpm_LSB = 0;
uint8_t Z1_rpm_MSB = 0;
uint16_t A2_rpm_int = 0;
uint16_t Z2_rpm_int = 0;
uint8_t A2_rpm_LSB = 0;
uint8_t A2_rpm_MSB = 0;
uint8_t Z2_rpm_LSB = 0;
uint8_t Z2_rpm_MSB = 0;
uint16_t pos1_int = 0;
uint16_t pos2_int = 0;
uint8_t pos1_LSB = 0;
uint8_t pos1_MSB = 0;
uint8_t pos2_LSB = 0;
uint8_t pos2_MSB = 0;
uint8_t setpoint = 0;

//I2C 
uint8_t Address = 96;

int time = 0;

void setup() {
  //Initialize UART Port
  Serial1.begin(115200);

  //Initiate i2c Library
  Wire.begin();
    
  //Pin Setup
  pinMode(A1_phase, INPUT);
  pinMode(Z1_phase, INPUT);
  pinMode(A2_phase, INPUT);
  pinMode(Z2_phase, INPUT);
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);

  //I2C setup
  Wire.beginTransmission(byte(Address));
  Wire.endTransmission();

  // Defining Interrupts
  attachInterrupt(A1_phase, isrA1, RISING);
  attachInterrupt(Z1_phase, isrZ1, RISING);
  attachInterrupt(A2_phase, isrA2, RISING);
  attachInterrupt(Z2_phase, isrZ2, RISING);
}

// ISR's

void isrA1() {
  prev_timeA1 = time; //keeps track of time interval for rpm calc
  time = micros();
  A1_cnt++; // keeps track of pos1ition of shaft
}

void isrZ1() {
  prev_timeZ1 = time; //keeps track of time interval for rpm calc
  time = micros();
  A1_cnt = 0;
  pos1_int = 0;
}

void isrA2() {
  prev_timeA2 = time; //keeps track of time interval for rpm calc
  time = micros();
  A2_cnt++; // keeps track of pos1ition of shaft
}

void isrZ2() {
  prev_timeZ2 = time; //keeps track of time interval for rpm calc
  time = micros();
  A2_cnt = 0;
  pos2_int = 0;
}

uint16_t Calc_A_rpm(volatile int t1, int t2) { //calculates Arpm
  double tr = 180 * (t2 - t1);
  uint16_t rpm = 0;
  if(tr != 0) {
    rpm = 1000000 * 60 * 1 / tr ;
  }
  return rpm;
}

uint16_t Calc_Z_rpm(volatile int t1, int t2) { //calculates Zrpm
  double tr = t2 - t1;
  uint16_t rpm = 0;
  if (tr != 0) {
    rpm = 1000000 * 60 * 1 / tr ;
  }
  return rpm;
}

void Control_Speed() { //Goes through DAC to tell VFD to control motor speed
  uint16_t control = setpoint - (Z1_rpm_int - setpoint); //new speed 
  Wire.beginTransmission(byte(Address)); //--> DAC I2C address
  Wire.write(byte(0x40)); //2nd byte in command sequence
  Wire.write(control >> 8); //2byte message with new speed
  Wire.write((control << 4) && 0xff);
  Wire.endTransmission();
}

void loop() {
  //Calculate relevant quantities
  A1_rpm_int = Calc_A_rpm(prev_timeA1, time);
  Z1_rpm_int = Calc_Z_rpm(prev_timeZ1, time);
  pos1_int = A1_cnt * 2;
  A2_rpm_int = Calc_A_rpm(prev_timeA2, time);
  Z2_rpm_int = Calc_Z_rpm(prev_timeZ2, time);
  pos2_int = A2_cnt * 2;

  //data processing for transmission & control
  A1_rpm_LSB = A1_rpm_int & 0xff; 
  A1_rpm_MSB = (A1_rpm_int >> 8);
  Z1_rpm_LSB = Z1_rpm_int & 0xff;
  Z1_rpm_MSB = (Z1_rpm_int >> 8);
  A2_rpm_LSB = A2_rpm_int & 0xff; 
  A2_rpm_MSB = (A2_rpm_int >> 8);
  Z2_rpm_LSB = Z2_rpm_int & 0xff;
  Z2_rpm_MSB = (Z2_rpm_int >> 8);
  pos1_LSB = pos1_int & 0xff; 
  pos1_MSB = (pos1_int >> 8);
  pos2_LSB = pos2_int & 0xff;
  pos2_MSB = (pos2_int >> 8);

  //speed control
  Control_Speed();
}

void serialEvent1() { //response to new data in RX buffer (RPi request)
  while (Serial1.available()) {//checks RX buffer
    first_message = Serial1.read(); //Acquires setpoint
    if (first_message  == 255) { //Checks Address sent from RPI
      while (Serial1.available() == false) {} //waits for next serial input
      setpoint = Serial1.read(); //Acquires setpoint
      Serial1.write(A1_rpm_MSB); //data transmission
      Serial1.write(A1_rpm_LSB);
      Serial1.write(Z1_rpm_MSB);
      Serial1.write(Z1_rpm_LSB);
      Serial1.write(pos1_MSB);
      Serial1.write(pos1_LSB);
      Serial1.write(A2_rpm_MSB); 
      Serial1.write(A2_rpm_LSB);
      Serial1.write(Z2_rpm_MSB);
      Serial1.write(Z2_rpm_LSB);
      Serial1.write(pos2_MSB);
      Serial1.write(pos2_LSB);
    }
  }
}

