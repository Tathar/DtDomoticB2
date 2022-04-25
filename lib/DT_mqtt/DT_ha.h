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
    ha_str,
    ha_float,
    ha_int32_t,
    ha_flash_cstr_tsprintf,
    ha_cstr_tsprintf,
    ha_str_tsprintf,
    ha_float_tsprintf,
    ha_int32_t_tsprintf
};
struct MQTT_data
{
    MQTT_data_type _type;
    uint8_t _num;
    const char *_topic;
    union
    {
        const char *_cstr;
        // String _str;
        float _float;
        int16_t _int;
    };
};

void MQTT_data_store_P(MQTT_data &self, const char *Topic, const char *Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, const char *Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, const float Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, const uint16_t Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, const uint8_t Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, const uint32_t Payload);

void MQTT_data_get(MQTT_data &self, char *topic, int topic_len, char *payload, int payload_len);

void MQTT_data_store_P(MQTT_data &self, const char *Topic, uint8_t num, const char *Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, uint8_t num, const char *Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, uint8_t num, const float Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, uint8_t num, const uint8_t Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, uint8_t num, const uint16_t Payload);
void MQTT_data_store(MQTT_data &self, const char *Topic, uint8_t num, const uint32_t Payload);

void MQTT_data_debug(MQTT_data &self);

template<typename T>
void MQTT_data_store(MQTT_data &self, const char *Topic, const T Payload)
{
    self._type = ha_int32_t;
    self._topic = Topic;
    self._int = Payload;
};

template<typename T>
void MQTT_data_store(MQTT_data &self, const char *Topic, uint8_t num, const T Payload)
{
    self._type = ha_int32_t_tsprintf;
    self._num = num;
    self._topic = Topic;
    self._int = Payload;
};


// #endif // MQTT
#endif // DT_HA
