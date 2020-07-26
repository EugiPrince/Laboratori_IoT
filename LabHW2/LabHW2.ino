#include <LiquidCrystal_PCF8574.h>
#include <TimerOne.h>
LiquidCrystal_PCF8574 lcd(0x27); 

//variabili globali
volatile float temp;
volatile float pwmLED;
volatile float velVentola;
volatile int percFan;
//sia le funzioni relative al PIR e al microfono possono rendere TRUE la variabile globale di presenza.
boolean isPresent = false;
//valori temporali
volatile unsigned long start_time_PIR = 0;
volatile unsigned long start_time_sound=0;
volatile unsigned long time_now;

//questi valori descrivono le rette del comportamento dei dispositivi. il loro valore varia a seconda che
//siano presenti o meno persone.
float coeffVentola, termNotoVentola, coeffLED, termNotoLED;

//stabilisco i valori arbitrari di temperatura per Condizionatore e Riscaldamento, che variano alla presenza
//o meno di qualcuno nella stanza
//SE PRESENZA
float AC_P_MIN = 22, AC_P_MAX = 30;
float HT_P_MIN = 22, HT_P_MAX = 30;

//SE NON PRESENZA
float AC_NP_MIN = 20, AC_NP_MAX = 30;
float HT_NP_MIN = 10, HT_NP_MAX = 20;

//globali riferimento post verifica
float HT_min, HT_max, AC_min, AC_max;

//inizializzo variabili sensore PIR
boolean isPresentPIR = false;
int PIR_PIN = 7;
unsigned long timeout_pir = 1e08;

//inizializzo variabili per sensore suono
int countSounds = 0;
boolean isPresentSound = false;
unsigned long timeout_sound = 2e08;
int SOUND_PIN = 12;
unsigned long sound_interval = 1e07; //intervallo in cui devono verificarsi event_limit suoni
int event_limit = 40; //limite massimo di eventi che devono essere rilevati per giustificare presenza

//inizializzo variabili per LED
int LED_PIN = 6; //perchè pwm 

//inizializzo variabili per fan
int FAN_PIN = 5;

//inizializzo variabili per termistore
const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15; //25 gradi centigradi, quindi 273.15 + 25

void setup() {
  // put your setup code here, to run once:
  pinMode(FAN_PIN, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);

  //inizializzo variabili schermo LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear(); 

  //inizializzo set point supponendo presenza iniziale
  coeffVentola = getCoeffVentola(true);
  termNotoVentola = getTermNotoVentola(true);
  coeffLED = getCoeffLED(true);
  termNotoLED = getTermNotoLED(true);
 
  //interrupt per sensore PIR, attivato solo alla rilevazione
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), verifyPresence_PIR, RISING);

  //interrupt per sensore suono, attivato solo al falling del segnale
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), verifyPresence_sound, FALLING);
}

void loop() {

  changeReferenceValuesFromSerial();
  //tempo corrente
  time_now = millis(); 
  //controllo continuo che negli ultimi 10 minuti non ci siano stati movimenti PIR
  checkPIR(time_now);

  //controllo sound
  checkSound(time_now);

  //se entrambi isPresentSound e isPresentPIR sono falsi anche la globale è falsa
  if(!isPresentSound && !isPresentPIR){
    isPresent = false;
  }

  //a seconda che ci siano persone o meno modifica i valori di riferimento
  changeReferenceValues(isPresent);

  //dopo aver verificato presenza o meno cambio i coefficienti delle funzioni degli attuatori
  updateCoeffTermNoto(isPresent);

  //dove y = velocitàVentola e x = temperatura
 
  temp = returnTemp();
  velVentola = updateVentola(temp, velVentola);
  
  pwmLED = updateLED(temp, pwmLED);

  percFan = getPercentageFan(velVentola);
  int percLED = getPercentageLED(pwmLED);
  
  LCDview(temp,isPresent,percFan, percLED);
}

void checkPIR(long int time_now){
  if(time_now - start_time_PIR > timeout_pir){
    isPresentPIR = false;
  }
}

void checkSound(long int time_now){
  if(time_now - start_time_sound > timeout_sound){ //se superata la finestra temporale
    //controllo numero eventi
    if(countSounds < event_limit){
  
      isPresentSound = false;
    }
    //al termine della finestra azzera counter
    countSounds = 0;
    
  }else { //se ancora nella finestra
    if(countSounds > event_limit){
      isPresent = true;
      isPresentSound = true;
    }
  }
}

void updateCoeffTermNoto(boolean isPresent){
  coeffVentola = getCoeffVentola(isPresent);
  termNotoVentola = getTermNotoVentola(isPresent);
  coeffLED = getCoeffLED(isPresent);
  termNotoLED = getTermNotoLED(isPresent);
}

float updateVentola(float temp, float velVentola){
  velVentola = coeffVentola*temp + termNotoVentola;
  velVentola = checkVelVentola(velVentola);
  
  //modifica intensità ventola
  analogWrite(FAN_PIN, (int)velVentola);
  return velVentola;
}

float updateLED(float temp, int pwmLED){
  //modifica intensità LED
  pwmLED = returnLedPWM(temp);
  //il valore 255 corrisponde al LED spento e 0 alla massima luminosità
  //si tratta tuttavia di una differenza di luminosità apprezzabile solo nell'oscurità
  analogWrite(LED_PIN, pwmLED);
  return pwmLED;
}

void LCDview(float temp, boolean isPresent, int percFan, int percLED){
  //PAGINA 1
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("T:");  lcd.print(temp); lcd.print("C");
  lcd.print("   P:"); lcd.print(isPresent);
  lcd.setCursor(0,1); lcd.print("AC:"); lcd.print(percFan); lcd.print("%  ");
  lcd.print("HT:"); lcd.print(percLED); 
  lcd.setCursor(14,1);lcd.print("%");
  delay(5000);
  //PAGINA 2
  lcd.clear();
  lcd.setCursor(0,0);
  
  lcd.print("AC m:");lcd.print(AC_min,1); lcd.print(" M:"); lcd.print(AC_max,1);
  lcd.setCursor(0,1);
  lcd.print("HT m:");lcd.print(HT_min,1); lcd.print(" M:"); lcd.print(HT_max,1);
  delay(5000);
}

float returnTemp(void){
  int a = analogRead(TEMP_PIN);
  float R = ((vcc/a)-1.0)*R0;
  float logR = log(R/R0);
  float tKel = 1.0/((logR/B)+(1.0/T0));
  float tCel = tKel - 273.15;
   
  return tCel;
}

int returnLedPWM(float temp){
  float pwm_;
  pwm_ = (coeffLED*temp) + termNotoLED;
  
  if(pwm_ > 255){
    pwm_ = 255;
  }
  if(pwm_<0){
    pwm_ =0;
  }
  
  return (int)pwm_;
}

//se viene rilevata una presenza col sensore PIR, il valore globale isPresent diventa true
//e fa partire timer millis per vedere se nei 10 minuti successivi ha altri movimenti.
//se questo non avviene (e non vengono rilevati gli n_eventi di suono) il booleano 
//torna a false

void verifyPresence_PIR(){
  //alla chiamata dell'interrupt rising cambia il valore del booleano
  isPresentPIR = true;
  isPresent = true;
  start_time_PIR = millis();
}

//funzione chiamata al FALLING del valore rilevato, aumenta il valore di countSounds
void verifyPresence_sound(){
  start_time_sound = millis();
  countSounds++; 
}

//coeffVentola, termNotoVentola, coeffLED, termNotoLED;
float getCoeffVentola(boolean isPresent){
  //imposto come ordinata minima 40 poichè al di sotto di questo valore la ventola non parte
  float ymax = 255, ymin = 40, coeffVentola;
  
  coeffVentola = (ymax-ymin)/(AC_max -AC_min);
  return coeffVentola;
}
float getTermNotoVentola(boolean isPresent){
  float ymax = 255, ymin = 40, termNotoVentola;
  
  termNotoVentola = -(( (ymax-ymin)/(AC_max-AC_min) ) * AC_min) + ymin;
  return termNotoVentola;
}

float getCoeffLED(boolean isPresent){
  float ymax = 255, ymin = 0, coeffLED;
  
  coeffLED = (ymax-ymin)/(HT_max - HT_min);
  return coeffLED;
}

float getTermNotoLED(boolean isPresent){
  float ymax = 255, ymin = 0, termNotoLED;
  
  termNotoLED = -(( (ymax-ymin)/(HT_max-HT_min) ) *HT_min ) + ymin;
  return termNotoLED;
}

int getPercentageFan(float val){
  Serial.println(val);
  float perc=0;
  perc = (val/255)*100;
  Serial.println(perc);
  
  return (int)perc;
}

int getPercentageLED(float val){
  float perc=0;
  perc = (val/255)*100;
  perc = 100-perc;
  return perc;
}

float checkVelVentola(float velVentola){
  if(velVentola > 255){
    return 255;
  }
  if(velVentola<0){
    return 0;
  }
  else{
    return velVentola;
  }
}

void changeReferenceValues(boolean val){
  if(val){ //se isPresent = true
    HT_min = HT_P_MIN;
    HT_max = HT_P_MAX;
    AC_min = AC_P_MIN;
    AC_max = AC_P_MAX;
    
  }else{
    HT_min = HT_NP_MIN;
    HT_max = HT_NP_MAX;
    AC_min = AC_NP_MIN;
    AC_max = AC_NP_MAX;   
  }
}

void changeReferenceValuesFromSerial(){
  volatile float val;
  
  if(Serial.available() > 0) {
    String str = Serial.readString();
    //la stringa fornita deve essere nel formato 'XXX val'
    
    if(str.startsWith("ACm")){
      str = str.substring(4);
      val = str.toFloat();
      AC_min = val;
    }
    else if(str.startsWith("ACM")){
      str = str.substring(4);
      val = str.toFloat();
      AC_max = val;
    }
    else if(str.startsWith("HTm")){
      str = str.substring(4);
      val = str.toFloat();
      HT_min = val;
    }
    else if(str.startsWith("HTM")){
      str = str.substring(4);
      val = str.toFloat();
      HT_max = val;
    }
    else{
      Serial.println("Formato non abilitato");
    }
  }
}
