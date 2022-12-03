#pragma once

#include <DT_eeprom.h>
#include <DT_mqtt.h>
#include <DT_ha.h>
// #include <DT_buffer.h>
#include <CircularBuffer.h>

inline void DT_mqtt_send(MQTT_data data)
{
    if (mem_config.MQTT_online)
    {
            if (!send_buffer.unshift(data))
            {
                Serial.println(F("buffer overwrite"));
            }
        
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
        if (!send_buffer.unshift(send))
        {
            Serial.println(F("buffer overwrite"));
        }
    }
    memory(false);
}

template <typename T>
inline void DT_mqtt_send(const __FlashStringHelper *topic, uint8_t num, T payload)
{
    memory(false);
    // debug(F(AT));
    MQTT_data send;
    if (mem_config.MQTT_online)
    {
        send.store(topic, num, payload);
        if (!send_buffer.unshift(send))
        {
            Serial.println(F("buffer overwrite"));
        }
    }
    memory(false);
}
