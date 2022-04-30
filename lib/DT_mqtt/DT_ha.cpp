#include <DT_ha.h>
#include <DT_eeprom.h>

#include <DT_mqtt_send.h>

#include <ArduinoJson.h>

#ifdef MQTT

#define max_topic 64
#define max_payload 272
bool homeassistant(bool start)
{
    uint32_t now = millis();
    static uint32_t time = 0;
    static uint8_t sequance = 253;
    static uint16_t max_len_topic = 0;
    static uint16_t max_len_payload = 0;
    static uint8_t num = 0;
    char topic[max_topic];
    char payload[max_payload];
    memory(true);

    // debug_wdt_reset();
    if (start)
    {
        // debug(AT);
        sequance = 0;
        return false;
    }
    else if (sequance == 0)
        Serial.println(F("homeassistant"));

    if (now - time >= 50)
    {
        time = now;
        switch (sequance)
        {
        case 0:
            // strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/status/config"), max_topic);
            // strlcpy_P(payload, PSTR("{\"unique_id\":\"" BOARD_IDENTIFIER "-status\",\"name\":\"status\",\"state_topic\":\"DtBoard/" BOARD_IDENTIFIER "/status\",\"device\":{\"identifiers\":\"" BOARD_IDENTIFIER "\",\"manufacturer\":\"" BOARD_MANUFACTURER "\",\"model\":\"" BOARD_MODEL "\",\"name\":\"" BOARD_NAME "\",\"sw_version\":\"" BOARD_SW_VERSION_PRINT "\"}}"), max_payload);
            DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/status/config"), F("{\"unique_id\":\"" BOARD_IDENTIFIER "-status\",\"name\":\"status\",\"state_topic\":\"DtBoard/" BOARD_IDENTIFIER "/status\",\"device\":{\"identifiers\":\"" BOARD_IDENTIFIER "\",\"manufacturer\":\"" BOARD_MANUFACTURER "\",\"model\":\"" BOARD_MODEL "\",\"name\":\"" BOARD_NAME "\",\"sw_version\":\"" BOARD_SW_VERSION_PRINT "\"}}"));
            // debug(AT);
            /*
                    // heartbeat

                    // const char* data = PSTR("{\"~\":\"" MQTT_ROOT_TOPIC "\"/\"" BOARD_IDENTIFIER "\",\"uniq_id\":" BOARD_IDENTIFIER "-heartbeat\",\"name\":\"heartbeat\",\"stat_t\":\"~/heartbeat\",\"dev\":{\"ids\" : \"" BOARD_IDENTIFIER "\"}}") ;

                    doc.clear();
                    doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
                    doc["uniq_id"] = F(BOARD_IDENTIFIER "-heartbeat");
                    doc["name"] = F("heartbeat");
                    doc["stat_t"] = F("~/heartbeat");
                    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

                    serializeJson(doc, payload, sizeof(payload));
                    // Serial.println(payload);
                    strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/heartbeat/config"), max_topic);
                    // DT_mqtt_send(topic, PSTR("{\"~\":\"" MQTT_ROOT_TOPIC "\"/\"" BOARD_IDENTIFIER "\",\"uniq_id\":" BOARD_IDENTIFIER "-heartbeat\",\"name\":\"heartbeat\",\"stat_t\":\"~/heartbeat\",\"dev\":{\"ids\" : \"" BOARD_IDENTIFIER "\"}}"));
                    // DT_mqtt_send(topic, payload);
            */
            break;
#ifdef OUF
        case 1:
            if (num < RELAY_NUM)
            {

                // snprintf_P(topic, max_topic, PSTR("homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config"), num + 1);
                snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/DTB02-001\",\"uniq_id\":\"" BOARD_IDENTIFIER "-relay-%02d\",\"name\":\"relay-%02d\",\"command_topic\":\"~/relay-%02d/set\",\"stat_t\":\"~/relay-%02d/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1, num + 1);
                // DT_mqtt_send(topic, payload);
                DT_mqtt_send(F("homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config"), num + 1, payload);
                num++;
                sequance--;
            }
            else
            {
                num = 0;
            }
            break;

        case 2: // input
            if (num < INPUT_NUM)
            {
                // sprintf_P(topic, PSTR("homeassistant/binary_sensor/" BOARD_IDENTIFIER "/input-%02d/config"), num + 1);
                // snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-input-%02d\",\"name\":\"input-%02d\",\"stat_t\":\"~/input-%02d/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1);
                // DT_mqtt_send(topic, payload);
                DT_mqtt_send(F("homeassistant/binary_sensor/" BOARD_IDENTIFIER "/input-%02d/config"), num + 1, F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-input-%02d\",\"name\":\"input-%02d\",\"stat_t\":\"~/input-%02d/state\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1);

                num++;
                sequance--;
            }
            else
            {
                num = 0;
            }
            break;
            // #ifdef COMMENT
        case 3:
#ifdef TEMP_NUM
            // PT100
            if (num < TEMP_NUM)
            {
                // snprintf_P(topic, max_topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pt100-%02d/config"), num + 1);
                // snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pt100-%02d\",\"name\":\"pt100-%02d\",\"stat_t\":\"~/pt100-%02d/temperature\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1);
                // DT_mqtt_send(topic, payload);
                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/pt100-%02d/config"), num + 1, F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pt100-%02d\",\"name\":\"pt100-%02d\",\"stat_t\":\"~/pt100-%02d/temperature\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1);

                num++;
                sequance--;
            }
            else
            {
                num = 0;
            }
#endif // PT100
            break;
        case 4:
            // BME280 temperature
            for (uint8_t num = 0; num < BME280_NUM; ++num)
            {
                // snprintf_P(topic, max_topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-temperature-%02d/config"), num + 1);
                // snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-temperature-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/temperature\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1);
                // DT_mqtt_send(topic, payload);
                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-temperature-%02d/config"), num + 1, F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-temperature-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/temperature\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1);
            }
            break;
        case 5:
            // BME280 humidity
            for (uint8_t num = 0; num < BME280_NUM; ++num)
            {
                // snprintf_P(topic, max_topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-humidity-%02d/config"), num + 1);
                // snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-humidity-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/humidity\",\"dev_cla\":\"humidity\",\"unit_of_meas\":\"%%\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1);
                // DT_mqtt_send(topic, payload);
                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-humidity-%02d/config"), num + 1, F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-humidity-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/humidity\",\"dev_cla\":\"humidity\",\"unit_of_meas\":\"%%\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1);
            }
            break;
        case 6:
            // BME280 pressure
            for (uint8_t num = 0; num < BME280_NUM; ++num)
            {
                // snprintf_P(topic, max_topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-pressure-%02d/config"), num + 1);
                // snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-pressure-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/pressure\",\"dev_cla\":\"pressure\",\"unit_of_meas\":\"Pa\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1);
                // DT_mqtt_send(topic, payload);
                DT_mqtt_send(F("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-pressure-%02d/config"), num + 1, F("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-bme280-pressure-%02d\",\"name\":\"BME280-%02d\",\"stat_t\":\"~/bme280-%02d/pressure\",\"dev_cla\":\"pressure\",\"unit_of_meas\":\"Pa\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1);
            }
            break;
        case 7:
            // CCS811 CO2
            for (uint8_t num = 0; num < CCS811_NUM; ++num)
            {
                snprintf_P(topic, max_topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-co2-%02d/config"), num + 1);
                snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ccs811-co2-%02d\",\"name\":\"ccs811-%02d\",\"stat_t\":\"~/ccs811-%02d/co2\",\"dev_cla\":\"carbon_dioxide\",\"unit_of_meas\":\"CO2\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1);

                DT_mqtt_send(topic, payload);
                DT_mqtt_send(F, F);
            }

            break;
        case 8:

            // CCS811 COV
            for (uint8_t num = 0; num < CCS811_NUM; ++num)
            {
                snprintf_P(topic, max_topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-cov-%02d/config"), num + 1);
                snprintf_P(payload, max_payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ccs811-cov-%02d\",\"name\":\"ccs811-%02d\",\"stat_t\":\"~/ccs811-%02d/cov\",\"dev_cla\":\"pm10\",\"unit_of_meas\":\"ppm\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), num + 1, num + 1, num + 1);
                DT_mqtt_send(topic, payload);
                DT_mqtt_send(F, F);
            }
            break;

            // #ifdef COMMENT
        case 9:
#ifdef POELE
            // Poele mode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/poele-mode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-mode\",\"name\":\"mode pcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);
#endif // POELE
            break;
        case 10:
#ifdef POELE
            //  V1 consigne poêle en mode force (70°C)
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V1/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V1\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V1\",\"name\":\"parametre poêle (V1)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"max\":85,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);
#endif // POELE
            break;
        case 11:
#ifdef POELE
            // C7
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C7/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C7\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C7\",\"name\":\"Band morte Poele  (C7)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"min\":-100,\"max\":100,\"step\":1,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // POELE
            break;
        case 12:
#ifdef POELE
            // C4
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C4/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C4\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C4\",\"name\":\"consigne Jacuzzi (C4)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // POELE
            break;

            // #ifdef COMMENT
        case 13:
#ifdef POELE
            // C5
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C5/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C5\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C5\",\"name\":\"consigne ECS1 & ECS2 (C5)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // POELE
            break;

            // #ifdef COMMENT
        case 14:
#ifdef POELE
            // C6
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C6/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/C6\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C6\",\"name\":\"consigne mode boost (C6)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // POELE
            break;

        case 15:
#ifdef VANNES
            // 3 voies PCBT mode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-mode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-mode\",\"name\":\"mode pcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 16:
#ifdef VANNES
            // consigne vanne 3 voies PCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C2/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C2\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C2\",\"name\":\"consigne PCBT (C2)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":38,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 17:
#ifdef VANNES
            // 3 voies MCBT mode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-mode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/mode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-mode\",\"name\":\"mode mcbt\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"Demmarage\",\"Normal\",\"Manuel\",\"Arret\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);
#endif // VANNES
            break;

        case 18:
#ifdef VANNES
            // consigne vanne 3 voies MCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C3/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C3\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C3\",\"name\":\"consigne MCBT (C3)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 19:
#ifdef VANNES
            // C8
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C8/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C8\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C8\",\"name\":\"consigne Temp PCBT a -10°C (C8)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 20:
#ifdef VANNES
            // C9
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C9/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/C9\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C9\",\"name\":\"consigne Temp PCBT a +10°C (C9)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 21:
#ifdef VANNES
            // C10
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C10/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C10\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C10\",\"name\":\"consigne Temp MCBT a -10°C (C10)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 22:
#ifdef VANNES
            // C11
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C11/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/C11\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C11\",\"name\":\"consigne Temp MCBT a +10°C (C11)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);
#endif // VANNE
            break;
        case 23:

#ifdef VANNES

            // C_PCBT_MIN
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MIN/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/min_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_PCBT_MIN\",\"name\":\"consigne Temp PCBT minimum (C_PCBT_MIN)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 24:
#ifdef VANNES
            // C_PCBT_MAX
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MAX/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/max_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_PCBT_MAX\",\"name\":\"consigne Temp PCBT maximum (C_PCBT_MAX)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 25:
#ifdef VANNES
            // C_MCBT_MIN
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MIN/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/min_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_MCBT_MIN\",\"name\":\"consigne Temp MCBT minimum (C_MCBT_MIN)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 26:
#ifdef VANNES
            // C_MCBT_MAX
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MAX/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/max_temp\",\"uniq_id\":\"" BOARD_IDENTIFIER "-C_MCBT_MAX\",\"name\":\"consigne Temp MCBT maximum (C_MCBT_MAX)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;
        case 27:
#ifdef VANNES
            // KP_PCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KP_PCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KP\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KP_PCBT\",\"name\":\"pid KP PCBT (KP_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 28:
#ifdef VANNES
            // KI_PCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KI_PCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KI_PCBT\",\"name\":\"pid KI PCBT (KI_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":1000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 29:
#ifdef VANNES
            // KD_PCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KD_PCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KD\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KD_PCBT\",\"name\":\"pid KD PCBT (KD_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 30:
#ifdef VANNES
            // KT_PCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KT_PCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/KT\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KT_PCBT\",\"name\":\"pid interval PCBT (en ms) (KT_PCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60000,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 31:
#ifdef VANNES
            // PID PCBT Action
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid_action/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_action\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_action\",\"name\":\"pcbt pid action \",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"direct\",\"reverse\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 32:
#ifdef VANNES
            // PID PCBT pMode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-pmode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_pmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_pmode\",\"name\":\"pcbt pid pmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"pOnError\",\"pOnMeas\",\"pOnErrorMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 33:
#ifdef VANNES
            // PID PCBT dMode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-dmode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_dmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_dmode\",\"name\":\"pcbt pid dmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"dOnError\",\"dOnMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 34:
#ifdef VANNES
            // PID PCBT dMode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-iawmode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/pid_iawmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-pid_iawmode\",\"name\":\"pcbt pid iawmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"iAwCondition\",\"iAwClamp\",\"iAwOff\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;
        case 35:
#ifdef VANNES
            // KP_MCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KP_MCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KP\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KP_MCBT\",\"name\":\"pid KP MCBT (KP_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 36:
#ifdef VANNES
            // KI_MCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KI_MCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KI\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KI_MCBT\",\"name\":\"pid KI MCBT (KI_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 37:
#ifdef VANNES
            // KD_MCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KD_MCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KD\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KD_MCBT\",\"name\":\"pid KD MCBT (KD_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":1000000,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 38:
#ifdef VANNES
            // KT_MCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KT_MCBT/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/KT\",\"uniq_id\":\"" BOARD_IDENTIFIER "-KT_MCBT\",\"name\":\"pid interval MCBT (en ms) (KT_MCBT)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":60000,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);
#endif // VANNES2

            break;
        case 39:
#ifdef VANNES

            // PID PCBT Action
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid_action/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_action\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_action\",\"name\":\"mcbt pid action \",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"direct\",\"reverse\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 40:
#ifdef VANNES
            // PID PCBT pMode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-pmode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_pmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_pmode\",\"name\":\"mcbt pid pmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"pOnError\",\"pOnMeas\",\"pOnErrorMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 41:
#ifdef VANNES
            // PID PCBT dMode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-dmode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_dmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_dmode\",\"name\":\"mcbt pid dmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"dOnError\",\"dOnMeas\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 42:
#ifdef VANNES
            // PID MCBT dMode
            strlcpy_P(topic, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-iawmode/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/pid_iawmode\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-pid_iawmode\",\"name\":\"mcbt pid iawmode\",\"command_topic\":\"~/set\",\"state_topic\":\"~/state\",\"options\":[\"iAwCondition\",\"iAwClamp\",\"iAwOff\"],\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 43:
#ifdef VANNES
            // PID PCBT
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-p/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-P\",\"name\":\"PCBT P\",\"stat_t\":\"~/P\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 44:
#ifdef VANNES
            // PID PCBT I
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-i/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-I\",\"name\":\"PCBT I\",\"stat_t\":\"~/I\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 45:
#ifdef VANNES
            // PID PCBT D
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-d/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-D\",\"name\":\"PCBT D\",\"stat_t\":\"~/D\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 46:
#ifdef VANNES
            // PID PCBT OUT
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-out/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-pcbt-out\",\"name\":\"PCBT out\",\"stat_t\":\"~/OUT\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 47:
#ifdef VANNES
            // PID MCBT P
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-p/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-P\",\"name\":\"MCBT P\",\"stat_t\":\"~/P\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 48:
#ifdef VANNES
            // PID MCBT I
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-i/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-I\",\"name\":\"MCBT I\",\"stat_t\":\"~/I\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 49:
#ifdef VANNES
            // PID MCBT D
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-d/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-D\",\"name\":\"MCBT D\",\"stat_t\":\"~/D\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 50:
#ifdef VANNES
            // PID MCBT OUT
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-out/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt\",\"uniq_id\":\"" BOARD_IDENTIFIER "-mcbt-out\",\"name\":\"MCBT out\",\"stat_t\":\"~/OUT\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES2
            break;
        case 51:
#ifdef VANNES
            // RATIO PCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/ratio-pcbt/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/ratio\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ratio-pcbt\",\"name\":\"Ratio PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":10,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 52:
#ifdef VANNES
            // RATIO MCBT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/ratio-mcbt/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/ratio\",\"uniq_id\":\"" BOARD_IDENTIFIER "-ratio-mcbt\",\"name\":\"Ratio MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":10,\"step\":0.01,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 53:
#ifdef VANNES
            // Offset PCBT OUT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-pcbt-out/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/offset-out\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-pcbt-out\",\"name\":\"Decalage Sortie PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-32768,\"max\":32767,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 54:
#ifdef VANNES
            // Offset MCBT OUT
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-mcbt-out/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/offset-out\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-mcbt-out\",\"name\":\"Decalage Sortie MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":65534,\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 55:
#ifdef VANNES
            // Offset PCBT IN
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-pcbt-in/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/pcbt/offset-in\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-pcbt-in\",\"name\":\"Decalage consigne PCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNES
            break;

        case 56:
#ifdef VANNES
            // Offset MCBT IN
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-mcbt-in/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/mcbt/offset-in\",\"uniq_id\":\"" BOARD_IDENTIFIER "-offset-mcbt-in\",\"name\":\"Decalage consigne MCBT\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":-100,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

#endif // VANNE
            break;
        case 57:

            // V2
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V2/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V2\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V2\",\"name\":\"Reserve chaleur Ballon (V2)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

            break;
        case 58:
            // V3
            strlcpy_P(topic, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V3/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "/V3\",\"uniq_id\":\"" BOARD_IDENTIFIER "-V3\",\"name\":\"Temp Demi plage Morte (V3)\",\"stat_t\":\"~/state\",\"command_topic\":\"~/set\",\"min\":0,\"max\":100,\"step\":0.01,\"dev_cla\":\"temperature\",\"unit_of_meas\":\"°C\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

            break;
        case 59:
            // load 1s
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1s/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-load_1s\",\"name\":\"Load 1s\",\"stat_t\":\"~/load_1s\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

            break;
        case 60:
            // load 1m
            strlcpy_P(topic, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1m/config"), max_topic);
            strlcpy_P(payload, PSTR("{\"~\":\"DtBoard/" BOARD_IDENTIFIER "\",\"uniq_id\":\"" BOARD_IDENTIFIER "-load_1m\",\"name\":\"Load 1m\",\"stat_t\":\"~/load_1m\",\"dev\":{\"ids\":\"" BOARD_IDENTIFIER "\"}}"), max_payload);
            DT_mqtt_send(topic, payload);

            break;
#endif //ouf
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

void MQTT_data_store_P(MQTT_data &self, const __FlashStringHelper *Topic, const char *Payload)
{
    self._type = ha_flash_cstr;
    self._topic = Topic;
    self._fcstr = reinterpret_cast<const __FlashStringHelper *>(Payload);
};

void MQTT_data_store_P(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const char *Payload)
{
    self._type = ha_flash_cstr_tsprintf;
    self._num_t = num_t;
    self._topic = Topic;
    self._fcstr = reinterpret_cast<const __FlashStringHelper *>(Payload);
};

void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const __FlashStringHelper *Payload)
{
    self._type = ha_flash_cstr;
    self._topic = Topic;
    self._fcstr = Payload;
};

void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const __FlashStringHelper *Payload)
{
    self._type = ha_flash_cstr_tsprintf;
    self._num_t = num_t;
    self._topic = Topic;
    self._fcstr = Payload;
};

void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, char const *Payload)
{
    debug(F(AT));
    self._type = ha_cstr;
    self._topic = Topic;
    self._cstr = strdup(Payload);
};
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, char const *Payload)
{
    debug(F(AT));
    self._type = ha_cstr_tsprintf;
    self._num_t = num_t;
    self._topic = Topic;
    self._cstr = strdup(Payload);
};

void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const float Payload)
{
    self._type = ha_float;
    self._topic = Topic;
    self._float = Payload;
};
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const float Payload)
{
    self._type = ha_float_tsprintf;
    self._num_t = num_t;
    self._topic = Topic;
    self._float = Payload;
};

// void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const uint8_t Payload)
// {
//     self._type = ha_int32_t;
//     self._topic = Topic;
//     self._int = Payload;
// };
// void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const uint8_t Payload)
// {
//     self._type = ha_int32_t_tsprintf;
//     self._num_t = num_t;
//     self._topic = Topic;
//     self._int = Payload;
// };

void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const uint16_t Payload)
{
    self._type = ha_int32_t;
    self._topic = Topic;
    self._int = Payload;
};
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const uint16_t Payload)
{
    self._type = ha_int32_t_tsprintf;
    self._num_t = num_t;
    self._topic = Topic;
    self._int = Payload;
};

void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, const uint32_t Payload)
{
    self._type = ha_int32_t;
    self._topic = Topic;
    self._int = Payload;
};
void MQTT_data_store(MQTT_data &self, const __FlashStringHelper *Topic, uint8_t num_t, const uint32_t Payload)
{
    self._type = ha_int32_t_tsprintf;
    self._num_t = num_t;
    self._topic = Topic;
    self._int = Payload;
};

void MQTT_data_get(MQTT_data &self, char *topic, int topic_len, char *payload, int payload_len)
{
    switch (self._type)
    {
    case ha_flash_cstr:
        strncpy_P(topic, reinterpret_cast<const char *>(self._topic), topic_len);
        strncpy_P(payload, reinterpret_cast<const char *>(self._fcstr), payload_len);
        break;

    case ha_cstr:
        strncpy_P(topic, reinterpret_cast<const char *>(self._topic), topic_len);
        strncpy(payload, self._cstr, payload_len);
        free((void *)self._cstr);
        /* code */
        break;

    case ha_float:
        strncpy_P(topic, reinterpret_cast<const char *>(self._topic), topic_len);
        dtostrf(self._float, 1, 2, payload);
        break;

    case ha_int32_t:
        strncpy_P(topic, reinterpret_cast<const char *>(self._topic), topic_len);
        snprintf_P(payload, payload_len, PSTR("%i"), (int)self._int);
        break;

    case ha_flash_cstr_tsprintf:
        snprintf_P(topic, topic_len, reinterpret_cast<const char *>(self._topic), self._num_t);
        strncpy_P(payload, reinterpret_cast<const char *>(self._fcstr), payload_len);
        break;

    case ha_cstr_tsprintf:
        snprintf_P(topic, topic_len, reinterpret_cast<const char *>(self._topic), self._num_t);
        strncpy(payload, self._cstr, payload_len);
        free((void *)self._cstr);
        /* code */
        break;

    case ha_float_tsprintf:
        snprintf_P(topic, topic_len, reinterpret_cast<const char *>(self._topic), self._num_t);
        dtostrf(self._float, 1, 2, payload);
        break;

    case ha_int32_t_tsprintf:
        snprintf_P(topic, topic_len, reinterpret_cast<const char *>(self._topic), self._num_t);
        snprintf_P(payload, payload_len, PSTR("%i"), (int)self._int);
        break;

    default:
        break;
    }
};

void MQTT_data_debug(MQTT_data &self)
{
    char topic[64];
    char payload[32];
    MQTT_data_get(self, topic, 64, payload, 32);
    Serial.print(topic);
    Serial.print(F("-->"));
    Serial.println(payload);
}

#endif // MQTT