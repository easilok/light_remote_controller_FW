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

typedef struct  {
  bool previousState;
  bool pendingChange;
  bool enable;
} BUTTON_DATA_t;

BUTTON_DATA_t buttonsData[BUTTON_COUNT];

#ifdef IO_EXPANDER_INTERRUPT_ENABLE
// Definition of the interrupt function
void ICACHE_RAM_ATTR  keyPressedOnPCF8574(){
#ifdef DEBUG_SERIAL_PRINT
  /* Serial.println("keyPressedOnPCF8574"); */
#endif
  keyPressed = true;
}
#endif

void pcf8574_init (void) {

  memset(&buttonsData, 0, sizeof(BUTTON_DATA_t) * BUTTON_COUNT);
  buttonsData[0].enable = true;
  buttonsData[1].enable = true;
  buttonsData[2].enable = false;

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

void pcf8574_process_state(uint8_t pin_state, uint8_t index) {
  if (index >= BUTTON_COUNT) {
    return;
  }

  if (buttonsData[index].previousState != pin_state) {
    if ((pin_state == BUTTON_PRESSED_STATE) && buttonsData[index].enable){
    /* if (pin_state == BUTTON_PRESSED_STATE) { */
      buttonsData[index].pendingChange = true; 
    }
    buttonsData[index].previousState = pin_state;
  }
}

bool pcf8574_get_pending_state(uint8_t index) {
  if (index >= BUTTON_COUNT) {
    return false;
  }

  if (!buttonsData[index].enable) {
    return false;
  }

  return buttonsData[index].pendingChange;
}

void pcf8574_clear_pending_state(uint8_t index) {
  if (index >= BUTTON_COUNT) {
    return;
  }

  buttonsData[index].pendingChange = false;
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
    pcf8574_process_state(val.p0, 0);
    pcf8574_process_state(val.p1, 1);
    pcf8574_process_state(val.p2, 2);
    pcf8574_process_state(val.p3, 3);
    pcf8574_process_state(val.p4, 4);
    pcf8574_process_state(val.p5, 5);
    pcf8574_process_state(val.p6, 6);
    pcf8574_process_state(val.p7, 7);
#ifdef DEBUG_SERIAL_PRINT
    for (int i = 0; i < BUTTON_COUNT; i++) {
      Serial.print("KEY");
      Serial.print(i);
      if (buttonsData[i].previousState == BUTTON_PRESSED_STATE) {
        Serial.print(" PRESSED");
      } else {
        Serial.print(" UNPRESSED");
      }
      Serial.print("; Pending Change: ");
      if (buttonsData[i].pendingChange) {
        Serial.print("Yes");
      } else {
        Serial.print("No");
      }
      Serial.println("");
    }
#endif
  }
}
