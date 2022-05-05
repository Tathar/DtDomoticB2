#pragma once

#include <DT_eeprom.h>
#include <DT_mqtt.h>
#include <DT_ha.h>
#include <DT_buffer.h>
// #include <CircularBuffer.h>

inline void DT_mqtt_send(MQTT_data data)
{
    if (mem_config.MQTT_online)
    {
        send_buffer.push(data);
    }
}

template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, T payload)
{
    memory(false);
    // debug(F(AT));
    MQTT_data send;
    if (mem_config.MQTT_online)
    {
        send.store(topic, payload);
        send_buffer.push(send);
    }
    memory(false);
}

template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, uint8_t num, T payload)
{
    memory(false);
    // debug(F(AT));
    if (mem_config.MQTT_online)
    {
        MQTT_data send;
        send.store(topic, num, payload);
        send_buffer.push(send);
    }
    memory(false);
}
