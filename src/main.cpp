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

#include <avr/wdt.h> //watchdog

#include <ArduinoJson.h>

// #include <pinout.h>
#include <config.h>

DynamicJsonDocument doc(256);
char buffer[BUFFER_SIZE];
char buffer_value[BUFFER_VALUE_SIZE];

// #include "Wire.h"
// #include "DFRobot_CCS811.h"
// DFRobot_CCS811 CCS811;

// Adafruit_CCS811 ccs811;

long int lastReconnectAttempt = 0;

void homeassistant(void)
{
  wdt_reset();
  // relay
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/switch/" BOARD_IDENTIFIER "/relay-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-relay-%02d", num + 1);
    doc["uniq_id"] = buffer_value; // unique_id
    sprintf(buffer_value, "relay-%02d", num + 1);
    doc["name"] = buffer_value; // name
    sprintf(buffer_value, "~/relay-%02d/set", num + 1);
    doc["command_topic"] = buffer_value;
    sprintf(buffer_value, "~/relay-%02d/state", num + 1);
    doc["stat_t"] = buffer_value; // state topic

    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers
    // JsonObject connection = doc["device"].createNestedArray("connection").createNestedObject();
    // sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    // connection["mac"] = buffer_value;
    doc["dev"]["mf"] = BOARD_MANUFACTURER; // manufacturer
    doc["dev"]["mdl"] = BOARD_MODEL;       // model
    doc["dev"]["name"] = BOARD_NAME;       // name
    doc["dev"]["sw"] = BOARD_SW_VERSION;   // software version

    Serial.println(buffer_value);
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    DT_mqtt_send(buffer, buffer_value);
  }

  // input
  for (uint8_t num = 0; num < INPUT_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/binary_sensor/" BOARD_IDENTIFIER "/input-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-input-%02d", num + 1);
    doc["uniq_id"] = buffer_value; // unique_id
    sprintf(buffer_value, "input-%02d", num + 1);
    doc["name"] = buffer_value; // name
    sprintf(buffer_value, "~/input-%02d/state", num + 1);
    doc["stat_t"] = buffer_value; // state topic

    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

    Serial.println(buffer_value);
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    DT_mqtt_send(buffer, buffer_value);
  }

  // PT100
  for (uint8_t num = 0; num < TEMP_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/pt100-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-pt100-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "pt100-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/pt100-%02d/temperature", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = "temperature";
    doc["unit_of_meas"] = "°C";
    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }
  // BME280 temperature
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-temperature-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-bme280-temperature-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%02d/temperature", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = "temperature";
    doc["unit_of_meas"] = "°C";

    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // BME280 humidity
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-humidity-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-bme280-humidity-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%02d/humidity", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = "humidity";
    doc["unit_of_meas"] = "%";

    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // BME280 pressure
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/bme280-pressure-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-bme280-pressure-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%02d/pressure", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = "pressure";
    doc["unit_of_meas"] = "Pa";

    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // CCS811 CO2
  for (uint8_t num = 0; num < CCS811_NUM; ++num)
  {
    wdt_reset(); // clear watchdog
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-co2-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-ccs811-co2-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "ccs811-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/ccs811-%02d/co2", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = "carbon_dioxide";
    doc["unit_of_meas"] = "CO2";

    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // CCS811 COV
  for (uint8_t num = 0; num < CCS811_NUM; ++num)
  {
    wdt_reset(); // clear watchdog
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/" BOARD_IDENTIFIER "/ccs811-cov-%02d/config", num + 1);
    doc["~"] = "DtBoard/" BOARD_IDENTIFIER;
    sprintf(buffer_value, BOARD_IDENTIFIER "-ccs811-cov-%02d", num + 1);
    doc["uniq_id"] = buffer_value;
    sprintf(buffer_value, "ccs811-%02d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/ccs811-%02d/cov", num + 1);
    doc["stat_t"] = buffer_value;
    doc["dev_cla"] = "pm10";
    doc["unit_of_meas"] = "ppm";

    doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  // Fake NTC
  wdt_reset();
  doc.clear();
  doc["~"] = "DtBoard/" BOARD_IDENTIFIER "/fake_NTC";
  doc["uniq_id"] = BOARD_IDENTIFIER "-fake_NTC";
  doc["name"] = "NTC Poele";
  doc["stat_t"] = "~/temperature";
  doc["dev_cla"] = "temperature";
  doc["unit_of_meas"] = "°C";
  doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  Serial.println(buffer_value);
  DT_mqtt_send("homeassistant/sensor/" BOARD_IDENTIFIER "/fake_NTC", buffer_value);

  // Poele mode
  wdt_reset(); // clear watchdog
  doc.clear();
  doc["~"] = "DtBoard/" BOARD_IDENTIFIER "/poele/mode";
  doc["uniq_id"] = BOARD_IDENTIFIER "-poele-mode";
  doc["name"] = "mode poele";
  doc["command_topic"] = "~/set";
  doc["stat_t"] = "~/state";

  JsonArray options = doc.createNestedArray("options");
  options.add("Silence");
  options.add("Secours");
  options.add("Normal");
  options.add("ECS");
  options.add("Boost");

  doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers

  serializeJson(doc, buffer_value, sizeof(buffer_value));
  DT_mqtt_send("homeassistant/select/" BOARD_IDENTIFIER "/poele-mode/config", buffer_value);

  //Poele ev1
  wdt_reset();
  doc.clear();
  doc["~"] = "DtBoard/" BOARD_IDENTIFIER "/poele";
  doc["uniq_id"] = BOARD_IDENTIFIER "-poele-ev1";
  doc["name"] = "Poele EV1";
  doc["stat_t"] = "~/ev1";
  doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers
  serializeJson(doc, buffer_value, sizeof(buffer_value));
  Serial.println(buffer_value);
  DT_mqtt_send("homeassistant/binary_sensor/" BOARD_IDENTIFIER "/poele-ev1/config", buffer_value);

  //Poele T4
  wdt_reset();
  doc.clear();
  doc["~"] = "DtBoard/" BOARD_IDENTIFIER "/poele";
  doc["uniq_id"] = BOARD_IDENTIFIER "-poele-T4";
  doc["name"] = "Poele T4";
  doc["stat_t"] = "~/T4";
  doc["dev_cla"] = "temperature";
  doc["unit_of_meas"] = "°C";
  doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers
  serializeJson(doc, buffer_value, sizeof(buffer_value));
  Serial.println(buffer_value);
  DT_mqtt_send("homeassistant/sensor/" BOARD_IDENTIFIER "/poele-t4/config", buffer_value);

  //Poele C1
  wdt_reset();
  doc.clear();
  doc["~"] = "DtBoard/" BOARD_IDENTIFIER "/poele";
  doc["uniq_id"] = BOARD_IDENTIFIER "-poele-C1";
  doc["name"] = "Poele C1";
  doc["stat_t"] = "~/C1";
  doc["dev_cla"] = "temperature";
  doc["unit_of_meas"] = "°C";
  doc["dev"]["ids"] = BOARD_IDENTIFIER; // identifiers
  serializeJson(doc, buffer_value, sizeof(buffer_value));
  Serial.println(buffer_value);
  DT_mqtt_send("homeassistant/sensor/" BOARD_IDENTIFIER "/poele-c1/config", buffer_value);
}

//Relay Callback
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
  Serial.println(buffer_value);
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
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/fake_ntc/temperature", buffer_value);
}

void poele_callback(const bool ev1, const float T4, const uint8_t C1)
{
  wdt_reset();
  //ev1
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(ev1);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/ev1", buffer_value);
  //T4
  variant = doc.to<JsonVariant>();
  variant.set(T4);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/T4", buffer_value);
  //C1
  variant = doc.to<JsonVariant>();
  variant.set(C1);
  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/C1", buffer_value);
}

void mqtt_publish()
{
  wdt_reset();

  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/availability", "online");

  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/FG1/mode_state", "Off");
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/FG1/action_state", "Off");
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/FG1/temp_state", "19");
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/FG1/away_state", "Off");
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/pt100-01/temperature", "25.12");
  DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/pt100-02/temperature", "12.25");

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

  // mode poele
  if (DT_Poele_get_mode() == DT_POELE_SILENCE)
  {
    DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Silence");
  }
  else if (DT_Poele_get_mode() == DT_POELE_SECOURS)
  {
    DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Secours");
  }
  else if (DT_Poele_get_mode() == DT_POELE_NORMAL)
  {
    DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Normal");
  }
  else if (DT_Poele_get_mode() == DT_POELE_ECS)
  {
    DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "ECS");
  }
  else if (DT_Poele_get_mode() == DT_POELE_BOOST)
  {
    DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Boost");
  }
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
  // Poele
  mqtt.subscribe("DtBoard/" BOARD_IDENTIFIER "/poele/mode/set");

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
  if (sscanf(topic, "DtBoard/" BOARD_IDENTIFIER "/relay-%02d/set", &num) == 1)
  {
    Serial.print("sscanf = ");
    Serial.println(num);
    if (strcmp(buffer, "ON") == 0)
      DT_relay(num, true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_relay(num, false);
  }
  else if (strcmp(topic, "DtBoard/" BOARD_IDENTIFIER "/poele/mode/set") == 0)
  {
    if (strcmp(buffer, "Silence") == 0)
    {
      DT_Poele_set_mode(DT_POELE_SILENCE);
      DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Silence");
    }

    else if (strcmp(buffer, "Secours") == 0)
    {
      DT_Poele_set_mode(DT_POELE_SECOURS);
      DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Secours");
    }
    else if (strcmp(buffer, "Normal") == 0)
    {
      DT_Poele_set_mode(DT_POELE_NORMAL);
      DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Normal");
    }
    else if (strcmp(buffer, "ECS") == 0)
    {
      DT_Poele_set_mode(DT_POELE_ECS);
      DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "ECS");
    }
    else if (strcmp(buffer, "Boost") == 0)
    {
      DT_Poele_set_mode(DT_POELE_BOOST);
      DT_mqtt_send("DtBoard/" BOARD_IDENTIFIER "/poele/mode/state", "Boost");
    }
  }
  else if (strcmp(topic, "homeassistant/status") == 0)
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

  Serial.println("starting fake_ntc");
  DT_fake_ntc_init();
  DT_fake_ntc_callback(fake_ntc_callback);

  Serial.println("starting Poele");
  DT_Poele_init();

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
}
