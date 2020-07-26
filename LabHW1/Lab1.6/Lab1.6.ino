#include <LiquidCrystal_PCF8574.h>
LiquidCrystal_PCF8574 lcd(0x27);

const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15; //25 gradi centigradi, quindi 273.15 + 25

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();
  lcd.print("Temp:");
  lcd.setCursor(10,0);
  lcd.print("C");
  pinMode(TEMP_PIN, INPUT); 
}

void loop() {
  // put your main code here, to run repeatedly:
  int a = analogRead(TEMP_PIN);
  
  float R = ((vcc/a)-1.0)*R0;
  float logR = log(R/R0);
  float tKel = 1.0/((logR/B)+(1.0/T0));
  float tCel = tKel - 273.15;
  lcd.setCursor(5,0);
  lcd.print(tCel);

  delay(5000); //5 secondi */
}
