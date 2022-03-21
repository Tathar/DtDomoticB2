#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>

#include <DT_relay.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_mqtt.h>
#include <DT_BME280.h>
#include <DT_CCS811.h>
#include <DT_mcp.h>

// #include <DT_fake_ntc.h>
#include <DT_poele.h>
#include <DT_eeprom.h>

#include <avr/wdt.h> //watchdog

#include <ArduinoJson.h>

// #include <pinout.h>
#include <config.h>

StaticJsonDocument<256> doc;

// #include "Wire.h"
// #include "DFRobot_CCS811.h"
// DFRobot_CCS811 CCS811;

// Adafruit_CCS811 ccs811;

long int lastReconnectAttempt = 0;

#ifdef MQTT

char buffer[BUFFER_SIZE];
char buffer_value[BUFFER_VALUE_SIZE];

void homeassistant(void)
{
  JsonArray options;
  // online
  //  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), BUFFER_SIZE);
  wdt_reset();
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
  wdt_reset();
  doc.clear();
  doc["~"] = F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER);
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-heartbeat");
  doc["name"] = F("heartbeat");
  doc["stat_t"] = F("~/heartbeat");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
  // doc["dev"]["mf"] = F(BOARD_MANUFACTURER); // manufacturer
  // doc["dev"]["mdl"] = F(BOARD_MODEL);       // model
  // doc["dev"]["name"] = F(BOARD_NAME);       // name
  // doc["dev"]["sw"] = F(BOARD_SW_VERSION_PRINT);   // software version
  serializeJson(doc, buffer_value, sizeof(buffer_value));
  // Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/heartbeat/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  wdt_reset();
  // relay
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
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
  }

  // input
  for (uint8_t num = 0; num < INPUT_NUM; ++num)
  {
    wdt_reset();
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
  }

  // PT100
  for (uint8_t num = 0; num < TEMP_NUM; ++num)
  {
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
  }
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

#endif // POELE

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
}

// Relay Callback
void relay_callback(const uint8_t num, const bool action)
{
  wdt_reset();
  sprintf_P(buffer, PSTR("relais numero %d dans l etat %d"), num, (int)action);
  // auto Serial.println(buffer);
  sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/state"), num);
  if (action)
    DT_mqtt_send(buffer, "ON");
  else
    DT_mqtt_send(buffer, "OFF");
}
#endif // MQTT
void input_callback(const uint8_t num, const Bt_Action action)
{
  wdt_reset();
  Serial.print(F("entrée numero "));
  Serial.print(num);
  Serial.print(F(" dans l etat "));

#ifdef MQTT
  sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num);
#endif
  switch (action)
  {
  case IN_PUSHED:
    Serial.println("ON");
#ifdef MQTT
    DT_mqtt_send(buffer, "ON");
#endif
    break;

  case IN_RELEASE:
    Serial.println("OFF");
#ifdef MQTT
    DT_mqtt_send(buffer, "OFF");
#endif
    break;

  case IN_PUSH:
    Serial.println("PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "PUSH");
#endif
    break;

  case IN_LPUSH:
    Serial.println("LPUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "LPUSH");
#endif
    break;

  case IN_LLPUSH:
    Serial.println("LLPUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "LLPUSH");
#endif
    break;

  case IN_XLLPUSH:
    Serial.println("XLLPUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "XLLPUSH");
#endif
    break;

  case IN_2PUSH:
    Serial.println("2PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "2PUSH");
#endif
    break;

  case IN_L2PUSH:
    Serial.println("L2PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "L2PUSH");
#endif
    break;

  case IN_LL2PUSH:
    Serial.println("LL2PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "LL2PUSH");
#endif
    break;

  case IN_XLL2PUSH:
    Serial.println("XLL2PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "XLL2PUSH");
#endif
    break;

  case IN_3PUSH:
    Serial.println("3PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "3PUSH");
#endif
    break;

  case IN_L3PUSH:
    Serial.println("L3PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "L3PUSH");
#endif
    break;

  case IN_LL3PUSH:
    Serial.println("LL3PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "LL3PUSH");
#endif
    break;

  case IN_XLL3PUSH:
    Serial.println("XLL3PUSH");
#ifdef MQTT
    DT_mqtt_send(buffer, "XLL3PUSH");
#endif
    break;

  default:
    Serial.println(action);
    break;
  }
}

#ifdef MQTT
void pt100_callback(const uint8_t num, const float temp)
{
  wdt_reset();
  // Serial.print("PT100_CALLBACK ");
  sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d/temperature"), num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(temp);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  // Serial.print(buffer);
  // Serial.print(" -> ");
  // Serial.println(buffer_value);
  DT_mqtt_send(buffer, buffer_value);
}

void bme280_callback_temperature(const uint8_t num, const float temperature)
{
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/temperature"), num);
    JsonVariant variant = doc.to<JsonVariant>();
    variant.set(temperature);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}

void bme280_callback_humidity(const uint8_t num, const float humidity)
{
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/humidity"), num);
    JsonVariant variant = doc.to<JsonVariant>();
    variant.set(humidity);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}

void bme280_callback_pressure(const uint8_t num, const float pressure)
{
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/pressure"), num);
    JsonVariant variant = doc.to<JsonVariant>();
    variant.set(pressure);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}

void ccs811_callback_co2(const uint8_t num, const float co2)
{
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/co2"), num);
    JsonVariant variant = doc.to<JsonVariant>();
    variant.set(co2);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}

void ccs811_callback_cov(const uint8_t num, const float cov)
{
  wdt_reset();
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/cov"), num);
    JsonVariant variant = doc.to<JsonVariant>();
    variant.set(cov);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}

#ifdef POELE
void poele_mode_callback(const DT_Poele_mode mode)
{
  // mode poele
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), BUFFER_SIZE);
  switch (mode)
  {
  case DT_POELE_ARRET:
    DT_mqtt_send(buffer, "Arret");
    break;
  case DT_POELE_NORMAL:
    DT_mqtt_send(buffer, "Normal");
    break;
  case DT_POELE_ECS:
    DT_mqtt_send(buffer, "ECS");
    break;
  case DT_POELE_FORCE:
    DT_mqtt_send(buffer, "Forcé");
    break;
  }
}
#endif // POELE

#ifdef VANNES
void dt3voies_callback(const float C2, const float C3)
{

  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    JsonVariant variant = doc.to<JsonVariant>();
    int32_t digit = C2 * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);

    wdt_reset();
    variant = doc.to<JsonVariant>();
    digit = C3 * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}

// retour des valleur du PID PCBT
void dt3voies_callback_pid_pcbt(const float P, const float I, const float D, const float OUT)
{
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    JsonVariant variant = doc.to<JsonVariant>();
    int32_t digit = P * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/P"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);

    wdt_reset();
    variant = doc.to<JsonVariant>();
    digit = I * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/I"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);

    wdt_reset();
    variant = doc.to<JsonVariant>();
    digit = D * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/D"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);

    wdt_reset();
    variant = doc.to<JsonVariant>();
    digit = OUT * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/OUT"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}

// retour des valleur du PID MCBT
void dt3voies_callback_pid_mcbt(const float P, const float I, const float D, const float OUT)
{
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH)
  {
    refresh = now;
    JsonVariant variant = doc.to<JsonVariant>();
    int32_t digit = P * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/P"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);

    wdt_reset();
    variant = doc.to<JsonVariant>();
    digit = I * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/I"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);

    wdt_reset();
    variant = doc.to<JsonVariant>();
    digit = D * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/D"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);

    wdt_reset();
    variant = doc.to<JsonVariant>();
    digit = OUT * 100;
    variant.set((float)digit / 100.0);
    serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/OUT"), BUFFER_SIZE);
    DT_mqtt_send(buffer, buffer_value);
  }
}
#endif // VANNES

void mqtt_publish()
{
  wdt_reset();

  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/availability"), BUFFER_SIZE);
  DT_mqtt_send(buffer, "online");

  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    relay_callback(num + 1, DT_relay_get(num + 1));
  }

  for (uint8_t num = 0; num < INPUT_NUM; ++num)
  {
    wdt_reset();
    input_callback(num + 1, DT_input_get_stats(num + 1));
  }

  // PT100
  for (uint8_t num = 0; num < TEMP_NUM; ++num)
  {
    wdt_reset();
    pt100_callback(num + 1, DT_pt100_get(num + 1));
  }

#ifdef POELE
  // mode poele
  wdt_reset();
  poele_mode_callback(DT_Poele_get_mode());

  // EEPROM
  //  V1
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.V1);

  // V2
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.V2);

  // V3
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.V3);
#endif // POELE

#ifdef VANNES
  // 3 voies PCBT mode
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
  switch (DT_3voies_PCBT_get_mode())
  {
  case DT_3VOIES_DEMMARAGE:
    DT_mqtt_send(buffer, "Demmarage");
    break;
  case DT_3VOIES_NORMAL:
    DT_mqtt_send(buffer, "Normal");
    break;
  case DT_3VOIES_MANUAL:
    DT_mqtt_send(buffer, "Manuel");
    break;
  case DT_3VOIES_OFF:
    DT_mqtt_send(buffer, "Arret");
    break;
  }

  // 3 voies MCBT mode
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
  switch (DT_3voies_MCBT_get_mode())
  {
  case DT_3VOIES_DEMMARAGE:
    DT_mqtt_send(buffer, "Demmarage");
    break;
  case DT_3VOIES_NORMAL:
    DT_mqtt_send(buffer, "Normal");
    break;
  case DT_3VOIES_MANUAL:
    DT_mqtt_send(buffer, "Manuel");
    break;
  case DT_3VOIES_OFF:
    DT_mqtt_send(buffer, "Arret");
    break;
  }

  // consigne vanne 3 voies PCBT & MCBT
  dt3voies_callback(DT_3voies_get_C2(), DT_3voies_get_C3());

  // C4
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C4);

  // C5
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C5);

  // C6
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C6);

  // C7
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C7);

  // C8
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C8);

  // C9
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C9);

  // C10
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C10);

  // C11
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C11);

  // C_PCBT_MIN
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_PCBT_MIN);

  // C_PCBT_MAX
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_PCBT_MAX);

  // C_MCBT_MIN
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_MCBT_MIN);

  // C_MCBT_MAX
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_MCBT_MAX);

  // KP_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KP());

  // KI_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KI());

  // KD_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KD());

  // KT_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KT());
  // KP_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KP());

  // KI_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KI());

  // KD_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KD());

  // KT_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KT());

  // PID PCBT Action
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_pcbt.action)
  {
  case QuickPID::Action::direct:
    DT_mqtt_send(buffer, "direct");
    break;
  case QuickPID::Action::reverse:
    DT_mqtt_send(buffer, "reverse");
    break;
  }

  // PID PCBT pMode
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_pcbt.pmode)
  {
  case QuickPID::pMode::pOnError:
    DT_mqtt_send(buffer, "pOnError");
    break;
  case QuickPID::pMode::pOnMeas:
    DT_mqtt_send(buffer, "pOnMeas");
    break;
  case QuickPID::pMode::pOnErrorMeas:
    DT_mqtt_send(buffer, "pOnErrorMeas");
    break;
  }

  // PID PCBT dMode
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_pcbt.dmode)
  {
  case QuickPID::dMode::dOnError:
    DT_mqtt_send(buffer, "dOnError");
    break;
  case QuickPID::dMode::dOnMeas:
    DT_mqtt_send(buffer, "dOnMeas");
    break;
  }

  // PID PCBT iAwMode
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_pcbt.iawmode)
  {
  case QuickPID::iAwMode::iAwCondition:
    DT_mqtt_send(buffer, "iAwCondition");
    break;
  case QuickPID::iAwMode::iAwClamp:
    DT_mqtt_send(buffer, "iAwClamp");
    break;
  case QuickPID::iAwMode::iAwOff:
    DT_mqtt_send(buffer, "iAwOff");
    break;
  }

  // PID MCBT Action
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_mcbt.action)
  {
  case QuickPID::Action::direct:
    DT_mqtt_send(buffer, "direct");
    break;
  case QuickPID::Action::reverse:
    DT_mqtt_send(buffer, "reverse");
    break;
  }

  // PID MCBT pMode
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_mcbt.pmode)
  {
  case QuickPID::pMode::pOnError:
    DT_mqtt_send(buffer, "pOnError");
    break;
  case QuickPID::pMode::pOnMeas:
    DT_mqtt_send(buffer, "pOnMeas");
    break;
  case QuickPID::pMode::pOnErrorMeas:
    DT_mqtt_send(buffer, "pOnErrorMeas");
    break;
  }

  // PID MCBT dMode
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_mcbt.dmode)
  {
  case QuickPID::dMode::dOnError:
    DT_mqtt_send(buffer, "dOnError");
    break;
  case QuickPID::dMode::dOnMeas:
    DT_mqtt_send(buffer, "dOnMeas");
    break;
  }

  // PID MCBT iAwMode
  wdt_reset(); // clear watchdog
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), BUFFER_SIZE);
  switch (eeprom_config.pid_mcbt.iawmode)
  {
  case QuickPID::iAwMode::iAwCondition:
    DT_mqtt_send(buffer, "iAwCondition");
    break;
  case QuickPID::iAwMode::iAwClamp:
    DT_mqtt_send(buffer, "iAwClamp");
    break;
  case QuickPID::iAwMode::iAwOff:
    DT_mqtt_send(buffer, "iAwOff");
    break;
  }

  // RATIO PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.ratio_PCBT);

  // RATIO MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.ratio_MCBT);

  // OFFSET_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.out_offset_PCBT);

  // OFFSET_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.out_offset_MCBT);

  // OFFSET_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.in_offset_PCBT);

  // OFFSET_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.in_offset_MCBT);
#endif

  // ONLINE
  wdt_reset();
  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), BUFFER_SIZE);
  DT_mqtt_send(buffer, "online");
}

void mqtt_subscribe(PubSubClient &mqtt)
{
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/mode_set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/temp_set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/away_set");

  // relay
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/set"), num + 1);
    mqtt.subscribe(buffer);
  }

#ifdef POELE
  // Poele
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/set");
  // 3 voies PCBT mode
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/set");
  // 3 voies PCBT consigne
#endif // POELE

  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/set");
  // 3 voies MCBT mode
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/set");
  // 3 voies MCBT consigne
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/set");

#ifdef POELE
  // EEPROM
  //  V1
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/set");

  // C7
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set");
#endif // POELE

  // V2
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/set");

  // V3
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/set");

  // C4
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set");

  // C5
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/set");

  // C6
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set");

  // C8
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/set");

  // C9
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set");

  // C10
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set");

  // C11
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set");

  // C_PCBT_MIN
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set");

  // C_PCBT_MAX
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set");

  // C_MCBT_MIN
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set");

  // C_MCBT_MAX
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set");

  // KP_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set");

  // KI_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set");

  // KD_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set");

  // KT_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set");

  // KP_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set");

  // KI_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set");

  // KD_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set");

  // KT_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set");

  // PID PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/set");

  // PID MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/set");

  // RATIO
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set");

  // OFFSET OUT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set");

  // OFFSET IN
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set");
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set");

  // HomeAssistant
  mqtt.subscribe("homeassistant/status");

  homeassistant();
  mqtt_publish();
}

void mqtt_receve(char *topic, uint8_t *payload, unsigned int length)
{
  wdt_reset();
  // auto Serial.print("receve topic ");
  // auto Serial.println(topic);

  // Copy the payload to the new buffer
  if (length < BUFFER_SIZE)
  {
    memcpy(buffer, payload, length);
    buffer[length] = '\0';

    // auto Serial.print("buffer = ");
    // auto Serial.println(buffer);
  }
  else
    return;

  int num = 0;
  uint8_t u8t_value = 0;
  if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/set"), &num) == 1) // relais
  {
    // auto Serial.print("sscanf = ");
    // auto Serial.println(num);
    if (strcmp(buffer, "ON") == 0)
      DT_relay(num, true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_relay(num, false);
  }
#ifdef POELE
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/set") == 0) // Mode du Poele
  {
    if (strcmp(buffer, "Arret") == 0)
    {
      DT_Poele_set_mode(DT_POELE_ARRET);
    }
    else if (strcmp(buffer, "Normal") == 0)
    {
      DT_Poele_set_mode(DT_POELE_NORMAL);
    }
    else if (strcmp(buffer, "ECS") == 0)
    {
      DT_Poele_set_mode(DT_POELE_ECS);
    }
    else if (strcmp(buffer, "Forcé") == 0)
    {
      DT_Poele_set_mode(DT_POELE_FORCE);
    }
  }                                                                            // identifiers  // EEPROM
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/set") == 0) // V1
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.V1 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/set") == 0) // V2
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.V2 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
#endif // Poele
#ifdef VANNES
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/set") == 0) // Mode de la vannes 3 voie PCBT
  {

    if (strcmp(buffer, "Normal") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_NORMAL);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Normal");
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_DEMMARAGE);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Demmarage");
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_MANUAL);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Manuel");
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_OFF);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Arret");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/set") == 0) // Mode de la vannes 3 voie PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_set_C2(doc.as<float>());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    if (strcmp(buffer, "Normal") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_NORMAL);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Normal");
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_DEMMARAGE);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Demmarage");
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_MANUAL);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Manuel");
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_OFF);
      strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer_value, "Arret");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_set_C3(doc.as<float>());
    }
  }
  else if (strcmp_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/set")) == 0) // V3
  {
    Serial.print("set V3 = ");
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.V3 = doc.as<float>();
      Serial.print(eeprom_config.V3);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.V3);
      sauvegardeEEPROM();
    }
    Serial.println(" ");
  }
  else if (strcmp_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set")) == 0) // C4
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C4 = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C4);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/set") == 0) // C5
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C5 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set") == 0) // C6
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C6 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set") == 0) // C7
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C7 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/set") == 0) // C8
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C8 = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C8);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set") == 0) // C9
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C9 = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C9);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set") == 0) // C10
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C10 = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C10);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set") == 0) // C11
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C11 = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C11);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set") == 0) // C_PCBT_MIN
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_PCBT_MIN = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_PCBT_MIN);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set") == 0) // C_PCBT_MAX
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_PCBT_MAX = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_PCBT_MAX);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set") == 0) // C_MCBT_MIN
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_MCBT_MIN = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_MCBT_MIN);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set") == 0) // C_MCBT_MAX
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_MCBT_MAX = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_MCBT_MAX);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set") == 0) // KP_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KP(doc.as<float>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KP());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set") == 0) // KI_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KI(doc.as<float>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KI());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set") == 0) // KD_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KD(doc.as<float>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KD());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set") == 0) // KT_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KT(doc.as<uint32_t>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KT());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set") == 0) // KP_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_MCBT_set_KP(doc.as<float>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KP());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set") == 0) // KI_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {

      DT_3voies_MCBT_set_KI(doc.as<float>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KI());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set") == 0) // KD_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_MCBT_set_KD(doc.as<float>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KD());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set") == 0) // KT_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_MCBT_set_KT(doc.as<uint32_t>());
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KT());
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/set") == 0) // PCBT Action
  {
    if (strcmp(buffer, "direct") == 0)
    {
      DT_3voies_PCBT_set_action(QuickPID::Action::direct);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "direct");
    }
    if (strcmp(buffer, "reverse") == 0)
    {
      DT_3voies_PCBT_set_action(QuickPID::Action::reverse);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "reverse");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/set") == 0) // PCBT pMode
  {
    if (strcmp(buffer, "pOnError") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "pOnError");
    }
    if (strcmp(buffer, "pOnErrorMeas") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnErrorMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "pOnErrorMeas");
    }
    if (strcmp(buffer, "pOnMeas") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "pOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/set") == 0) // PCBT dMode
  {
    if (strcmp(buffer, "dOnError") == 0)
    {
      DT_3voies_PCBT_set_dmode(QuickPID::dMode::dOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "dOnError");
    }
    if (strcmp(buffer, "dOnMeas") == 0)
    {
      DT_3voies_PCBT_set_dmode(QuickPID::dMode::dOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "dOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/set") == 0) // PCBT iawmode
  {
    if (strcmp(buffer, "iAwClamp") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwClamp);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "iAwClamp");
    }
    if (strcmp(buffer, "iAwCondition") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwCondition);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "iAwCondition");
    }
    if (strcmp(buffer, "iAwOff") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwOff);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "iAwOff");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/set") == 0) // MCBT Action
  {
    if (strcmp(buffer, "direct") == 0)
    {
      DT_3voies_MCBT_set_action(QuickPID::Action::direct);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "direct");
    }
    if (strcmp(buffer, "reverse") == 0)
    {
      DT_3voies_MCBT_set_action(QuickPID::Action::reverse);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "reverse");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/set") == 0) // MCBT pMode
  {
    if (strcmp(buffer, "pOnError") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "pOnError");
    }
    if (strcmp(buffer, "pOnErrorMeas") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnErrorMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "pOnErrorMeas");
    }
    if (strcmp(buffer, "pOnMeas") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "pOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/set") == 0) // MCBT dMode
  {
    if (strcmp(buffer, "dOnError") == 0)
    {
      DT_3voies_MCBT_set_dmode(QuickPID::dMode::dOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "dOnError");
    }
    if (strcmp(buffer, "dOnMeas") == 0)
    {
      DT_3voies_MCBT_set_dmode(QuickPID::dMode::dOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "dOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/set") == 0) // MCBT iawmode
  {
    if (strcmp(buffer, "iAwClamp") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwClamp);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "iAwClamp");
    }
    if (strcmp(buffer, "iAwCondition") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwCondition);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "iAwCondition");
    }
    if (strcmp(buffer, "iAwOff") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwOff);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "iAwOff");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/set") == 0) // RATIO_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.ratio_PCBT = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.ratio_PCBT);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set") == 0) // RATIO_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.ratio_MCBT = doc.as<float>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.ratio_MCBT);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set") == 0) // OFFSET_PCBT_OUT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.out_offset_PCBT = doc.as<uint16_t>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.out_offset_PCBT);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set") == 0) // OFFSET_MCBT_OUT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.out_offset_MCBT = doc.as<uint16_t>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.out_offset_MCBT);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set") == 0) // OFFSET_PCBT_IN
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.in_offset_PCBT = doc.as<int8_t>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.in_offset_PCBT);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set") == 0) // OFFSET_MCBT_IN
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.in_offset_MCBT = doc.as<int8_t>();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.in_offset_MCBT);
      sauvegardeEEPROM();
    }
  }
#endif                                                 // VANNES
  else if (strcmp(topic, "homeassistant/status") == 0) // Home Assistant Online / Offline
  {
    if (strcmp(buffer, "online") == 0)
    {
      homeassistant();
      mqtt_publish();
      mem_config.MQTT_online = true;
    }
    else if (strcmp(buffer, "offline") == 0)
    {
      mem_config.MQTT_online = false;
    }
  }
}
#endif // MQTT

void setup()
{
  // Serial.begin(9600);
  Serial.begin(57600);

  // auto Serial.println("starting board version " BOARD_SW_VERSION_PRINT);

  // auto Serial.println("Load eeprom");
  chargeEEPROM();

  Wire.begin();
  // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
  // Wire.write(MCP_CHANNEL); // channel 1
  // Wire.endTransmission();

  DT_mcp_init();

#ifdef MQTT
  Serial.println("starting mqtt");
  DT_mqtt_init();
  DT_mqtt_set_subscribe_callback(mqtt_subscribe);
  DT_mqtt_set_receve_callback(mqtt_receve);
#endif // MQTT

  // auto Serial.println("starting relay");
  DT_relay_init();
#ifdef MQTT
  DT_relay_set_callback(relay_callback);
#endif // MQTT
  // auto Serial.println("starting input");
  DT_input_init();
  DT_input_set_callback(input_callback);

  // auto Serial.println("starting PT100");
  DT_pt100_init();
#ifdef MQTT
  DT_pt100_set_callback(pt100_callback);
#endif // MQTT

  // auto Serial.println("starting BME280");
  DT_BME280_init();
#ifdef MQTT
  DT_BME280_set_callback_temperature(bme280_callback_temperature);
  DT_BME280_set_callback_humidity(bme280_callback_humidity);
  DT_BME280_set_callback_pressure(bme280_callback_pressure);
#endif // MQTT

  // auto Serial.println("starting BCCS811");
  DT_CCS811_init();
#ifdef MQTT
  DT_CCS811_set_callback_co2(ccs811_callback_co2);
  DT_CCS811_set_callback_cov(ccs811_callback_cov);
#endif // MQTT

// auto Serial.println("starting fake_NTC");
// DT_fake_ntc_init();
// DT_fake_ntc_callback(fake_ntc_callback);

// auto Serial.println("starting Poele");
#ifdef POELE
  DT_Poele_init();
#ifdef MQTT
  DT_Poele_set_mode_callback(poele_mode_callback);
#endif // MQTT
#endif // POELE

  // auto Serial.println("starting 3 voies");
#ifdef VANNES
  DT_3voies_init();
#ifdef MQTT
  DT_3voies_set_callback(dt3voies_callback);
  DT_3voies_pcbt_set_callback_pid(dt3voies_callback_pid_pcbt);
  DT_3voies_mcbt_set_callback_pid(dt3voies_callback_pid_mcbt);
#endif // MQTT
#endif // VANNE
  // client.setServer(server, 1883);
  // client.setCallback(callback);

  ////auto Serial.println("start network");
  // Ethernet.init(53);
  // Ethernet.begin(mac, ip);
  // Ethernet.begin(mac);
  // delay(1500);

  ////auto Serial.println("network started");

  // while (CCS811.begin() != 0)
  // {
  //  //auto Serial.println("failed to init chip, please check if the chip connection is fine");
  //   delay(1000);
  // }

  // lastReconnectAttempt = 0;

  // if (!ccs811.begin())
  // {
  //  //auto Serial.println("Failed to start sensor! Please check your wiring.");
  // }

  wdt_enable(WATCHDOG_TIME);
  Serial.println("Board started");
  Serial.print("version: ");
  Serial.println(F(BOARD_SW_VERSION_PRINT));
}

void loop()
{
  uint32_t now = millis();

  wdt_reset();

#ifdef MQTT
  DT_mqtt_loop();
#endif
  DT_relay_loop();
  DT_input_loop();
  DT_BME280_loop();
  DT_CCS811_loop();
  DT_pt100_loop();
#ifdef POELE
  DT_Poele_loop();
#endif
#ifdef VANNES
  DT_3voies_loop();
#endif
  //  DT_fake_ntc_loop();

  // adjust CCS811
  static uint32_t ccs811_environmental = 0;
  if (now - ccs811_environmental > 600000) // toute les 10 minutes
  {
    ccs811_environmental = now;
    float humidity = DT_BME280_get_humidity(1);
    float temperature = DT_BME280_get_temperature(1);
    DT_CCS811_set_environmental_data(1, humidity, temperature);
    humidity = DT_BME280_get_humidity(2);
    temperature = DT_BME280_get_temperature(2);
    DT_CCS811_set_environmental_data(2, humidity, temperature);
  }

  static uint32_t old = 0;

  if (now - old > 1000)
  {

    /*
    old = now;

    if (ccs811.available())
    {
      if (!ccs811.readData())
      {
       //auto Serial.print("CO2: ");
       //auto Serial.print(ccs811.geteCO2());
       //auto Serial.print("ppm, TVOC: ");
       //auto Serial.println(ccs811.getTVOC());
      }
      else
      {
       //auto Serial.println("ccs811 ERROR!");
      }
    }*/
  }

#ifdef MQTT
  static uint32_t heartbeat_time = 0;
  static bool heartbeat_status = false;
  if (now - heartbeat_time > 1000) // Backup data in eeprom
  {
    heartbeat_time = now;
    if (heartbeat_status == false)
      heartbeat_status = true;
    else
      heartbeat_status = false;

    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/heartbeat"), BUFFER_SIZE);
    DT_mqtt_send(buffer, heartbeat_status);
  }
#endif // MQTT
  /*
  static uint32_t save_eeprom = 0;
  if (now - save_eeprom > SAVE_EEPROM) // Backup data in eeprom
  {
    save_eeprom = now;
    sauvegardeEEPROM();
  }
*/
  static uint32_t load_1s_count = 0;
  static uint32_t load_1s_time = 0;
  load_1s_count += 1;
  if (now - load_1s_time >= 1000)
  {
    float load = ((now - load_1s_time) / 20.0) / load_1s_count;

    load_1s_count = load * 100;

#ifdef MQTT
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1s"), BUFFER_SIZE);
    DT_mqtt_send(buffer, (float)(load_1s_count / 100.0));
#endif
    load_1s_count = 0;
    load_1s_time = now;
  }

  static uint8_t load_10s_num = 0;
  static uint32_t load_10s_count = 0;
  static uint32_t load_10s_time = 0;
  static float load_10s[6] = {0, 0, 0, 0, 0, 0};
  load_10s_count += 1;
  if (now - load_10s_time >= 10000)
  {
    // Serial.println(load_10s_count);
    // Serial.println(now - load_10s_time);
    load_10s[load_10s_num] = ((now - load_10s_time) / 20.0) / load_10s_count;
    // Serial.println( load_10s[load_10s_num]);

    float temp = 0;
    for (uint8_t num = 0; num < 6; ++num)
    {
      temp += load_10s[num];
    }

    // Serial.println(temp);
    load_10s_count = (temp / 6.0) * 100;
    // Serial.println(load_10s_count);
    // Serial.println((float)((load_10s_count / 100.0)));

#ifdef MQTT
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1m"), BUFFER_SIZE);
    DT_mqtt_send(buffer, (float)((load_10s_count / 100.0)));
#else
    Serial.print(F("load 10m ="));
    Serial.println((float)((load_10s_count / 100.0)));
#endif
    load_10s_count = 0;
    load_10s_time = now;
    load_10s_num += 1;
    if (load_10s_num == 6)
      load_10s_num = 0;

    // Debug heap
    char *heap = (char *)malloc(1);
    static char *old_heap = 0;
    if (heap != old_heap)
    {
      old_heap = heap;
      Serial.print(F("new dynamic alocation heap = "));
      Serial.println((uint16_t)old_heap, HEX);

      Serial.print(F("heap size= "));
      Serial.println((uint16_t)(old_heap - __malloc_heap_start));
    }
    free(heap);
  }
}
