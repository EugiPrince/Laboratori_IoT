#include <Bridge.h>
#include <Process.h>
#include <ArduinoJson.h>

//inizializzo variabili per termistore
const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15; //25 gradi centigradi, quindi 273.15 + 25;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);

int curl_function(String text);

void setup() {
  // put your setup code here, to run once:
  pinMode(TEMP_PIN, INPUT);
  Bridge.begin();
}

void loop() {
  // put your main code here, to run repeatedly
  volatile float temp = returnTemp();
  Serial.println(temp);
  String txt = senMlEncode(temp, "Celsius");
  volatile int ret;
  ret = curl_function(txt);
  Serial.println(ret);
  
  delay(4000);
}

int curl_function(String text){
Process p;
 p.begin("curl");
 p.addParameter("-H");
 p.addParameter("Content-Type: application/json");
 p.addParameter("-X");
 p.addParameter("POST");
 p.addParameter("-d");
 p.addParameter(text);
 p.addParameter("http://192.168.43.184:8080/refreshTemperature");
 p.run();
 return(p.exitValue());
}

String senMlEncode(float val, String unit) {
    doc_snd.clear();
    doc_snd["bn"] = "ArduinoYun";
    doc_snd["e"][0]["n"] = "temp";
    doc_snd["e"][0]["t"] = millis();
    doc_snd["e"][0]["v"]= val;
    doc_snd["e"][0]["u"] = unit;
  
    String output;
    serializeJson(doc_snd, output);
    return output;
}

float returnTemp(void){
  int a = analogRead(TEMP_PIN);
  float R = ((vcc/a)-1.0)*R0;
  float logR = log(R/R0);
  float tKel = 1.0/((logR/B)+(1.0/T0));
  float tCel = tKel - 273.15;
   
   return tCel;
}
