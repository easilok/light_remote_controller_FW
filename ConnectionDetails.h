#ifndef CONNECTION_DETAILS_H
#define CONNECTION_DETAILS_H

/* #include <stdio.h> */

#define WIFI_MAIN 0
#define WIFI_BEDROOM 1
/* #define WIFI_CONNECTION WIFI_MAIN */
#define WIFI_CONNECTION WIFI_BEDROOM
/* #define SENSOR_LOCATION LOCATION_LIVINGROOM */
/*Put your SSID & Password*/
#if WIFI_CONNECTION == WIFI_MAIN
const char *ssid = "Gandhi Vladimir";    // Enter SSID here
#else
const char *ssid = "Ramones";    // Enter SSID here
#endif
const char *password = "touro_06052404"; // Enter Password here


const char *mqtt_server = "192.168.1.240";
const char *mqtt_username = "mqttuser";
const char *mqtt_password = "nodemqttlogin";

#define MQTT_STATUS_TOPIC             "cmnd/remote/status"
#define MQTT_SWITCH_COMMAND_VALUE     "TOGGLE"
#define MQTT_OFFICE_SWITCH_TOPIC      "cmnd/tasmota_A505DF/Power1"
#define MQTT_HALL_SWITCH_TOPIC        "cmnd/tasmota_A768C1/Power1"
#define MQTT_KITCHEN_SWITCH_TOPIC     "cmnd/tasmota_C2ED53/Power1"
#define MQTT_WC_SWITCH_TOPIC          "cmnd/tasmota_C57050/Power1"
const char *mqtt_command_topic = MQTT_STATUS_TOPIC;
const char *mqtt_office_btn = MQTT_OFFICE_SWITCH_TOPIC;
const char *mqtt_entrance_btn = MQTT_HALL_SWITCH_TOPIC;

#endif

