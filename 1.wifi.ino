#include "ConnectionDetails.h"
#include <ESP8266WiFi.h>

WiFiClient espClient;

bool wifi_isConnected() {
#ifdef WIFI_ENABLE
  return WiFi.status() == WL_CONNECTED;
#else
  return true;
#endif
}

void wifi_reconnect() {
#ifndef WIFI_ENABLE
  // Do nothing
  return;
#else

  // connect to your local wi-fi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // check wi-fi is connected to wi-fi network
  while (!wifi_isConnected()) {
    delay(500);
    // turn led on
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    // turn led off
    digitalWrite(LED_BUILTIN, LOW);
#ifdef DEBUG_SERIAL_PRINT
    Serial.print(".");
#endif
  }
#ifdef DEBUG_SERIAL_PRINT
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());
#endif

  // TODO - This handling must be only in this file
  AsyncDelay_StartTimer(&ledToggleTimer);
#endif
}
