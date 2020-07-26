const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15;

float current_speed = 0.0;
int FAN_PIN = 3;
const float stepp = 10.0;

void setup() {
  // put your setup code here, to run once:
  pinMode(TEMP_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  
  Serial.begin(9600);
  while(!Serial);
  
  Serial.println("Lab SW4 starting");
  analogWrite(FAN_PIN, (int)current_speed);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(returnTemp()); utilizzata per testare il programma
  delay(0.3);
  serialFanStatus();
}

float returnTemp(void){
 int a = analogRead(TEMP_PIN);
 float R = ((vcc/a)-1.0)*R0;
 float logR = log(R/R0);
 float tKel = 1.0/((logR/B)+(1.0/T0));
 float tCel = tKel - 273.15;
   
 return tCel;
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
