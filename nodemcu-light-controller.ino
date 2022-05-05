#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "ConnectionDetails.h"
#include "PCF8574.h"

#define MQTT_CONNECT_LED 1000
/* #define MQTT_ENABLE */
/* #define WIFI_ENABLE */

#define DEBUG_SERIAL_PRINT 1

unsigned long ledToggleTimer = 0;
int ledState = LOW;

/* MQTT */
#define MQTT_RECONNECT_STANDBY 5000

WiFiClient espClient;
#ifdef MQTT_ENABLE
PubSubClient client(espClient);
unsigned long mqttReconnectTimer = 0;
#endif

#define DEBOUNCE_TIMEOUT 400
#define BUTTON_COMMAND 14

/* PCF8574 I/O Expander */
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

void setup() {
  Serial.begin(115200);
  delay(1000);

#ifdef DEBUG_SERIAL_PRINT
  Serial.println("Connecting to ");
  Serial.println(ssid);
#endif

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  // initialize the pushbutton pin as an input
  pinMode(BUTTON_COMMAND, INPUT);

  pcf8574_init();

#ifdef WIFI_ENABLE
  wifi_reconnect();
#endif

  randomSeed(micros());

#ifdef MQTT_ENABLE
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
#endif
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
  client.loop();
#endif

  /* LED Signal Running */
  if (!mqtt_isConnected()) {
    if (millis() - ledToggleTimer > MQTT_CONNECT_LED) {
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }
      ledToggleTimer = millis();
    }
  } else if ((wifi_isConnected()) && (mqtt_isConnected())) {
    ledState = LOW;
    read_command_button();
  }
  pcf8574_readInputs();
  digitalWrite(LED_BUILTIN, ledState);
}

bool mqtt_isConnected() {
#ifdef MQTT_ENABLE
  return client.connected();
#else
  return true;
#endif
}

bool wifi_isConnected() {
#ifdef WIFI_ENABLE
  return WiFi.status() == WL_CONNECTED;
#else
  return true;
#endif
}

#ifdef WIFI_ENABLE
void wifi_reconnect() {

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

  ledToggleTimer = millis();
}
#endif

void callback(char *topic, byte *payload, unsigned int length) {
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

#ifdef MQTT_ENABLE
void mqtt_reconnect() {
  // If not connected to wifi -> don't attempt connection
  if (!wifi_isConnected())
    return;
  if (mqtt_isConnected())
    return;
  // Loop until we're reconnected
  if (millis() - mqttReconnectTimer > MQTT_RECONNECT_STANDBY) {
    mqttReconnectTimer = millis();
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
}
#endif

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

void read_command_button() {

  return;
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
#ifdef MQTT_ENABLE
      if (!client.publish(mqtt_office_btn, "TOGGLE")) {
#ifdef DEBUG_SERIAL_PRINT
        Serial.println("Failed to publish command actuation");
#endif
      }
#endif
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

void pcf8574_init (void) {

#ifdef IO_EXPANDER_INTERRUPT_ENABLE
  pinMode(IO_EXPANDER_INTERRUPTED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IO_EXPANDER_INTERRUPTED_PIN), keyPressedOnPCF8574, FALLING);
#endif

  for(int i = 0;i < IO_EXPANDER_PIN_COUNT;i++) {
    pcf8574.pinMode(i, INPUT);
  }
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

void pcf8574_readInputs(void) {
#ifdef IO_EXPANDER_INTERRUPT_ENABLE
  if (keyPressed) {
#else
  if (millis() - standByTimer > STANDBY_TIME_INTERVAL) {
#endif
    keyPressed = false;
    PCF8574::DigitalInput val = pcf8574.digitalReadAll();
    standByTimer = millis();
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

