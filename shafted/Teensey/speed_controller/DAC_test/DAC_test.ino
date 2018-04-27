#include <i2c_t3.h>
#define I2C_SCL 19 //i2c clock and data pins
#define I2C_SDA 18

uint8_t Address = 96; 
uint8_t Result = 0x00;
int i = 0;
void setup()
{
  Serial.begin(9600);
  Wire.begin();

  Wire.setSDA(I2C_SDA); 
  Wire.setSCL(I2C_SCL);
  Wire.beginTransmission(byte(Address));
  Wire.endTransmission();
}

void loop()
{
    Wire.beginTransmission(byte(Address)); //address
    //Wire.write(byte(Address));
    Wire.write(byte(0x40)); //2nd Byte
    Wire.write(byte(0x5D)); //DAC input (3rd byte)     
    Wire.write(byte(0xC0)); //DAC input (4th byte) (last 4 bits unused)
    i = Wire.endTransmission();
    Serial.println(i);
    delay(10);
  Serial.println("done");
  Serial.println(Result);
}
