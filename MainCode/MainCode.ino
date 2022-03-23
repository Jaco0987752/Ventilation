#include <ArduinoJson.h>
#include "ventilator.h"

Ventilator ventilator;

void setup() {
  Serial.begin(115200);
  ventilator.start();
}

void loop() {

  delay(2000);
  ventilator.manage();
  StaticJsonDocument<200> doc;

  doc ["state"]["function"] = toString(ventilator.state.function);
  doc ["state"]["power"] = toString(ventilator.state.power);
  doc ["state"]["bypass"] = toString(ventilator.state.bypass);

  doc ["measurements"]["indoorIn"]["temp"]   = ventilator.measurements.indoorIn.temp;
  doc ["measurements"]["indoorIn"]["hum"]    = ventilator.measurements.indoorIn.hum;

  doc ["measurements"]["indoorOut"]["temp"]  = ventilator.measurements.indoorOut.temp;
  doc ["measurements"]["indoorOut"]["hum"]   = ventilator.measurements.indoorOut.hum;

  doc ["measurements"]["outsiteIn"]["temp"]  = ventilator.measurements.outsiteIn.temp;
  doc ["measurements"]["outsiteIn"]["hum"]   = ventilator.measurements.outsiteIn.hum;

  doc ["measurements"]["outsiteOut"]["temp"] = ventilator.measurements.outsiteOut.temp;
  doc ["measurements"]["outsiteOut"]["hum"]  = ventilator.measurements.outsiteOut.hum;

  CalculateEfficienty(&doc);

  String serialized;
  serializeJson(doc, serialized);
  Serial.println(serialized);
}

void AddRecord(char* name, DHT* sensor, JsonDocument* doc) {
  JsonObject IndoorIn = doc->createNestedObject(name);
  IndoorIn["temp"] = sensor->readTemperature();
  IndoorIn["hum"]  = sensor->readHumidity();
}

void CalculateEfficienty(JsonDocument* doc)
{
  (*doc) ["Delta"]            = (int)(*doc)["IndoorIn"]["temp"] - (int)(*doc)["outsiteIn"]["temp"];
  (*doc) ["Efficenty"]["Out"] = 100 - (100 / (int)(*doc) ["Delta"] * ((int)(*doc)["IndoorIn"]["temp"]  - (int)(*doc)["IndoorOut"]["temp"]));
  (*doc) ["Efficenty"]["In"]  = 100 - (100 / (int)(*doc) ["Delta"] * ((int)(*doc)["outsiteIn"]["temp"] - (int)(*doc)["outsiteOut"]["temp"]));
}

String toString(Power power) {
  return (power == Power::standby) ? "standby" : (power == Power::eco) ? "eco" : "boost";
}

String toString(Function function) {
  return function == 0 ? "cooling" : "heating";
}

String toString(Bypass bypass) {
  return bypass == 0 ? "open" : "closed";
}

String toString(Control control) {
  return control == 0 ? "automatic" : "manuel";
}
