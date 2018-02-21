//Libraries
#include <TimeLib.h>
#include <Wire.h>
//#include <uart.h>

//Defining Pins
#define I2C_SCL 19
#define I2C_SDA 18



//Global Variables
int setpoint = 0;
uint16_t actual_in = 0;
uint16_t actual_out = 0;
int time = 0;
int prev_time = 0;

//Defining ADS registers/commands
byte Set_Inboard[2];

byte Set_Outboard[2];


void setup() {
  //Initialize Serial
  Serial.begin(9600);
  Serial1.begin(115200);
  //uart_init(9600);

  delay(5000); //time to open serial monitor
  
  // Initiate Wire Library
  Wire.begin();

  //Pin Setup
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  
  // Enable Measurement
  Wire.beginTransmission(0x48);
  Wire.endTransmission(); 

  Set_Inboard[0] = 0x81;
  Set_Inboard[1] = 0x83;

  Set_Outboard[0] = 0xB1;
  Set_Outboard[1] = 0x83;
}


uint16_t Read_Inboard_Torque() {
  //Configure ADS to show outboard torque data
  Wire.beginTransmission(byte(0x48)); 
  Wire.write(byte(0x01)); 
  Wire.write(Set_Inboard,2);
  Wire.endTransmission();
  //delay(2);
  //Ask register for data
  Wire.beginTransmission(byte(0x48));
  Wire.write(byte(0x00));
  Wire.endTransmission(); //End transmission, transmits data from register
  Wire.requestFrom(0x48,2); //request the transmitted bytes from the register
  uint16_t result = (Wire.read() << 8) | Wire.read() ;// (high byte low byte)
 // delay(2);
  return result;
}

/*uint16_t Read_Outboard_Torque() {
  //Configure ADS to show outboard torque data
  Wire.beginTransmission(byte(0x48)); 
  Wire.write(byte(0x01)); 
  Wire.write(Set_Outboard,2);
  Wire.endTransmission();
 // delay(2);
  //Ask register for data
  Wire.beginTransmission(byte(0x48));
  Wire.write(byte(0x00));
  Wire.endTransmission(); //End transmission, transmits data from register
  Wire.requestFrom(0x48,2); //request the transmitted bytes from the register
  uint16_t result = (Wire.read() << 8) | Wire.read() ;// (high byte low byte)
  //delay(2);
  return result;
}*/

uint16_t Acquire_Setpoint() {
  int incomingMSB = 0;
  int incomingLSB = 0;
  int incoming = 0;
  if (Serial1.available() > 1) {
    /*setpoint = Serial1.read();
    incomingLSB = setpoint & 0xFF;
    incomingMSB = setpoint >> 8;*/
    incoming = Serial1.read();
  }
  /*if (uart_available() > 0) {
    incomingMSB = uart_getchar();
  }
  if (uart_available() > 0) {
    incomingLSB = uart_getchar();
  }*/
  uint16_t result = (incomingMSB << 8) | incomingLSB;
  return result;
}

void Send_data (uint16_t out_torque, uint16_t in_torque) {
  uint8_t outLSB = out_torque & 0xff;
  uint8_t outMSB = (out_torque >> 8);
  uint8_t inLSB = in_torque & 0xff;
  uint8_t inMSB = (in_torque >> 8);
  Serial.println(" ");
  Serial.print("Inboard MSB = ");
  Serial.print(inMSB);
  Serial.print("   Inboard LSB = ");
  Serial.print(inLSB);
  Serial.println(" ");
  Serial.print("Outboard MSB = ");
  Serial.print(outMSB);
  Serial.print("   Outbboard LSB = ");
  Serial.print(outLSB);
  Serial1.write(inMSB);
  Serial1.write(inLSB);
  Serial1.write(outMSB);
  Serial1.write(outLSB);
  /*uart_putchar(inMSB);
  uart_putchar(inLSB);
  uart_putchar(outMSB);
  uart_putchar(outLSB);*/
}

void loop() {
  //Acquire Relevant Data
  time  = millis() - prev_time;
  actual_in = Read_Inboard_Torque();
  //actual_out = Read_Outboard_Torque();
  if (time > 10) {
     setpoint = Acquire_Setpoint();
     Serial.println(" ");
     Serial.print(setpoint);
     Send_data(actual_out, actual_in);
     Serial.println(" ");
     Serial.print(millis());
     prev_time = millis();
  }
}  


