/*
  Basic test for MatrixControl.
  
  Compare performance with LedControl.
*/

#include "MatrixControl.h"
#include "font8x8_basic.h"

#define numDevices 4

#if defined(__AVR__)
/*
 Now we need a MatrixControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have 4 MAX72XX.
 */
MatrixControl lc=MatrixControl(12,11,10,numDevices);

#elif defined(ESP8266)
/*
 Now we need a MatrixControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 GPIO13(D7) is connected to the DataIn 
 GPIO14(D5) is connected to the CLK 
 GPIO15(D8) is connected to the CS 
 We have 4 MAX72XX.
 */
MatrixControl lc=MatrixControl(13,14,15,numDevices);
#endif

void setup() {
  Serial.begin(115200);
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  for (int addr=0;addr<numDevices;addr++) {
    lc.shutdown(addr,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(addr,8);
    /* and clear the display */
    lc.clearDisplay(addr);
  }

  unsigned long startMillis;
  unsigned long endMillis;
  unsigned long diffMillis;

  startMillis = millis();
  lc.setLed(0, 0, 0, 1);
  lc.setLed(1, 0, 0, 1);
  lc.setLed(1, 0, 1, 1);
  lc.setLed(2, 0, 0, 1);
  lc.setLed(2, 0, 1, 1);
  lc.setLed(2, 0, 2, 1);
  lc.setLed(3, 0, 0, 1);
  lc.setLed(3, 0, 1, 1);
  lc.setLed(3, 0, 2, 1);
  lc.setLed(3, 0, 3, 1);
  endMillis = millis();
  diffMillis = endMillis - startMillis;
  Serial.print("diffMillis[true]=");
  Serial.print(diffMillis);
  Serial.println("[ms]");
  delay(1000);



  lc.clearAllDisplay();
  startMillis = millis();
  lc.setLed(0, 0, 0, 1, false);
  lc.setLed(1, 0, 0, 1, false);
  lc.setLed(1, 0, 1, 1, false);
  lc.setLed(2, 0, 0, 1, false);
  lc.setLed(2, 0, 1, 1, false);
  lc.setLed(2, 0, 2, 1, false);
  lc.setLed(3, 0, 0, 1, false);
  lc.setLed(3, 0, 1, 1, false);
  lc.setLed(3, 0, 2, 1, false);
  lc.setLed(3, 0, 3, 1, false);
  lc.pushRow(0, 0);
  lc.pushRow(1, 0);
  lc.pushRow(2, 0);
  lc.pushRow(3, 0);
  endMillis = millis();
  diffMillis = endMillis - startMillis;
  Serial.print("diffMillis[false]=");
  Serial.print(diffMillis);
  Serial.println("[ms]");
  delay(1000);


  lc.clearAllDisplay();
  startMillis = millis();
  for (int addr=0;addr<numDevices;addr++) {
    for(int row=0;row<8;row++) {
      for(int column=0;column<8;column++) {
        lc.setLed(addr, row, column, 1);
      }
    }
  }
  endMillis = millis();
  diffMillis = endMillis - startMillis;
  Serial.print("diffMillis[true]=");
  Serial.print(diffMillis);
  Serial.println("[ms]");
  delay(1000);

  lc.clearAllDisplay();
  startMillis = millis();
  for (int addr=0;addr<numDevices;addr++) {
    for(int row=0;row<8;row++) {
      for(int column=0;column<8;column++) {
        lc.setLed(addr, row, column, 1, false);
      }
    }
  }
  lc.pushAll();
  endMillis = millis();
  diffMillis = endMillis - startMillis;
  Serial.print("diffMillis[false]=");
  Serial.print(diffMillis);
  Serial.println("[ms]");
  delay(1000);

  lc.clearAllDisplay();
  startMillis = millis();
  lc.setColumn(0, 0, 0x01);
  lc.setColumn(1, 0, 0x03);
  lc.setColumn(2, 0, 0x07);
  lc.setColumn(3, 0, 0x0f);
  endMillis = millis();
  diffMillis = endMillis - startMillis;
  Serial.print("diffMillis[true]=");
  Serial.print(diffMillis);
  Serial.println("[ms]");
  delay(1000);
  
  lc.clearAllDisplay();
  startMillis = millis();
  lc.setColumn(0, 0, 0x01, false);
  lc.setColumn(1, 0, 0x03, false);
  lc.setColumn(2, 0, 0x07, false);
  lc.setColumn(3, 0, 0x0f, false);
  lc.pushAll();
  endMillis = millis();
  diffMillis = endMillis - startMillis;
  Serial.print("diffMillis[false]=");
  Serial.print(diffMillis);
  Serial.println("[ms]");
  delay(1000);

}

void loop() {

}
