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
        const char *f_payload;
        static uint8_t num = 0;
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

        if (now - time >= 100)
        {
                time = now;
                switch (sequance)
                {
                case BOOST_PP_COUNTER:
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/status/config"), F("{\"unique_id\":\"" BOARD_IDENTIFIER "-status\",\"name\":\"" BOARD_IDENTIFIER " status\",\"state_topic\":\"" MQTT_WILL_TOPIC "\",\"ret\":\"true\",\"device\":{\"identifiers\":\"" BOARD_IDENTIFIER "\",\"manufacturer\":\"" BOARD_MANUFACTURER "\",\"model\":\"" BOARD_MODEL "\",\"name\":\"" BOARD_IDENTIFIER " " BOARD_NAME "\",\"sw_version\":\"" BOARD_SW_VERSION_PRINT "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER: // relay
                        if (num < RELAY_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/relay-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-relay-%02d\",\"name\":\"" BOARD_IDENTIFIER " relay-%02d\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"ret\":\"true\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#if OPT_RELAY_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER: // Dimmer_relay
                        if (num < OPT_RELAY_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/opt-relay-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-opt-relay-%02d\",\"name\":\"" BOARD_IDENTIFIER " opt-relay-%02d\",\"ret\":\"true\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/switch/" BOARD_IDENTIFIER "/opt-relay-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // OPT_RELAY_NUM > 0

#if DIMMER_LIGHT_NUM > 0
#include BOOST_PP_UPDATE_COUNTER() //declaration des dimmer
                case BOOST_PP_COUNTER:
                        if (num < DIMMER_LIGHT_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d\",\"name\":\"" BOARD_IDENTIFIER " dimmer-%02d\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"bri_cmd_t\":\"~/bri_set\",\"bri_stat_t\":\"~/bri_state\",\"bri_scl\":\"255\",\"fx_list\":[\"NONE\",\"CANDLE\"],\"fx_stat_t\":\"~/fx_state\",\"fx_cmd_t\":\"~/fx_set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/light/" BOARD_IDENTIFIER "/dimmer-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                                        f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MIN\",\"name\":\"" BOARD_IDENTIFIER " dimmer-%02d-MIN\",\"stat_t\":\"~/min_state\",\"command_topic\":\"~/min_set\",\"min\":0,\"max\":19840,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MIN/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                        num++;
                                        sequance--;
                                }
                                else
                                {
                                        f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MIN\",\"name\":\"" BOARD_IDENTIFIER " dimmer-%02d-MIN\",\"stat_t\":\"~/min_state\",\"command_topic\":\"~/min_set\",\"min\":0,\"max\":255,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MIN/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                                        f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MAX\",\"name\":\"" BOARD_IDENTIFIER " dimmer-%02d-MAX\",\"stat_t\":\"~/max_state\",\"command_topic\":\"~/max_set\",\"min\":0,\"max\":19840,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MAX/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                        num++;
                                        sequance--;
                                }
                                else
                                {
                                        f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/dimmer-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-dimmer-%02d-MAX\",\"name\":\"" BOARD_IDENTIFIER " dimmer-%02d-MAX\",\"stat_t\":\"~/max_state\",\"command_topic\":\"~/max_set\",\"min\":0,\"max\":255,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/dimmer-%02d-MAX/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                case BOOST_PP_COUNTER: // COVER
                        if (num < COVER_NUM)
                        {
                                // position_topic
                                // set_position_topic
                                // command_topic
                                // state_topic
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/cover-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-cover-%02d\",\"name\":\"" BOARD_IDENTIFIER " cover-%02d\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"pos_t\":\"~/pos_state\",\"set_pos_t\":\"~/pos_set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/cover/" BOARD_IDENTIFIER "/cover-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/cover-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-cover-%02d-up\",\"name\":\"" BOARD_IDENTIFIER " cover-%02d-up-time\",\"stat_t\":\"~/up_state\",\"command_topic\":\"~/up_set\",\"min\":0,\"max\":65535,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/cover-%02d-up/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/cover-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-cover-%02d-down\",\"name\":\"" BOARD_IDENTIFIER " cover-%02d-down-time\",\"stat_t\":\"~/down_state\",\"command_topic\":\"~/down_set\",\"min\":0,\"max\":65535,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/cover-%02d-down/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // COVER_NUM > 0

#if PORTAL_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER: // COVER
                        if (num < PORTAL_NUM)
                        {
                                // position_topic
                                // set_position_topic
                                // command_topic
                                // state_topic
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/portal-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-portal-%02d\",\"name\":\"" BOARD_IDENTIFIER " portal-%02d\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/cover/" BOARD_IDENTIFIER "/portal-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // PORTAL_NUM > 0

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER: // input
                        if (num < INPUT_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-input-%02d\",\"name\":\"" BOARD_IDENTIFIER " input-%02d\",\"stat_t\":\"~/input-%02d/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/binary_sensor/" BOARD_IDENTIFIER "/input-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#if PT100_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PT100
                        if (num < PT100_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pt100-%02d\",\"name\":\"" BOARD_IDENTIFIER " pt100-%02d\",\"stat_t\":\"~/pt100-%02d\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pt100-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // PT100

#if RADIATOR_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // RADIATOR
                        if (num < RADIATOR_NUM)
                        {

                                // OK current_temperature_topic = curr_temp_t = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d")
                                // OK action_topic = act_t =(off, heating, cooling, drying, idle, fan) = ~/state

                                // OK temperature_command_topic = temp_cmd_t = ~/temp_set
                                // OK temperature_state_topic = temp_stat_t = ~/temp_state
                                // OK temp_step = 0.1
                                // OK max_temp = RADIATOR_HA_MAX_TEMP
                                // OK min_temp = RADIATOR_HA_MIN_TEMP

                                // OK modes ([“auto”, “off”, “cool”, “heat”, “dry”, “fan_only”]) = [“off”, “heat”]
                                // OK mode_command_topic = mode_cmd_t = ~/mode_set
                                // OK mode_state_topic = mode_stat_t = ~/mode_state

                                // TODO:Radiator preset_modes (eco, away, boost, comfort, home, sleep and activity) = pr_modes
                                // TODO:Radiator preset_mode_state_topic = pr_mode_stat_t
                                // TODO:Radiator preset_mode_command_topic = pr_mode_cmd_t

                                uint8_t PT100_NUM = pgm_read_byte(DIMMER_RADIATOR_PT100_ARRAY + num);
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/radiator-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-radiator-%02d-heat\",\"name\":\"" BOARD_IDENTIFIER " radiator-%02d\",\"curr_temp_t\":\"" MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d\",\"act_t\":\"~/state\",\"temp_cmd_t\":\"~/temp_set\",\"temp_stat_t\":\"~/temp_state\",\"temp_step\":\"0.1\",\"max_temp\":\"RADIATOR_HA_MAX_TEMP\",\"min_temp\":\"RADIATOR_HA_MIN_TEMP\",\"modes\":\"[“off”,“heat”]\",\"mode_cmd_t\":\"~/mode_set\",\"mode_stat_t\":\"~/mode_state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/climate/" BOARD_IDENTIFIER "/radiator-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1, num + 1);
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
                        // RADIATOR
                        if (num < RADIATOR_NUM) // temp de cycle
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/radiator-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-radiator-%02d-cycle\",\"name\":\"" BOARD_IDENTIFIER " radiator-%02d-cycle\",\"stat_t\":\"~/cycle-state\",\"command_topic\":\"~/cycle-set\",\"unit_of_meas\":\"ms\",\"min\":100,\"max\":3600000,\"step\":1,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/radiator-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // RADIATOR
                        if (num < RADIATOR_NUM) //-10
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/radiator-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-radiator-%02d-m10\",\"name\":\"" BOARD_IDENTIFIER " radiator-%02d-m10\",\"stat_t\":\"~/m10-state\",\"command_topic\":\"~/m10-set\",\"unit_of_meas\":\"%\",\"min\":0,\"max\":100,\"step\":\"0,01\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/radiator-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // RADIATOR
                        if (num < RADIATOR_NUM) //+10
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/radiator-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-radiator-%02d-p10\",\"name\":\"" BOARD_IDENTIFIER " radiator-%02d-p10\",\"stat_t\":\"~/p10-state\",\"command_topic\":\"~/p10-set\",\"unit_of_meas\":\"%\",\"min\":0,\"max\":100,\"step\":\"0,01\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/radiator-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // RADIATOR
                        if (num < RADIATOR_NUM) // KI
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/radiator-%02d\",\"uniq_id\":\"" BOARD_IDENTIFIER "-radiator-%02d-KI\",\"name\":\"" BOARD_IDENTIFIER " radiator-%02d-KI\",\"stat_t\":\"~/KI-state\",\"command_topic\":\"~/KI-set\",\"min\":0,\"max\":10000,\"step\":\"0,01\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"),
                                DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/radiator-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // RADIATOR_NUM

#if BME280_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // BME280 temperature
                        if (num < BME280_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-temp-%02d\",\"name\":\"" BOARD_IDENTIFIER " BME280-temperature-%02d\",\"stat_t\":\"~/bme280-%02d/temp\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-temperature-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // BME280 humidity
                        if (num < BME280_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-humidity-%02d\",\"name\":\"" BOARD_IDENTIFIER " BME280-humidity-%02d\",\"stat_t\":\"~/bme280-%02d/humidity\",\"dev_cla\":\"humidity\",\"unit_of_meas\":\"%%\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-humidity-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // BME280 pressure
                        if (num < BME280_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-pressure-%02d\",\"name\":\"" BOARD_IDENTIFIER " BME280-pressure-%02d\",\"stat_t\":\"~/bme280-%02d/pressure\",\"dev_cla\":\"pressure\",\"unit_of_meas\":\"Pa\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-pressure-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // BME280_NUM
#if CCS811_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // CCS811 CO2
                        if (num < CCS811_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ccs811-co2-%02d\",\"name\":\"" BOARD_IDENTIFIER " ccs811-co2-%02d\",\"stat_t\":\"~/ccs811-%02d/co2\",\"dev_cla\":\"carbon_dioxide\",\"unit_of_meas\":\"CO2\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-co2-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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

                        // CCS811 COV
                        if (num < CCS811_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ccs811-cov-%02d\",\"name\":\"" BOARD_IDENTIFIER " ccs811-cov-%02d\",\"stat_t\":\"~/ccs811-%02d/cov\",\"dev_cla\":\"pm10\",\"unit_of_meas\":\"ppm\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-cov-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#endif // CCS811_NUM
#if SCD4X_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // SCD4X temperature
                        if (num < SCD4X_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-scd4x-temperature-%02d\",\"name\":\"" BOARD_IDENTIFIER " scd4x-temperature-%02d\",\"stat_t\":\"~/scd4x-%02d/temperature\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/scd4x-temperature-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // SCD4X humidity
                        if (num < SCD4X_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-scd4x-humidity-%02d\",\"name\":\"" BOARD_IDENTIFIER " scd4x-humidity-%02d\",\"stat_t\":\"~/scd4x-%02d/humidity\",\"dev_cla\":\"humidity\",\"unit_of_meas\":\"%%\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/scd4x-humidity-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // SCD4X co2
                        if (num < SCD4X_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-scd4x-co2-%02d\",\"name\":\"" BOARD_IDENTIFIER " scd4x-co2-%02d\",\"stat_t\":\"~/scd4x-%02d/co2\",\"dev_cla\":\"carbon_dioxide\",\"unit_of_meas\":\"ppm\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/scd4x-co2-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // SCD4X_NUM

#if HDC1080_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // HDC1080 temperature
                        if (num < HDC1080_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-hdc1080-temp-%02d\",\"name\":\"" BOARD_IDENTIFIER " hdc1080-temperature-%02d\",\"stat_t\":\"~/hdc1080-%02d/temp\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/hdc1080-temp-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // HDC1080 humidity
                        if (num < HDC1080_NUM)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-hdc1080-humidity-%02d\",\"name\":\"" BOARD_IDENTIFIER " hdc1080-humidity-%02d\",\"stat_t\":\"~/hdc1080-%02d/humidity\",\"dev_cla\":\"humidity\",\"unit_of_meas\":\"%%\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/hdc1080-humidity-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;
#endif // HDC1080_NUM

#if CPT_PULSE_INPUT > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // input pulse counter
                        if (num < CPT_PULSE_INPUT)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-%02d\",\"name\":\"" BOARD_IDENTIFIER " counter-%02d\",\"stat_t\":\"~/counter-%02d\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/counter-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // input pulse counter
                        if (num < CPT_PULSE_INPUT)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-%02d-btn\",\"name\":\"" BOARD_IDENTIFIER " reset-counter-%02d\",\"command_topic\":\"~/counter-%02d/btn\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/button/" BOARD_IDENTIFIER "/counter-%02d-btn/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#endif // CPT_PULSE_INPUT

#if CPT_PULSE_INPUT_IF_OUT > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // input pulse counter if
                        if (num < CPT_PULSE_INPUT_IF_OUT)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-if-out-true-%02d\",\"name\":\"counter-if-out-%02d-true\",\"stat_t\":\"~/counter-if-out-%02d/true\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/counter-if-out-true-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);

                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-if-out-false-%02d\",\"name\":\"counter-if-out-%02d-false\",\"stat_t\":\"~/counter-if-out-%02d/false\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/counter-if-out-false-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // input pulse counter if
                        if (num < CPT_PULSE_INPUT_IF_OUT)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-if-out-btn-%02d\",\"name\":\"reset-counter-if-out-%02d\",\"command_topic\":\"~/counter-if-out-%02d/btn\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/button/" BOARD_IDENTIFIER "/counter-if-out-btn-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#endif // CPT_PULSE_INPUT_IF_OUT

#if CPT_PULSE_INPUT_IF_IN > 0
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // input pulse counter if
                        if (num < CPT_PULSE_INPUT_IF_IN)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-if-in-true-%02d\",\"name\":\"counter-if-in-%02d-true\",\"stat_t\":\"~/counter-if-in-%02d/true\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/counter-if-in-true-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);

                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-if-in-false-%02d\",\"name\":\"counter-if-in-%02d-false\",\"stat_t\":\"~/counter-if-in-%02d/false\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/counter-if-in-false-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
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
                        // input pulse counter if
                        if (num < CPT_PULSE_INPUT_IF_IN)
                        {
                                f_payload = PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-counter-if-in-btn-%02d\",\"name\":\"reset-counter-if-in-%02d\",\"command_topic\":\"~/counter-if-in-%02d/btn\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}");
                                DT_mqtt_send(F("homeassistant/button/" BOARD_IDENTIFIER "/counter-if-in-btn-%02d/config"), num + 1, reinterpret_cast<const __FlashStringHelper *>(f_payload), num + 1);
                                num++;
                                sequance--;
                        }
                        else
                        {
                                num = 0;
                        }
                        break;

#endif // CPT_PULSE_INPUT_IF_IN

#ifdef POELE
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Poele mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/poele-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/poele/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-poele-mode\",\"name\":\"" BOARD_IDENTIFIER " mode poele\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Arret\",\"Normal\",\"Forcé\",\"Veille\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  V1 consigne poêle en mode force (70°C)
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/V1/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V1\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V1\",\"name\":\"" BOARD_IDENTIFIER " parametre poêle (V1)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"max\":85,\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C7
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C7/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C7\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C7\",\"name\":\"" BOARD_IDENTIFIER " Band morte Poele  (C7)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"min\":-100,\"max\":100,\"step\":1,\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C4
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C4/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C4\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C4\",\"name\":\"" BOARD_IDENTIFIER " consigne Jacuzzi (C4)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

                        // #ifdef COMMENT
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C5
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C5/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C5\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C5\",\"name\":\"" BOARD_IDENTIFIER " consigne ECS (C5)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

                        // #ifdef COMMENT
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C6
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C6/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C6\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C6\",\"name\":\"" BOARD_IDENTIFIER " consigne mode boost (C6)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:

                        // V2
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/V2/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V2\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V2\",\"name\":\"" BOARD_IDENTIFIER " Reserve chaleur Ballon (V2)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // V3
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/V3/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V3\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V3\",\"name\":\"" BOARD_IDENTIFIER " Temp Demi plage Morte (V3)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));

                        break;
#endif // POELE

#ifdef CHAUFFAGE
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Chauffage mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/chauffage-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-mode\",\"name\":\"" BOARD_IDENTIFIER " mode chauffage\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Eté\",\"Inter-saison\",\"Hiver\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Temperature arret poele hiver
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/chauffage_TAPH/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/TAPH\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-TAPH\",\"name\":\"" BOARD_IDENTIFIER " temperature arret poele hiver\",\"stat_t\":\"~/state\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Temperature arret poele intersaison
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/chauffage_TAPI/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/TAPI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-TAPI\",\"name\":\"" BOARD_IDENTIFIER " temperature arret poele inter-saison\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":95,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // temperature_balon_max
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/chauffage_TBMa/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/TBMa\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-TBMa\",\"name\":\"" BOARD_IDENTIFIER " temperature balon max\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":95,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // temperature_balon_min;
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/chauffage_TBMi/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/TBMi\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-TBMi\",\"name\":\"" BOARD_IDENTIFIER " temperature balon min\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":95,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // temp_inter_demmarage;
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/chauffage_TID/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/TID\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-TID\",\"name\":\"" BOARD_IDENTIFIER " temps inter demmarage\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":24,\"step\":1,\"unit_of_meas\":\"h\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;


#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // uint32_t date_retour_vacance;
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/chauffage_DRV/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/DRV\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-DRV\",\"name\":\"" BOARD_IDENTIFIER " Date retour vacance (timestamp)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":1,\"min\":0,\"max\":4294967295,\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //ha_arret_meteo
                        DT_mqtt_send(F("homeassistant/switch/" BOARD_IDENTIFIER "/chauffage_meteo/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/chauffage/AM\",\"uniq_id\":\"" BOARD_IDENTIFIER "-CH-AM\",\"name\":\"" BOARD_IDENTIFIER " Arret Météo\",\"command_topic\":\"~/set\",\"stat_t\":\"~/state\",\"ret\":\"true\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
#endif // CHAUFFAGE

#ifdef VANNES
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // 3 voies PCBT mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-mode\",\"name\":\"" BOARD_IDENTIFIER " mode pcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\",\"Veille\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // consigne vanne 3 voies PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C2/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C2\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C2\",\"name\":\"" BOARD_IDENTIFIER " consigne PCBT (C2)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":38,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // 3 voies MCBT mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-mode\",\"name\":\"" BOARD_IDENTIFIER " mode mcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\",\"Veille\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // consigne vanne 3 voies MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C3/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C3\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C3\",\"name\":\"" BOARD_IDENTIFIER " consigne MCBT (C3)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C8
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C8/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C8\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C8\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp PCBT a -10°C (C8)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C9
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C9/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C9\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C9\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp PCBT a +10°C (C9)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C10
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C10/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C10\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C10\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp MCBT a -10°C (C10)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C11
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C11/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C11\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C11\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp MCBT a +10°C (C11)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_PCBT_MIN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MIN/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/min_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_PCBT_MIN\",\"name\":\"consigne Temp PCBT minimum T° ext. (C_PCBT_MIN)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_PCBT_MAX
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MAX/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/max_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_PCBT_MAX\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp PCBT maximum (C_PCBT_MAX)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_MCBT_MIN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MIN/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/min_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_MCBT_MIN\",\"name\":\"consigne Temp MCBT minimum T° ext (C_MCBT_MIN)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_MCBT_MAX
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MAX/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/max_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_MCBT_MAX\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp MCBT maximum (C_MCBT_MAX)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KP_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KP_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KP\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KP_PCBT\",\"name\":\"" BOARD_IDENTIFIER " pid KP PCBT (KP_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KI_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KI_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KI_PCBT\",\"name\":\"" BOARD_IDENTIFIER " pid KI PCBT (KI_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":1000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KD_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KD_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KD\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KD_PCBT\",\"name\":\"" BOARD_IDENTIFIER " pid KD PCBT (KD_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KT_PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KT_PCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KT\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KT_PCBT\",\"name\":\"" BOARD_IDENTIFIER " pid interval PCBT (en ms) (KT_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60000,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT Action
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid_action/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_action\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_action\",\"name\":\"" BOARD_IDENTIFIER " pcbt pid action \",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"direct\",\"reverse\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT pMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-pmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_pmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_pmode\",\"name\":\"" BOARD_IDENTIFIER " pcbt pid pmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"pOnError\",\"pOnMeas\",\"pOnErrorMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-dmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_dmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_dmode\",\"name\":\"" BOARD_IDENTIFIER " pcbt pid dmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"dOnError\",\"dOnMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-iawmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_iawmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_iawmode\",\"name\":\"" BOARD_IDENTIFIER " pcbt pid iawmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"iAwCondition\",\"iAwClamp\",\"iAwOff\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KP_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KP_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KP\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KP_MCBT\",\"name\":\"" BOARD_IDENTIFIER " pid KP MCBT (KP_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KI_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KI_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KI_MCBT\",\"name\":\"" BOARD_IDENTIFIER " pid KI MCBT (KI_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KD_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KD_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KD\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KD_MCBT\",\"name\":\"" BOARD_IDENTIFIER " pid KD MCBT (KD_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":1000000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KT_MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KT_MCBT/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KT\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KT_MCBT\",\"name\":\"" BOARD_IDENTIFIER " pid interval MCBT (en ms) (KT_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60000,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT Action
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid_action/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_action\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_action\",\"name\":\"" BOARD_IDENTIFIER " mcbt pid action \",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"direct\",\"reverse\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT pMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-pmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_pmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_pmode\",\"name\":\"" BOARD_IDENTIFIER " mcbt pid pmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"pOnError\",\"pOnMeas\",\"pOnErrorMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-dmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_dmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_dmode\",\"name\":\"" BOARD_IDENTIFIER " mcbt pid dmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"dOnError\",\"dOnMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-iawmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_iawmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_iawmode\",\"name\":\"" BOARD_IDENTIFIER " mcbt pid iawmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"iAwCondition\",\"iAwClamp\",\"iAwOff\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-p/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-P\",\"name\":\"" BOARD_IDENTIFIER " PCBT P\",\"stat_t\":\"~/P\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT I
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-i/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-I\",\"name\":\"" BOARD_IDENTIFIER " PCBT I\",\"stat_t\":\"~/I\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT D
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-d/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-D\",\"name\":\"" BOARD_IDENTIFIER " PCBT D\",\"stat_t\":\"~/D\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT OUT
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-out\",\"name\":\"" BOARD_IDENTIFIER " PCBT out\",\"stat_t\":\"~/OUT\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT P
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-p/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-P\",\"name\":\"" BOARD_IDENTIFIER " MCBT P\",\"stat_t\":\"~/P\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT I
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-i/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-I\",\"name\":\"" BOARD_IDENTIFIER " MCBT I\",\"stat_t\":\"~/I\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT D
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-d/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-D\",\"name\":\"" BOARD_IDENTIFIER " MCBT D\",\"stat_t\":\"~/D\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID MCBT OUT
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-out\",\"name\":\"" BOARD_IDENTIFIER " MCBT out\",\"stat_t\":\"~/OUT\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // RATIO PCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/ratio-pcbt/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/ratio\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ratio-pcbt\",\"name\":\"" BOARD_IDENTIFIER " Ratio PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":10,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // RATIO MCBT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/ratio-mcbt/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/ratio\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ratio-mcbt\",\"name\":\"" BOARD_IDENTIFIER " Ratio MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":10,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset PCBT OUT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/inhib-pcbt-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/inhib-out\",\"uniq_id\":\"" BOARD_IDENTIFIER "-inhib-pcbt-out\",\"name\":\"" BOARD_IDENTIFIER " inhibition Sortie PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":32767,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset MCBT OUT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/inhib-mcbt-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/inhib-out\",\"uniq_id\":\"" BOARD_IDENTIFIER "-inhib-mcbt-out\",\"name\":\"" BOARD_IDENTIFIER " inhibition Sortie MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":32767,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset PCBT IN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/offset-pcbt-in/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/offset-in\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-pcbt-in\",\"name\":\"" BOARD_IDENTIFIER " Decalage consigne PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset MCBT IN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/offset-mcbt-in/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/offset-in\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-mcbt-in\",\"name\":\"" BOARD_IDENTIFIER " Decalage consigne MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  temperature moyenne
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/avg-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-avg-temp\",\"name\":\"" BOARD_IDENTIFIER " Temperature moyenne\",\"stat_t\":\"~/state\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  temperature moyenne
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/avg-offset-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-avg-offset-temp\",\"name\":\"Temperature moyenne décalée\",\"stat_t\":\"~/offset-state\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  decalage temperature moyenne
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/max-offset-avg-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp/offset-sup\",\"uniq_id\":\"" BOARD_IDENTIFIER "-max-offset-avg-temp\",\"name\":\"Decalage Exterieur sup\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  decalage temperature moyenne
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/min-offset-avg-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp/offset-inf\",\"uniq_id\":\"" BOARD_IDENTIFIER "-min-offset-avg-temp\",\"name\":\"Decalage Exterieur inf\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#endif // VANNES


#ifdef DT_3VOIES_1_NATH

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // 3 voies v1n mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/v1n-mode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-mode\",\"name\":\"" BOARD_IDENTIFIER " mode v1n\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Normal\",\"Manuel\",\"Arret\",\"Veille\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // consigne vanne 3 voies v1n manuel
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/v1n-cm/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/C\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C3\",\"name\":\"" BOARD_IDENTIFIER " consigne v1n (manuel)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // consigne Temp v1n a -10°C
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/v1n-m10/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/m10\",\"uniq_id\":\"" BOARD_IDENTIFIER "-m10\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp v1n a -10°C\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // consigne Temp v1n a +10°C
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/v1n-p10/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/p10\",\"uniq_id\":\"" BOARD_IDENTIFIER "-p10\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp v1n a +10°C\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_v1n_MIN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/v1n_MIN/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/min_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_v1n_MIN\",\"name\":\"consigne Temp v1n minimum T° ext (C_v1n_MIN)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // C_v1n_MAX
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/v1n_MAX/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/max_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_v1n_MAX\",\"name\":\"" BOARD_IDENTIFIER " consigne Temp v1n maximum (C_v1n_MAX)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"mode\":\"box\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KP_v1n
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KP_v1n/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/KP\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KP_v1n\",\"name\":\"" BOARD_IDENTIFIER " pid KP v1n (KP_v1n)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KI_v1n
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KI_v1n/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/KI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KI_v1n\",\"name\":\"" BOARD_IDENTIFIER " pid KI v1n (KI_v1n)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KD_v1n
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KD_v1n/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/KD\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KD_v1n\",\"name\":\"" BOARD_IDENTIFIER " pid KD v1n (KD_v1n)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":1000000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // KT_v1n
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/KT_v1n/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/KT\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KT_v1n\",\"name\":\"" BOARD_IDENTIFIER " pid interval v1n (en ms) (KT_v1n)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60000,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
                        
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT Action
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/v1n-pid_action/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/pid_action\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-pid_action\",\"name\":\"" BOARD_IDENTIFIER " v1n pid action \",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"direct\",\"reverse\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;


#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT pMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/v1n-pid-pmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/pid_pmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-pid_pmode\",\"name\":\"" BOARD_IDENTIFIER " v1n pid pmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"pOnError\",\"pOnMeas\",\"pOnErrorMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID PCBT dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/v1n-pid-dmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/pid_dmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-pid_dmode\",\"name\":\"" BOARD_IDENTIFIER " v1n pid dmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"dOnError\",\"dOnMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID v1n dMode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/v1n-pid-iawmode/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/pid_iawmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-pid_iawmode\",\"name\":\"" BOARD_IDENTIFIER " v1n pid iawmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"iAwCondition\",\"iAwClamp\",\"iAwOff\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID v1n P
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/v1n-pid-p/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-P\",\"name\":\"" BOARD_IDENTIFIER " v1n P\",\"stat_t\":\"~/P\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID v1n I
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/v1n-pid-i/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-I\",\"name\":\"" BOARD_IDENTIFIER " v1n I\",\"stat_t\":\"~/I\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID v1n D
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/v1n-pid-d/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-D\",\"name\":\"" BOARD_IDENTIFIER " v1n D\",\"stat_t\":\"~/D\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // PID v1n OUT
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/v1n-pid-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n\",\"uniq_id\":\"" BOARD_IDENTIFIER "-v1n-out\",\"name\":\"" BOARD_IDENTIFIER " v1n out\",\"stat_t\":\"~/OUT\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // RATIO v1n
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/ratio-v1n/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/ratio\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ratio-v1n\",\"name\":\"" BOARD_IDENTIFIER " Ratio v1n\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":10,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset v1n OUT
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/inhib-v1n-out/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/inhib-out\",\"uniq_id\":\"" BOARD_IDENTIFIER "-inhib-v1n-out\",\"name\":\"" BOARD_IDENTIFIER " inhibition Sortie v1n\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":32767,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Offset v1n IN
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/offset-v1n-in/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/v1n/offset-in\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-v1n-in\",\"name\":\"" BOARD_IDENTIFIER " Decalage consigne v1n\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#endif // DT_3VOIES_1_NATH

#ifdef DT_PT100_EXT

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  temperature moyenne
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/avg-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-avg-temp\",\"name\":\"" BOARD_IDENTIFIER " Temperature moyenne\",\"stat_t\":\"~/state\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  temperature moyenne
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/avg-offset-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-avg-offset-temp\",\"name\":\"Temperature moyenne décalée\",\"stat_t\":\"~/offset-state\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  decalage temperature moyenne
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/max-offset-avg-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp/offset-sup\",\"uniq_id\":\"" BOARD_IDENTIFIER "-max-offset-avg-temp\",\"name\":\"Decalage Exterieur sup\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        //  decalage temperature moyenne
                        DT_mqtt_send(F("homeassistant/number/" BOARD_IDENTIFIER "/min-offset-avg-temp/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/avg-temp/offset-inf\",\"uniq_id\":\"" BOARD_IDENTIFIER "-min-offset-avg-temp\",\"name\":\"Decalage Exterieur inf\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#endif //DT_PT100_EXT

#ifdef RELAY_ECS1
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // 3 voies MCBT mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/ecs1/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/ecs1\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ecs1-mode\",\"name\":\"" BOARD_IDENTIFIER " mode ECS1\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Marche\",\"Arret\",\"Veille\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#endif // RELAY_ECS1

#ifdef RELAY_ECS2
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // 3 voies MCBT mode
                        DT_mqtt_send(F("homeassistant/select/" BOARD_IDENTIFIER "/ecs2/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/ecs2\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ecs2-mode\",\"name\":\"" BOARD_IDENTIFIER " mode ECS2\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Marche\",\"Arret\",\"Veille\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#endif // RELAY_ECS2

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // load 1s
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1s/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-load_1s\",\"name\":\"" BOARD_IDENTIFIER " Load 1s\",\"stat_t\":\"~/load_1s\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // used memory
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/used_memory/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/memory\",\"uniq_id\":\"" BOARD_IDENTIFIER "-used-memory\",\"name\":\"" BOARD_IDENTIFIER " Used memory\",\"stat_t\":\"~/used\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // free memory
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/free_memory/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/memory\",\"uniq_id\":\"" BOARD_IDENTIFIER "-free-memory\",\"name\":\"" BOARD_IDENTIFIER " Free memory\",\"stat_t\":\"~/free\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // Large free memory
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/large_memory/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/memory\",\"uniq_id\":\"" BOARD_IDENTIFIER "-large-memory\",\"name\":\"" BOARD_IDENTIFIER " Large memory\",\"stat_t\":\"~/large\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // memory free list
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/memory_free_list/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/memory\",\"uniq_id\":\"" BOARD_IDENTIFIER "-memory-free-list\",\"name\":\"" BOARD_IDENTIFIER " memory free list\",\"stat_t\":\"~/number\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;

#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // load 1m
                        DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1m/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-load_1m\",\"name\":\"" BOARD_IDENTIFIER " Load 1m\",\"stat_t\":\"~/load_1m\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#ifdef WATCHDOG_TIME
#include BOOST_PP_UPDATE_COUNTER()
                case BOOST_PP_COUNTER:
                        // debug
                        DT_mqtt_send(F("homeassistant/text/" BOARD_IDENTIFIER "/debug_str/config"), F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/debug_str\",\"uniq_id\":\"" BOARD_IDENTIFIER "-debug_str\",\"name\":\"" BOARD_IDENTIFIER " debug_str\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"max\":\"64\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"));
                        break;
#endif
                default:
                        return true;
                        break;
                }
                memory(false);
                sequance += 1;
                Serial.print(F("sequance = "));
                Serial.println(sequance - 1);
                memory(false);
        }

        return false;
}
/*
MQTT_data ::~MQTT_data()
{
        if (_type == ha_cstr && _cstr != nullptr)
        {
                free(_cstr);
                _cstr = nullptr;
        }
        else if (_type == ha_cstr_tsprintf && _cstr != nullptr)
        {
                free(_cstr);
                _cstr = nullptr;
        }
}
*/
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

void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const __FlashStringHelper *Payload, uint8_t num_p)
{
        _type = ha_flash_cstr_tsprintf_2;
        _num_t = num_t;
        _topic = Topic;
        _fcstr = Payload;
        _num_p = num_p;
};

// void MQTT_data::store(const __FlashStringHelper *Topic, char const *Payload)
// {
//         // 220502  debug(F(AT));
//         _type = ha_cstr;
//         _topic = Topic;
//         _cstr = strdup(Payload);
// };

// void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, char const *Payload)
// {
//         // 220502  debug(F(AT));
//         _type = ha_cstr_tsprintf;
//         _num_t = num_t;
//         _topic = Topic;
//         _cstr = strdup(Payload);
// };

void MQTT_data::store(const __FlashStringHelper *Topic, const String Payload)
{
        _type = ha_str;
        _topic = Topic;
        _str = Payload;
        // if (_str.length() != Payload.length())
        // {
        //         _type = ha_int32_t;
        //         _topic = F("DtBoard/" BOARD_IDENTIFIER "/memory");
        //         _int = memory(true);
        // }
};

void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const String Payload)
{
        debug(F(AT));
        // Serial.println(Payload);
        _type = ha_str_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _str = Payload;
        // if (_str.length() != Payload.length())
        // {
        //         _type = ha_int32_t;
        //         _topic = F("DtBoard/" BOARD_IDENTIFIER "/memory");
        //         _int = memory(true);
        // }
        // Serial.println(_str);
        debug(F(AT));
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

void MQTT_data::store(const __FlashStringHelper *Topic, const int16_t Payload)
{
        _type = ha_int32_t;
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
        _type = ha_uint32_t;
        _topic = Topic;
        _int = Payload;
};
void MQTT_data::store(const __FlashStringHelper *Topic, uint8_t num_t, const uint32_t Payload)
{
        _type = ha_uint32_t_tsprintf;
        _num_t = num_t;
        _topic = Topic;
        _int = Payload;
};

void MQTT_data::get(char *topic, int topic_len, char *payload, unsigned int payload_len)
{
        // 220502  debug(F(AT));
        // Serial.println(_type);
        switch (_type)
        {
        case ha_flash_cstr:
                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                strlcpy_P(payload, reinterpret_cast<const char *>(_fcstr), payload_len);
                debug(F(AT));
                break;
                /*
                        case ha_cstr:
                                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                                debug(F(AT));
                                if (_cstr != nullptr)
                                {
                                        strncpy(payload, _cstr, payload_len);
                                        free((void *)_cstr);
                                        _cstr = nullptr;
                                }
                                break;
                */
        case ha_str:
                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                debug(F(AT));
                // Serial.println(_str);
                if ((_str.length() + 1) < payload_len)
                {
                        payload = strncpy(payload, _str.c_str(), _str.length() + 1);
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

        case ha_uint32_t:
                strncpy_P(topic, reinterpret_cast<const char *>(_topic), topic_len);
                snprintf_P(payload, payload_len, PSTR("%" PRIu32), (uint32_t)_int);
                break;

        case ha_flash_cstr_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                strncpy_P(payload, reinterpret_cast<const char *>(_fcstr), payload_len);
                break;

        case ha_flash_cstr_tsprintf_2:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                snprintf_P(payload, payload_len, reinterpret_cast<const char *>(_fcstr), _num_p, _num_p, _num_p);
                break;
                /*
                        case ha_cstr_tsprintf:
                                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                                strncpy(payload, _cstr, payload_len);
                                free((void *)_cstr);
                                _cstr = nullptr;
                                break;
                */
        case ha_str_tsprintf:
                debug(F(AT));
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                // Serial.println(_str);
                if ((_str.length() + 1) < payload_len)
                {
                        debug(F(AT));
                        payload = strncpy(payload, _str.c_str(), _str.length() + 1);
                        // Serial.println(_str);
                }
                break;

        case ha_float_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                dtostrf(_float, 1, 2, payload);
                break;

        case ha_int32_t_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                snprintf_P(payload, payload_len, PSTR("%" PRId32), (int32_t)_int);
                break;

        case ha_uint32_t_tsprintf:
                snprintf_P(topic, topic_len, reinterpret_cast<const char *>(_topic), _num_t);
                snprintf_P(payload, payload_len, PSTR("%" PRIu32), (uint32_t)_int);
                break;

        default:
                strncpy_P(topic, PSTR("Error"), topic_len);
                strncpy_P(payload, PSTR("Error"), payload_len);
        }

        static uint16_t payload_size = 0;
        if (strlen(payload) > payload_size)
        {
                payload_size = strlen(payload);
                // Serial.print(F("max payload = "));
                // Serial.println(payload_size);
        }
        static uint16_t topic_size = 0;
        if (strlen(topic) > topic_size)
        {
                topic_size = strlen(topic);
                // Serial.print(F("max topic = "));
                // Serial.println(topic_size);
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