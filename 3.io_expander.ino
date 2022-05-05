#include <Wire.h>
#include "PCF8574.h"

#define IO_EXPANDER_PIN_COUNT 8
#define IO_EXPANDER_INTERRUPTED_PIN 14
#define IO_EXPANDER_INTERRUPT_ENABLE
// Set i2c address
PCF8574 pcf8574(0x20);
#define STANDBY_TIME_INTERVAL 500
unsigned long standByTimer = 0;
// Function interrupt pending changes
bool keyPressed = false;

#ifdef IO_EXPANDER_INTERRUPT_ENABLE
// Definition of the interrupt function
void ICACHE_RAM_ATTR  keyPressedOnPCF8574(){
#ifdef DEBUG_SERIAL_PRINT
  Serial.println("keyPressedOnPCF8574");
#endif
  keyPressed = true;
}
#endif

void pcf8574_init (void) {

#ifdef IO_EXPANDER_INTERRUPT_ENABLE
  pinMode(IO_EXPANDER_INTERRUPTED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IO_EXPANDER_INTERRUPTED_PIN), keyPressedOnPCF8574, FALLING);
#endif

  for(int i = 0;i < IO_EXPANDER_PIN_COUNT;i++) {
    pcf8574.pinMode(i, INPUT);
  }
  AsyncDelay_StartTimer(&standByTimer);
#ifdef DEBUG_SERIAL_PRINT
  Serial.print("Init pcf8574...");
#endif
	if (pcf8574.begin()){
#ifdef DEBUG_SERIAL_PRINT
		Serial.println("OK");
#endif
	}else{
#ifdef DEBUG_SERIAL_PRINT
		Serial.println("KO");
#endif
	}
}

void pcf8574_manage(void) {
#ifdef IO_EXPANDER_INTERRUPT_ENABLE
  if (keyPressed) {
#else
  if (AsyncDelay_HasMillisElapsed(standByTimer, STANDBY_TIME_INTERVAL)) {
#endif
    keyPressed = false;
    PCF8574::DigitalInput val = pcf8574.digitalReadAll();
    AsyncDelay_StartTimer(&standByTimer);
#ifdef DEBUG_SERIAL_PRINT
    Serial.print("KEY0 ");
    if (val.p0==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
    Serial.print("KEY1 ");
    if (val.p1==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
    Serial.print("KEY2 ");
    if (val.p2==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
    Serial.print("KEY3 ");
    if (val.p3==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
    Serial.print("KEY4 ");
    if (val.p4==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
    Serial.print("KEY5 ");
    if (val.p5==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
    Serial.print("KEY6 ");
    if (val.p6==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
    Serial.print("KEY7 ");
    if (val.p7==HIGH) {
      Serial.println("PRESSED");
    } else {
      Serial.println("UNPRESSED");
    }
#endif
  }
}
