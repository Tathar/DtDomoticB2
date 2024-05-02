#pragma once

#include <DT_eeprom.h>
#include <DT_mqtt.h>
#include <DT_ha.h>
// #include <DT_buffer.h>
#include <CircularBuffer.h>

inline void DT_mqtt_send(MQTT_data* data)
{
    debug(F(AT));
    if (mem_config.MQTT_online && !send_buffer.isFull())
    {
        send_buffer.unshift(data);
    }
    // else
    // {
    //     Serial.println(F("send buffer is full"));
    // }
    memory(false);
    debug(F(AT));
}

template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, T payload)
{
    memory(false);
    debug(F(AT));
    if (mem_config.MQTT_online && !send_buffer.isFull())
    {

        MQTT_data *send = new MQTT_data();
        send->store(topic, payload);
        send_buffer.unshift(send);
    }
    // else
    // {
    //     Serial.println(F("send buffer is full"));
    // }
    memory(false);
    debug(F(AT));
}

template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, uint8_t num, T payload)
{
    memory(false);
    debug(F(AT));
    if (mem_config.MQTT_online && !send_buffer.isFull())
    {
        MQTT_data *send = new MQTT_data();
        send->store(topic, num, payload);
        send_buffer.unshift(send);
    }
    // else
    // {
    //     Serial.println(F("send buffer is full"));
    // }
    memory(false);
    debug(F(AT));
}


template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, uint8_t num_t, T payload , uint8_t num_p)
{
    memory(false);
    debug(F(AT));
    if (mem_config.MQTT_online && !send_buffer.isFull())
    {
        MQTT_data *send = new MQTT_data();
        send->store(topic, num_t, payload, num_p);
        send_buffer.unshift(send);
    }
    // else
    // {
    //     Serial.println(F("send buffer is full"));
    // }
    memory(false);
    debug(F(AT));
}
