#ifndef DT_MQTT
#define DT_MQTT

#include <SPI.h>
#include <Ethernet.h>
// #define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>

#include <pinout.h>
#include <config.h>

void DT_mqtt_loop();
void DT_mqtt_init();
void DT_mqtt_set_update_callback(void (*mqtt_update)(PubSubClient &mqtt));
void DT_mqtt_set_subscribe_callback(void (*mqtt_subscribe)(PubSubClient &mqtt));
void DT_mqtt_set_receve_callback(void (*mqtt_receve)(char *, uint8_t *, unsigned int));
void DT_mqtt_update();
bool DT_mqtt_send(const char *tag, const char *value);
bool DT_mqtt_send(const char *tag, const int value);
bool DT_mqtt_send(const char *tag, const unsigned int value);
bool DT_mqtt_send(const char *tag, const uint32_t value);
bool DT_mqtt_send(const char *tag, const float value);

#endif