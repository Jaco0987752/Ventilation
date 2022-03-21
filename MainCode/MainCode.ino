#include <ArduinoJson.h>

#include <DHT.h>
#include <DHT_U.h>

enum Function {cooling, heating};
enum Power {eco, boost};
enum ByPass {open, closed};

struct State {
  Function function;
  Power power;
  ByPass bypass;
} state;



class Ventilator {
  public:
    State state{Function::heating, Power::eco, ByPass::closed};

    const int HOT_IN = 2;
    const int HOT_OUT = 3;
    const int COLD_IN = 4;
    const int COLD_OUT = 5;

    const int VALVE = 6;

    const int VENTILATOR_IN_SMALL = 8;
    const int VENTILATOR_OUT_SMALL = 9;
    const int VENTILATOR_IN_BIG = 11;
    const int VENTILATOR_OUT_BIG = 12;
    
    DHT HotInSensor = DHT(HOT_IN, DHT22);
    DHT HotOutSensor = DHT(HOT_OUT, DHT22);
    DHT ColdInSensor = DHT(COLD_IN, DHT22);
    DHT ColdOutSensor = DHT(COLD_OUT, DHT22);

    void start() {
      HotInSensor.begin();
      HotOutSensor.begin();
      ColdInSensor.begin();
      ColdOutSensor.begin();


      pinMode(VALVE, OUTPUT);
      pinMode(VENTILATOR_IN_SMALL, OUTPUT);
      pinMode(VENTILATOR_OUT_SMALL, OUTPUT);
      pinMode(VENTILATOR_IN_BIG, OUTPUT);
      pinMode(VENTILATOR_OUT_BIG, OUTPUT);

      digitalWrite(VENTILATOR_IN_SMALL, HIGH);
      digitalWrite(VENTILATOR_OUT_SMALL, HIGH);
      digitalWrite(VENTILATOR_IN_BIG, LOW);
      digitalWrite(VENTILATOR_OUT_BIG, LOW);
    }
};

Ventilator ventilator;

void setup() {
  Serial.begin(115200);
  ventilator.start();
}

void loop() {

  delay(2000);

  StaticJsonDocument<200> doc;
  
  doc ["state"]["function"] = toString(ventilator.state.function);
  doc ["state"]["power"] = toString(ventilator.state.power);
  doc ["state"]["bypass"] = toString(ventilator.state.bypass);
  
  AddRecord("HotIn", &ventilator.HotInSensor, &doc);
  AddRecord("HotOut", &ventilator.HotOutSensor, &doc);
  AddRecord("ColdIn", &ventilator.ColdInSensor, &doc);
  AddRecord("ColdOut", &ventilator.ColdOutSensor, &doc);
  CalculateEfficienty(&doc);

  String serialized;
  serializeJson(doc, serialized);
  Serial.println(serialized);
}

void AddRecord(char* name, DHT* sensor, JsonDocument* doc) {
  JsonObject HotIn = doc->createNestedObject(name);
  HotIn["temp"] = sensor->readTemperature();
  HotIn["hum"]  = sensor->readHumidity();
}

void CalculateEfficienty(JsonDocument* doc)
{
  (*doc) ["Delta"]            = (int)(*doc)["HotIn"]["temp"] - (int)(*doc)["ColdIn"]["temp"];
  (*doc) ["Efficenty"]["Out"] = 100 - (100 / (int)(*doc) ["Delta"] * ((int)(*doc)["HotIn"]["temp"]  - (int)(*doc)["HotOut"]["temp"]));
  (*doc) ["Efficenty"]["In"]  = 100 - (100 / (int)(*doc) ["Delta"] * ((int)(*doc)["ColdIn"]["temp"] - (int)(*doc)["ColdOut"]["temp"])); 
}

String toString(Power power){
  return (power == 0) ? "eco" : "boost";
  }

String toString(Function function){
  return function == 0 ? "cooling" : "heating";
}

String toString(ByPass bypass){
  return bypass == 0 ? "open" : "closed";
}
