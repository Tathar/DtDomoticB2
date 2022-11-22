#ifndef DT_ha
#define DT_ha

#include <avr/wdt.h> //watchdog

#include <config.h>
#include <DT_mqtt.h>

// #ifdef MQTT

const char TOPIC_STATUS[] PROGMEM = "homeassistant/sensor/" BOARD_IDENTIFIER "/status/config";
const char TOPIC_RELAY[] PROGMEM = "homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config";

bool homeassistant(bool start);

enum __attribute__((__packed__)) type
{
    ha_null,
    ha_flash_cstr,
    ha_cstr,
    ha_str,
    ha_float,
    ha_int32_t,
    ha_flash_cstr_tsprintf,
    ha_cstr_tsprintf,
    ha_str_tsprintf,
    ha_float_tsprintf,
    ha_int32_t_tsprintf,
    ha_flash_cstr_tpsprintf,
    ha_cstr_tpsprintf,
};

class MQTT_data
{
public:
    ~MQTT_data();
    void store_P(const __FlashStringHelper *Topic, const char *Payload);
    void store(const __FlashStringHelper *Topic, const __FlashStringHelper *Payload);
    // void store(const __FlashStringHelper *Topic, char const *Payload);
    void store(const __FlashStringHelper *Topic, const String Payload);
    void store(const __FlashStringHelper *Topic, const float Payload);
    void store(const __FlashStringHelper *Topic, const int8_t Payload);
    void store(const __FlashStringHelper *Topic, const uint8_t Payload);
    void store(const __FlashStringHelper *Topic, const int16_t Payload);
    void store(const __FlashStringHelper *Topic, const uint16_t Payload);
    void store(const __FlashStringHelper *Topic, const uint32_t Payload);

    void store_P(const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const __FlashStringHelper *Payload);
    // void store(const __FlashStringHelper *Topic, uint8_t num_t, char const *Payload);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const String Payload);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const float Payload);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const int8_t Payload);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const uint8_t Payload);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const uint16_t Payload);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const uint32_t Payload);

    void store_P(const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload, uint8_t num_p);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const __FlashStringHelper *Payload, uint8_t num_p);
    void store(const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload, uint8_t num_p);

    void get(char *topic, int topic_len, char *payload, int payload_len);

    void print();

private:
    type _type;
    uint8_t _num_t;
    const __FlashStringHelper *_topic;
    uint8_t _num_p;
    String _str;
    union
    {
        const __FlashStringHelper *_fcstr;
        char *_cstr;
        float _float;
        int32_t _int;
    };
};

class MQTT_recv_msg
{
public:
    MQTT_recv_msg()
    {
        _topic = nullptr;
        _payload = nullptr;
        _length = 0;
    };

    MQTT_recv_msg(char *topic, char *payload, int length)
    {
        _topic = strdup(topic);
        _payload = (char *)malloc(sizeof(char) * length);
        memcpy(_payload, payload, length);
        _length = length;
    };

    void clean()
    {
        if (_topic != nullptr)
            free(_topic);

        if (_payload != nullptr)
            free(_payload);
    };

    char *_topic;
    char *_payload;
    int _length;
};

// #endif // MQTT
#endif // DT_HA
