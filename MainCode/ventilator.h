#include <Servo.h>
#include <SoftPWM.h>
#include <DHT.h>
#include <DHT_U.h>

#include "pinDeclarations.h"


enum Control {automatic, manual};
enum Function {cooling, heating};
enum Power {standby, eco, normal, boost, turbo};
enum Bypass {open, closed};

struct State {
  Function function;
  Power power;
  Bypass bypass;
  Control control;
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
  Measurement outdoorIn;
  Measurement outdoorOut;
};

class Ventilator {
  public:
    State state{Function::heating, Power::eco, Bypass::closed, Control::automatic};
    Measurements measurements;
    Servo bypass;

    DHT indoorInSensor = DHT(INDOOR_IN, DHT22);
    DHT indoorOutSensor = DHT(INDOOR_OUT, DHT22);
    DHT outdoorInSensor = DHT(OUTDOOR_IN, DHT22);
    DHT outdoorOutSensor = DHT(OUTDOOR_OUT, DHT22);

    void start() {
      setPwmFreq();
      indoorInSensor.begin();
      indoorOutSensor.begin();
      outdoorInSensor.begin();
      outdoorOutSensor.begin();
      //bypass.attach(BYPASS);

      pinMode(BYPASS, OUTPUT);
      pinMode(VENTILATOR_IN_SMALL, OUTPUT);
      pinMode(VENTILATOR_OUT_SMALL, OUTPUT);
      pinMode(VENTILATOR_IN_BIG, OUTPUT);
      pinMode(VENTILATOR_OUT_BIG, OUTPUT);

      digitalWrite(VENTILATOR_IN_SMALL, LOW);
      digitalWrite(VENTILATOR_OUT_SMALL, LOW);
      digitalWrite(VENTILATOR_IN_BIG, LOW);
      digitalWrite(VENTILATOR_OUT_BIG, LOW);
      applySettings();
    }

    void manage(State* newState) {
      Measure();

      state.control = newState->control;

      if (state.control == Control::automatic) {
        // Check if all readings are correct, if not do run the machine automatically
        if (CheckMeasurementResults()) {
          state.power = Power::normal;
          state.bypass = Bypass::closed;
          state.function = Function::heating;
        }
        else
        {
          // Set first the function state depending on the incoming temperatures.
          state.function = (measurements.indoorIn.temp > 21 && measurements.outdoorIn.temp > 18) ? Function::cooling : Function::heating;

          // Set the correct configurations for the heating and cooling state.
          // If there is a high humidity, then switch to a higer power state.
          // Open the bypass valve if the outdoor temperature is closer to the desired temperature than the indoor temperature.
          if (state.function == Function::heating ) {
            if (measurements.indoorIn.temp >= measurements.outdoorIn.temp) {
              state.bypass = Bypass::closed;
              state.power  = (measurements.indoorIn.hum > 80) ? (measurements.indoorIn.hum > 90) ? Power::turbo : Power::boost : Power::normal;
            }
            else { // outside warmer than inside.
              state.bypass = Bypass::open;
              state.power  = Power::boost;
            }

          } else { // cooling
            if (measurements.indoorIn.temp <= measurements.outdoorIn.temp) {
              state.bypass = Bypass::closed;
              state.power  = (measurements.indoorIn.hum > 80) ? (measurements.indoorIn.hum > 90) ? Power::turbo : Power::boost : Power::normal;
            }
            else { // outside colder than inside.
              state.bypass = Bypass::open;
              state.power  = Power::boost;
            }
          }
        }
      } else {
        memcpy(&state, newState, sizeof(State));
      }
      applySettings();
    }

    void applySettings() {

      analogWrite(VENTILATOR_IN_SMALL, SmallFanSpeed());
      //delay(100);
      analogWrite(VENTILATOR_OUT_SMALL, SmallFanSpeed());
      //delay(200);
      analogWrite(VENTILATOR_IN_BIG, bigFanSpeed());
      //delay(300);
      analogWrite(VENTILATOR_OUT_BIG, bigFanSpeed());
      //bypass.write((state.bypass == Bypass::open) ? 180 : 0);
    }

    int bigFanSpeed() {
      if (state.power == Power::turbo) {
        return 255;
      }
      else if (state.power == Power::boost) {
        return 200;
      }
      else if (state.power == Power::normal) {
        return 0;
      }
      else if (state.power == Power::eco) {
        return 0;
      }
      else {
        return 0;
      }
    }

    int SmallFanSpeed() {
      if (state.power == Power::turbo) {
        return 255;
      }
      else if (state.power == Power::boost) {
        return 255;
      }
      else if (state.power == Power::normal) {
        return 255;
      }
      else if (state.power == Power::eco) {
        return 180;
      }
      else {
        return 0;
      }
    }

    void Measure() {
      //            measurements.indoorIn = {12.0, 88.0};    //measure(&indoorInSensor);
      //            measurements.indoorOut = {9.9, 50.0};    //measure(&indoorOutSensor);
      //            measurements.outdoorIn = {1.5, 50};   //measure(&outdoorInSensor);
      //            measurements.outdoorOut = {9.0, 50};  // measure(&outdoorOutSensor);

      //DisablePWM();
      measurements.indoorIn.measure(&indoorInSensor);
      measurements.indoorOut.measure(&indoorOutSensor);
      measurements.outdoorIn.measure(&outdoorInSensor);
      measurements.outdoorOut.measure(&outdoorOutSensor);
      applySettings();
    }

    bool CheckMeasurementResults() {
      return isnan(measurements.indoorIn.temp) &&
             isnan(measurements.indoorOut.temp) &&
             isnan(measurements.outdoorIn.temp) &&
             isnan(measurements.outdoorOut.temp) &&
             isnan(measurements.indoorIn.hum)  &&
             isnan(measurements.indoorOut.hum)  &&
             isnan(measurements.outdoorIn.hum) &&
             isnan(measurements.outdoorOut.hum);
    }

    //https://fdossena.com/?p=ArduinoFanControl/i.md
    void DisablePWM() {
      if (state.power == Power::eco) {
        digitalWrite(VENTILATOR_IN_SMALL, HIGH);
        digitalWrite(VENTILATOR_OUT_SMALL, HIGH);
        digitalWrite(VENTILATOR_IN_BIG, LOW);
        digitalWrite(VENTILATOR_OUT_BIG, LOW);
      }
      if (state.power == Power::boost) {
        digitalWrite(VENTILATOR_IN_SMALL, HIGH);
        digitalWrite(VENTILATOR_OUT_SMALL, HIGH);
        digitalWrite(VENTILATOR_IN_BIG, HIGH);
        digitalWrite(VENTILATOR_OUT_BIG, HIGH);
      }
      delay(20);
    }

    void setPwmFreq() {

      // Pins D5 and D6 - 61 Hz
      //TCCR0B = 0b00000101; // x1024
      //TCCR0A = 0b00000011; // fast pwm

      // Pins D5 and D6 - 244 Hz
      TCCR0B = 0b00000100; // x256
      TCCR0A = 0b00000011; // fast pwm

      // Pins D5 and D6 - 31.4 kHz
      //TCCR0B = 0b00000001; // x1
      //TCCR0A = 0b00000001; // phase correct

      // Pins D5 and D6 - 30 Hz
      //TCCR0B = 0b00000101; // x1024
      //TCCR0A = 0b00000001; // phase correct

      // Pins D9 and D10 - 30 Hz
      //TCCR1A = 0b00000001; // 8bit
      //TCCR1B = 0b00000101; // x1024 phase correct

      // Pins D3 and D11 - 30 Hz
      //TCCR2B = 0b00000111; // x1024
      //TCCR2A = 0b00000001; // phase correct
    }
};
