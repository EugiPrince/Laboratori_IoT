#include <TimerOne.h>

const int RLED_PIN = 12;
const int GLED_PIN = 13;

const float R_HALF_PERIOD = 1.5;
const float G_HALF_PERIOD = 3.5;

int volatile greenLedState = LOW;
int volatile redLedState = LOW;

void blinkGreen() {
  greenLedState = !greenLedState;
  digitalWrite(GLED_PIN, greenLedState);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 1.2 starting");
  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  Timer1.initialize(G_HALF_PERIOD * 1e06);
  Timer1.attachInterrupt(blinkGreen);
}

void serialPrintStatus() {
  if(Serial.available() > 0) {
    
    int inByte = Serial.read();
    
    if((char)inByte == 'R') {
      if(redLedState == HIGH) {
        Serial.println("LED red status = 1");
      }
      else {
        Serial.println("LED red status = 0");
      }
    }
    else if((char)inByte == 'L') {
      if(greenLedState == HIGH) {
        Serial.println("LED green status = 1");
      }
      else {
        Serial.println("LED green status = 0");
      }
    }
    else {
      Serial.println("Invalid command.");
    } 
  }
  return;
}

void loop() {
  // put your main code here, to run repeatedly:
  redLedState = !redLedState;
  digitalWrite(RLED_PIN, redLedState);
  delay(R_HALF_PERIOD * 1e03);
  serialPrintStatus();
}
