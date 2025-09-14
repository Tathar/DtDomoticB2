#ifndef DT_MQTT
#define DT_MQTT

#include <SPI.h>
#include <Ethernet.h>
#include <CircularBuffer.h>
#include <MQTT.h>

struct MQTT_data;

#include <pinout.h>
#include <config.h>
void DT_mqtt_loop();
void DT_mqtt_init();
void init_ethernet();
void DT_mqtt_set_update_callback(bool (*mqtt_update)(MQTTClient &mqtt, bool start));
void DT_mqtt_set_subscribe_callback(bool (*mqtt_subscribe)(MQTTClient &mqtt, bool start));
void DT_mqtt_set_publish_callback(bool (*mqtt_publish)(bool start));
void DT_mqtt_set_receve_callback(void (*mqtt_receve)(MQTTClient *client, const char topic[], const char bytes[], const int length));
void DT_mqtt_set_connection_lost_callback(void (*callback_connection_lost)());
void DT_mqtt_update();

extern CircularBuffer<MQTT_data*, 5> send_buffer;
extern bool as_ethernet;

#endif