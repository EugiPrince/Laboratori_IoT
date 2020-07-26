#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>

//inizializzo variabili per termistore
const int TEMP_PIN = A3;
const float B = 4275.0;
const long int R0 = 100000.0;
const float vcc = 1023.0;
const float T0 = 298.15; //25 gradi centigradi, quindi 273.15 + 25

BridgeServer server;
int LED_PIN = 6;
const int capacity = JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMP_PIN, INPUT);
  digitalWrite(LED_PIN, LOW);
  Bridge.begin();
  digitalWrite(LED_PIN, HIGH);
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  BridgeClient client = server.accept();
  if(client) {
    process(client);
    client.stop();
  }
  delay(1000);
}

void process(BridgeClient client) {
  String command = client.readStringUntil('/');
  command.trim();
  
  if(command == "led") {
    int val = client.parseInt();
    if(val == 0 || val == 1) {
      digitalWrite(LED_PIN, val);
      printResponse(client, 200, senMlEncode(F("led"), val, F("")));
    }
    else{
      printResponse(client, 400, "");
    }
    
  }
  else if (command == "temp"){
      volatile float tmp = returnTemp();
      printResponse(client, 200, senMlEncode(F("temp"), tmp, F("Cel")));
      
    }
    else{
      printResponse(client, 404, "");
    }
}

void printResponse(BridgeClient client, int code, String body) {
  client.println("Status: " + String(code));
  if(code == 200) {
    client.println(F("Content-type: application/json; charset=utf-8"));
    client.println();
    client.println(body);
  }
  else if(code == 404){
    client.println(body);
  }
  
}

String senMlEncode(String res, float val, String unit) {
  doc_snd.clear();
  doc_snd["bn"] = "Yun";

  if(unit != "") {
    doc_snd["e"][0]["n"] = "temp";
    doc_snd["e"][0]["t"] = millis();
    doc_snd["e"][0]["v"]= val;
    doc_snd["e"][0]["u"] = unit;
  }
  else {
    doc_snd["e"][0]["n"] = "led";
    doc_snd["e"][0]["t"] = millis();
    doc_snd["e"][0]["v"]= val;
    doc_snd["e"][0]["u"] = (char*)NULL;
  }

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
