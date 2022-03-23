#include "pinDeclarations.h"
#include <DHT.h>
#include <DHT_U.h>

enum Control {automatic, manual};
enum Function {cooling, heating};
enum Power {standby, eco, boost};
enum Bypass {open, closed};

struct State {
  Function function;
  Power power;
  Bypass bypass;
};

struct Measurement
{
  float temp;
  float hum;

  void measure(DHT* sensor) {
    temp = sensor->readTemperature();
    hum = sensor->readHumidity();
  }
};

struct Measurements
{
  Measurement indoorIn;
  Measurement indoorOut;
  Measurement outsiteIn;
  Measurement outsiteOut;
};


class Ventilator {
  public:
    State state{Function::heating, Power::standby, Bypass::closed};
    Measurements measurements;
    Control control = Control::automatic;
    

    DHT indoorInSensor = DHT(INDOOR_IN, DHT22);
    DHT indoorOutSensor = DHT(INDOOR_OUT, DHT22);
    DHT outsiteInSensor = DHT(OUTDOOR_IN, DHT22);
    DHT outsiteOutSensor = DHT(OUTDOOR_OUT, DHT22);

    void start() {
      indoorInSensor.begin();
      indoorOutSensor.begin();
      outsiteInSensor.begin();
      outsiteOutSensor.begin();


      pinMode(BYPASS, OUTPUT);
      pinMode(VENTILATOR_IN_SMALL, OUTPUT);
      pinMode(VENTILATOR_OUT_SMALL, OUTPUT);
      pinMode(VENTILATOR_IN_BIG, OUTPUT);
      pinMode(VENTILATOR_OUT_BIG, OUTPUT);

      digitalWrite(VENTILATOR_IN_SMALL, HIGH);
      digitalWrite(VENTILATOR_OUT_SMALL, HIGH);
      digitalWrite(VENTILATOR_IN_BIG, LOW);
      digitalWrite(VENTILATOR_OUT_BIG, LOW);
    }

    void manage() {
      Measure();

      if (control == Control::automatic) {
        state.function = (measurements.indoorIn.temp > 21) ? Function::cooling : Function::heating;
        if (state.function == Function::heating ) {
          if (measurements.indoorIn.temp >= measurements.outsiteIn.temp) {
            state.bypass = Bypass::closed;
            state.power  = (measurements.indoorIn.hum > 80) ? Power::boost : Power::eco;
          }
          else { // outside warmer than inside.
            state.bypass = Bypass::open;
            state.power  = Power::boost;
          }

        } else { // cooling
          if (measurements.indoorIn.temp <= measurements.outsiteIn.temp) {
            state.bypass = Bypass::closed;
            state.power  = (measurements.indoorIn.hum > 80) ? Power::boost : Power::eco;
          }
          else { // outside colder than inside.
            state.bypass = Bypass::open;
            state.power  = Power::boost;
          }
        }


      } else {
        //control manually
      }
    }

    void Measure() {
      measurements.indoorIn = {12.0, 88.0};    //measure(&indoorInSensor);
      measurements.indoorOut = {9.9, 50.0};    //measure(&indoorOutSensor);
      measurements.outsiteIn = {1.5, 50};   //measure(&outsiteInSensor);
      measurements.outsiteOut = {9.0, 50};  // measure(&outsiteOutSensor);

//      measurements.indoorIn.measure(&indoorInSensor);
//      measurements.indoorOut.measure(&indoorOutSensor);
//      measurements.outsiteIn.measure(&outsiteInSensor);
//      measurements.outsiteOut.measure(&outsiteOutSensor);
    }
};
