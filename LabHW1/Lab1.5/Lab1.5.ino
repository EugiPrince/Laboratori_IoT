const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15; //25 gradi centigradi, quindi 273.15 + 25

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 1.5 starting");
  pinMode(TEMP_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int a = analogRead(TEMP_PIN);
  Serial.println(a);
  float R = ((vcc/a)-1.0)*R0;
  float logR = log(R/R0);
  float tKel = 1.0/((logR/B)+(1.0/T0));
  float tCel = tKel - 273.15;

  Serial.print("The temperature now is: ");
  Serial.println(tCel);

  delay(5000); //5 secondi
}
