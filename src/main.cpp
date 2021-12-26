#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

#include <DT_relay.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_mqtt.h>
#include <DT_BME280.h>
#include <DT_CCS811.h>
#include <DT_fake_ntc.h>
#include <DT_poele.h>
#include <DT_eeprom.h>

#include <avr/wdt.h> //watchdog

#include <ArduinoJson.h>

// #include <pinout.h>
#include <config.h>

StaticJsonDocument<256> doc;
char buffer[BUFFER_SIZE];
char buffer_value[BUFFER_VALUE_SIZE];

// #include "Wire.h"
// #include "DFRobot_CCS811.h"
// DFRobot_CCS811 CCS811;

// Adafruit_CCS811 ccs811;

long int lastReconnectAttempt = 0;

void homeassistant(void)
{
  // heartbeat
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-heartbeat");
  doc["name"] = F("heartbeat");
  doc["stat_t"] = F("~/heartbeat");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
  // JsonObject connection = doc["device"].createNestedArray("connection").createNestedObject();
  // sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
  // connection["mac"] = buffer_value;
  doc["dev"]["mf"] = F(BOARD_MANUFACTURER); // manufacturer
  doc["dev"]["mdl"] = F(BOARD_MODEL);       // model
  doc["dev"]["name"] = F(BOARD_NAME);       // name
  doc["dev"]["sw"] = F(BOARD_SW_VERSION);   // software version
  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/heartbeat/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  wdt_reset();
  // relay
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-relay-%02d", num + 1);
    doc["uniq_id"] = buffer_value; // unique_id
    sprintf(buffer_value, "relay-%02d", num + 1);
    doc["name"] = buffer_value; // name
    sprintf(buffer_value, "~/relay-%02d/set", num + 1);
    doc["command_topic"] = buffer_value;
    sprintf(buffer_value, "~/relay-%02d/state", num + 1);
    doc["stat_t"] = buffer_value;            // state topic
    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    //Serial.println(buffer_value);
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    DT_mqtt_send(buffer, buffer_value);
  }

  // input
  for (uint8_t num = 0; num < INPUT_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/binary_sensor/" BOARD_IDENTIFIER "/input-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-input-%02d", num + 1);
    doc["uniq_id"] = buffer_value; // unique_id
    sprintf(buffer_value, "input-%02d", num + 1);
    doc["name"] = buffer_value; // name
    sprintf(buffer_value, "~/input-%02d/state", num + 1);
    doc["stat_t"] = buffer_value; // state topic

    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    //Serial.println(buffer_value);
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    DT_mqtt_send(buffer, buffer_value);
  }

  // PT100
  for (uint8_t num = 0; num < TEMP_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/pt100-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-pt100-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "pt100-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/pt100-%02d/temperature", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = F("temperature");
    doc["unit_of_meas"] = F("°C");
    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    //Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }
  // BME280 temperature
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-temperature-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-bme280-temperature-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%02d/temperature", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = F("temperature");
    doc["unit_of_meas"] = F("°C");

    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    //Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // BME280 humidity
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-humidity-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-bme280-humidity-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%02d/humidity", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = F("humidity");
    doc["unit_of_meas"] = F("%");

    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    //Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // BME280 pressure
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-pressure-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-bme280-pressure-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%02d/pressure", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = F("pressure");
    doc["unit_of_meas"] = F("Pa");

    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    //Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // CCS811 CO2
  for (uint8_t num = 0; num < CCS811_NUM; ++num)
  {
    wdt_reset(); // clear watchdog
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-co2-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-ccs811-co2-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "ccs811-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/ccs811-%02d/co2", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = F("carbon_dioxide");
    doc["unit_of_meas"] = F("CO2");

    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    //Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // CCS811 COV
  for (uint8_t num = 0; num < CCS811_NUM; ++num)
  {
    wdt_reset(); // clear watchdog
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-cov-%02d/config", num + 1);
    doc["~"] = F("DtBoard/" BOARD_IDENTIFIER);
    sprintf(buffer_value, BOARD_IDENTIFIER "-ccs811-cov-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "ccs811-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/ccs811-%02d/cov", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = F("pm10");
    doc["unit_of_meas"] = F("ppm");

    doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    //Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // Fake NTC
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/fake_NTC");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-fake_NTC");
  doc["name"] = F("NTC Poele");
  doc["stat_t"] = F("~/temperature");
  doc["command_topic"] = F("~/temperature_set");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/fake_NTC/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // Poele mode
  wdt_reset(); // clear watchdog
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/poele/mode");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-poele-mode");
  doc["name"] = F("mode poele");
  doc["command_topic"] = F("~/set");
  doc["state_topic"] = F("~/state");

  JsonArray options = doc.createNestedArray("options");
  options.add("Silence");
  options.add("Secours");
  options.add("Normal");
  options.add("ECS");
  options.add("Boost");
  options.add("Manuel");
  options.add("Arret");

  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  strlcpy_P(buffer, PSTR("homeassistant/select/" BOARD_IDENTIFIER "/poele-mode/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // Poele C1
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/poele");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-poele-C1");
  doc["name"] = F("Poele C1");
  doc["stat_t"] = F("~/C1");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/poele-c1/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // Poele T1
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/poele");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-poele-T4");
  doc["name"] = F("Poele T4");
  doc["stat_t"] = F("~/T4");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers
  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/sensor/" BOARD_IDENTIFIER "/poele-t4/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // 3 voies PCBT mode
  wdt_reset(); // clear watchdog
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/pcbt/mode");
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

  //consigne vanne 3 voies PCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C2");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C2");
  doc["name"] = F("consigne PCBT (C2)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = TEMPERATURE_DEFAULT_POELE;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C2/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // 3 voies MCBT mode
  wdt_reset(); // clear watchdog
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/mcbt/mode");
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

  //consigne vanne 3 voies MCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C3");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C3");
  doc["name"] = F("consigne MCBT (C3)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = TEMPERATURE_DEFAULT_POELE;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  // EEPROM
  //  V1
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/V1");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-V1");
  doc["name"] = F("parametre poêle (V1)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V1/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // V2
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/V2");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-V2");
  doc["name"] = F("Reserve chaleur Ballon (V2)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V2/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // V3
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/V3");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-V3");
  doc["name"] = F("Temp Demi plage Morte (V3)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/V3/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C4
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C4");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C4");
  doc["name"] = F("consigne Jacuzzi (C4)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  doc["dev_cla"] = F("temperature");
  doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C4/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C5
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C5");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C5");
  doc["name"] = F("consigne ECS1 & ECS2 (C5)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C5/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C6
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C6");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C6");
  doc["name"] = F("consigne mode boost (C6)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C6/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C7
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C7");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C7");
  doc["name"] = F("consigne Mode Silence (C7)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C7/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C8
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C8");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C8");
  doc["name"] = F("consigne Temp PCBT a -10°C (C8)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C8/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C9
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C9");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C9");
  doc["name"] = F("consigne Temp PCBT a +10°C (C9)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C9/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C10
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C10");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C10");
  doc["name"] = F("consigne Temp MCBT a -10°C (C10)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C10/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C11
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C11");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C11");
  doc["name"] = F("consigne Temp MCBT a +10°C (C11)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C11/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C_PCBT_MIN
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MIN");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_PCBT_MIN");
  doc["name"] = F("consigne Temp PCBT minimum (C_PCBT_MIN)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MIN/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C_PCBT_MAX
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MAX");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_PCBT_MAX");
  doc["name"] = F("consigne Temp PCBT maximum (C_PCBT_MAX)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_PCBT_MAX/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C_MCBT_MIN
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MIN");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_MCBT_MIN");
  doc["name"] = F("consigne Temp MCBT minimum (C_MCBT_MIN)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MIN/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // C_MCBT_MAX
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MAX");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-C_MCBT_MAX");
  doc["name"] = F("consigne Temp MCBT maximum (C_MCBT_MAX)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C_MCBT_MAX/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KP_PCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KP_PCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KP_PCBT");
  doc["name"] = F("pid KP PCBT (KP_PCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 1000;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KP_PCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KI_PCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KI_PCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KI_PCBT");
  doc["name"] = F("pid KI PCBT (KI_PCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 1000;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KI_PCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KD_PCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KD_PCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KD_PCBT");
  doc["name"] = F("pid KD PCBT (KD_PCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 1000;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KD_PCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KT_PCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KT_PCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KT_PCBT");
  doc["name"] = F("pid interval PCBT (en ms) (KT_PCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 60000;
  doc["mode"] = F("box");
  // doc["step"] = 0.01;
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KT_PCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KP_MCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KP_MCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KP_MCBT");
  doc["name"] = F("pid KP MCBT (KP_MCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 1000;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KP_MCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KI_MCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KI_MCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KI_MCBT");
  doc["name"] = F("pid KI MCBT (KI_MCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 1000;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KI_MCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KD_MCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KD_MCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KD_MCBT");
  doc["name"] = F("pid KD MCBT (KD_MCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 1000;
  doc["step"] = 0.01;
  doc["mode"] = F("box");
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KD_MCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // KT_MCBT
  wdt_reset();
  doc.clear();
  doc["~"] = F("DtBoard/" BOARD_IDENTIFIER "/KT_MCBT");
  doc["uniq_id"] = F(BOARD_IDENTIFIER "-KT_MCBT");
  doc["name"] = F("pid interval MCBT (en ms) (KT_MCBT)");
  doc["stat_t"] = F("~/state");
  doc["command_topic"] = F("~/set");
  doc["min"] = 0;
  doc["max"] = 60000;
  doc["mode"] = F("box");
  // doc["step"] = 0.01;
  // doc["dev_cla"] = F("temperature");
  // doc["unit_of_meas"] = F("°C");
  doc["dev"]["ids"] = F(BOARD_IDENTIFIER); // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/KT_MCBT/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

// Relay Callback
void relay_callback(const uint8_t num, const bool action)
{
  wdt_reset();
  sprintf(buffer, "relais numero %d dans l etat %d", num, (int)action);
  Serial.println(buffer);
  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/relay-%02d/state", num);
  if (action)
    DT_mqtt_send(buffer, "ON");
  else
    DT_mqtt_send(buffer, "OFF");
}

void input_callback(const uint8_t num, const uint8_t action)
{
  wdt_reset();
  sprintf(buffer, "entrée numero %d dans l etat %d", num, (int)action);
  Serial.println(buffer);
  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/input-%02d/state", num);
  if (action == HIGH)
    DT_mqtt_send(buffer, "ON");
  else
    DT_mqtt_send(buffer, "OFF");
}

void pt100_callback(const uint8_t num, const float temp)
{
  wdt_reset();
  Serial.print("PT100_CALLBACK ");

  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/pt100-%02d/temperature", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(temp);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  Serial.print(buffer);
  Serial.print(" -> ");
  //Serial.println(buffer_value);
  DT_mqtt_send(buffer, buffer_value);
}

void bme280_callback_temperature(const uint8_t num, const float temperature)
{
  wdt_reset();

  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/bme280-%02d/temperature", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(temperature);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void bme280_callback_humidity(const uint8_t num, const float humidity)
{
  wdt_reset();

  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/bme280-%02d/humidity", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(humidity);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void bme280_callback_pressure(const uint8_t num, const float pressure)
{
  wdt_reset();

  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/bme280-%02d/pressure", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(pressure);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void ccs811_callback_co2(const uint8_t num, const float co2)
{
  wdt_reset();

  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/ccs811-%02d/co2", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(co2);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void ccs811_callback_cov(const uint8_t num, const float cov)
{
  wdt_reset();

  sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/ccs811-%02d/cov", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(cov);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void fake_ntc_callback(uint8_t value)
{
  wdt_reset();

  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(value);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/fake_NTC/temperature"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void poele_C1_callback(const uint8_t C1)
{
  wdt_reset();
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(C1);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/poele/C1"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void poele_mode_callback(const DT_Poele_mode mode)
{
  // mode poele
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state"), BUFFER_SIZE);
  switch (mode)
  {
  case DT_POELE_SILENCE:
    DT_mqtt_send(buffer, "Silence");
    break;
  case DT_POELE_SECOURS:
    DT_mqtt_send(buffer, "Secours");
    break;
  case DT_POELE_NORMAL:
    DT_mqtt_send(buffer, "Normal");
    break;
  case DT_POELE_ECS:
    DT_mqtt_send(buffer, "ECS");
    break;
  case DT_POELE_BOOST:
    DT_mqtt_send(buffer, "Boost");
    break;
  case DT_POELE_MANUAL:
    DT_mqtt_send(buffer, "Manuel");
    break;
  case DT_POELE_OFF:
    DT_mqtt_send(buffer, "Arret");
    break;
  }
}

void poele_t4_callback(const float t4)
{
  wdt_reset();
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(t4);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/poele/T4"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);
}

void mqtt_publish()
{
  wdt_reset();

  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/availability"), BUFFER_SIZE);
  DT_mqtt_send(buffer, "online");

  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/relay-%02d/state", num + 1);
    if (DT_relay_get(num + 1))
      DT_mqtt_send(buffer, "ON");
    else
      DT_mqtt_send(buffer, "OFF");
  }

  for (uint8_t num = 0; num < INPUT_NUM; ++num)
  {
    wdt_reset();
    sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/input-%02d/state", num + 1);
    if (DT_input_get(num + 1) == HIGH)
      DT_mqtt_send(buffer, "ON");
    else
      DT_mqtt_send(buffer, "OFF");
  }

  // PT100
  for (uint8_t num = 0; num < TEMP_NUM; ++num)
  {
    wdt_reset();
    sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/pt100-%02d/temperature", num + 1);
    DT_mqtt_send(buffer, DT_pt100_get(num + 1));
  }

  // Fake NTC
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/fake_NTC/temperature"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_fake_ntc_get());

  // mode poele
  wdt_reset();
  poele_mode_callback(DT_Poele_get_mode());

  // Poele C1
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/poele/C1"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_Poele_get_C1());

  // Poele T4
  wdt_reset();
  poele_t4_callback(DT_Poele_get_T4());

  // 3 voies PCBT mode
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
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

  //consigne vanne 3 voies PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C2"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_get_C2());

  // 3 voies MCBT mode
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
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

  //consigne vanne 3 voies MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C3"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_get_C3());

  // EEPROM
  //  V1
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/V1/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.V1);

  // V2
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/V2/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.V2);

  // V3
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/V3/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.V3);

  // C4
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C4/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C4);

  // C5
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C5/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C5);

  // C6
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C6/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C6);

  // C7
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C7/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C7);

  // C8
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C8/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C8);

  // C9
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C9/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C9);

  // C10
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C10/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C10);

  // C11
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C11/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C11);

  // C_PCBT_MIN
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MIN/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_PCBT_MIN);

  // C_PCBT_MAX
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MAX/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_PCBT_MAX);

  // C_MCBT_MIN
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MIN/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_MCBT_MIN);

  // C_MCBT_MAX
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MAX/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, eeprom_config.C_MCBT_MAX);

  // KP_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KP_PCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KP());

  // KI_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KI_PCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KI());

  // KD_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KD_PCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KI());

  // KT_PCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KT_PCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_PCBT_get_KT());
  // KP_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KP_MCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KP());

  // KI_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KI_MCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KI());

  // KD_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KD_MCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KD());

  // KT_MCBT
  wdt_reset();
  strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KT_MCBT/state"), BUFFER_SIZE);
  DT_mqtt_send(buffer, DT_3voies_MCBT_get_KT());
}

void mqtt_subscribe(PubSubClient &mqtt)
{
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/FG1/mode_set");
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/FG1/temp_set");
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/FG1/away_set");

  // relay
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    sprintf(buffer, "DtBoard/" BOARD_IDENTIFIER "/relay-%02d/set", num + 1);
    mqtt.subscribe(buffer);
  }

  // fake_NTC
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/fake_NTC/temperature_set");
  // Poele
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/poele/mode/set");
  // 3 voies PCBT mode
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/mcbt/mode/set");
  // 3 voies PCBT consigne
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C2/set");
  // 3 voies MCBT mode
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/pcbt/mode/set");
  // 3 voies MCBT consigne
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C3/set");

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  //Serial.println(buffer_value);
  strlcpy_P(buffer, PSTR("homeassistant/number/" BOARD_IDENTIFIER "/C3/config"), BUFFER_SIZE);
  DT_mqtt_send(buffer, buffer_value);

  // EEPROM
  //  V1
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/V1/set");

  // V2
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/V2/set");

  // V3
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/V3/set");

  // C4
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C4/set");

  // C5
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C5/set");

  // C6
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C6/set");

  // C7
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C7/set");

  // C8
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C8/set");

  // C9
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C9/set");

  // C10
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C10/set");

  // C11
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C11/set");

  // C_PCBT_MIN
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MIN/set");

  // C_PCBT_MAX
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MAX/set");

  // C_MCBT_MIN
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MIN/set");

  // C_MCBT_MAX
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MAX/set");

  // KP_PCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KP_PCBT/set");

  // KI_PCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KI_PCBT/set");

  // KD_PCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KD_PCBT/set");

  // KT_PCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KT_PCBT/set");

  // KP_MCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KP_MCBT/set");

  // KI_MCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KI_MCBT/set");

  // KD_MCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KD_MCBT/set");

  // KT_MCBT
  wdt_reset();
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/KT_MCBT/set");

  mqtt.subscribe("homeassistant/status");
  mqtt_publish();
  homeassistant();
}

void mqtt_receve(char *topic, uint8_t *payload, unsigned int length)
{
  wdt_reset();
  Serial.print("receve topic ");
  Serial.println(topic);

  // Copy the payload to the new buffer
  if (length < BUFFER_SIZE)
  {
    memcpy(buffer, payload, length);
    buffer[length] = '\0';

    Serial.print("buffer = ");
    Serial.println(buffer);
  }
  else
    return;

  int num = 0;
  uint8_t u8t_value = 0;
  if (sscanf(topic, "DtBoard/" BOARD_IDENTIFIER "/relay-%02d/set", &num) == 1) // relais
  {
    Serial.print("sscanf = ");
    Serial.println(num);
    if (strcmp(buffer, "ON") == 0)
      DT_relay(num, true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_relay(num, false);
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/fake_NTC/temperature_set") == 0) // Fake NTC
  {
    uint8_t temperature = 0;
    if (sscanf(buffer, "%" SCNu8, &temperature) == 1)
    {
      DT_fake_ntc_set(temperature);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/poele/mode/set") == 0) // Mode du Poele
  {
    if (strcmp(buffer, "Silence") == 0)
    {
      DT_Poele_set_mode(DT_POELE_SILENCE);
    }
    else if (strcmp(buffer, "Secours") == 0)
    {
      DT_Poele_set_mode(DT_POELE_SECOURS);
    }
    else if (strcmp(buffer, "Normal") == 0)
    {
      DT_Poele_set_mode(DT_POELE_NORMAL);
    }
    else if (strcmp(buffer, "ECS") == 0)
    {
      DT_Poele_set_mode(DT_POELE_ECS);
    }
    else if (strcmp(buffer, "Boost") == 0)
    {
      DT_Poele_set_mode(DT_POELE_BOOST);
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_Poele_set_mode(DT_POELE_MANUAL);
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_Poele_set_mode(DT_POELE_OFF);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/pcbt/mode/set") == 0) // Mode de la vannes 3 voie PCBT
  {
    strlcpy_P(buffer_value, PSTR("DtBoard/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);

    if (strcmp(buffer, "Normal") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_NORMAL);
      DT_mqtt_send(buffer_value, buffer);
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_DEMMARAGE);
      DT_mqtt_send(buffer_value, buffer);
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_MANUAL);
      DT_mqtt_send(buffer_value, buffer);
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_OFF);
      DT_mqtt_send(buffer_value, buffer);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C2/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_set_C2(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C2/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_get_C2());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/mcbt/mode/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    strlcpy_P(buffer_value, PSTR("DtBoard/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);

    if (strcmp(buffer, "Normal") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_NORMAL);
      DT_mqtt_send(buffer_value, buffer);
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_DEMMARAGE);
      DT_mqtt_send(buffer_value, buffer);
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_MANUAL);
      DT_mqtt_send(buffer_value, buffer);
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_OFF);
      DT_mqtt_send(buffer_value, buffer);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C3/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_set_C3(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C3/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_get_C3());
    }
  }                                                                   // identifiers                                                                   // EEPROM
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/V1/set") == 0) // V1
  {
    if (sscanf(buffer, "%" SCNu8, &u8t_value) == 1)
    {
      eeprom_config.V1 = u8t_value;
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/V1/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/V2/set") == 0) // V2
  {
    if (sscanf(buffer, "%" SCNu8, &u8t_value) == 1)
    {
      eeprom_config.V2 = u8t_value;
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/V2/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/V3/set") == 0) // V3
  {
    if (sscanf(buffer, "%" SCNu8, &u8t_value) == 1)
    {
      eeprom_config.V3 = u8t_value;
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/V3/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C4/set") == 0) // C4
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C4 = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C4/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C4);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C5/set") == 0) // C5
  {
    if (sscanf(buffer, "%" SCNu8, &u8t_value) == 1)
    {
      eeprom_config.C5 = u8t_value;
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C5/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C6/set") == 0) // C6
  {
    if (sscanf(buffer, "%" SCNu8, &u8t_value) == 1)
    {
      eeprom_config.C6 = u8t_value;
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C6/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C7/set") == 0) // C7
  {
    if (sscanf(buffer, "%" SCNu8, &u8t_value) == 1)
    {
      eeprom_config.C7 = u8t_value;
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C7/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, u8t_value);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C8/set") == 0) // C8
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C8 = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C8/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C8);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C9/set") == 0) // C9
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C9 = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C9/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C9);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C10/set") == 0) // C10
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C10 = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C10/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C10);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C11/set") == 0) // C11
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C11 = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C11/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C11);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MIN/set") == 0) // C_PCBT_MIN
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_PCBT_MIN = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MIN/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_PCBT_MIN);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MAX/set") == 0) // C_PCBT_MAX
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_PCBT_MAX = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_PCBT_MAX/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_PCBT_MAX);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MIN/set") == 0) // C_MCBT_MIN
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_MCBT_MIN = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MIN/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_MCBT_MIN);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MAX/set") == 0) // C_MCBT_MAX
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      eeprom_config.C_MCBT_MAX = doc.as<float>();
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/C_MCBT_MAX/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, eeprom_config.C_MCBT_MAX);
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KP_PCBT/set") == 0) // KP_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KP(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KP_PCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KP());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KI_PCBT/set") == 0) // KI_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KI(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KI_PCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KI());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KD_PCBT/set") == 0) // KD_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KD(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KD_PCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KD());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KT_PCBT/set") == 0) // KT_PCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_PCBT_set_KT(doc.as<uint32_t>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KT_PCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_PCBT_get_KT());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KP_MCBT/set") == 0) // KP_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_MCBT_set_KP(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KP_MCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KP());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KI_MCBT/set") == 0) // KI_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {

      DT_3voies_MCBT_set_KI(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KI_MCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KI());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KD_MCBT/set") == 0) // KD_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_MCBT_set_KD(doc.as<float>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KD_MCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KD());
    }
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/KT_MCBT/set") == 0) // KT_MCBT
  {
    DeserializationError error = deserializeJson(doc, buffer, length);
    if (!error)
    {
      DT_3voies_MCBT_set_KT(doc.as<uint32_t>());
      strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/KT_MCBT/state"), BUFFER_SIZE);
      DT_mqtt_send(buffer, DT_3voies_MCBT_get_KT());
    }
  }
  else if (strcmp(topic, "homeassistant/status") == 0) // Home Assistant Online / Offline
  {
    if (strcmp(buffer, "online") == 0)
    {
      homeassistant();
      mqtt_publish();
    }
  }
}

void setup()
{
  Serial.begin(9600);

  Serial.println("starting board version " BOARD_SW_VERSION);

  Serial.println("Load eeprom");
  chargeEEPROM();

  Serial.println("starting mqtt");
  DT_mqtt_init();
  DT_mqtt_set_subscribe_callback(mqtt_subscribe);
  DT_mqtt_set_receve_callback(mqtt_receve);

  Serial.println("starting relay");
  DT_relay_init();
  DT_relay_set_callback(relay_callback);

  Serial.println("starting input");
  DT_input_init();
  DT_input_set_callback(input_callback);

  Serial.println("starting PT100");
  DT_pt100_init();
  DT_pt100_set_callback(pt100_callback);

  Serial.println("starting BME280");
  DT_BME280_init();
  DT_BME280_set_callback_temperature(bme280_callback_temperature);
  DT_BME280_set_callback_humidity(bme280_callback_humidity);
  DT_BME280_set_callback_pressure(bme280_callback_pressure);

  Serial.println("starting BCCS811");
  DT_CCS811_init();
  DT_CCS811_set_callback_co2(ccs811_callback_co2);
  DT_CCS811_set_callback_cov(ccs811_callback_cov);

  Serial.println("starting fake_NTC");
  DT_fake_ntc_init();
  DT_fake_ntc_callback(fake_ntc_callback);

  Serial.println("starting Poele");
  DT_Poele_init();
  DT_Poele_set_C1_callback(poele_C1_callback);
  DT_Poele_set_mode_callback(poele_mode_callback);
  DT_Poele_T4_callback(poele_t4_callback);

  // client.setServer(server, 1883);
  // client.setCallback(callback);

  // Serial.println("start network");
  // Ethernet.init(53);
  // Ethernet.begin(mac, ip);
  // Ethernet.begin(mac);
  // delay(1500);

  // Serial.println("network started");

  // while (CCS811.begin() != 0)
  // {
  //   Serial.println("failed to init chip, please check if the chip connection is fine");
  //   delay(1000);
  // }

  // lastReconnectAttempt = 0;

  // if (!ccs811.begin())
  // {
  //   Serial.println("Failed to start sensor! Please check your wiring.");
  // }

  wdt_enable(WATCHDOG_TIME);
  Serial.println("Board started");
}

void loop()
{
  uint32_t now = millis();

  wdt_reset();

  DT_mqtt_loop();
  DT_relay_loop();
  DT_input_loop();
  DT_BME280_loop();
  DT_CCS811_loop();
  DT_pt100_loop();
  DT_Poele_loop();
  DT_3voies_loop();
  DT_fake_ntc_loop();

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
        Serial.print("CO2: ");
        Serial.print(ccs811.geteCO2());
        Serial.print("ppm, TVOC: ");
        Serial.println(ccs811.getTVOC());
      }
      else
      {
        Serial.println("ccs811 ERROR!");
      }
    }*/
  }

  static uint32_t heartbeat_time = 0;
  static bool heartbeat_status = false;
  if (now - heartbeat_time > 1000) // Backup data in eeprom
  {
    heartbeat_time = now;
    if (heartbeat_status == false)
      heartbeat_status = true;
    else
      heartbeat_status = false;

    strlcpy_P(buffer, PSTR("DtBoard/" BOARD_IDENTIFIER "/heartbeat"), BUFFER_SIZE);
    DT_mqtt_send(buffer, heartbeat_status);
  }

  static uint32_t save_eeprom = 0;
  if (now - save_eeprom > SAVE_EEPROM) // Backup data in eeprom
  {
    save_eeprom = now;
    sauvegardeEEPROM();
  }
}
