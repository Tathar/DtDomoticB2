#include <DT_ha.h>

StaticJsonDocument<256> doc;
char buffer[BUFFER_SIZE];
char buffer_value[BUFFER_VALUE_SIZE];

// const uint8_t HA_JSON[RELAY_NUM] PROGMEM = {23, 25, 27, 29, 31, 33, 35, 37, 22, 24, 26, 28, 30, 32, 34, 36, 100, 101, 102, 103, 104, 105, 106, 107, 200, 201, 202, 203, 204, 205, 206, 207};

void homeassistant(bool start)
{
    uint32_t now = millis();
    static uint8_t sequance = 0;
    JsonArray options;
    static uint8_t num = 0;

    wdt_reset();
    if (start)
    {
        sequance = 0;
        return;
    }

    if (sequance <= 16)
    {
        Serial.print(F("homeassistant"));
    }

    switch (sequance)
    {
    case 0:
        // online
        //  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), BUFFER_SIZE);
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-status");
        doc["name"] = F("status");
        doc["stat_t"] = F("~/status");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        // JsonObject connection = doc["device"].createNestedArray("connection").createNestedObject();
        // sprintf_P(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4,PSTR( MAC5), MAC6);
        // connection["mac"] = buffer_value;
        doc["dev"]["mf"] = F(BOARD_MANUFACTURER);     // manufacturer
        doc["dev"]["mdl"] = F(BOARD_MODEL);           // model
        doc["dev"]["name"] = F(BOARD_NAME);           // name
        doc["dev"]["sw"] = F(BOARD_SW_VERSION_PRINT); //  software version
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/status/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // heartbeat

        // const char* data = PSTR("{\"~\":\"" MQTT_ROOT_TOPIC "\"/\"" BOARD_IDENTIFIER "\",\"uniq_id\":" BOARD_IDENTIFIER "-heartbeat\",\"name\":\"heartbeat\",\"stat_t\":\"~/heartbeat\",\"dev\":{\"ids\" : \"" BOARD_IDENTIFIER "\"}}") ;

        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-heartbeat");
        doc["name"] = F("heartbeat");
        doc["stat_t"] = F("~/heartbeat");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/heartbeat/config"), BUFFER_SIZE);
        // DT_mqtt_send(buffer, PSTR("{\"~\":\"" MQTT_ROOT_TOPIC "\"/\"" BOARD_IDENTIFIER "\",\"uniq_id\":" BOARD_IDENTIFIER "-heartbeat\",\"name\":\"heartbeat\",\"stat_t\":\"~/heartbeat\",\"dev\":{\"ids\" : \"" BOARD_IDENTIFIER "\"}}"));
        // DT_mqtt_send(buffer, buffer_value);

        break;

    case 1:
        // relay
        // for (uint8_t num = 0; num < RELAY_NUM; ++num)
        if (num < RELAY_NUM)
        {
            Serial.print(F(" relay"));
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-relay-%02d"), num + 1);
            doc["uniq_id"] = buffer_value; // unique_id
            sprintf_P(buffer_value, PSTR("relay-%02d"), num + 1);
            doc["name"] = buffer_value; // name
            sprintf_P(buffer_value, PSTR("~/relay-%02d/set"), num + 1);
            doc["command_topic"] = buffer_value;
            sprintf_P(buffer_value, PSTR("~/relay-%02d/state"), num + 1);
            doc["stat_t"] = buffer_value;            // state topic
            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            // Serial.println(buffer_value);
            serializeJson(doc, buffer_value, sizeof(buffer_value));
            DT_mqtt_send(buffer, buffer_value);
            num++;
            sequance--;
        }
        else
        {
            num = 0;
        }
        break;
    case 2: // input
        // for (uint8_t num = 0; num < INPUT_NUM; ++num)
        if (num < INPUT_NUM)
        {
            Serial.print(F(" input"));
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/binary_sensor/" BOARD_IDENTIFIER "/input-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-input-%02d"), num + 1);
            doc["uniq_id"] = buffer_value; // unique_id
            sprintf_P(buffer_value, PSTR("input-%02d"), num + 1);
            doc["name"] = buffer_value; // name
            sprintf_P(buffer_value, PSTR("~/input-%02d/state"), num + 1);
            doc["stat_t"] = buffer_value; // state topic

            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            // Serial.println(buffer_value);
            serializeJson(doc, buffer_value, sizeof(buffer_value));
            DT_mqtt_send(buffer, buffer_value);
            num++;
            sequance--;
        }
        else
        {
            num = 0;
        }
        break;
    case 3:
#ifdef TEMP_NUM
        // PT100
        // for (uint8_t num = 0; num < TEMP_NUM; ++num)
        if (num < TEMP_NUM)
        {
            Serial.print(F(" temp"));
            wdt_reset();
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pt100-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-pt100-%02d"), num + 1);
            doc["uniq_id"] = buffer_value;
            sprintf_P(buffer_value, PSTR("pt100-%02d"), num + 1);
            doc["name"] = buffer_value;
            sprintf_P(buffer_value, PSTR("~/pt100-%02d/temperature"), num + 1);
            doc["stat_t"] = buffer_value;
            doc["dev_cla"] = F("temperature");
            doc["unit_of_meas"] = F("°C");
            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            serializeJson(doc, buffer_value, sizeof(buffer_value));
            // Serial.println(buffer_value);
            DT_mqtt_send(buffer, buffer_value);
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
            wdt_reset();
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-temperature-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-bme280-temperature-%02d"), num + 1);
            doc["uniq_id"] = buffer_value;
            sprintf_P(buffer_value, PSTR("BME280-%02d"), num + 1);
            doc["name"] = buffer_value;
            sprintf_P(buffer_value, PSTR("~/bme280-%02d/temperature"), num + 1);
            doc["stat_t"] = buffer_value;
            doc["dev_cla"] = F("temperature");
            doc["unit_of_meas"] = F("°C");

            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            serializeJson(doc, buffer_value, sizeof(buffer_value));
            // Serial.println(buffer_value);
            DT_mqtt_send(buffer, buffer_value);
        }
        break;
    case 5:
        // BME280 humidity
        for (uint8_t num = 0; num < BME280_NUM; ++num)
        {
            wdt_reset();
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-humidity-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-bme280-humidity-%02d"), num + 1);
            doc["uniq_id"] = buffer_value;
            sprintf_P(buffer_value, PSTR("BME280-%02d"), num + 1);
            doc["name"] = buffer_value;
            sprintf_P(buffer_value, PSTR("~/bme280-%02d/humidity"), num + 1);
            doc["stat_t"] = buffer_value;
            doc["dev_cla"] = F("humidity");
            doc["unit_of_meas"] = F("%");

            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            serializeJson(doc, buffer_value, sizeof(buffer_value));
            // Serial.println(buffer_value);
            DT_mqtt_send(buffer, buffer_value);
        }
        break;
    case 6:
        // BME280 pressure
        for (uint8_t num = 0; num < BME280_NUM; ++num)
        {
            wdt_reset();
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-pressure-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-bme280-pressure-%02d"), num + 1);
            doc["uniq_id"] = buffer_value;
            sprintf_P(buffer_value, PSTR("BME280-%02d"), num + 1);
            doc["name"] = buffer_value;
            sprintf_P(buffer_value, PSTR("~/bme280-%02d/pressure"), num + 1);
            doc["stat_t"] = buffer_value;
            doc["dev_cla"] = F("pressure");
            doc["unit_of_meas"] = F("Pa");

            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            serializeJson(doc, buffer_value, sizeof(buffer_value));
            // Serial.println(buffer_value);
            DT_mqtt_send(buffer, buffer_value);
        }
        break;
    case 7:
        // CCS811 CO2
        for (uint8_t num = 0; num < CCS811_NUM; ++num)
        {
            wdt_reset(); // clear watchdog
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-co2-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-ccs811-co2-%02d"), num + 1);
            doc["uniq_id"] = buffer_value;
            sprintf_P(buffer_value, PSTR("ccs811-%02d"), num + 1);
            doc["name"] = buffer_value;
            sprintf_P(buffer_value, PSTR("~/ccs811-%02d/co2"), num + 1);
            doc["stat_t"] = buffer_value;
            doc["dev_cla"] = F("carbon_dioxide");
            doc["unit_of_meas"] = F("CO2");

            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            serializeJson(doc, buffer_value, sizeof(buffer_value));
            // Serial.println(buffer_value);
            DT_mqtt_send(buffer, buffer_value);
        }

        break;
    case 8:

        // CCS811 COV
        for (uint8_t num = 0; num < CCS811_NUM; ++num)
        {
            wdt_reset(); // clear watchdog
            doc.clear();
            sprintf_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-cov-%02d/config"), num + 1);
            doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
            sprintf_P(buffer_value, PSTR(BOARD_IDENTIFIER "-ccs811-cov-%02d"), num + 1);
            doc["uniq_id"] = buffer_value;
            sprintf_P(buffer_value, PSTR("ccs811-%02d"), num + 1);
            doc["name"] = buffer_value;
            sprintf_P(buffer_value, PSTR("~/ccs811-%02d/cov"), num + 1);
            doc["stat_t"] = buffer_value;
            doc["dev_cla"] = F("pm10");
            doc["unit_of_meas"] = F("ppm");

            doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

            serializeJson(doc, buffer_value, sizeof(buffer_value));
            // Serial.println(buffer_value);
            DT_mqtt_send(buffer, buffer_value);
        }
        break;
    case 9:
#ifdef POELE
        // Poele mode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-poele-mode");
        doc["name"] = F("mode poele");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("Arret");
        options.add("Normal");
        options.add("ECS");
        options.add("Forcé");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/poele-mode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // EEPROM
        //  V1 consigne poêle en mode force (70°C)
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-V1");
        doc["name"] = F("parametre poêle (V1)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["max"] = POELE_MAX_TEMPERATURE;
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V1/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C7
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C7");
        doc["name"] = F("Band morte Poele  (C7)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["min"] = -100;
        doc["max"] = 100;
        doc["step"] = 1;
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C7/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
        // C4
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C4");
        doc["name"] = F("consigne Jacuzzi (C4)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C4/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C5
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C5");
        doc["name"] = F("consigne ECS1 & ECS2 (C5)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C5/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C6
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C6");
        doc["name"] = F("consigne mode boost (C6)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C6/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

#endif // POELE
        break;

    case 10:
#ifdef VANNES
        // 3 voies PCBT mode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-mode");
        doc["name"] = F("mode pcbt");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("Demmarage");
        options.add("Normal");
        options.add("Manuel");
        options.add("Arret");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-mode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // consigne vanne 3 voies PCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C2");
        doc["name"] = F("consigne PCBT (C2)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = TMP_EAU_PCBT_MAX;
        doc["step"] = 0.01;
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C2/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // 3 voies MCBT mode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-mode");
        doc["name"] = F("mode mcbt");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("Demmarage");
        options.add("Normal");
        options.add("Manuel");
        options.add("Arret");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-mode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // consigne vanne 3 voies MCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C3");
        doc["name"] = F("consigne MCBT (C3)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = TMP_EAU_MCBT_MAX;
        doc["step"] = 0.01;
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C3/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
        // C8
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C8");
        doc["name"] = F("consigne Temp PCBT a -10°C (C8)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C8/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C9
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C9");
        doc["name"] = F("consigne Temp PCBT a +10°C (C9)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C9/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C10
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C10");
        doc["name"] = F("consigne Temp MCBT a -10°C (C10)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C10/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C11
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C11");
        doc["name"] = F("consigne Temp MCBT a +10°C (C11)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C11/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
#endif // VANNE
        break;
    case 11:

#ifdef VANNES

        // C_PCBT_MIN
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_PCBT_MIN");
        doc["name"] = F("consigne Temp PCBT minimum (C_PCBT_MIN)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MIN/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C_PCBT_MAX
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_PCBT_MAX");
        doc["name"] = F("consigne Temp PCBT maximum (C_PCBT_MAX)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MAX/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C_MCBT_MIN
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_MCBT_MIN");
        doc["name"] = F("consigne Temp MCBT minimum (C_MCBT_MIN)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MIN/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // C_MCBT_MAX
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_MCBT_MAX");
        doc["name"] = F("consigne Temp MCBT maximum (C_MCBT_MAX)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MAX/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
#endif // VANNES
        break;
    case 12:
#ifdef VANNES
        // KP_PCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KP_PCBT");
        doc["name"] = F("pid KP PCBT (KP_PCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 100000;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KP_PCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // KI_PCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KI_PCBT");
        doc["name"] = F("pid KI PCBT (KI_PCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 1000;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KI_PCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // KD_PCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KD_PCBT");
        doc["name"] = F("pid KD PCBT (KD_PCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 100000;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KD_PCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // KT_PCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KT_PCBT");
        doc["name"] = F("pid interval PCBT (en ms) (KT_PCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 60000;
        // doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KT_PCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT Action
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-pid_action");
        doc["name"] = F("pcbt pid action ");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("direct");
        options.add("reverse");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid_action/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT pMode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-pid_pmode");
        doc["name"] = F("pcbt pid pmode");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("pOnError");
        options.add("pOnMeas");
        options.add("pOnErrorMeas");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-pmode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT dMode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-pid_dmode");
        doc["name"] = F("pcbt pid dmode");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("dOnError");
        options.add("dOnMeas");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-dmode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT dMode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-pid_iawmode");
        doc["name"] = F("pcbt pid iawmode");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("iAwCondition");
        options.add("iAwClamp");
        options.add("iAwOff");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/pcbt-pid-iawmode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
#endif // VANNES
        break;
    case 13:
#ifdef VANNES
        // KP_MCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KP_MCBT");
        doc["name"] = F("pid KP MCBT (KP_MCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 100000;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KP_MCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // KI_MCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KI_MCBT");
        doc["name"] = F("pid KI MCBT (KI_MCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 100000;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KI_MCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // KD_MCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KD_MCBT");
        doc["name"] = F("pid KD MCBT (KD_MCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 1000000;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KD_MCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // KT_MCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-KT_MCBT");
        doc["name"] = F("pid interval MCBT (en ms) (KT_MCBT)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 60000;
        // doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KT_MCBT/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
#endif // VANNES

        break;
    case 14:
#ifdef VANNES

        // PID PCBT Action
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-pid_action");
        doc["name"] = F("mcbt pid action ");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("direct");
        options.add("reverse");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid_action/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT pMode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-pid_pmode");
        doc["name"] = F("mcbt pid pmode");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("pOnError");
        options.add("pOnMeas");
        options.add("pOnErrorMeas");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-pmode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT dMode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-pid_dmode");
        doc["name"] = F("mcbt pid dmode");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("dOnError");
        options.add("dOnMeas");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-dmode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID MCBT dMode
        wdt_reset(); // clear watchdog
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-pid_iawmode");
        doc["name"] = F("mcbt pid iawmode");
        doc["command_topic"] = F("~/set");
        doc["state_topic"] = F("~/state");

        options = doc.createNestedArray("options");
        options.add("iAwCondition");
        options.add("iAwClamp");
        options.add("iAwOff");

        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/mcbt-pid-iawmode/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT P
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-P");
        doc["name"] = F("PCBT P");
        doc["stat_t"] = F("~/P");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-p/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT I
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-I");
        doc["name"] = F("PCBT I");
        doc["stat_t"] = F("~/I");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-i/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT D
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-D");
        doc["name"] = F("PCBT D");
        doc["stat_t"] = F("~/D");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-d/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID PCBT OUT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-pcbt-out");
        doc["name"] = F("PCBT out");
        doc["stat_t"] = F("~/OUT");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/pcbt-pid-out/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID MCBT P
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-P");
        doc["name"] = F("MCBT P");
        doc["stat_t"] = F("~/P");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-p/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID MCBT I
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-I");
        doc["name"] = F("MCBT I");
        doc["stat_t"] = F("~/I");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-i/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID MCBT D
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-D");
        doc["name"] = F("MCBT D");
        doc["stat_t"] = F("~/D");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-d/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // PID MCBT OUT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-mcbt-out");
        doc["name"] = F("MCBT out");
        doc["stat_t"] = F("~/OUT");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/mcbt-pid-out/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
#endif // VANNES
        break;
    case 15:
#ifdef VANNES
        // RATIO PCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-ratio-pcbt");
        doc["name"] = F("Ratio PCBT");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 10;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/ratio-pcbt/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // RATIO MCBT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-ratio-mcbt");
        doc["name"] = F("Ratio MCBT");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 10;
        doc["step"] = 0.01;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/ratio-mcbt/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // Offset PCBT OUT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-offset-pcbt-out");
        doc["name"] = F("Decalage Sortie PCBT");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = -32768;
        doc["max"] = 32767;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-pcbt-out/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // Offset MCBT OUT
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-offset-mcbt-out");
        doc["name"] = F("Decalage Sortie MCBT");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 65534;
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-mcbt-out/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // Offset PCBT IN
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-offset-pcbt-in");
        doc["name"] = F("Decalage consigne PCBT");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = -100;
        doc["max"] = 100;
        doc["step"] = 0.01;
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-pcbt-in/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // Offset MCBT IN
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-offset-mcbt-in");
        doc["name"] = F("Decalage consigne MCBT");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = -100;
        doc["max"] = 100;
        doc["step"] = 0.01;
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/offset-mcbt-in/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);
#endif // VANNE
        break;
    case 16:

        // V2
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-V2");
        doc["name"] = F("Reserve chaleur Ballon (V2)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V2/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // V3
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3");
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-V3");
        doc["name"] = F("Temp Demi plage Morte (V3)");
        doc["stat_t"] = F("~/state");
        doc["command_topic"] = F("~/set");
        doc["min"] = 0;
        doc["max"] = 100;
        doc["step"] = 0.01;
        doc["dev_cla"] = F("temperature");
        doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V3/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // load 1s
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-load_1s");
        doc["name"] = F("Load 1s");
        doc["stat_t"] = F("~/load_1s");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1s/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        // load 1m
        wdt_reset();
        doc.clear();
        doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
        doc["uniq_id"] = F(BOARD_IDENTIFIER "-load_1m");
        doc["name"] = F("Load 1m");
        doc["stat_t"] = F("~/load_1m");
        // doc["dev_cla"] = F("temperature");
        // doc["unit_of_meas"] = F("°C");
        doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
        serializeJson(doc, buffer_value, sizeof(buffer_value));
        // Serial.println(buffer_value);
        strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/load_1m/config"), BUFFER_SIZE);
        DT_mqtt_send(buffer, buffer_value);

        break;

    default:
        return;
        break;
    }
    sequance += 1;

    Serial.print(F(" = "));
    Serial.println(millis() - now);
}
