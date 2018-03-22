/* Speed Teensy Code
 *  Action Items to be completed related to this code by priority
 *  1) Test data acquisition from encoders
 *  2) Troubleshoot SPI communication code
 *  3) Test ability to write to the DAC (i2c) and change motor speed
 *  
 *  Data Processing notes:
 *  - position value to be multiplied by 2 by RPi (avoids sending extra byte)
 */

//Libraries
#include <TimeLib.h>
//#include <i2c_t3.h> //i2c Library
#include "t3spi.h"


//Defining Pins
#define A1_phase 22 //encoder data inputs
#define Z1_phase 21
#define A2_phase 3
#define Z2_phase 2

//#define I2C_SCL 19 //i2c clock and data pins
//#define I2C_SDA 18

// Counters
volatile int A1_cnt = 0;
volatile int prev_timeA1 = 0;
volatile int prev_timeZ1 = 0;
volatile int A2_cnt = 0;
volatile int prev_timeA2 = 0;
volatile int prev_timeZ2 = 0;
volatile int i = 0;

//Global Variables

uint16_t A1_rpm_int = 0;
uint16_t Z1_rpm_int = 0;
volatile uint8_t A1_rpm_LSB = 0;
volatile uint8_t A1_rpm_MSB = 0;
volatile uint8_t Z1_rpm_LSB = 0;
volatile uint8_t Z1_rpm_MSB = 0;

uint16_t A2_rpm_int = 0;
uint16_t Z2_rpm_int = 0;
volatile uint8_t A2_rpm_LSB = 0;
volatile uint8_t A2_rpm_MSB = 0;
volatile uint8_t Z2_rpm_LSB = 0;
volatile uint8_t Z2_rpm_MSB = 0;

volatile uint8_t pos1_int = 0;
volatile uint8_t pos2_int = 0;


uint16_t setpoint = 0;
volatile uint8_t set_inMSB = 0;
volatile uint8_t set_inLSB = 0;



int time = 0;

void setup() {
  //Initialize Serial Monitor
  Serial.begin(9600);

  //Setup SPI in slave mode
  pinMode (MISO, OUTPUT);
  SPCR |= _BV(SPE);
  SPCR |= _BV(SPIE);
  
  //Initiate Wire Library
  //Wire.begin();
  
  //Pin Setup
  pinMode(A1_phase, INPUT);
  pinMode(Z1_phase, INPUT);
  pinMode(A2_phase, INPUT);
  pinMode(Z2_phase, INPUT);
  //pinMode(SSpin, OUTPUT);
  //Wire.setSDA(I2C_SDA);
  //Wire.setSCL(I2C_SCL);

  // Defining Interrupts
  attachInterrupt(A1_phase, isrA1, RISING);
  attachInterrupt(Z1_phase, isrZ1, RISING);
  attachInterrupt(A2_phase, isrA2, RISING);
  attachInterrupt(Z2_phase, isrZ2, RISING);
 // SPI.attachInterrupt();
}

// ISR's
/*
ISR (SPI_STC_vect) {
  switch(i) {
    case 0: 
      set_inMSB = SPDR;
      SPDR = A1_rpm_MSB;
      i++;
      break;
    case 1: 
      set_inLSB = SPDR;
      SPDR = A1_rpm_LSB;
      i++;
      break;
    case 2: 
      SPDR = Z1_rpm_MSB;
      i++;
      break;
    case 3: 
      SPDR = Z1_rpm_LSB;
      i++;
      break;
    case 4: 
      SPDR = pos1_int;
      i++;
      break;
    case 5: 
      SPDR = A2_rpm_MSB;
      i++;
      break;
    case 6: 
      SPDR = A2_rpm_LSB;
      i++;
      break;
    case 7: 
      SPDR = Z2_rpm_MSB;
      i++;
      break;
    case 8: 
      SPDR = Z2_rpm_LSB;
      i++;
      break;
    case 9: 
      SPDR = pos2_int;
      i = 0;
      break;
  }
}
*/
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



/*void Print_Data() { //Printing to serial monitor (Testing)
  Serial.println(" "); 
  Serial.print("A1rpm MSB = ");
  Serial.print(A1_rpm_MSB);
  Serial.print("   A1rpm LSB = ");
  Serial.print(A1_rpm_LSB);
  Serial.println(" ");
  Serial.print("Z1rpm MSB = ");
  Serial.print(Z1_rpm_MSB);
  Serial.print("   Z1rpm LSB = ");
  Serial.print(Z1_rpm_LSB);
  Serial.println(" ");
  Serial.print("position1 = ");
  Serial.print(pos1_int);
    
  Serial.println(" "); 
  Serial.print("A2rpm MSB = ");
  Serial.print(A2_rpm_MSB);
  Serial.print("   A2rpm LSB = ");
  Serial.print(A2_rpm_LSB);
  Serial.println(" ");
  Serial.print("Z2rpm MSB = ");
  Serial.print(Z2_rpm_MSB);
  Serial.print("   Z2rpm LSB = ");
  Serial.print(Z2_rpm_LSB);
  Serial.println(" ");
  Serial.print("position2 = ");
  Serial.print(pos2_int);
  Serial.println(" "); 
}*/

uint16_t Calc_A_rpm(volatile int t1, int t2) { //calculates Arpm
  double tr = 180 * (t2 - t1);
  uint16_t rpm = 1000000 * 60 * 1 / tr ;
  return rpm;
}

uint16_t Calc_Z_rpm(volatile int t1, int t2) { //calculates Zrpm
  double tr = t2 - t1;
  uint16_t rpm = 1000000 * 60 * 1 / tr ;
  return rpm;
}

/*void Control_Speed() { //Goes through DAC to tell VFD to control motor speed
  uint16_t control = setpoint - (Z1_rpm_int - setpoint_int); //new speed 
  Wire.beginTransmission(byte(0x62)); //--> DAC I2C address
  Wire.write(control >> 8); //2byte message with new speed
  Wire.write(control && 0xff);
  Wire.endTransmission();
}*/

void loop() {
  //Calculate relevant quantities
  A1_rpm_int = Calc_A_rpm(prev_timeA1, time);
  Z1_rpm_int = Calc_Z_rpm(prev_timeZ1, time);
  pos1_int = A1_cnt * 2;
  A2_rpm_int = Calc_A_rpm(prev_timeA2, time);
  Z2_rpm_int = Calc_Z_rpm(prev_timeZ2, time);
  pos2_int = A2_cnt * 2;


  //data processing for transmission
  A1_rpm_LSB = A1_rpm_int & 0xff; 
  A1_rpm_MSB = (A1_rpm_int >> 8);
  Z1_rpm_LSB = Z1_rpm_int & 0xff;
  Z1_rpm_MSB = (Z1_rpm_int >> 8);
  A2_rpm_LSB = A2_rpm_int & 0xff; 
  A2_rpm_MSB = (A2_rpm_int >> 8);
  Z2_rpm_LSB = Z2_rpm_int & 0xff;
  Z2_rpm_MSB = (Z2_rpm_int >> 8);

  //data processing for control
  setpoint = (set_inMSB << 8) | set_inLSB; 

  //speed control
  //Control_Speed();*/
  //Print data to serial monitor
  //Print_Data();
}


