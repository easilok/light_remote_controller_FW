/* #include <Wire.h> */
#include "BuildFlags.h"
#include "Defines.h"

unsigned long ledToggleTimer = 0;
int ledState = LOW;

void setup() {
  Serial.begin(115200);
  delay(1000);

#ifdef DEBUG_SERIAL_PRINT
  Serial.println("Connecting to ");
  Serial.println(ssid);
#endif

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  /* button_init(); */

  pcf8574_init();

#ifdef WIFI_ENABLE
  wifi_reconnect();
#endif

  randomSeed(micros());

#ifdef MQTT_ENABLE
  mqtt_init();
#endif

  AsyncDelay_StartTimer(&ledToggleTimer);
}

void loop() {
#ifdef WIFI_ENABLE
  if (!wifi_isConnected()) {
    wifi_reconnect();
  }
#endif

  /* MQTT */
#ifdef MQTT_ENABLE
  if (!mqtt_isConnected()) {
    mqtt_reconnect();
  }
  mqtt_manage();
#endif

  /* LED Signal Running */
  if (!mqtt_isConnected()) {
    if (AsyncDelay_HasMillisElapsed(ledToggleTimer, MQTT_CONNECT_LED)) {
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }
      AsyncDelay_StartTimer(&ledToggleTimer);
    }
  } else if ((wifi_isConnected()) && (mqtt_isConnected())) {
    ledState = LOW;
    /* button_read_command(); */
  }
  pcf8574_manage();
  digitalWrite(LED_BUILTIN, ledState);
}

