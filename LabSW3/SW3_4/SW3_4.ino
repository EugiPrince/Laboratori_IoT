#include <MQTTclient.h>
#include <Bridge.h>
#include <Process.h>
#include <TimerOne.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_PCF8574.h>
LiquidCrystal_PCF8574 lcd(0x27); //Schermo LCD

//Variabili necessarie a creare le rette che descrivono il comportamento del LED e del fan
volatile float temp;
volatile float pwmLED;
volatile float velVentola;

float coeffVentola, termNotoVentola, coeffLED, termNotoLED;

//Varaibili per il sensore di temperatura
const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15; //25 gradi centigradi, quindi 273.15 + 25

boolean isPresent = false;
//valori temporali
volatile unsigned long start_time_PIR = 0;
volatile unsigned long start_time_sound=0;
volatile unsigned long time_now;

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

//Variabili per LED e FAN
int LED_PIN = 6;
int FAN_PIN = 5;

//Valori arbitrari scelti in caso di presenza
float AC_P_MIN = 22, AC_P_MAX = 30;
float HT_P_MIN = 22, HT_P_MAX = 30;

//Valori arbitrari scelti nel caso in cui sia rilevata presenza
float AC_NP_MIN = 20, AC_NP_MAX = 30;
float HT_NP_MIN = 10, HT_NP_MAX = 20;

//Variabili globali per AC e HT, decise dall'utente
float HT_min, HT_max, AC_min, AC_max;

//Topic
String my_base_topic = "/tiot/9";

StaticJsonDocument<256> doc;
JsonObject root = doc.to<JsonObject>();

const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec_references(capacity);
DynamicJsonDocument doc_rec_lcd(2048);

int curl_function(String text);

void setup() {
  pinMode(TEMP_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  //Inizializzo variabili schermo LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear(); 
  
  Bridge.begin();
  mqtt.begin("test.mosquitto.org", 1883);
  //Controllare anche resources e endpoints
  mqtt.subscribe(my_base_topic + String("/changeReferences"), setReferenceValues);
  mqtt.subscribe(my_base_topic + String("/lcd"), setLcdView);

  //inizializzo set point supponendo presenza iniziale (isPresent = true)
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
  //put your main code here, to run repeatedly:
  //Formato dati per la registrazione della scheda Arduino al Catalog
  root["device_id"] = "Yun";
  JsonArray resources = root.createNestedArray("resources"); //Array per le risorse
  resources.add("temperature sensor"); resources.add("presence sensor"); resources.add("sound sensor");
  resources.add("fan"); resources.add("led"); resources.add("LCD screen");

  //Gli endpoint che vengono registrati rappresentano solamente i topic a cui ci si sottoscrive e attraverso
  //i quali la scheda arduino manderà le informazioni da essa misurate
  JsonArray endpoints = root.createNestedArray("endpoints"); //Array per gli endpoints
  endpoints.add("/tiot/9/presence"); endpoints.add("/tiot/9/temperature");
  endpoints.add("/tiot/9/sound");
  
  mqtt.monitor();
  
  String output;
  serializeJson(root, output);
  volatile int ret;
  ret = curl_function(output);
  
  time_now = millis();
  checkPIR(time_now);
  checkSound(time_now);
  
  if(!isPresentSound && !isPresentPIR){
    isPresent = false;
  }
  
  //a seconda che ci siano persone o meno modifica i valori di riferimento
  changeReferenceValues(isPresent);

  //Controllo se è presente qualcuno o meno e cambio i valori di riferimento
  updateCoeffTermNoto(isPresent);

  //Misuro la temperatura e cambio i valori del fan e del LED
  temp = returnTemp();
  velVentola = updateVentola(temp, velVentola);
  pwmLED = updateLED(temp, pwmLED);
  
  //Il valore sarà la temp misurata
  String messageTemp = senMlEncode("temperature", temp, "Cel");
  mqtt.publish(my_base_topic + String("/temperature"), messageTemp);

  //Il valore nel caso di presenza e suono è il boolean (true -> presenza, false -> no presenza)
  String messagePresence = senMlEncode("presence", isPresent, "null");
  mqtt.publish(my_base_topic + String("/presence"), messagePresence);

  String messageSound = senMlEncode("sound", isPresentSound, "null");
  mqtt.publish(my_base_topic + String("/sound"), messageSound);
  
  delay(5000);
}

int curl_function(String text) {
  Process p;
  p.begin("curl");
  p.addParameter("-H");
  p.addParameter("Content-Type: application/json");
  p.addParameter("-X");
  p.addParameter("POST");
  p.addParameter("-d");
  p.addParameter(text);
  p.addParameter("http://192.168.43.184:8080/addDevice"); //Per registrarsi al Catalog
  p.run();
  return(p.exitValue());
}

String senMlEncode(String type, float val, String unit) {
  if(type == "temperature") {
    doc_snd.clear();
    doc_snd["bn"] = "Yun";
    doc_snd["e"][0]["n"] = "temperature";
    doc_snd["e"][0]["t"] = millis();
    doc_snd["e"][0]["v"]= val;
    doc_snd["e"][0]["u"] = unit;
    
    String output;
    serializeJson(doc_snd, output);
    return output;
  }
  else if(type == "presence") {
    doc_snd.clear();
    doc_snd["bn"] = "Yun";
    doc_snd["e"][0]["n"] = "presence";
    doc_snd["e"][0]["t"] = millis();
    doc_snd["e"][0]["v"]= val;
    doc_snd["e"][0]["u"] = unit; //In questo caso null
    
    String output;
    serializeJson(doc_snd, output);
    return output;
  }
  else if(type == "sound") {
    doc_snd.clear();
    doc_snd["bn"] = "Yun";
    doc_snd["e"][0]["n"] = "sound";
    doc_snd["e"][0]["t"] = millis();
    doc_snd["e"][0]["v"]= val;
    doc_snd["e"][0]["u"] = unit; //Anche qui sarà null
    
    String output;
    serializeJson(doc_snd, output);
    return output;
  }
}

float returnTemp(void){
  int a = analogRead(TEMP_PIN);
  float R = ((vcc/a)-1.0)*R0;
  float logR = log(R/R0);
  float tKel = 1.0/((logR/B)+(1.0/T0));
  float tCel = tKel - 273.15;
  
  return tCel;
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

void verifyPresence_PIR(){
  //alla chiamata dell'interrupt rising cambia il valore del booleano
  isPresentPIR = true;
  isPresent = true;
  start_time_PIR = millis();
}

void verifyPresence_sound(){
  start_time_sound = millis();
  countSounds++; 
}

void setReferenceValues(const String& topic, const String& subtopic, const String& message) {
  DeserializationError err = deserializeJson(doc_rec_references, message);
  volatile float val;
  
  if(err){
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  if(doc_rec_references["e"][0]["n"] == "ACm") {
    val = doc_rec_references["e"][0]["v"];
    AC_min = val;
  }
  else if(doc_rec_references["e"][0]["n"] == "ACM") {
    val = doc_rec_references["e"][0]["v"];
    AC_max = val;
  }
  else if(doc_rec_references["e"][0]["n"] == "HTm") {
    val = doc_rec_references["e"][0]["v"];
    HT_min = val;
  }
  else if(doc_rec_references["e"][0]["n"] == "HTM") {
    val = doc_rec_references["e"][0]["v"];
    HT_max = val;
  }
  else {
    Serial.println("Attenzione, comando non valido");
  }
}

void setLcdView(const String& topic, const String& subtopic, const String& message) {
  DeserializationError err = deserializeJson(doc_rec_lcd, message);
  String displayMessage;
  
  if(err) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  if(doc_rec_lcd["device_id"] == "Yun") {
    displayMessage = (const char*)doc_rec_lcd["msg"];
    
    //Messaggio ricevuto mostrato sullo schermo LCD
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(displayMessage);
  }
  else {
    Serial.println("Attenzione, dispositivo non valido");
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

void updateCoeffTermNoto(boolean isPresent){
  coeffVentola = getCoeffVentola(isPresent);
  termNotoVentola = getTermNotoVentola(isPresent);
  coeffLED = getCoeffLED(isPresent);
  termNotoLED = getTermNotoLED(isPresent);
}

float updateVentola(float temp, float velVentola){
  velVentola = coeffVentola*temp + termNotoVentola;
  velVentola = checkVelVentola(velVentola);
  
  analogWrite(FAN_PIN, (int)velVentola);
  return velVentola;
}

float updateLED(float temp, int pwmLED){
  //modifica intensità LED
  pwmLED = returnLedPWM(temp);
  //si tratta tuttavia di una differenza di luminosità apprezzabile solo nell'oscurità
  analogWrite(LED_PIN, pwmLED);
  return pwmLED;
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

float getCoeffVentola(boolean isPresent){
  //Imposto come ordinata minima 40 poichè al di sotto di questo valore la ventola non parte
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
