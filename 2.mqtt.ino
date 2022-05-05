#include <PubSubClient.h>
#include "ConnectionDetails.h"


#define MQTT_RECONNECT_STANDBY 5000

#ifdef MQTT_ENABLE
PubSubClient client(espClient);
unsigned long mqttReconnectTimer = 0;
#endif

void mqtt_init() {
#ifdef MQTT_ENABLE
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);
#endif
}

void mqtt_manage() {
#ifdef MQTT_ENABLE
  client.loop();
#endif
}

bool mqtt_isConnected() {
#ifdef MQTT_ENABLE
  return client.connected();
#else
  return true;
#endif
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
#ifdef DEBUG_SERIAL_PRINT
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    // digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is
    // the voltage level but actually the LED is on; this is because it is
    // active low on the ESP-01)
  } else {
    // digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the
    // voltage HIGH
  }
}

void mqtt_reconnect() {
#ifndef MQTT_ENABLE
  return; // do nothing if not enabled
#else
  // If not connected to wifi -> don't attempt connection
  if (!wifi_isConnected())
    return;
  if (mqtt_isConnected())
    return;
  // Loop until we're reconnected
  if (AsyncDelay_HasMillisElapsed(mqttReconnectTimer, MQTT_RECONNECT_STANDBY)) {
    AsyncDelay_StartTimer(&mqttReconnectTimer);
#ifdef DEBUG_SERIAL_PRINT
    Serial.print("Attempting MQTT connection...");
#endif
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password,
                       mqtt_command_topic, 0, true, "Disconnected")) {
#ifdef DEBUG_SERIAL_PRINT
      Serial.println("connected");
#endif
      // Once connected, publish an announcement...
      client.publish(mqtt_command_topic, "Connected");
      // ... and resubscribe
      // client.subscribe("inTopic");
    } else {
#ifdef DEBUG_SERIAL_PRINT
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
#endif
    }
  }
#endif
}

bool mqtt_publish(const char *topic, const char *payload) {
#ifndef MQTT_ENABLE
  return true; // do nothing if not enabled
#else
  return client.publish(topic, payload);
#endif
}
