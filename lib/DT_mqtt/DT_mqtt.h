#ifndef DT_MQTT
#define DT_MQTT

#include <SPI.h>
#include <Ethernet.h>
// #define MQTT_MAX_PACKET_SIZE 512
#include <DT_buffer.h>
// #include <CircularBuffer.h>
#include <MQTT.h>

struct MQTT_data;

#include <pinout.h>
#include <config.h>
void DT_mqtt_loop();
void DT_mqtt_init();
void DT_mqtt_set_update_callback(bool (*mqtt_update)(MQTTClient &mqtt, bool start));
void DT_mqtt_set_subscribe_callback(bool (*mqtt_subscribe)(MQTTClient &mqtt, bool start));
void DT_mqtt_set_publish_callback(bool (*mqtt_publish)(bool start));
void DT_mqtt_set_receve_callback(void (*mqtt_receve)(MQTTClient *client, const char topic[], const char bytes[], const int length));
void DT_mqtt_update();




// bool DT_mqtt_send(const char *tag, const char *value);
// bool DT_mqtt_send(const char *tag, const int value);
// bool DT_mqtt_send(const char *tag, const unsigned int value);
// bool DT_mqtt_send(const char *tag, const uint32_t value);
// bool DT_mqtt_send(const char *tag, const float value);


extern DT_buffer<MQTT_data> send_buffer;
// extern CircularBuffer<MQTT_data,20> send_buffer;

#endif