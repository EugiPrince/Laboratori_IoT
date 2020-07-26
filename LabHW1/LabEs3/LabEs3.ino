const int LED_PIN = 13;
const int PIR_PIN = 7;
volatile int count = 0;

int volatile ledState = LOW;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), checkPresence, CHANGE);
}

void checkPresence() {
  int volatile pirValue;
  pirValue = digitalRead(PIR_PIN);
  if(pirValue == HIGH){
    count++;
    Serial.print("Rilevamenti: ");
    Serial.println(count);
    ledState = HIGH;
    digitalWrite(LED_PIN, ledState);
  }
  else { 
    ledState = LOW;
    digitalWrite(LED_PIN, ledState);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
