float current_speed = 0.0;
int FAN_PIN = 5;
const float stepp = 10.0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 1.4 starting");
  pinMode(FAN_PIN, OUTPUT);
  analogWrite(FAN_PIN, (int)current_speed);
}

void serialFanStatus() {
  if(Serial.available() > 0) {
    int bytes = Serial.read();

    if((char)bytes == '+'){
      if(current_speed == 255) {
        Serial.println("Already at max speed");
      }
      else if((current_speed + stepp) > 255) {
        current_speed = 255;
        Serial.print("Speed: ");
        Serial.println(current_speed);
        analogWrite(FAN_PIN, (int)current_speed);
      }
      else {
        current_speed += stepp;
        Serial.print("Speed: ");
        Serial.println(current_speed);
        analogWrite(FAN_PIN, (int)current_speed);
      }
    }
    else if((char)bytes == '-'){
      if(current_speed == 0) {
        Serial.println("Already at min speed");
      }
      else if((current_speed - stepp) < 0) {
        current_speed = 0;
        Serial.print("Speed: ");
        Serial.println(current_speed);
        analogWrite(FAN_PIN, (int)current_speed);
      }
      else {
        current_speed -= stepp;
        Serial.print("Speed: ");
        Serial.println(current_speed);
        analogWrite(FAN_PIN, (int)current_speed);
      }
    }
    else {
      Serial.println("Invalid Command");
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  serialFanStatus();
}
