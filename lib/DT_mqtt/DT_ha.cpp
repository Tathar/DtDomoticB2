#include <DT_ha.h>
#include <DT_eeprom.h>

#include <DT_mqtt_send.h>

#include <ArduinoJson.h>

#include <boost/preprocessor/slot/counter.hpp>

#ifdef MQTT

bool homeassistant(bool start)
{
        uint32_t now = millis();
        static uint32_t time = 0;
        static uint8_t sequance = 253;
        static uint16_t max_len_topic = 0;
        static uint16_t max_len_payload = 0;
        static uint8_t num = 0;
        char topic[MAX_TOPIC];
        char payload[MAX_PAYLOAD];
        memory(true);

        // debug_wdt_reset();
        if (start)
        {
                // debug(AT);
                sequance = BOOST_PP_COUNTER;
                return false;
        }
        else if (sequance == BOOST_PP_COUNTER)
                Serial.println(F("homeassistant"));

        if (now - time >= 50)
        {
                time = now;
                switch (sequance)
                {
                case BOOST_PP_COUNTER:
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/status/config"), F("{\"unique_id\":\"" BOARD_IDENTIFIER "-status\",\"name\":\"status\",\"state_topic\":\"DtBoard/" BOARD_IDENTIFIER "/status\",\"device\":{\"identifiers\":\"" BOARD_IDENTIFIER "\",\"manufacturer\":\"" BOARD_MANUFACTURER "\",\"model\":\"" BOARD_MODEL "\",\"name\":\"" BOARD_NAME "\",\"sw_version\":\"" BOARD_SW_VERSION_PRINT "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        if (num < RELAY_NUM - RELAY_RESERVED)
                        {

                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/relay-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-relay-%02d\",\"name\":\"relay-%02d\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + RELAY_RESERVED, num + RELAY_RESERVED, num + RELAY_RESERVED);
                                DT_mqtt_send(F("homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config"), num + RELAY_RESERVED, payload);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#if DIMMER_LIGHT_NUM > 0
#include BOOST_PP_UPDATE_COUNTER() //declaration des dimmer
                case BOOST_PP_COUNTER:
                        if (num < DIMMER_LIGHT_NUM)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d\",\"name\":\"dimmer-%02d\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"bri_cmd_t\":\"~/bri_set\",\"bri_stat_t\":\"~/bri_state\",\"bri_scl\":\"255\",\"fx_list\":[\"NONE\",\"CANDLE\"],\"fx_stat_t\":\"~/fx_state\",\"fx_cmd_t\":\"~/fx_set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/light/" BOARD_IDENTIFIER "/dimmer-%02d/config"), num, payload);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Min dimmer count for 1%
                        if (num < DIMMER_LIGHT_NUM)
                        {
                                if (num < 12)
                                {
                                        snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MIN\",\"name\":\"dimmer-%02d-MIN\",\"stat_t\":\"~/min_state\",\"command_topic\":\"~/min_set\",\"min\":0,\"max\":19840,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MIN/config"), num, payload);
                                        num++;
                                        sequance--;
                                }
                                else
                                {
                                        snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MIN\",\"name\":\"dimmer-%02d-MIN\",\"stat_t\":\"~/min_state\",\"command_topic\":\"~/min_set\",\"min\":0,\"max\":255,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MIN/config"), num, payload);
                                        num++;
                                        sequance--;
                                }
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Max dimmer count for 99%
                        if (num < DIMMER_LIGHT_NUM)
                        {

                                if (num < 12)
                                {
                                        snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MAX\",\"name\":\"dimmer-%02d-MAX\",\"stat_t\":\"~/max_state\",\"command_topic\":\"~/max_set\",\"min\":0,\"max\":19840,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MAX/config"), num, payload);
                                        num++;
                                        sequance--;
                                }
                                else
                                {
                                        snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MAX\",\"name\":\"dimmer-%02d-MAX\",\"stat_t\":\"~/max_state\",\"command_topic\":\"~/max_set\",\"min\":0,\"max\":255,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MAX/config"), num, payload);
                                        num++;
                                        sequance--;
                                }
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // DIMMER_LIGHT_NUM

#if COVER_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PT100
                        if (num < COVER_NUM)
                        {
                                // position_topic
                                // set_position_topic
                                // command_topic
                                // state_topic
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/cover-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-cover-%02d\",\"name\":\"cover-%02d\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"pos_t\":\"~/pos_state\",\"set_pos_t\":\"~/pos_set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/cover/" BOARD_IDENTIFIER "/cover-%02d/config"), num, payload);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        if (num < COVER_NUM)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/cover-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-cover-%02d-up\",\"name\":\"cover-%02d-up-time\",\"stat_t\":\"~/up_state\",\"command_topic\":\"~/up_set\",\"min\":0,\"max\":65535,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MAX/config"), num, payload);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        if (num < COVER_NUM)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/cover-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-cover-%02d-down\",\"name\":\"cover-%02d-down-time\",\"stat_t\":\"~/down_state\",\"command_topic\":\"~/down_set\",\"min\":0,\"max\":65535,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MAX/config"), num, payload);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // COVER_NUM > 0

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER: // input
                        if (num < INPUT_NUM)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-input-%02d\",\"name\":\"input-%02d\",\"stat_t\":\"~/input-%02d/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/binary_sensor/" BOARD_IDENTIFIER "/input-%02d/config"), num, payload);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#ifdef TEMP_NUM
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PT100
                        if (num < TEMP_NUM)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pt100-%02d\",\"name\":\"pt100-%02d\",\"stat_t\":\"~/pt100-%02d/temperature\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pt100-%02d/config"), num, payload);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // PT100
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // BME280 temperature
                        for (uint8_t num = 0; num < BME280_NUM; ++num)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-temperature-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/temperature\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-temperature-%02d/config"), num, payload);
                        }
                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // BME280 humidity
                        for (uint8_t num = 0; num < BME280_NUM; ++num)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-humidity-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/humidity\",\"dev_cla\":\"humidity\",\"unit_of_meas\":\"%%\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-humidity-%02d/config"), num, payload);
                        }
                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // BME280 pressure
                        for (uint8_t num = 0; num < BME280_NUM; ++num)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-pressure-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/pressure\",\"dev_cla\":\"pressure\",\"unit_of_meas\":\"Pa\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-pressure-%02d/config"), num, payload);
                        }
                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // CCS811 CO2
                        for (uint8_t num = 0; num < CCS811_NUM; ++num)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ccs811-co2-%02d\",\"name\":\"ccs811-%02d\",\"stat_t\":\"~/ccs811-%02d/co2\",\"dev_cla\":\"carbon_dioxide\",\"unit_of_meas\":\"CO2\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-co2-%02d/config"), num, payload);
                        }

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:

                        // CCS811 COV
                        for (uint8_t num = 0; num < CCS811_NUM; ++num)
                        {
                                snprintf_P(payload, MAX_PAYLOAD, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ccs811-cov-%02d\",\"name\":\"ccs811-%02d\",\"stat_t\":\"~/ccs811-%02d/cov\",\"dev_cla\":\"pm10\",\"unit_of_meas\":\"ppm\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num, num, num);
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-cov-%02d/config"), num, payload);
                        }
                        break;

#ifdef POELE
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Poele mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/poele-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-mode\",\"name\":\"mode pcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  V1 consigne poêle en mode force (70°C)
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/V1/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V1\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V1\",\"name\":\"parametre poêle (V1)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"max\":85,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C7
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C7/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C7\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C7\",\"name\":\"Band morte Poele  (C7)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"min\":-100,\"max\":100,\"step\":1,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C4
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C4/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C4\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C4\",\"name\":\"consigne Jacuzzi (C4)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

                        // #ifdef COMMENT
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C5
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C5/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C5\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C5\",\"name\":\"consigne ECS1 & ECS2 (C5)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

                        // #ifdef COMMENT
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C6
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C6/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C6\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C6\",\"name\":\"consigne mode boost (C6)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#endif // POELE
#ifdef VANNES
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // 3 voies PCBT mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-mode\",\"name\":\"mode pcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // consigne vanne 3 voies PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C2/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C2\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C2\",\"name\":\"consigne PCBT (C2)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":38,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // 3 voies MCBT mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-mode\",\"name\":\"mode mcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // consigne vanne 3 voies MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C3/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C3\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C3\",\"name\":\"consigne MCBT (C3)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C8
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C8/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C8\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C8\",\"name\":\"consigne Temp PCBT a -10°C (C8)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C9
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C9/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C9\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C9\",\"name\":\"consigne Temp PCBT a +10°C (C9)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C10
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C10/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C10\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C10\",\"name\":\"consigne Temp MCBT a -10°C (C10)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C11
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C11/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C11\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C11\",\"name\":\"consigne Temp MCBT a +10°C (C11)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_PCBT_MIN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MIN/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/min_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_PCBT_MIN\",\"name\":\"consigne Temp PCBT minimum (C_PCBT_MIN)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_PCBT_MAX
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MAX/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/max_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_PCBT_MAX\",\"name\":\"consigne Temp PCBT maximum (C_PCBT_MAX)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_MCBT_MIN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MIN/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/min_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_MCBT_MIN\",\"name\":\"consigne Temp MCBT minimum (C_MCBT_MIN)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_MCBT_MAX
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MAX/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/max_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_MCBT_MAX\",\"name\":\"consigne Temp MCBT maximum (C_MCBT_MAX)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KP_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KP_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KP\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KP_PCBT\",\"name\":\"pid KP PCBT (KP_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KI_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KI_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KI_PCBT\",\"name\":\"pid KI PCBT (KI_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":1000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KD_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KD_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KD\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KD_PCBT\",\"name\":\"pid KD PCBT (KD_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KT_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KT_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KT\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KT_PCBT\",\"name\":\"pid interval PCBT (en ms) (KT_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60000,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT Action
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid_action/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_action\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_action\",\"name\":\"pcbt pid action \",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"direct\",\"reverse\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT pMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-pmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_pmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_pmode\",\"name\":\"pcbt pid pmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"pOnError\",\"pOnMeas\",\"pOnErrorMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-dmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_dmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_dmode\",\"name\":\"pcbt pid dmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"dOnError\",\"dOnMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-iawmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_iawmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_iawmode\",\"name\":\"pcbt pid iawmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"iAwCondition\",\"iAwClamp\",\"iAwOff\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KP_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KP_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KP\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KP_MCBT\",\"name\":\"pid KP MCBT (KP_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KI_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KI_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KI_MCBT\",\"name\":\"pid KI MCBT (KI_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KD_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KD_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KD\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KD_MCBT\",\"name\":\"pid KD MCBT (KD_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":1000000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KT_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KT_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KT\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KT_MCBT\",\"name\":\"pid interval MCBT (en ms) (KT_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60000,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT Action
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid_action/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_action\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_action\",\"name\":\"mcbt pid action \",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"direct\",\"reverse\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT pMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-pmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_pmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_pmode\",\"name\":\"mcbt pid pmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"pOnError\",\"pOnMeas\",\"pOnErrorMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-dmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_dmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_dmode\",\"name\":\"mcbt pid dmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"dOnError\",\"dOnMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-iawmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_iawmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_iawmode\",\"name\":\"mcbt pid iawmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"iAwCondition\",\"iAwClamp\",\"iAwOff\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-p/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-P\",\"name\":\"PCBT P\",\"stat_t\":\"~/P\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT I
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-i/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-I\",\"name\":\"PCBT I\",\"stat_t\":\"~/I\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT D
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-d/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-D\",\"name\":\"PCBT D\",\"stat_t\":\"~/D\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT OUT
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-out\",\"name\":\"PCBT out\",\"stat_t\":\"~/OUT\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT P
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-p/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-P\",\"name\":\"MCBT P\",\"stat_t\":\"~/P\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT I
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-i/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-I\",\"name\":\"MCBT I\",\"stat_t\":\"~/I\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT D
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-d/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-D\",\"name\":\"MCBT D\",\"stat_t\":\"~/D\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT OUT
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-out\",\"name\":\"MCBT out\",\"stat_t\":\"~/OUT\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // RATIO PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/ratio-pcbt/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/ratio\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ratio-pcbt\",\"name\":\"Ratio PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":10,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // RATIO MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/ratio-mcbt/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/ratio\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ratio-mcbt\",\"name\":\"Ratio MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":10,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset PCBT OUT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/offset-pcbt-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/offset-out\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-pcbt-out\",\"name\":\"Decalage Sortie PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-32768,\"max\":32767,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset MCBT OUT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/offset-mcbt-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/offset-out\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-mcbt-out\",\"name\":\"Decalage Sortie MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":65534,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset PCBT IN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/offset-pcbt-in/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/offset-in\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-pcbt-in\",\"name\":\"Decalage consigne PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset MCBT IN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/offset-mcbt-in/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/offset-in\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-mcbt-in\",\"name\":\"Decalage consigne MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#endif // VANNES
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:

                        // V2
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/V2/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V2\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V2\",\"name\":\"Reserve chaleur Ballon (V2)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // V3
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/V3/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V3\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V3\",\"name\":\"Temp Demi plage Morte (V3)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // load 1s
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1s/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-load_1s\",\"name\":\"Load 1s\",\"stat_t\":\"~/load_1s\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // load 1m
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1m/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-load_1m\",\"name\":\"Load 1m\",\"stat_t\":\"~/load_1m\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
                default:
                        return true;
                        break;
                }
                memory(false);
                sequance += 1;
                Serial.print(F("sequance = "));
                Serial.println(sequance - 1);
                Serial.print(F("len topic = "));
                Serial.print(strlen(topic));
                Serial.print(F(" payload = "));
                Serial.println(strlen(payload));
                if (strlen(topic) > max_len_topic)
                {
                        max_len_topic = strlen(topic);
                        Serial.print(F("max len topic = "));
                        Serial.println(max_len_topic);
                }
                if (strlen(payload) > max_len_payload)
                {
                        max_len_payload = strlen(payload);
                        Serial.print(F("max len payload = "));
                        Serial.println(max_len_payload);
                }
                memory(false);
        }

        return false;
}

MQTT_data ::~MQTT_data()
{
        if (_type == ha_cstr && _cstr != nullptr)
                free(_cstr);
}

void MQTT_data::store_P(const __FlashStringHelper *Topic, const char *Payload)
{
        _type = ha_flash_cstr;
        _topic = Topic;
        _fcstr = reinterpret_cast<const __FlashStringHelper *>(Payload);
};

void MQTT_data::store_P(const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload)
{
        _type = ha_flash_cstr_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _fcstr = reinterpret_cast<const __FlashStringHelper *>(Payload);
};

void MQTT_data::store(const __FlashStringHelper *Topic, const __FlashStringHelper *Payload)
{
        _type = ha_flash_cstr;
        _topic = Topic;
        _fcstr = Payload;
};

void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const __FlashStringHelper *Payload)
{
        _type = ha_flash_cstr_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _fcstr = Payload;
};

void MQTT_data::store(const __FlashStringHelper *Topic, char const *Payload)
{
        // 220502  debug(F(AT));
        _type = ha_cstr;
        _topic = Topic;
        _cstr = strdup(Payload);
};
void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, char const *Payload)
{
        // 220502  debug(F(AT));
        _type = ha_cstr_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _cstr = strdup(Payload);
};

void MQTT_data::store(const __FlashStringHelper *Topic, const float Payload)
{
        _type = ha_float;
        _topic = Topic;
        _float = Payload;
};
void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const float Payload)
{
        _type = ha_float_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _float = Payload;
};

void MQTT_data::store(const __FlashStringHelper *Topic, const int8_t Payload)
{
        _type = ha_int32_t;
        _topic = Topic;
        _int = Payload;
};
void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const int8_t Payload)
{
        _type = ha_int32_t_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _int = Payload;
};

void MQTT_data::store(const __FlashStringHelper *Topic, const uint8_t Payload)
{
        _type = ha_int32_t;
        _topic = Topic;
        _int = Payload;
};
void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const uint8_t Payload)
{
        _type = ha_int32_t_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _int = Payload;
};

void MQTT_data::store(const __FlashStringHelper *Topic, const uint16_t Payload)
{
        _type = ha_int32_t;
        _topic = Topic;
        _int = Payload;
};
void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const uint16_t Payload)
{
        _type = ha_int32_t_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _int = Payload;
};

void MQTT_data::store(const __FlashStringHelper *Topic, const uint32_t Payload)
{
        _type = ha_int32_t;
        _topic = Topic;
        _int = Payload;
};
void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const uint32_t Payload)
{
        _type = ha_int32_t_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _int = Payload;
};

void MQTT_data::get(char *topic, int topic_len, char *payload, int payload_len)
{
        // 220502  debug(F(AT));
        Serial.println(_type);
        switch (_type)
        {
        case ha_flash_cstr:
                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                strncpy_P(payload, reinterpret_cast<const char *>(_fcstr), payload_len);
                break;

        case ha_cstr:
                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                if (_cstr != nullptr)
                {
                        strncpy(payload, _cstr, payload_len);
                        free((void *)_cstr);
                        _cstr = nullptr;
                }
                break;

        case ha_float:
                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                dtostrf(_float, 1, 2, payload);
                break;

        case ha_int32_t:
                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                snprintf_P(payload, payload_len, PSTR("%i"), (int)_int);
                break;

        case ha_flash_cstr_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                strncpy_P(payload, reinterpret_cast<const char *>(_fcstr), payload_len);
                break;

        case ha_cstr_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                strncpy(payload, _cstr, payload_len);
                free((void *)_cstr);
                /* code */
                break;

        case ha_float_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                dtostrf(_float, 1, 2, payload);
                break;

        case ha_int32_t_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                snprintf_P(payload, payload_len, PSTR("%i"), (int)_int);
                break;

        default:
                strncpy_P(topic, PSTR("Error"), topic_len);
                strncpy_P(payload, PSTR("Error"), payload_len);
        }
};

void MQTT_data::print()
{
        char topic[64];
        char payload[32];
        get(topic, 64, payload, 32);
        Serial.print(topic);
        Serial.print(F("-->"));
        Serial.println(payload);
}

#endif // MQTT