#include <MQTTclient.h>
#include <ArduinoJson.h>
#include <Bridge.h>

const int LED_PIN = 7;
const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15; //25 gradi centigradi, quindi 273.15 + 25;
String my_base_topic = "/tiot/9";

const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_rec(capacity);
DynamicJsonDocument doc_snd(capacity);

void setup() {
  // put your setup code here, to run once:
  pinMode(TEMP_PIN, INPUT);
  Bridge.begin();
  mqtt.begin("test.mosquitto.org", 1883);
  //mqtt.subscribe(my_base_topic + String("/led"), setLedValue); -> In questo caso non serve
  
}

void loop() {
  // put your main code here, to run repeatedly:
  volatile float temp = returnTemp();
  Serial.println(temp);

  mqtt.monitor();

  String message = senMlEncode("temperature", temp, "Cel");
  mqtt.publish("/tiot/9/temperature", message);

  delay(1000);
}

void setLedValue(const String& topic, const String& subtopic, const String& message){

  DeserializationError err = deserializeJson(doc_rec, message);
  volatile int val;
  
  if(err){
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  if(doc_rec["e"][0]["n"] == "led"){
    val = doc_rec["e"][0]["v"];

    if(val == 1 || val == 0){
      digitalWrite(LED_PIN, val);
    }
    else{
      Serial.println("Errore di comando, valore non accettato");
    }
  }

}

String senMlEncode(String type, float val, String unit) {
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

float returnTemp(void){
 int a = analogRead(TEMP_PIN);
 float R = ((vcc/a)-1.0)*R0;
 float logR = log(R/R0);
 float tKel = 1.0/((logR/B)+(1.0/T0));
 float tCel = tKel - 273.15;
   
 return tCel;
}
