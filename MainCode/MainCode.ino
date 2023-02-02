#include <ArduinoJson.h>
#include "ventilator.h"

Ventilator ventilator;

String fromSerial = "{\"empty\":\"empty\"}";
State stateFromSerial;

void setup() {
  Serial.begin(115200);
  ventilator.start();
}

void loop() {
  delay(500 );
  ventilator.manage(&stateFromSerial);
  sentToSerial();
  receiveFromSerial();

}

void sentToSerial() {
  StaticJsonDocument<200> doc;

  doc ["state"]["function"] = toString(ventilator.state.function);
  doc ["state"]["power"] = toString(ventilator.state.power);
  doc ["state"]["bypass"] = toString(ventilator.state.bypass);
  doc ["state"]["control"] = toString(ventilator.state.control);

  doc ["measurements"]["indoorIn"]["t"]   = ventilator.measurements.indoorIn.temp;
  doc ["measurements"]["indoorIn"]["h"]    = ventilator.measurements.indoorIn.hum;

  doc ["measurements"]["indoorOut"]["t"]  = ventilator.measurements.indoorOut.temp;
  doc ["measurements"]["indoorOut"]["h"]   = ventilator.measurements.indoorOut.hum;

  doc ["measurements"]["outdoorIn"]["t"]  = ventilator.measurements.outdoorIn.temp;
  doc ["measurements"]["outdoorIn"]["h"]   = ventilator.measurements.outdoorIn.hum;

  doc ["measurements"]["outdoorOut"]["t"] = ventilator.measurements.outdoorOut.temp;
  doc ["measurements"]["outdoorOut"]["h"]  = ventilator.measurements.outdoorOut.hum;

  CalculateEfficienty(&doc);

  String serialized;
  serializeJson(doc, serialized);
  Serial.println(serialized);
}

void receiveFromSerial() {
  String temp = Serial.readString();
  if (temp.length() > 20) {
    fromSerial = temp;

    StaticJsonDocument<200> doc;
    deserializeJson(doc, temp);

    stateFromSerial.function = functionFromString(doc["function"]);
    stateFromSerial.power = powerFromString(doc["power"]);
    stateFromSerial.bypass = bypassFromString(doc["bypass"]);
    stateFromSerial.control = controlFromString(doc["control"]);
  }
}

void AddRecord(char* name, DHT* sensor, JsonDocument* doc) {
  JsonObject IndoorIn = doc->createNestedObject(name);
  IndoorIn["t"] = sensor->readTemperature();
  IndoorIn["h"]  = sensor->readHumidity();
}

void CalculateEfficienty(JsonDocument* doc)
{
  float delta = ventilator.measurements.indoorIn.temp - ventilator.measurements.outdoorIn.temp;
  (*doc) ["delta"]                 = delta;
  JsonObject efficiency = doc->createNestedObject("efficiency");
  efficiency["indoor"]  = 100 - (100 / delta * (ventilator.measurements.indoorIn.temp  - ventilator.measurements.indoorOut.temp));
  efficiency["outdoor"] = 100 - (100 / delta * (ventilator.measurements.outdoorOut.temp - ventilator.measurements.outdoorIn.temp));
}


String toString(Power power) {
  switch (power) {
    case Power::standby:
      return "standby";
      break;
    case Power::eco:
      return "eco";
      break;
    case Power::normal:
      return "normal";
      break;
    case Power::boost:
      return "boost";
      break;
    case Power::turbo:
      return "turbo";
      break;
    default:
      return "standby";
      break;
  }
}

String toString(Function function) {
  return function == Function::cooling ? "cooling" : "heating";
}

String toString(Bypass bypass) {
  return bypass == Bypass::open ? "open" : "closed";
}

String toString(Control control) {
  return control == Control::automatic ? "automatic" : "manual";
}


Power powerFromString(String power) {
  if (power == "standby") {
    return Power::standby;
  }
  if (power == "eco") {
    return Power::eco;
  }
  if (power == "normal") {
    return Power::normal;
  }
  if(power == "boost") {
    return Power::boost;
  }
  if (power == "turbo") {
    return Power::turbo;
  }
}

Function functionFromString(String function) {
  return function == "cooling" ? Function::cooling : Function::heating;
}

Bypass bypassFromString(String bypass) {
  return bypass == "open" ? Bypass::open : Bypass::closed;
}

Control controlFromString(String control) {
  return control == "automatic" ? Control::automatic : Control::manual;
}
