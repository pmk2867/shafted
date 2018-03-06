/* Speed Teensy Code
 *  Action Items to be completed related to this code by priority
 *  1) Test data acquisition from encoders
 *  2) Test SPI communication to RPi
 *  3) Test ability to write to the DAC (i2c) and change motor speed
 *  
 *  Data Processing notes:
 *  - Position value to be multiplied by 2 by RPi (avoids sending extra byte)
 */

//Libraries
#include <SPI.h>
#include <TimeLib.h>
#include <Wire.h> //i2c Library

//Defining Pins
#define A_phase 22 //encoder data inputs
#define Z_phase 21

#define SSpin 10 //Spi slave select

#define I2C_SCL 19 //i2c clock and data pins
#define I2C_SDA 18

//SPI settings
SPISettings settings(2000000, MSBFIRST, SPI_MODE0); //2000000 = clock speed

// Counters
volatile int A_cnt = 0;
volatile int prev_timeA = 0;
volatile int prev_timeZ = 0;

//Global Variables

uint16_t A_rpm = 0; //rpm and position data variables
uint16_t Z_rpm = 0;
uint8_t A_rpm_LSB = 0;
uint8_t A_rpm_MSB = 0;
uint8_t Z_rpm_LSB = 0;
uint8_t Z_rpm_MSB = 0;
uint8_t pos = 0;

uint16_t setpoint = 0; //control variables
uint8_t setpointMSB = 0;
uint8_t setpointLSB = 0;
int time = 0;

//Timer
IntervalTimer transaction;

void setup() {
  //Initialize Serial Monitor
  Serial.begin(9600);
  
  //Initiate SPI Library
  SPI.begin();

  //Initiate Wire Library
  Wire.begin();
  
  //Pin Setup
  pinMode(A_phase, INPUT);
  pinMode(Z_phase, INPUT);
  pinMode(SSpin, OUTPUT);
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);

  // Defining Interrupts
  attachInterrupt(A_phase, isrA, RISING);
  attachInterrupt(Z_phase, isrZ, RISING);

  //Timing Functions
  transaction.begin(talk_to_rpi, 100000); //Every 1/10th (0.1) seconds
}

// ISR's
void isrA() {
  prev_timeA = time; //keeps track of time interval for rpm calc
  time = micros();
  A_cnt++; // keeps track of position of shaft
}

void isrZ() {
  prev_timeZ = time; //keeps track of time interval for rpm calc
  time = micros();
  A_cnt = 0;
  pos = 0;
}

//Timing Functions
void talk_to_rpi() { //sends to and recieves data from rpi
  SPI.beginTransaction(settings);
  digitalWrite(SSpin, LOW); 
  setpoint = SPI.transfer16(A_rpm); //recieves setpoint and sends Arpm
  SPI.transfer16(Z_rpm); //sends Zrpm
  SPI.transfer(pos); //send position(degrees)/2
  digitalWrite(SSpin, HIGH);
  SPI.endTransaction();
  
  Serial.println(" "); //Printing to serial monitor (Testing)
  Serial.print("Arpm MSB = ");
  Serial.print(A_rpm_MSB);
  Serial.print("   Arpm LSB = ");
  Serial.print(A_rpm_LSB);
  Serial.println(" ");
  Serial.print("Zrpm MSB = ");
  Serial.print(Z_rpm_MSB);
  Serial.print("   Zrpm LSB = ");
  Serial.print(Z_rpm_LSB);
  Serial.println(" ");
  Serial.print("setpoint MSB = ");
  Serial.print(setpointMSB);
  Serial.print("   setpoint LSB = ");
  Serial.print(setpointLSB);
  Serial.println(" ");
  Serial.print("Position = ");
  Serial.print(pos);
  Serial.println(" ");
  Serial.print(millis());
  Serial.println(" ");
}

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

void Control_Speed() { //Goes through DAC to tell VFD to control motor speed
  uint16_t control = setpoint - (Z_rpm - setpoint); //new speed 
  Wire.beginTransmission(byte(0x62)); //--> DAC I2C address
  Wire.write(control >> 8); //2byte message with new speed
  Wire.write(control && 0xff);
  Wire.endTransmission();
}

void loop() {
  //Calculate relevant quantities
  A_rpm = Calc_A_rpm(prev_timeA, time);
  Z_rpm = Calc_Z_rpm(prev_timeZ, time);
  pos = A_cnt;

  //data processing for serial monitor (and possibly data transmission)
  A_rpm_LSB = A_rpm & 0xff; 
  A_rpm_MSB = (A_rpm >> 8);
  Z_rpm_LSB = Z_rpm & 0xff;
  Z_rpm_MSB = (Z_rpm >> 8);
  setpointLSB = setpoint & 0xff;
  setpointMSB = (setpoint >> 8);
}
