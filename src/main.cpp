#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

#include <DT_relay.h>
#include <DT_temp.h>
#include <DT_mqtt.h>
#include <DT_BME280.h>

#include <avr/wdt.h> //watchdog

#include <Adafruit_CCS811.h>

#include <ArduinoJson.h>

// #include <pinout.h>
#include <config.h>

DynamicJsonDocument doc(256);

// Update these with values suitable for your hardware/network.
// byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};
// IPAddress ip(192, 168, 1, 201);
// IPAddress server(90, 78, 191, 235);
// EthernetClient ethClient;
// PubSubClient client(ethClient);

// #include "Wire.h"
// #include "DFRobot_CCS811.h"
// DFRobot_CCS811 CCS811;

Adafruit_CCS811 ccs811;
// void callback(char *topic, byte *payload, unsigned int length)
// {
//   char *p = (char *)malloc(length + 1);
//   // Copy the payload to the new buffer
//   memcpy(p, payload, length);
//   p[length] = '\0';
//   // handle message arrived
//   if (strcmp(topic, "DtBoard/41/FG1/mode_set") == 0)
//   {
//     client.publish("DtBoard/41/FG1/mode_state", p);
//     if (strcmp(p, "off") == 0)
//     {
//       client.publish("DtBoard/41/FG1/action_state", "off");
//     }
//     else if (strcmp(p, "heat") == 0)
//     {
//       client.publish("DtBoard/41/FG1/action_state", "heating");
//     }
//     else if (strcmp(p, "dry") == 0)
//     {
//       client.publish("DtBoard/41/FG1/action_state", "drying");
//     }
//     else if (strcmp(p, "auto") == 0)
//     {
//       client.publish("DtBoard/41/FG1/action_state", "idle");
//     }
//     Serial.print("DtBoard/41/FG1/mode_set = ");
//     Serial.println(p);
//   }
//   else if (strcmp(topic, "DtBoard/41/FG1/temp_set") == 0)
//   {
//     client.publish("DtBoard/41/FG1/temp_state", p);
//     Serial.print("DtBoard/41/FG1/temp_set = ");
//     Serial.println(p);
//   }
//   else if (strcmp(topic, "DtBoard/41/FG1/away_set") == 0)
//   {
//     client.publish("DtBoard/41/FG1/away_state", p);
//     Serial.print("DtBoard/41/FG1/away_set = ");
//     Serial.println(p);
//   }

//   // Free the memory
//   free(p);
// }

long int lastReconnectAttempt = 0;

// boolean reconnect()
// {
//   String clientId = "Board01";

//   Serial.println("start MQTT conection");
//   if (client.connect(clientId.c_str(), "DtBoard", "1MotdePasse"))
//   {
//     // Once connected, publish an announcement...
//     client.publish("DtBoard/41/availability", "online");
//     client.publish("DtBoard/41/FG1/mode_state", "Off");
//     client.publish("DtBoard/41/FG1/action_state", "Off");
//     client.publish("DtBoard/41/FG1/temp_state", "19");
//     client.publish("DtBoard/41/FG1/away_state", "Off");
//     // ... and resubscribe
//     client.subscribe("DtBoard/41/FG1/mode_set");
//     client.subscribe("DtBoard/41/FG1/temp_set");
//     client.subscribe("DtBoard/41/FG1/away_set");

//     Serial.println("MQTT connected");
//   }
//   else
//   { // si echec affichage erreur
//     Serial.print("ECHEC, rc=");
//     Serial.print(client.state());
//     Serial.println(" nouvelle tentative dans 5 secondes");
//     delay(5000);
//   }
//   return client.connected();
// }

void homeassistant(void)
{
  wdt_reset();
  char buffer[64];
  char buffer_value[512];
  //relay
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/switch/41/relay-%d/config", num + 1);
    doc["~"] = "DtBoard/41";
    sprintf(buffer_value, "41-relay-%d", num + 1);
    doc["unique_id"] = buffer_value;
    sprintf(buffer_value, "relay-%d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/relay-%d/set", num + 1);
    doc["command_topic"] = buffer_value;
    sprintf(buffer_value, "~/relay-%d/state", num + 1);
    doc["state_topic"] = buffer_value;

    sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    doc["device"]["identifiers"] = buffer_value;
    // JsonObject connection = doc["device"].createNestedArray("connection").createNestedObject();
    // sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    // connection["mac"] = buffer_value;
    doc["device"]["manufacturer"] = "DOUET Touch Familly";
    doc["device"]["model"] = "DTBoard02";
    doc["device"]["name"] = "Chauffage";
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }

  //PT100
  for (uint8_t num = 0; num < TEMP_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/41/pt100-%d/config", num + 1);
    doc["~"] = "DtBoard/41";
    sprintf(buffer_value, "41-pt100-%d", num + 1);
    doc["unique_id"] = buffer_value;
    sprintf(buffer_value, "pt100-%d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/pt100-%d/temperature", num + 1);
    doc["state_topic"] = buffer_value;

    doc["device_class"] = "temperature";

    sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    doc["device"]["identifiers"] = buffer_value;
    // JsonObject connection = doc["device"].createNestedArray("connection").createNestedObject();
    // sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    // connection["mac"] = buffer_value;
    doc["device"]["manufacturer"] = "DOUET Touch Familly";
    doc["device"]["model"] = "DTBoard02";
    doc["device"]["name"] = "Chauffage";
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }
  //BME280 temperature
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/41/bme280-temperature-%d/config", num + 1);
    doc["~"] = "DtBoard/41";
    sprintf(buffer_value, "41-bme280-temperature-%d", num + 1);
    doc["unique_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%d/temperature", num + 1);
    doc["state_topic"] = buffer_value;

    doc["device_class"] = "temperature";

    sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    doc["device"]["identifiers"] = buffer_value;
    // JsonObject connection = doc["device"].createNestedArray("connection").createNestedObject();
    // sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    // connection["mac"] = buffer_value;
    // doc["device"]["manufacturer"] = "DOUET Touch Familly";
    // doc["device"]["model"] = "DTBoard02";
    // doc["device"]["name"] = "Chauffage";
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }
  //BME280 humidity

  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/41/bme280-humidity-%d/config", num + 1);
    doc["~"] = "DtBoard/41";
    sprintf(buffer_value, "41-bme280-humidity-%d", num + 1);
    doc["unique_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%d/humidity", num + 1);
    doc["state_topic"] = buffer_value;

    doc["device_class"] = "humidity";

    sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    doc["device"]["identifiers"] = buffer_value;
    // doc["device"]["manufacturer"] = "DOUET Touch Familly";
    // doc["device"]["model"] = "DTBoard02";
    // doc["device"]["name"] = "Chauffage";
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }
  //BME280 pressure
  for (uint8_t num = 0; num < BME280_NUM; ++num)
  {
    wdt_reset();
    doc.clear();
    sprintf(buffer, "homeassistant/sensor/41/bme280-pressure-%d/config", num + 1);
    doc["~"] = "DtBoard/41";
    sprintf(buffer_value, "41-bme280-pressure-%d", num + 1);
    doc["unique_id"] = buffer_value;
    sprintf(buffer_value, "BME280-%d", num + 1);
    doc["name"] = buffer_value;
    sprintf(buffer_value, "~/bme280-%d/pressure", num + 1);
    doc["state_topic"] = buffer_value;

    doc["device_class"] = "pressure";

    sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    doc["device"]["identifiers"] = buffer_value;
    // JsonObject connection = doc["device"].createNestedArray("connection").createNestedObject();
    // sprintf(buffer_value, "%X:%X:%X:%X:%X:%X", MAC1, MAC2, MAC3, MAC4, MAC5, MAC6);
    // connection["mac"] = buffer_value;
    doc["device"]["manufacturer"] = "DOUET Touch Familly";
    doc["device"]["model"] = "DTBoard02";
    doc["device"]["name"] = "Chauffage";
    serializeJson(doc, buffer_value, sizeof(buffer_value));
    Serial.println(buffer_value);
    DT_mqtt_send(buffer, buffer_value);
  }
}

void relay_callback(const uint8_t num, const bool action)
{
  wdt_reset();
  char buffer[32];
  sprintf(buffer, "relais numero %d dans l etat %d", num, (int)action);
  Serial.println(buffer);
  sprintf(buffer, "DtBoard/41/relay-%d/state", num);
  if (action)
    DT_mqtt_send(buffer, "ON");
  else
    DT_mqtt_send(buffer, "OFF");
}

void pt100_callback(const uint8_t num, const float temp)
{
  wdt_reset();
  Serial.print("PT100_CALLBACK ");
  char buffer[48];
  char buffer_value[32];
  sprintf(buffer, "DtBoard/41/pt100-%d/temperature", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(temp);
  serializeJson(variant, buffer_value, 32);
  Serial.print(buffer);
  Serial.print(" -> ");
  Serial.println(buffer_value);
  DT_mqtt_send(buffer, buffer_value);
}

void bme280_callback_temperature(const uint8_t num, const float temperature)
{
  wdt_reset();
  char buffer[48];
  char buffer_value[32];

  sprintf(buffer, "DtBoard/41/bme280-%d/temperature", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(temperature);
  serializeJson(variant, buffer_value, 32);
  DT_mqtt_send(buffer, buffer_value);
}

void bme280_callback_humidity(const uint8_t num, const float humidity)
{
  wdt_reset();
  char buffer[48];
  char buffer_value[32];

  sprintf(buffer, "DtBoard/41/bme280-%d/humidity", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(humidity);
  serializeJson(variant, buffer_value, 32);
  DT_mqtt_send(buffer, buffer_value);
}

void bme280_callback_pressure(const uint8_t num, const float pressure)
{
  wdt_reset();
  char buffer[48];
  char buffer_value[32];

  sprintf(buffer, "DtBoard/41/bme280-%d/pressure", num);
  JsonVariant variant = doc.to<JsonVariant>();
  variant.set(pressure);
  serializeJson(variant, buffer_value, 32);
  DT_mqtt_send(buffer, buffer_value);
}

void mqtt_publish()
{
  wdt_reset();
  char buffer[BUFFER_SIZE];
  DT_mqtt_send("DtBoard/41/availability", "online");
  DT_mqtt_send("DtBoard/41/FG1/mode_state", "Off");
  DT_mqtt_send("DtBoard/41/FG1/action_state", "Off");
  DT_mqtt_send("DtBoard/41/FG1/temp_state", "19");
  DT_mqtt_send("DtBoard/41/FG1/away_state", "Off");
  DT_mqtt_send("DtBoard/41/pt100-1/temperature", "25.12");
  DT_mqtt_send("DtBoard/41/pt100-2/temperature", "12.25");

  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    sprintf(buffer, "DtBoard/41/relay-%d/state", num);
    if (DT_relay_get(2))
      DT_mqtt_send(buffer, "ON");
    else
      DT_mqtt_send(buffer, "OFF");
  }
}

void mqtt_subscribe(PubSubClient &mqtt)
{
  wdt_reset();
  mqtt.subscribe("DtBoard/41/FG1/mode_set");
  mqtt.subscribe("DtBoard/41/FG1/temp_set");
  mqtt.subscribe("DtBoard/41/FG1/away_set");

  char buffer[32];
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    sprintf(buffer, "DtBoard/41/relay-%d/set", num + 1);
    mqtt.subscribe(buffer);
  }
  mqtt.subscribe("homeassistant/status");
  mqtt_publish();
  homeassistant();
}

void mqtt_receve(char *topic, uint8_t *payload, unsigned int length)
{
  wdt_reset();
  Serial.print("receve topic ");
  Serial.println(topic);
  char buffer[BUFFER_SIZE];
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
  if (sscanf(topic, "DtBoard/41/relay-%d/set", &num) == 1)
  {
    Serial.print("sscanf = ");
    Serial.println(num);
    if (strcmp(buffer, "ON") == 0)
      DT_relay(num, true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_relay(num, false);
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
  Serial.println("starting board");
  DT_mqtt_init();
  DT_mqtt_set_subscribe_callback(&mqtt_subscribe);
  DT_mqtt_set_receve_callback(&mqtt_receve);

  Serial.println("starting PT100");
  DT_temp_init();
  DT_temp_set_callback(pt100_callback);

  Serial.println("starting BME280");
  DT_BME280_init();
  DT_BME280_set_callback_temperature(bme280_callback_temperature);
  DT_BME280_set_callback_humidity(bme280_callback_humidity);
  DT_BME280_set_callback_pressure(bme280_callback_pressure);

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

  Serial.println("starting relay");
  DT_relay_init();
  DT_relay_set_callback(&relay_callback);
  wdt_enable(WATCHDOG_TIME);
  Serial.println("Board started");
}

void loop()
{
  wdt_reset();

  DT_relay_loop();
  DT_temp_loop();
  DT_BME280_loop();
  DT_mqtt_loop();

  uint32_t now = millis();
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
