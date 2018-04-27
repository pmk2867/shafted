/* Torque Teensy Controller Code
 *  Action Items to be completed related to this code by priority
 *  1) Calibration
 *  
 */


//Libraries
#include <TimeLib.h>
#include <i2c_t3.h>

//Defining Pins
#define I2C_SCL 19 //i2c clock and data pins
#define I2C_SDA 18
#define PWM_out 3  //motor controller I/O
#define IN_A 4
#define IN_B 5
#define EN_A 6
#define EN_B 7

//Global Variables
uint8_t first_message = 0; //UART Address
uint8_t t_setpoint = 0; //torque setpoint variable

uint16_t raw_inboard_data = 10; //inboard and outboard adc data
uint16_t raw_outboard_data = 20; 

int time = 0; 
int prev_time = 0;

byte Set_Inboard[2]; //2 byte arrays to be written to adc configuration register 
byte Set_Outboard[2];

void setup() {
  //Initialize UART Port
  Serial1.begin(115200);
  
  //Initiate i2c Library
  Wire.begin();
  
  //Pin Setup
  pinMode(IN_A, OUTPUT);
  pinMode(IN_B, OUTPUT);
  pinMode(EN_A, OUTPUT);
  pinMode(EN_B, OUTPUT);
  pinMode(PWM_out, OUTPUT);
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
  Wire.beginTransmission(byte(0x48)); //ADC Address = 0x48
  Wire.write(byte(0x00)); //conversion register address = 0x00
  Wire.endTransmission(); 
  Wire.requestFrom(0x48,2); //request the transmitted bytes from the conversion register
  uint16_t result = (Wire.read() << 8) | Wire.read() ;// (high byte low byte) 
  //Configure adc to show outboard data
  Wire.beginTransmission(byte(0x48)); 
  Wire.write(byte(0x01)); //config register address = 0x01
  Wire.write(Set_Outboard,2); //Sets conversion register to show Outboard data
  Wire.endTransmission();
  return result;
}

uint16_t Read_Outboard_Torque() { // Reads Outboard data  
  //Ask register for data
  Wire.beginTransmission(byte(0x48));
  Wire.write(byte(0x00)); 
  Wire.endTransmission(); 
  Wire.requestFrom(0x48,2); //request the transmitted bytes from the conversion register
  uint16_t result = (Wire.read() << 8) | Wire.read() ;
  Wire.beginTransmission(byte(0x48)); 
  Wire.write(byte(0x01)); 
  Wire.write(Set_Inboard,2); //Sets conversion register to show Inboard data
  Wire.endTransmission(); 
  return result;
}

//Serial Monitor printing for testing purposes
/*void Print_data (uint16_t out_torque, uint16_t in_torque) {
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
  time = millis();
  Serial.println(time - prev_time);
}*/

void loop() {
  prev_time = time ;
  raw_inboard_data = Read_Inboard_Torque();
  delay(3);
  raw_outboard_data = Read_Outboard_Torque();
  delay(3);
  //insert data processing here
  //insert control fucntion here
  //Print_data(raw_outboard_data, raw_inboard_data);  
} 

void serialEvent1() { //response to new data in RX buffer (RPi request)
  while (Serial1.available()) {//checks RX buffer
    first_message = Serial1.read();
    if (first_message == 254) {  //Checks address sent from RPI
      while (Serial1.available() == false) {} // waits for next serial input
      t_setpoint = Serial1.read(); // Acquires Setpoint
      uint8_t outLSB = raw_outboard_data & 0xff; //data processing for transmission
      uint8_t outMSB = (raw_outboard_data >> 8);
      uint8_t inLSB = raw_inboard_data & 0xff;
      uint8_t inMSB = (raw_inboard_data >> 8);   
      Serial1.write(inMSB); //data transmission
      Serial1.write(inLSB);
      Serial1.write(outMSB);
      Serial1.write(outLSB);
    }
    if(first_message == 255){ //ensures no communication conflict
      while (Serial1.available() == false) {}
      uint8_t s_setpoint = Serial1.read();
      uint8_t ignore_me [12] = {};
      for (int i = 0; i <= 11; i++){
        ignore_me[i] = Serial1.read();
      }
    }
  }
}




