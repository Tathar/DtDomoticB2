#pragma once

#include <DT_eeprom.h>
#include <DT_mqtt.h>
#include <DT_ha.h>
#include <DT_buffer.h>

inline void DT_mqtt_send(MQTT_data data)
{
    if (mem_config.MQTT_online)
    {
        send_buffer.append(data);
    }
}

template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, T payload)
{
    memory(false);
    debug(F(AT));
    if (mem_config.MQTT_online)
    {
        MQTT_data send;
        MQTT_data_store(send, topic, payload);
        send_buffer.append(send);
    }
    memory(false);
}

template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, uint8_t num, T payload)
{
    memory(false);
    debug(F(AT));
    if (mem_config.MQTT_online)
    {
        MQTT_data send;
        MQTT_data_store(send, topic, num, payload);
        send_buffer.append(send);
    }
    memory(false);
}
