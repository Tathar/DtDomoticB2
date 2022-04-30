#ifndef DT_ha
#define DT_ha

#include <avr/wdt.h> //watchdog

#include <config.h>
#include <DT_mqtt.h>

// #ifdef MQTT

const char TOPIC_STATUS[] PROGMEM = "homeassistant/sensor/" BOARD_IDENTIFIER "/status/config";
const char TOPIC_RELAY[] PROGMEM = "homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config";

bool homeassistant(bool start);

enum __attribute__((__packed__)) MQTT_data_type
{
    ha_flash_cstr,
    ha_cstr,
    ha_float,
    ha_int32_t,
    ha_flash_cstr_tsprintf,
    ha_cstr_tsprintf,
    ha_float_tsprintf,
    ha_int32_t_tsprintf,
    ha_flash_cstr_tpsprintf,
    ha_cstr_tpsprintf,
};
struct MQTT_data
{
    MQTT_data_type _type;
    uint8_t _num_t;
    const __FlashStringHelper *_topic;
    uint8_t _num_p;
    union
    {
        const __FlashStringHelper *_fcstr;
        char *_cstr;
        float _float;
        int32_t _int;
    };
};

void MQTT_data_store_P(MQTT_data &self, const __FlashStringHelper *Topic, const char *Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const __FlashStringHelper *Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, char const *Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const float Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const uint16_t Payload);
// void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const uint8_t Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const uint32_t Payload);

void MQTT_data_store_P(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const __FlashStringHelper *Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, char const *Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const float Payload);
// void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const uint8_t Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const uint16_t Payload);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const uint32_t Payload);

void MQTT_data_store_P(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload, uint8_t num_p);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const __FlashStringHelper *Payload, uint8_t num_p);
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload, uint8_t num_p);

void MQTT_data_get(MQTT_data &self, char *topic, int topic_len, char *payload, int payload_len);

void MQTT_data_debug(MQTT_data &self);

template <typename T>
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, T Payload)
{
    self._type = ha_int32_t;
    self._topic = Topic;
    self._int = Payload;
};

template <typename T>
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, T Payload)
{
    self._type = ha_int32_t_tsprintf;
    self._num_t = num_t;
    self._topic = Topic;
    self._int = Payload;
};

// #endif // MQTT
#endif // DT_HA
