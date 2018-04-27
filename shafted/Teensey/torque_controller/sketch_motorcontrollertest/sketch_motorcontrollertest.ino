#include <TimeLib.h>
//Defining Pins
#define PWM_out 3
#define IN_A 4
#define IN_B 5
#define EN_A 6
#define EN_B 7

int time = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(IN_A, OUTPUT);
  pinMode(IN_B, OUTPUT);
  pinMode(EN_A, OUTPUT);
  pinMode(EN_B, OUTPUT);
  pinMode(PWM_out, OUTPUT);

  digitalWrite(EN_A, HIGH);
  digitalWrite(EN_B, HIGH);
  digitalWrite(IN_A, HIGH);
  digitalWrite(IN_B, LOW);
  
}

void loop() {
  if (millis() > 3000) {
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, HIGH);
    analogWrite(PWM_out, 20);
  }
  else {
    analogWrite(PWM_out, 40);
  }
  if (millis() > 6000) {
    digitalWrite(EN_A, LOW);
    digitalWrite(EN_B, LOW);
    Serial.println("Stopped");
    
  }
} 
