#include <OneWireModified.h>

// Board-specific macros for direct GPIO
//#include "util/OneWire_direct_regtype.h"
//#include <util/crc16.h>

#include "util/OneWire_direct_gpio.h"

OneWire  sensor(2);  // on pin 2

IO_REG_TYPE bitmask;
 volatile IO_REG_TYPE *baseReg;

void setup(void) {
  Serial.begin(9600);
  bitmask = PIN_TO_BITMASK(2);
  baseReg = PIN_TO_BASEREG(2);

  delay(2000);
}

void loop() {
   IO_REG_TYPE mask IO_REG_MASK_ATTR = bitmask;
   volatile IO_REG_TYPE *reg IO_REG_BASE_ATTR = baseReg;
   int8_t data[5];
#define TEST
#ifdef TEST

    uint8_t buf[1000];

    DIRECT_WRITE_LOW(reg, mask);
    DIRECT_MODE_OUTPUT(reg, mask);  // drive output low
    delayMicroseconds(1100);
    DIRECT_MODE_INPUT(reg, mask); 
    
    for(int i = 0; i < 500; i++){
      buf[i] = DIRECT_READ(reg, mask);
      delayMicroseconds(1);
    }

    for(int i = 0; i < 500; i++){
      //Serial.print(i); Serial.print(" "); 
      Serial.println(buf[i]);
    }

    delay(2000);

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    delayMicroseconds(1100);
    pinMode(2, INPUT);
  int index = 70;
  if(buf[index] == LOW){
  index += 180;
  if (buf[index] == LOW){
      index += 50;
      if(buf[index] == HIGH){
        Serial.println("high"); 
      }else{
        Serial.println("low");
        }
    }
    else{
      Serial.println("fail to read bit");
    }
}
    
#else
    
if(sensor.reset()){
    Serial.println();

  sensor.read_bytes(data, 5);
  for(int i = 7; i > 0; i--){
    Serial.print(bitRead(data[0], i));
  }
  Serial.println();
  
  int hum = data[1];
  hum = hum << 8;
  hum += data[1];
  
  int temp = data[2];
  temp = temp << 8;
  temp += data[3];

  int crc = data[4];
  int crc_check = data[0] + data[1] + data[2] + data[3];
  if(crc == crc_check){
      Serial.println("crc_correct");
    }
    else{
      Serial.println("crc_incorrect");  
    }
  
  Serial.println();
  for(int i = 15; i > 0; i--){
    Serial.print(bitRead(hum, i));
  }
  Serial.println();
  for(int i = 15; i > 0; i--){
    Serial.print(bitRead(temp, i));
  }
  Serial.println();
    for(int i = 0; i <= 8; i++){
    Serial.print(bitRead(crc, i));
  }
  Serial.println();


  Serial.print("hum: "); Serial.println(hum);
  Serial.print("temp: "); Serial.println(temp);
  }
  else
  {
    Serial.println("no device presence");
   }

  #endif

  delay(200000);

}
