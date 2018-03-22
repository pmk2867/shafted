/* Torque Teensy Controller Code
 *  Action Items to be completed related to this code by priority
 *  1) Test motor controller
 *  2) Write code to control Linear Actuator based on RPi torque setpoint 
 *  
 */


//Libraries
#include <TimeLib.h>

#include <i2c_t3.h>

//Defining Pins
#define I2C_SCL 19 //i2c clock and data pins
#define I2C_SDA 18

//Global Variables
uint8_t t_setpoint = 0; //torque setpoint variable

uint16_t raw_inboard_data = 10; //inboard and outboard adc data
uint16_t raw_outboard_data = 20; 

int time = 0;
int prev_time = 0;

byte Set_Inboard[2]; //2 byte arrays to be written to adc configuration register 
byte Set_Outboard[2];

void setup() {
  //Initialize Serial ports
  Serial.begin(9600);
  Serial1.begin(115200);
  
  //Initiate Wire (i2c) Library
  Wire.begin();
  
  //Pin Setup
  Wire.setSDA(I2C_SDA); 
  Wire.setSCL(I2C_SCL);

  //I2c Clock setup
  Wire.setClock(400000);

  //Set Configuration Register Arrays
  Set_Inboard[0] = 0x00; //Bytes to be written that set Conversion Register to show Inboard Data
  Set_Inboard[1] = 0xE3;
  Set_Outboard[0] = 0x30; //Bytes to be written that set Conversion Register to show Outboard Data 
  Set_Outboard[1] = 0xE3;
  
  //Enable i2c Measurement
  Wire.beginTransmission(byte(0x48)); //0x48 = adc i2c address
  Wire.write(byte(0x01)); //config register address
  Wire.write(Set_Inboard,2); //Sets conversion register to show Outboard data
  Wire.endTransmission(); 

}

//Data Acquisition Functions
uint16_t Read_Inboard_Torque() { // Reads Inboard data
  //Ask register for data
  Wire.beginTransmission(byte(0x48));
  Wire.write(byte(0x00)); //conversion register address
  Wire.endTransmission(); 
  Wire.requestFrom(0x48,2); //request the transmitted bytes from the conversion register
  uint16_t result = (Wire.read() << 8) | Wire.read() ;// (high byte low byte) 
  //Configure adc to show outboard data
  Wire.beginTransmission(byte(0x48)); 
  Wire.write(byte(0x01)); //config register address
  Wire.write(Set_Outboard,2); //Sets conversion register to show Inboard data
  Wire.endTransmission();
  return result;
}

uint16_t Read_Outboard_Torque() { // Reads Outboard data  
  //Ask register for data
  Wire.beginTransmission(byte(0x48));
  Wire.write(byte(0x00)); //conversion register address
  Wire.endTransmission(); 
  Wire.requestFrom(0x48,2); //request the transmitted bytes from the conversion register
  uint16_t result = (Wire.read() << 8) | Wire.read() ;// (high byte low byte)
  Wire.beginTransmission(byte(0x48)); //0x48 = adc i2c address
  Wire.write(byte(0x01)); //config register address
  Wire.write(Set_Inboard,2); //Sets conversion register to show Outboard data
  Wire.endTransmission(); 
  return result;
}

//Serial Monitor printing for testing purposes
void Print_data (uint16_t out_torque, uint16_t in_torque) {
  uint8_t outLSB = out_torque & 0xff; //data processing for transmission
  uint8_t outMSB = (out_torque >> 8);
  uint8_t inLSB = in_torque & 0xff;
  uint8_t inMSB = (in_torque >> 8);
  
  Serial.println(" "); //Printing to serial monitor
  Serial.print("Inboard MSB = ");
  Serial.print(inMSB);
  Serial.print("   Inboard LSB = ");
  Serial.print(inLSB);
  Serial.println(" ");
  Serial.print("Outboard MSB = ");
  Serial.print(outMSB);
  Serial.print("   Outbboard LSB = ");
  Serial.print(outLSB);
  Serial.println(" ");
  Serial.print("Setpoint = ");
  Serial.print(t_setpoint);
  Serial.println(" ");
  Serial.print("Time(ms) = ");
  Serial.print(millis());
}

void loop() {
  //time  = millis() - prev_time;
  raw_inboard_data = Read_Inboard_Torque();
  /*while(raw_inboard_data == raw_outboard_data){
    raw_inboard_data = Read_Inboard_Torque();
  }*/
  delay(3);
  raw_outboard_data = Read_Outboard_Torque();
  /*while(raw_inboard_data == raw_outboard_data){
    raw_outboard_data = Read_Outboard_Torque();
  }*/
  delay(3);
  //prints data to serial monitor
  Print_data(raw_outboard_data, raw_inboard_data);
  //prev_time = millis();
  
} 

/*void serialEvent1() { //response to new data in RX buffer (RPi request)
  while (Serial1.available()) {//checks RX buffer
    t_setpoint = Serial1.read(); //Acquires setpoint
    
    uint8_t outLSB = raw_outboard_data & 0xff; //data processing for transmission
    uint8_t outMSB = (raw_outboard_data >> 8);
    uint8_t inLSB = raw_inboard_data & 0xff;
    uint8_t inMSB = (raw_inboard_data >> 8);
    
    Serial1.write(inMSB); //data transmission
    Serial1.write(inLSB);
    Serial1.write(outMSB);
    Serial1.write(outLSB);

    Print_data(raw_outboard_data, raw_inboard_data); //Serial Monitor Print
  }
}*/ 




