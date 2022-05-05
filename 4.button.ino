#include <Wire.h>

#define DEBOUNCE_TIMEOUT 400
#define BUTTON_COMMAND 14

enum BUTTON_READ_FSM {
  BTN_READ_INIT,
  BTN_READ_WAIT_PRESS,
  BTN_READ_DEBOUNCE_PRESS,
  BTN_READ_PRESSED,
  BTN_READ_WAIT_UNPRESS,
  BTN_READ_DEBOUNCE_UNPRESS,
};

enum BUTTON_READ_FSM buttonReadFSM = BTN_READ_INIT;
unsigned long debounceTimer = 0;

void button_init() {
  // initialize the pushbutton pin as an input
  pinMode(BUTTON_COMMAND, INPUT);
}

void button_read_command() {

  // state machine
  // 1 - wait press
  // 2 - wait 1 second with button pressed
  // 3 - activate output
  // 4 - wait release
  // 5 - wait x seconds to release
  switch(buttonReadFSM){
    case BTN_READ_INIT:
#ifdef DEBUG_SERIAL_PRINT
      Serial.print("Button on startup is: ");
      Serial.println(digitalRead(BUTTON_COMMAND));
#endif
      buttonReadFSM = BTN_READ_WAIT_PRESS;
      break;
    case BTN_READ_WAIT_PRESS:
      if (!digitalRead(BUTTON_COMMAND)) {
        debounceTimer = millis();
        buttonReadFSM = BTN_READ_DEBOUNCE_PRESS;
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Button pressed");
#endif
      }
      break;
    case BTN_READ_DEBOUNCE_PRESS:
      if (digitalRead(BUTTON_COMMAND)) {
        buttonReadFSM = BTN_READ_WAIT_PRESS;
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Button released");
#endif
      }
      if (millis() - debounceTimer > DEBOUNCE_TIMEOUT) {
        buttonReadFSM = BTN_READ_PRESSED;
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Button pressed debounced");
#endif
      }
      break;
    case BTN_READ_PRESSED:
      if (!mqtt_publish(mqtt_office_btn, "TOGGLE")) {
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Failed to publish command actuation");
#endif
      }
      buttonReadFSM = BTN_READ_WAIT_UNPRESS;
      break;
    case BTN_READ_WAIT_UNPRESS:
      if (digitalRead(BUTTON_COMMAND)) {
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Button released");
#endif
        debounceTimer = millis();
        buttonReadFSM = BTN_READ_DEBOUNCE_UNPRESS;
      }
      break;
    case BTN_READ_DEBOUNCE_UNPRESS:
      if (!digitalRead(BUTTON_COMMAND)) {
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Button pressed");
#endif
        buttonReadFSM = BTN_READ_WAIT_UNPRESS;
      }
      if (millis() - debounceTimer > DEBOUNCE_TIMEOUT) {
        buttonReadFSM = BTN_READ_WAIT_PRESS;
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Button released debounced");
#endif
      }
      break;
    default:
      break;
  }
}

