#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>

#include <DT_ha.h>

#include <DT_relay.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_mqtt.h>
#include <DT_ha.h>
#include <DT_BME280.h>
#include <DT_CCS811.h>
#include <DT_mcp.h>

// #include <DT_fake_ntc.h>
#include <DT_poele.h>
#include <DT_eeprom.h>

#include <DT_buffer.h>

#include <avr/wdt.h> //watchdog

//#include <ArduinoJson.h>

// #include <pinout.h>
#include <config.h>

// #include "Wire.h"
// #include "DFRobot_CCS811.h"
// DFRobot_CCS811 CCS811;

// Adafruit_CCS811 ccs811;

// DT_buffer buffer_test;

long int lastReconnectAttempt = 0;

uint32_t watchdog_reset = 0;

void debug(const char *var)
{
  static uint32_t old_call = 0;
  uint32_t now = millis();
  Serial.print(var);
  Serial.print(F(","));
  Serial.print(now - watchdog_reset);
  Serial.print(F(","));
  Serial.println(now - old_call);
  old_call = now;
}

void debug(const __FlashStringHelper *var)
{
  static uint32_t old_call = 0;
  uint32_t now = millis();
  Serial.print(var);
  Serial.print(F(","));
  Serial.print(now - watchdog_reset);
  Serial.print(F(","));
  Serial.println(now - old_call);
  old_call = now;
}

void debug_wdt_reset()
{
  watchdog_reset = millis();
  wdt_reset();
}

void debug_wdt_reset(const char *var)
{
  debug(var);
  watchdog_reset = millis();
  wdt_reset();
}

void debug_wdt_reset(const __FlashStringHelper *var)
{
  debug(var);
  watchdog_reset = millis();
  wdt_reset();
}

extern void *__brkval;
void memory(bool print)
{
  static unsigned int min_free_memory = 65535;
  unsigned int free_memory = (uint16_t)SP - (uint16_t)__brkval;

  if (free_memory < min_free_memory)
  {
    min_free_memory = free_memory;
    // Serial.print(millis());
    Serial.print(F("new minimum free memory = "));
    Serial.println(min_free_memory);
    // Serial.print(millis());
    Serial.print(F("constant memory = "));
    Serial.println((uint16_t)__malloc_heap_start);

    // Serial.print(millis());
    Serial.print(F("heap size= "));
    Serial.println((uint16_t)((uint16_t)__brkval - (uint16_t)__malloc_heap_start));

    Serial.print(F("stack size= "));
    Serial.println((uint16_t)(RAMEND - SP));
  }
  if (print)
  {
    Serial.print(F("free memory = "));
    Serial.println(free_memory);
  }
}

void load()
{
  MQTT_data send;
  // char topic[56];
  uint32_t now = millis();
  static uint32_t load_1s_count = 0;
  static uint32_t load_1s_time = 0;
  load_1s_count += 1;
  if (now - load_1s_time >= 1000)
  {
    debug(F(AT));
    float load = ((now - load_1s_time) / 20.0) / load_1s_count;

    load_1s_count = load * 100;

#ifdef MQTT
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1s"), 56);
    // debug(AT);
    // DT_mqtt_send(topic, (float)(load_1s_count / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1s"), (float)(load_1s_count / 100.0));
    send_buffer.append(send);
#endif
    Serial.print(F("Load 1s = "));
    Serial.println((float)(load_1s_count / 100.0));

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
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1m"), 56);
    // debug(AT);
    // DT_mqtt_send(topic, (float)((load_10s_count / 100.0)));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1m"), (float)(load_10s_count / 100.0));
    send_buffer.append(send);
#else
    Serial.print(F("load 10m ="));
    Serial.println((float)((load_10s_count / 100.0)));
#endif
    load_10s_count = 0;
    load_10s_time = now;
    load_10s_num += 1;
    if (load_10s_num == 6)
      load_10s_num = 0;
  }
}

#ifdef MQTT

// Relay Callback
void relay_callback(const uint8_t num, const bool action)
{
  // char topic[56];
  MQTT_data send;
  const char *payload;
  debug(F(AT));
  memory(true);
  // debug_wdt_reset();
  if (mem_config.MQTT_online)
  {
    // sprintf_P(buffer, PSTR("relais numero %d dans l etat %d"), num, (int)action);
    //  auto Serial.println(buffer);
    // snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/state"), num);
    if (action)
    {
      // DT_mqtt_send(topic, "ON");
      payload = PSTR("ON");
    }
    else
    {
      // DT_mqtt_send(topic, "OFF");
      payload = PSTR("OFF");
    }
    MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/state"), num, payload);
    Serial.print(F("Relay num = "));
    Serial.print(num);
    Serial.print(F("= "));
    if (action)
    {
      // DT_mqtt_send(topic, "ON");
      Serial.println(F("ON"));
    }
    else
    {
      // DT_mqtt_send(topic, "OFF");
      Serial.println(F("OFF"));
    }

    send_buffer.append(send);
  }
}
#endif // MQTT

void input_callback(const uint8_t num, const Bt_Action action)
{
  // char topic[56];
  debug(F(AT));
  MQTT_data send;
  const char *payload;
  memory(true);
  // debug_wdt_reset();
  // Serial.print(F("entrée numero "));
  // Serial.print(num);
  // Serial.print(F(" dans l etat "));

  if (mem_config.MQTT_online)
  {
#ifdef MQTT
    // snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num);
#endif
    switch (action)
    {
    case IN_PUSHED:
      // Serial.println("ON");
#ifdef MQTT
      // DT_mqtt_send(topic, "ON");
      payload = PSTR("ON");
#endif
      break;

    case IN_RELEASE:
      // Serial.println("OFF");
#ifdef MQTT
      // DT_mqtt_send(topic, "OFF");
      payload = PSTR("OFF");
#endif
      break;

    case IN_PUSH:
      // Serial.println("PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "PUSH");
#endif
      break;

    case IN_LPUSH:
      // Serial.println("LPUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "LPUSH");
#endif
      break;

    case IN_LLPUSH:
      // Serial.println("LLPUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "LLPUSH");
#endif
      break;

    case IN_XLLPUSH:
      // Serial.println("XLLPUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "XLLPUSH");
#endif
      break;

    case IN_2PUSH:
      // Serial.println("2PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "2PUSH");
#endif
      break;

    case IN_L2PUSH:
      // Serial.println("L2PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "L2PUSH");
#endif
      break;

    case IN_LL2PUSH:
      // Serial.println("LL2PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "LL2PUSH");
#endif
      break;

    case IN_XLL2PUSH:
      // Serial.println("XLL2PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "XLL2PUSH");
#endif
      break;

    case IN_3PUSH:
      // Serial.println("3PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "3PUSH");
#endif
      break;

    case IN_L3PUSH:
      // Serial.println("L3PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "L3PUSH");
#endif
      break;

    case IN_LL3PUSH:
      // Serial.println("LL3PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "LL3PUSH");
#endif
      break;

    case IN_XLL3PUSH:
      // Serial.println("XLL3PUSH");
#ifdef MQTT
      // DT_mqtt_send(buffer, "XLL3PUSH");
#endif
      break;

    default:
      // Serial.println(action);
      break;
    }
  }
  MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num, payload);
  send_buffer.append(send);
}

#ifdef MQTT
#if TEMP_NUM > 0
void pt100_callback(const uint8_t num, const float temp)
{
  debug(F(AT));
  // char topic[60];
  MQTT_data send;
  memory(true);
  // debug_wdt_reset();
  Serial.println("PT100_CALLBACK ");

  if (mem_config.MQTT_online)
  {
    // snprintf_P(topic, 60, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d/temperature"), num);
    // DT_mqtt_send(topic, temp);
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d/temperature"), num, temp);
    send_buffer.append(send);
  }
}
#endif // PT100

void bme280_callback_temperature(const uint8_t num, const float temperature)
{
  // char topic[60];
  debug(F(AT));
  MQTT_data send;
  memory(true);
  // debug(AT);
  // debug_wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // snprintf_P(topic, 60, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/temperature"), num);
    // DT_mqtt_send(topic, temperature);
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/temperature"), num, temperature);
    send_buffer.append(send);
  }
}

void bme280_callback_humidity(const uint8_t num, const float humidity)
{
  debug(F(AT));
  // char topic[60];
  MQTT_data send;
  memory(true);
  // debug_wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // snprintf_P(topic, 60, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/humidity"), num);
    // DT_mqtt_send(topic, humidity);
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/humidity"), num, humidity);
    send_buffer.append(send);
  }
}

void bme280_callback_pressure(const uint8_t num, const float pressure)
{
  debug(F(AT));
  // char topic[60];
  MQTT_data send;
  memory(true);
  // debug_wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // snprintf_P(topic, 60, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/pressure"), num);
    // DT_mqtt_send(topic, pressure);
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/pressure"), num, pressure);
    send_buffer.append(send);
  }
}

void ccs811_callback_co2(const uint8_t num, const float co2)
{
  debug(F(AT));
  // char topic[56];
  MQTT_data send;
  memory(true);
  // debug_wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/co2"), num);
    // DT_mqtt_send(topic, co2);
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/co2"), num, co2);
    send_buffer.append(send);
  }
}

void ccs811_callback_cov(const uint8_t num, const float cov)
{
  // debug(AT);
  MQTT_data send;
  // char topic[56];
  memory(true);
  // debug_wdt_reset();
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/cov"), num);
    // DT_mqtt_send(topic, cov);

    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/cov"), num, cov);
    send_buffer.append(send);
  }
}

#ifdef POELE
void poele_mode_callback(const DT_Poele_mode mode)
{
  // char topic[56];
  debug(F(AT));
  MQTT_data send;
  const char *payload;
  // mode poele
  // debug_wdt_reset();
  if (mem_config.MQTT_online)
  {
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), 56);
    switch (mode)
    {
    case DT_POELE_ARRET:
      // DT_mqtt_send(topic, "Arret");
      payload = PSTR("Arret");
      break;
    case DT_POELE_NORMAL:
      // DT_mqtt_send(topic, "Normal");
      payload = PSTR("Normal");
      break;
    case DT_POELE_ECS:
      // DT_mqtt_send(topic, "ECS");
      payload = PSTR("ECS");
      break;
    case DT_POELE_FORCE:
      // DT_mqtt_send(topic, "Forcé");
      payload = PSTR("Forcé");
      break;
    }
    MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), payload);
    send_buffer.append(send);
  }
}
#endif // POELE

#ifdef VANNES
void dt3voies_callback(const float C2, const float C3)
{
  debug(F(AT));
  // debug_wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    debug(F(AT));
    // char topic[56];
    MQTT_data send;
    send_buffer.reseve(2);
    int32_t digit = C2 * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/state"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/state"), (float)(digit / 100.0));
    send_buffer.append(send);

    // debug_wdt_reset();
    digit = C3 * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/state"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/state"), (float)(digit / 100.0));
    send_buffer.append(send);
  }
}

// retour des valleur du PID PCBT
void dt3voies_callback_pid_pcbt(const float P, const float I, const float D, const float OUT)
{
  // char topic[56];
  // debug(AT);
  // debug_wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    debug(F(AT));
    MQTT_data send;
    send_buffer.reseve(4);

    int32_t digit = P * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/P"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/P"), (float)(digit / 100.0));
    send_buffer.append(send);
    // debug(AT);
    // debug_wdt_reset();
    digit = I * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/I"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/I"), (float)(digit / 100.0));
    send_buffer.append(send);

    // debug(AT);
    // debug_wdt_reset();
    digit = D * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/D"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));

    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/D"), (float)(digit / 100.0));
    send_buffer.append(send);

    // debug(AT);
    // debug_wdt_reset();
    digit = OUT * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/OUT"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/OUT"), (float)(digit / 100.0));
    send_buffer.append(send);
    debug(F(AT));
    memory(false);
  }
}

// retour des valleur du PID MCBT
void dt3voies_callback_pid_mcbt(const float P, const float I, const float D, const float OUT)
{
  // char topic[56];
  // debug_wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    debug(F(AT));
    refresh = now;
    MQTT_data send;
    send_buffer.reseve(4);

    int32_t digit = P * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/P"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/P"), (float)(digit / 100.0));
    send_buffer.append(send);

    // debug_wdt_reset();
    digit = I * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/I"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/I"), (float)(digit / 100.0));
    send_buffer.append(send);

    // debug_wdt_reset();
    digit = D * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/D"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/D"), (float)(digit / 100.0));
    send_buffer.append(send);

    // debug_wdt_reset();
    digit = OUT * 100;
    // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/OUT"), 56);
    // DT_mqtt_send(topic, (float)(digit / 100.0));
    MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/OUT"), (float)(digit / 100.0));
    send_buffer.append(send);
    debug(F(AT));
  }
}
#endif // VANNES

bool mqtt_publish(bool start)
{
  char topic[56];
  MQTT_data send;
  const char *payload;
  // debug(AT);
  static uint8_t sequance = 254;
  static uint8_t num = 0;
  static uint32_t time = 0;
  if (start)
  {
    sequance = 0;
    return false;
  }
  else if (sequance == 0)
    Serial.println(F("mqtt_publish"));

  uint32_t now = millis();

  // Serial.print(millis());
  // debug_wdt_reset();
  if (now - time >= 50)
  {
    time = now;
    switch (sequance)
    {
    case 0:
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/availability"), 56);
      // DT_mqtt_send(topic, "online");
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/availability"), PSTR("online"));
      send_buffer.append(send);
      break;

    case 1:
      if (num < RELAY_NUM)
      {
        relay_callback(num + 1, DT_relay_get(num + 1));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
    case 2:
      if (num < INPUT_NUM)
      {
        input_callback(num + 1, DT_input_get_stats(num + 1));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

      // PT100

    case 3:
#if TEMP_NUM > 0
      if (num < TEMP_NUM)
      {
        // // debug_wdt_reset();
        pt100_callback(num + 1, DT_pt100_get(num + 1));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
#endif
      break;

    case 4:
#ifdef POELE
      // mode poele
      // // debug_wdt_reset();
      poele_mode_callback(DT_Poele_get_mode());
      // EEPROM
      //  V1
#endif
      break;
    case 5:
#ifdef POELE
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.V1);
      // debug(AT);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/state"), eeprom_config.V1);
      send_buffer.append(send);
#endif
      break;
    case 6:
#ifdef POELE
      // V2
      // debug_wdt_reset();
      // debug(AT);
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.V2);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/state"), eeprom_config.V2);
      send_buffer.append(send);
#endif
      break;
    case 7:
#ifdef POELE
      // V3
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.V3);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), eeprom_config.V3);
      send_buffer.append(send);
#endif // POELE
      break;

    case 8:
#ifdef VANNES
      // 3 voies PCBT mode
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), 56);
      switch (DT_3voies_PCBT_get_mode())
      {
      case DT_3VOIES_DEMMARAGE:
        // DT_mqtt_send(topic, "Demmarage");
        payload = PSTR("Demmarage");
        break;
      case DT_3VOIES_NORMAL:
        // DT_mqtt_send(topic, "Normal");
        payload = PSTR("Normal");
        break;
      case DT_3VOIES_MANUAL:
        // DT_mqtt_send(topic, "Manuel");
        payload = PSTR("Manuel");
        break;
      case DT_3VOIES_OFF:
        // DT_mqtt_send(topic, "Arret");
        payload = PSTR("Arret");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), payload);
      send_buffer.append(send);
#endif // vanne
      break;
    case 9:
#ifdef VANNES
      // 3 voies MCBT mode
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), 56);
      switch (DT_3voies_MCBT_get_mode())
      {
      case DT_3VOIES_DEMMARAGE:
        // DT_mqtt_send(topic, "Demmarage");
        payload = PSTR("Demmarage");
        break;
      case DT_3VOIES_NORMAL:
        // DT_mqtt_send(topic, "Normal");
        payload = PSTR("Normal");
        break;
      case DT_3VOIES_MANUAL:
        // DT_mqtt_send(topic, "Manuel");
        payload = PSTR("Manuel");
        break;
      case DT_3VOIES_OFF:
        // DT_mqtt_send(topic, "Arret");
        payload = PSTR("Arret");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), payload);
      send_buffer.append(send);
#endif // vanne
      break;
    case 10:
#ifdef VANNES
      // consigne vanne 3 voies PCBT & MCBT
      dt3voies_callback(DT_3voies_get_C2(), DT_3voies_get_C3());
#endif // vanne
      break;
    case 11:
#ifdef VANNES
      // C4
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C4);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), eeprom_config.C4);
      send_buffer.append(send);
#endif // vanne
      break;
    case 12:
#ifdef VANNES
      // C5
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C5);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), eeprom_config.C5);
      send_buffer.append(send);
#endif // vanne
      break;
    case 13:
#ifdef VANNES
      // C6
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C6);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), eeprom_config.C6);
      send_buffer.append(send);
#endif // vanne
      break;
    case 14:
#ifdef VANNES
      // C7
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C7);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), eeprom_config.C7);
      send_buffer.append(send);
#endif // vanne
      break;
    case 15:
#ifdef VANNES
      // C8
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C8);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C8/state"), eeprom_config.C8);
      send_buffer.append(send);
#endif // vanne
      break;
    case 16:
#ifdef VANNES
      // C9
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C9);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C9/state"), eeprom_config.C9);
      send_buffer.append(send);
#endif // vanne
      break;
    case 17:
#ifdef VANNES
      // C10
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C10);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C10/state"), eeprom_config.C10);
      send_buffer.append(send);
#endif // vanne
      break;
    case 18:
#ifdef VANNES
      // C11
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C11);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C11/state"), eeprom_config.C11);
      send_buffer.append(send);
#endif // vanne
      break;
    case 19:
#ifdef VANNES

      // C_PCBT_MIN
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C_PCBT_MIN);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), eeprom_config.C_PCBT_MIN);
      send_buffer.append(send);

#endif // vanne
      break;
    case 20:
#ifdef VANNES
      // C_PCBT_MAX
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C_PCBT_MAX);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), eeprom_config.C_PCBT_MAX);
      send_buffer.append(send);

#endif // vanne
      break;
    case 21:
#ifdef VANNES
      // C_MCBT_MIN
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C_MCBT_MIN);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), eeprom_config.C_MCBT_MIN);
      send_buffer.append(send);

#endif // vanne
      break;
    case 22:
#ifdef VANNES
      // C_MCBT_MAX
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.C_MCBT_MAX);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), eeprom_config.C_MCBT_MAX);
      send_buffer.append(send);

#endif // vanne
      break;
    case 23:
#ifdef VANNES
      // KP_PCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_PCBT_get_KP());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), DT_3voies_PCBT_get_KP());
      send_buffer.append(send);

#endif // vanne
      break;
    case 24:
#ifdef VANNES
      // KI_PCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_PCBT_get_KI());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), DT_3voies_PCBT_get_KI());
      send_buffer.append(send);

#endif // vanne
      break;
    case 25:
#ifdef VANNES
      // KD_PCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_PCBT_get_KD());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), DT_3voies_PCBT_get_KD());
      send_buffer.append(send);

#endif // vanne
      break;
    case 26:
#ifdef VANNES
      // KT_PCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_PCBT_get_KT());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), DT_3voies_PCBT_get_KT());
      send_buffer.append(send);
#endif // vanne
      break;
    case 27:
#ifdef VANNES
      // KP_MCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_MCBT_get_KP());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), DT_3voies_MCBT_get_KP());
      send_buffer.append(send);

#endif // vanne
      break;
    case 28:
#ifdef VANNES
      // KI_MCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_MCBT_get_KI());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), DT_3voies_MCBT_get_KI());
      send_buffer.append(send);

#endif // vanne
      break;
    case 29:
#ifdef VANNES
      // KD_MCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_MCBT_get_KD());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), DT_3voies_MCBT_get_KD());
      send_buffer.append(send);

#endif // vanne
      break;
    case 30:
#ifdef VANNES
      // KT_MCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), 56);
      // DT_mqtt_send(topic, DT_3voies_MCBT_get_KT());
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), DT_3voies_MCBT_get_KT());
      send_buffer.append(send);

#endif // vanne
      break;
    case 31:
#ifdef VANNES
      // PID PCBT Action
      // debug_wdt_reset(); // clear watchdog
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), 56);
      switch (eeprom_config.pid_pcbt.action)
      {
      case QuickPID::Action::direct:
        // DT_mqtt_send(topic, "direct");
        payload = PSTR("direct");
        break;
      case QuickPID::Action::reverse:
        // DT_mqtt_send(topic, "reverse");
        payload = PSTR("reverse");
        break;
      }

      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 32:
#ifdef VANNES
      // PID PCBT pMode
      // debug_wdt_reset(); // clear watchdog
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), 56);
      switch (eeprom_config.pid_pcbt.pmode)
      {
      case QuickPID::pMode::pOnError:
        // DT_mqtt_send(topic, "pOnError");
        payload = PSTR("pOnError");
        break;
      case QuickPID::pMode::pOnMeas:
        // DT_mqtt_send(topic, "pOnMeas");
        payload = PSTR("pOnMeas");
        break;
      case QuickPID::pMode::pOnErrorMeas:
        // DT_mqtt_send(topic, "pOnErrorMeas");
        payload = PSTR("pOnErrorMeas");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 33:
#ifdef VANNES
      // PID PCBT dMode
      // debug_wdt_reset(); // clear watchdog
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), 56);
      switch (eeprom_config.pid_pcbt.dmode)
      {
      case QuickPID::dMode::dOnError:
        // DT_mqtt_send(topic, "dOnError");
        payload = PSTR("dOnError");
        break;
      case QuickPID::dMode::dOnMeas:
        // DT_mqtt_send(topic, "dOnMeas");
        payload = PSTR("dOnMeas");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 34:
#ifdef VANNES
      // PID PCBT iAwMode
      // debug_wdt_reset(); // clear watchdog
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), 56);
      switch (eeprom_config.pid_pcbt.iawmode)
      {
      case QuickPID::iAwMode::iAwCondition:
        // DT_mqtt_send(topic, "iAwCondition");
        payload = PSTR("iAwCondition");
        break;
      case QuickPID::iAwMode::iAwClamp:
        // DT_mqtt_send(topic, "iAwClamp");
        payload = PSTR("iAwClamp");
        break;
      case QuickPID::iAwMode::iAwOff:
        // DT_mqtt_send(topic, "iAwOff");
        payload = PSTR("iAwOff");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 35:
#ifdef VANNES
      // PID MCBT Action
      // debug_wdt_reset(); // clear watchdog
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), 56);
      switch (eeprom_config.pid_mcbt.action)
      {
      case QuickPID::Action::direct:
        // DT_mqtt_send(topic, "direct");
        payload = PSTR("direct");
        break;
      case QuickPID::Action::reverse:
        // DT_mqtt_send(topic, "reverse");
        payload = PSTR("reverse");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 36:
#ifdef VANNES
      // PID MCBT pMode
      // debug_wdt_reset(); // clear watchdog
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), 56);
      switch (eeprom_config.pid_mcbt.pmode)
      {
      case QuickPID::pMode::pOnError:
        // DT_mqtt_send(topic, "pOnError");
        payload = PSTR("pOnError");
        break;
      case QuickPID::pMode::pOnMeas:
        // DT_mqtt_send(topic, "pOnMeas");
        payload = PSTR("pOnMeas");
        break;
      case QuickPID::pMode::pOnErrorMeas:
        // DT_mqtt_send(topic, "pOnErrorMeas");
        payload = PSTR("pOnErrorMeas");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 37:
#ifdef VANNES
      // PID MCBT dMode
      // debug_wdt_reset(); // clear watchdog
      strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), 56);
      switch (eeprom_config.pid_mcbt.dmode)
      {
      case QuickPID::dMode::dOnError:
        // DT_mqtt_send(topic, "dOnError");
        payload = PSTR("dOnError");
        break;
      case QuickPID::dMode::dOnMeas:
        // DT_mqtt_send(topic, "dOnMeas");
        payload = PSTR("dOnMeas");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 38:
#ifdef VANNES
      // PID MCBT iAwMode
      // debug_wdt_reset(); // clear watchdog
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), 56);
      switch (eeprom_config.pid_mcbt.iawmode)
      {
      case QuickPID::iAwMode::iAwCondition:
        // DT_mqtt_send(topic, "iAwCondition");
        payload = PSTR("iAwCondition");
        break;
      case QuickPID::iAwMode::iAwClamp:
        // DT_mqtt_send(topic, "iAwClamp");
        payload = PSTR("iAwClamp");
        break;
      case QuickPID::iAwMode::iAwOff:
        // DT_mqtt_send(topic, "iAwOff");
        payload = PSTR("iAwOff");
        break;
      }
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), payload);
      send_buffer.append(send);

#endif // vanne
      break;
    case 39:
#ifdef VANNES
      // RATIO PCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.ratio_PCBT);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), eeprom_config.ratio_PCBT);
      send_buffer.append(send);

#endif // vanne
      break;
    case 40:
#ifdef VANNES
      // RATIO MCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.ratio_MCBT);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), eeprom_config.ratio_MCBT);
      send_buffer.append(send);

#endif // vanne
      break;
    case 41:
#ifdef VANNES
      // OFFSET_PCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.out_offset_PCBT);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), eeprom_config.out_offset_PCBT);
      send_buffer.append(send);

#endif // vanne
      break;
    case 42:
#ifdef VANNES
      // OFFSET_MCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.out_offset_MCBT);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), eeprom_config.out_offset_MCBT);
      send_buffer.append(send);

#endif // vanne
      break;
    case 43:
#ifdef VANNES
      // OFFSET_PCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.in_offset_PCBT);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), eeprom_config.in_offset_PCBT);
      send_buffer.append(send);

#endif // vanne
      break;
    case 44:
#ifdef VANNES
      // OFFSET_MCBT
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), 56);
      // DT_mqtt_send(topic, eeprom_config.in_offset_MCBT);
      MQTT_data_store(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), eeprom_config.in_offset_MCBT);
      send_buffer.append(send);
#endif
      break;

    case 45:
      // ONLINE
      // debug_wdt_reset();
      // strlcpy_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), 56);
      // DT_mqtt_send(topic, "online");
      MQTT_data_store_P(send, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), PSTR("online"));
      send_buffer.append(send);
      break;

    default:
      return true;
      break;
    }

    sequance += 1;
    memory(true);
  }
  return false;
}

bool mqtt_subscribe(MQTTClient &mqtt, bool start)
{
  char topic[56];
  memory(false);
  uint32_t now = millis();
  // Serial.print(millis());
  // debug_wdt_reset();
  static uint8_t sequance = 254;
  static uint8_t num = 0;
  static uint32_t time = 0;
  if (start)
  {
    sequance = 0;
    return false;
  }
  else if (sequance == 0)
    Serial.println(F("mqtt_subscribe"));

  // Serial.print(millis());
  // debug_wdt_reset();
  if (now - time >= 50)
  {
    time = now;
    switch (sequance)
    {
    case 0:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/mode_set");
      break;
    case 1:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/temp_set");
      break;
    case 2:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/away_set");
      break;
    case 3:

      // relay
      // for (uint8_t num = 0; num < RELAY_NUM; ++num)
      // {
      //   // debug_wdt_reset();
      //   snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/set"), num + 1);
      //   mqtt.subscribe(topic);
      // mqtt.loop();
      // }
      if (num < RELAY_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/set"), num + 1);
        mqtt.subscribe(topic);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }

#ifdef POELE
      // Poele
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/set");
      // 3 voies PCBT mode
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/set");
      break;
    case 4:
#endif // POELE

      // 3 voies PCBT consigne
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/set");
      // 3 voies MCBT mode
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/set");
      // 3 voies MCBT consigne
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/set");

      break;
    case 5:
#ifdef POELE
      // EEPROM
      //  V1
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/set");

      // C7
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set");
#endif // POELE
      break;
    case 6:

      // V2
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/set");
      break;
    case 7:

      // V3
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/set");
      break;
    case 8:

      // C4
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set");
      break;
    case 9:

      // C5
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/set");
      break;
    case 10:

      // C6
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set");
      break;
    case 11:

      // C8
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/set");
      break;
    case 12:

      // C9
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set");
      break;
    case 13:

      // C10
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set");
      break;
    case 14:

      // C11
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set");
      break;
    case 15:

      // C_PCBT_MIN
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set");
      break;
    case 16:

      // C_PCBT_MAX
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set");
      break;
    case 17:

      // C_MCBT_MIN
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set");
      break;
    case 18:

      // C_MCBT_MAX
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set");
      break;
    case 19:

      // KP_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set");
      break;
    case 20:

      // KI_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set");
      break;
    case 21:

      // KD_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set");
      break;
    case 22:

      // KT_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set");
      break;
    case 23:

      // KP_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set");
      break;
    case 24:

      // KI_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set");
      break;
    case 25:

      // KD_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set");
      break;
    case 26:

      // KT_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set");
      break;
    case 27:

      // PID PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/set");
      break;
    case 28:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/set");
      break;
    case 29:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/set");
      break;
    case 30:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/set");

      break;
    case 31:

      // PID MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/set");
      break;
    case 32:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/set");
      break;
    case 33:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/set");
      break;
    case 34:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/set");
      break;
    case 35:

      // RATIO
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/set");
      break;
    case 36:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set");
      break;
    case 37:

      // OFFSET OUT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set");
      break;
    case 38:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set");
      break;
    case 39:

      // OFFSET IN
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set");
      break;
    case 40:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set");
      break;
    case 41:

      // HomeAssistant
      mqtt.subscribe("homeassistant/status");
      break;

    default:
      return true;
      break;
    }

    sequance += 1;
    memory(true);
  }
  return false;
}

void mqtt_receve(MQTTClient *client, const char topic[], const char payload[], const int length)
{

  debug(F(AT));
  char buffer[56];
  memory(true);
  // debug_wdt_reset();
  String str_buffer;
  uint32_t now = millis();
  Serial.print("receve topic ");
  Serial.println(topic);

  // Copy the payload to the new buffer
  if (length < 56)
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
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/state"), 56);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/set") == 0) // V2
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.V2 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/state"), 56);
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
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Normal");
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_DEMMARAGE);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Demmarage");
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_MANUAL);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Manuel");
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_OFF);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Arret");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/set") == 0) // Mode de la vannes 3 voie PCBT
  {
    str_buffer = buffer;
    DT_3voies_set_C2(str_buffer.toFloat());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    if (strcmp(buffer, "Normal") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_NORMAL);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Normal");
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_DEMMARAGE);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Demmarage");
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_MANUAL);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Manuel");
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_OFF);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), 56);
      DT_mqtt_send(buffer, "Arret");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    str_buffer = buffer;
    DT_3voies_set_C3(str_buffer.toFloat());
  }
  else if (strcmp_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/set")) == 0) // V3
  {
    Serial.print("set V3 = ");
    str_buffer = buffer;
    eeprom_config.V3 = str_buffer.toFloat();
    Serial.print(eeprom_config.V3);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.V3);
    sauvegardeEEPROM();

    Serial.println(" ");
  }
  else if (strcmp_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set")) == 0) // C4
  {
    str_buffer = buffer;
    eeprom_config.C4 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C4);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/set") == 0) // C5
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C5 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), 56);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set") == 0) // C6
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C6 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), 56);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set") == 0) // C7
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C7 = u8t_value;
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), 56);
      DT_mqtt_send(buffer, u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/set") == 0) // C8
  {
    str_buffer = buffer;
    eeprom_config.C8 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C8);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set") == 0) // C9
  {
    str_buffer = buffer;
    eeprom_config.C9 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C9);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set") == 0) // C10
  {
    str_buffer = buffer;
    eeprom_config.C10 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C10);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set") == 0) // C11
  {
    str_buffer = buffer;
    eeprom_config.C11 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C11);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set") == 0) // C_PCBT_MIN
  {
    str_buffer = buffer;
    eeprom_config.C_PCBT_MIN = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C_PCBT_MIN);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set") == 0) // C_PCBT_MAX
  {
    str_buffer = buffer;
    eeprom_config.C_PCBT_MAX = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set") == 0) // C_MCBT_MIN
  {
    str_buffer = buffer;
    eeprom_config.C_MCBT_MIN = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C_MCBT_MIN);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set") == 0) // C_MCBT_MAX
  {
    str_buffer = buffer;
    eeprom_config.C_MCBT_MAX = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.C_MCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set") == 0) // KP_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KP(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KP());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set") == 0) // KI_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KI(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KI());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set") == 0) // KD_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KD(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KD());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set") == 0) // KT_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KT(str_buffer.toInt());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KT());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set") == 0) // KP_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KP(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KP());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set") == 0) // KI_MCBT
  {
    str_buffer = buffer;

    DT_3voies_MCBT_set_KI(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KI());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set") == 0) // KD_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KD(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KD());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set") == 0) // KT_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KT(str_buffer.toInt());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), 56);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KT());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/set") == 0) // PCBT Action
  {
    if (strcmp(buffer, "direct") == 0)
    {
      DT_3voies_PCBT_set_action(QuickPID::Action::direct);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), 56);
      DT_mqtt_send(buffer, "direct");
    }
    if (strcmp(buffer, "reverse") == 0)
    {
      DT_3voies_PCBT_set_action(QuickPID::Action::reverse);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), 56);
      DT_mqtt_send(buffer, "reverse");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/set") == 0) // PCBT pMode
  {
    if (strcmp(buffer, "pOnError") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), 56);
      DT_mqtt_send(buffer, "pOnError");
    }
    if (strcmp(buffer, "pOnErrorMeas") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnErrorMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), 56);
      DT_mqtt_send(buffer, "pOnErrorMeas");
    }
    if (strcmp(buffer, "pOnMeas") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), 56);
      DT_mqtt_send(buffer, "pOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/set") == 0) // PCBT dMode
  {
    if (strcmp(buffer, "dOnError") == 0)
    {
      DT_3voies_PCBT_set_dmode(QuickPID::dMode::dOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), 56);
      DT_mqtt_send(buffer, "dOnError");
    }
    if (strcmp(buffer, "dOnMeas") == 0)
    {
      DT_3voies_PCBT_set_dmode(QuickPID::dMode::dOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), 56);
      DT_mqtt_send(buffer, "dOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/set") == 0) // PCBT iawmode
  {
    if (strcmp(buffer, "iAwClamp") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwClamp);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), 56);
      DT_mqtt_send(buffer, "iAwClamp");
    }
    if (strcmp(buffer, "iAwCondition") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwCondition);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), 56);
      DT_mqtt_send(buffer, "iAwCondition");
    }
    if (strcmp(buffer, "iAwOff") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwOff);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), 56);
      DT_mqtt_send(buffer, "iAwOff");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/set") == 0) // MCBT Action
  {
    if (strcmp(buffer, "direct") == 0)
    {
      DT_3voies_MCBT_set_action(QuickPID::Action::direct);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), 56);
      DT_mqtt_send(buffer, "direct");
    }
    if (strcmp(buffer, "reverse") == 0)
    {
      DT_3voies_MCBT_set_action(QuickPID::Action::reverse);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), 56);
      DT_mqtt_send(buffer, "reverse");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/set") == 0) // MCBT pMode
  {
    if (strcmp(buffer, "pOnError") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), 56);
      DT_mqtt_send(buffer, "pOnError");
    }
    if (strcmp(buffer, "pOnErrorMeas") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnErrorMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), 56);
      DT_mqtt_send(buffer, "pOnErrorMeas");
    }
    if (strcmp(buffer, "pOnMeas") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), 56);
      DT_mqtt_send(buffer, "pOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/set") == 0) // MCBT dMode
  {
    if (strcmp(buffer, "dOnError") == 0)
    {
      DT_3voies_MCBT_set_dmode(QuickPID::dMode::dOnError);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), 56);
      DT_mqtt_send(buffer, "dOnError");
    }
    if (strcmp(buffer, "dOnMeas") == 0)
    {
      DT_3voies_MCBT_set_dmode(QuickPID::dMode::dOnMeas);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), 56);
      DT_mqtt_send(buffer, "dOnMeas");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/set") == 0) // MCBT iawmode
  {
    if (strcmp(buffer, "iAwClamp") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwClamp);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), 56);
      DT_mqtt_send(buffer, "iAwClamp");
    }
    if (strcmp(buffer, "iAwCondition") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwCondition);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), 56);
      DT_mqtt_send(buffer, "iAwCondition");
    }
    if (strcmp(buffer, "iAwOff") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwOff);
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), 56);
      DT_mqtt_send(buffer, "iAwOff");
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/set") == 0) // RATIO_PCBT
  {
    str_buffer = buffer;
    eeprom_config.ratio_PCBT = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.ratio_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set") == 0) // RATIO_PCBT
  {
    str_buffer = buffer;
    eeprom_config.ratio_MCBT = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.ratio_MCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set") == 0) // OFFSET_PCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_offset_PCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.out_offset_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set") == 0) // OFFSET_MCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_offset_MCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.out_offset_MCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set") == 0) // OFFSET_PCBT_IN
  {
    str_buffer = buffer;
    eeprom_config.in_offset_PCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.in_offset_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set") == 0) // OFFSET_MCBT_IN
  {
    str_buffer = buffer;
    eeprom_config.in_offset_MCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), 56);
    DT_mqtt_send(buffer, eeprom_config.in_offset_MCBT);
    sauvegardeEEPROM();
  }

#endif                                                 // VANNES
  else if (strcmp(topic, "homeassistant/status") == 0) // Home Assistant Online / Offline
  {
    if (strcmp(buffer, "online") == 0)
    {
      mqtt_publish(true);
      mem_config.MQTT_online = true; // TODO : ne fonctionne plus
    }
    else if (strcmp(buffer, "offline") == 0)
    {
      mem_config.MQTT_online = false; // TODO : ne fonctionne plus
    }
  }
  Serial.print(F("malloc margin= "));
  Serial.println((uint16_t)(__malloc_margin));
  Serial.print(F("stack size= "));
  Serial.println((uint16_t)(RAMEND - SP));
  Serial.print(F("end mqtt_receve = "));
  Serial.println(millis() - now);
}
#endif // MQTT

void setup()
{
  // Serial.begin(9600);
  Serial.begin(57600);
  Serial.println(F("starting board"));
  memory(true);
  // auto Serial.println("starting board version " BOARD_SW_VERSION_PRINT);

  Serial.println("Load eeprom");
  chargeEEPROM();
  memory(false);

  Wire.begin();
  memory(false);
  // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
  // Wire.write(MCP_CHANNEL); // channel 1
  // Wire.endTransmission();

  Serial.println("init mcp");
  DT_mcp_init();

  memory(false);

#ifdef MQTT
  // Serial.print(millis());
  Serial.println(F("starting mqtt"));
  DT_mqtt_init();
  Serial.println(F("mqtt initialized"));
  DT_mqtt_set_subscribe_callback(mqtt_subscribe);
  DT_mqtt_set_receve_callback(mqtt_receve);
  DT_mqtt_set_publish_callback(mqtt_publish);

#endif // MQTT

  Serial.println("starting relay");
  DT_relay_init();
#ifdef MQTT
  DT_relay_set_callback(relay_callback);
#endif // MQTT
  Serial.println("starting input");
  DT_input_init();
  DT_input_set_callback(input_callback);

#if TEMP_NUM > 0
  Serial.println("starting PT100");
  DT_pt100_init();
#ifdef MQTT
  DT_pt100_set_callback(pt100_callback);
#endif // MQTT
#endif // PT100

  Serial.println("starting BME280");
  DT_BME280_init();
#ifdef MQTT
  DT_BME280_set_callback_temperature(bme280_callback_temperature);
  DT_BME280_set_callback_humidity(bme280_callback_humidity);
  DT_BME280_set_callback_pressure(bme280_callback_pressure);
#endif // MQTT

  Serial.println("starting BCCS811");
  DT_CCS811_init();
#ifdef MQTT
  DT_CCS811_set_callback_co2(ccs811_callback_co2);
  DT_CCS811_set_callback_cov(ccs811_callback_cov);
#endif // MQTT

  // auto Serial.println("starting fake_NTC");
  // DT_fake_ntc_init();
  // DT_fake_ntc_callback(fake_ntc_callback);

  Serial.println("starting Poele");
#ifdef POELE
  DT_Poele_init();
#ifdef MQTT
  DT_Poele_set_mode_callback(poele_mode_callback);
#endif // MQTT
#endif // POELE

  Serial.println("starting 3 voies");
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
  homeassistant(true);
  // Serial.print(millis());
  Serial.println(F("Board started"));
  Serial.print(F("version: "));
  Serial.println(F(BOARD_SW_VERSION_PRINT));
  memory(true);
}

void loop()
{
  // debug(AT);
  uint32_t now = millis();

  debug_wdt_reset(F(AT));
  ;

#ifdef MQTT
  DT_mqtt_loop();
  // mqtt_publish(false);
#endif
  DT_relay_loop();
  DT_input_loop();
  // DT_BME280_loop();
  // DT_CCS811_loop();
#if TEMP_NUM > 0
  DT_pt100_loop();
#endif
#ifdef POELE
  DT_Poele_loop();
#endif
#ifdef VANNES
  DT_3voies_loop();
#endif
  load();
  memory(false);
  //  DT_fake_ntc_loop();

  // adjust CCS811
  // static uint32_t ccs811_environmental = 0;
  // if (now - ccs811_environmental > 600000) // toute les 10 minutes
  // {
  //   ccs811_environmental = now;
  //   float humidity = DT_BME280_get_humidity(1);
  //   float temperature = DT_BME280_get_temperature(1);
  //   DT_CCS811_set_environmental_data(1, humidity, temperature);
  //   humidity = DT_BME280_get_humidity(2);
  //   temperature = DT_BME280_get_temperature(2);
  //   DT_CCS811_set_environmental_data(2, humidity, temperature);
  // }

  static uint32_t old = 0;
  if (now - old > 1000)
  {
    old = now;
    // test de buffer
    // MQTT_BUFF snd;
    // MQTT_BUFF rcv;
    // char topic[16];
    // char payload[16];
    // snd.store_P(PSTR("TOPIC1"), PSTR("PAYLODAD1"));
    // debug(AT);
    // // memory(true);
    // // buffer_test.reseve(3);
    // debug(AT);
    // memory(true);
    // buffer_test.append(snd);
    // Serial.print(F("buffer size = "));
    // Serial.println(buffer_test.size());
    // debug(AT);
    // memory(true);
    // snd.store_P(PSTR("TOPIC2"), PSTR("PAYLODAD2"));
    // buffer_test.append(snd);
    // Serial.print(F("buffer size = "));
    // Serial.println(buffer_test.size());
    // memory(true);
    // snd.store_P(PSTR("TOPIC3"), PSTR("PAYLODAD3"));
    // buffer_test.append(snd);
    // Serial.print(F("buffer size = "));
    // Serial.println(buffer_test.size());
    // memory(true);

    // debug(AT);
    // rcv = buffer_test.get();
    // Serial.print(F("buffer size = "));
    // Serial.println(buffer_test.size());
    // memory(true);
    // rcv.get(topic, 16, payload, 16);
    // Serial.print(F("buffer topic = "));
    // Serial.print(topic);
    // Serial.print(F(" payload = "));
    // Serial.println(payload);

    // debug(AT);
    // rcv = buffer_test.get();
    // Serial.print(F("buffer size = "));
    // Serial.println(buffer_test.size());
    // memory(true);
    // rcv.get(topic, 16, payload, 16);
    // Serial.print(F("buffer topic = "));
    // Serial.print(topic);
    // Serial.print(F(" payload = "));
    // Serial.println(payload);

    // debug(AT);
    // rcv = buffer_test.get();
    // Serial.print(F("buffer size = "));
    // Serial.println(buffer_test.size());
    // memory(true);
    // rcv.get(topic, 16, payload, 16);
    // Serial.print(F("buffer topic = "));
    // Serial.print(topic);
    // Serial.print(F(" payload = "));
    // Serial.println(payload);

    /*
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
  // static uint32_t heartbeat_time = 0;
  // static bool heartbeat_status = false;
  // if (now - heartbeat_time > 1000) // Backup data in eeprom
  // {
  //   heartbeat_time = now;
  //   if (heartbeat_status == false)
  //     heartbeat_status = true;
  //   else
  //     heartbeat_status = false;

  //   strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/heartbeat"), BUFFER_SIZE);
  //  DT_mqtt_send(buffer, heartbeat_status);
  // }
#endif // MQTT
       /*
       static uint32_t save_eeprom = 0;
       if (now - save_eeprom > SAVE_EEPROM) // Backup data in eeprom
       {
         save_eeprom = now;
         sauvegardeEEPROM();
       }
     */
}
