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

#include <avr/wdt.h> //watchdog

#include <ArduinoJson.h>

// #include <pinout.h>
#include <config.h>

// #include "Wire.h"
// #include "DFRobot_CCS811.h"
// DFRobot_CCS811 CCS811;

// Adafruit_CCS811 ccs811;

long int lastReconnectAttempt = 0;

void debug(const char *var)
{
  Serial.println(var);
}

extern void *__brkval;
void memory(void)
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
}

#ifdef MQTT

// Relay Callback
void relay_callback(const uint8_t num, const bool action)
{
  debug(AT);
  wdt_reset();
  if (mem_config.MQTT_online)
  {
    // sprintf_P(buffer, PSTR("relais numero %d dans l etat %d"), num, (int)action);
    //  auto Serial.println(buffer);
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/state"), num);
    if (action)
    {
      DT_mqtt_send(buffer, "ON");
    }
    else
    {
      DT_mqtt_send(buffer, "OFF");
    }
  }
}
#endif // MQTT

void input_callback(const uint8_t num, const Bt_Action action)
{
  wdt_reset();
  // Serial.print(F("entrée numero "));
  // Serial.print(num);
  // Serial.print(F(" dans l etat "));

  if (mem_config.MQTT_online)
  {
#ifdef MQTT
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num);
#endif
    switch (action)
    {
    case IN_PUSHED:
      // Serial.println("ON");
#ifdef MQTT
      DT_mqtt_send(buffer, "ON");
#endif
      break;

    case IN_RELEASE:
      // Serial.println("OFF");
#ifdef MQTT
      DT_mqtt_send(buffer, "OFF");
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
}

#ifdef MQTT
#if TEMP_NUM > 0
void pt100_callback(const uint8_t num, const float temp)
{
  debug(AT);
  wdt_reset();
  Serial.println("PT100_CALLBACK ");

  if (mem_config.MQTT_online)
  {
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d/temperature"), num);
    // JsonVariant variant = doc.to<JsonVariant>();
    // variant.set(temp);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);

    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, temp);
  }
}
#endif // PT100

void bme280_callback_temperature(const uint8_t num, const float temperature)
{
  // debug(AT);
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/temperature"), num);
    // JsonVariant variant = doc.to<JsonVariant>();
    // variant.set(temperature);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, temperature);
  }
}

void bme280_callback_humidity(const uint8_t num, const float humidity)
{
  // debug(AT);
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/humidity"), num);
    // JsonVariant variant = doc.to<JsonVariant>();
    // variant.set(humidity);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, humidity);
  }
}

void bme280_callback_pressure(const uint8_t num, const float pressure)
{
  // debug(AT);
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/pressure"), num);
    // JsonVariant variant = doc.to<JsonVariant>();
    // variant.set(pressure);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, pressure);
  }
}

void ccs811_callback_co2(const uint8_t num, const float co2)
{
  // debug(AT);
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/co2"), num);
    // JsonVariant variant = doc.to<JsonVariant>();
    // variant.set(co2);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, co2);
  }
}

void ccs811_callback_cov(const uint8_t num, const float cov)
{
  // debug(AT);
  wdt_reset();
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/cov"), num);
    // JsonVariant variant = doc.to<JsonVariant>();
    // variant.set(cov);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, cov);
  }
}

#ifdef POELE
void poele_mode_callback(const DT_Poele_mode mode)
{
  debug(AT);
  // mode poele
  wdt_reset();
  if (mem_config.MQTT_online)
  {
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
  }}
}
#endif // POELE

#ifdef VANNES
void dt3voies_callback(const float C2, const float C3)
{
  debug(AT);
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // JsonVariant variant = doc.to<JsonVariant>();
    int32_t digit = C2 * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/state"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));

    wdt_reset();
    // variant = doc.to<JsonVariant>();
    digit = C3 * 100;
    // variant.set((float)digit / 100.0);
    //  serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    //  strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/state"), BUFFER_SIZE);
    //  DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));
  }
}

// retour des valleur du PID PCBT
void dt3voies_callback_pid_pcbt(const float P, const float I, const float D, const float OUT)
{
  debug(AT);
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // JsonVariant variant = doc.to<JsonVariant>();
    int32_t digit = P * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/P"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));

    wdt_reset();
    // variant = doc.to<JsonVariant>();
    digit = I * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/I"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));

    wdt_reset();
    // variant = doc.to<JsonVariant>();
    digit = D * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/D"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));

    wdt_reset();
    // variant = doc.to<JsonVariant>();
    digit = OUT * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/OUT"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));
  }
}

// retour des valleur du PID MCBT
void dt3voies_callback_pid_mcbt(const float P, const float I, const float D, const float OUT)
{
  debug(AT);
  wdt_reset();

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // JsonVariant variant = doc.to<JsonVariant>();
    int32_t digit = P * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/P"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));

    wdt_reset();
    // variant = doc.to<JsonVariant>();
    digit = I * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/I"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));

    wdt_reset();
    // variant = doc.to<JsonVariant>();
    digit = D * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/D"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));

    wdt_reset();
    // variant = doc.to<JsonVariant>();
    digit = OUT * 100;
    // variant.set((float)digit / 100.0);
    // serializeJson(variant, buffer_value, BUFFER_VALUE_SIZE);
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/OUT"), BUFFER_SIZE);
    // DT_mqtt_send(buffer, buffer_value);
    DT_mqtt_send(buffer, (float)(digit / 100.0));
  }
}
#endif // VANNES

void mqtt_publish(bool start)
{
  // debug(AT);
  static uint8_t sequance = 254;
  static uint8_t num = 0;
  static uint32_t time = 0;
  if (start)
  {
    sequance = 0;
    return;
  }
  else if (sequance == 0)
    Serial.println(F("mqtt_publish"));

  uint32_t now = millis();

  // Serial.print(millis());
  wdt_reset();
  if (now - time >= 100 && mem_config.MQTT_online)
  {
    switch (sequance)
    {
    case 0:
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/availability"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "online");
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
        // wdt_reset();
        pt100_callback(num + 1, DT_pt100_get(num + 1));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif

    case 4:
#ifdef POELE
      // mode poele
      // wdt_reset();
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
      break;

    case 5:
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
      break;

    case 6:
      // ONLINE
      wdt_reset();
      strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), BUFFER_SIZE);
      DT_mqtt_send(buffer, "online");
      break;

    default:
      return;
      break;
    }

    sequance += 1;
  }
}

void mqtt_subscribe(MQTTClient &mqtt)
{

  uint32_t now = millis();
  // Serial.print(millis());
  Serial.print(F("mqtt_subscribe = "));
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/mode_set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/temp_set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/away_set");
  mqtt.loop();

  // relay
  for (uint8_t num = 0; num < RELAY_NUM; ++num)
  {
    wdt_reset();
    sprintf_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/set"), num + 1);
    mqtt.subscribe(buffer);
    mqtt.loop();
  }

#ifdef POELE
  // Poele
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/set");
  mqtt.loop();
  // 3 voies PCBT mode
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/set");
  mqtt.loop();
  // 3 voies PCBT consigne
#endif // POELE

  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/set");
  mqtt.loop();
  // 3 voies MCBT mode
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/set");
  mqtt.loop();
  // 3 voies MCBT consigne
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/set");
  mqtt.loop();

#ifdef POELE
  // EEPROM
  //  V1
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/set");
  mqtt.loop();

  // C7
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set");
  mqtt.loop();
#endif // POELE

  // V2
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/set");
  mqtt.loop();

  // V3
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/set");
  mqtt.loop();

  // C4
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set");
  mqtt.loop();

  // C5
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/set");
  mqtt.loop();

  // C6
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set");
  mqtt.loop();

  // C8
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/set");
  mqtt.loop();

  // C9
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set");
  mqtt.loop();

  // C10
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set");
  mqtt.loop();

  // C11
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set");
  mqtt.loop();

  // C_PCBT_MIN
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set");
  mqtt.loop();

  // C_PCBT_MAX
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set");
  mqtt.loop();

  // C_MCBT_MIN
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set");
  mqtt.loop();

  // C_MCBT_MAX
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set");
  mqtt.loop();

  // KP_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set");
  mqtt.loop();

  // KI_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set");
  mqtt.loop();

  // KD_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set");
  mqtt.loop();

  // KT_PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set");
  mqtt.loop();

  // KP_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set");
  mqtt.loop();

  // KI_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set");
  mqtt.loop();

  // KD_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set");
  mqtt.loop();

  // KT_MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set");
  mqtt.loop();

  // PID PCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/set");
  mqtt.loop();

  // PID MCBT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/set");
  mqtt.loop();

  // RATIO
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set");
  mqtt.loop();

  // OFFSET OUT
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set");
  mqtt.loop();

  // OFFSET IN
  wdt_reset();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set");
  mqtt.loop();
  mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set");
  mqtt.loop();

  // HomeAssistant
  mqtt.subscribe("homeassistant/status");
  mqtt.loop();

  Serial.println(millis() - now);

  // homeassistant(true);
  // mqtt_publish();
}

void mqtt_receve(MQTTClient *client, const char topic[], const char payload[], const int length)
{

  wdt_reset();
  String str_buffer;
  uint32_t now = millis();
  Serial.print("receve topic ");
  Serial.println(topic);

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
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), "Normal");
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_DEMMARAGE);
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), "Demmarage");
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_MANUAL);
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), "Manuel");
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_OFF);
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), "Arret");
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
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), "Normal");
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_DEMMARAGE);
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), "Demmarage");
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_MANUAL);
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), "Manuel");
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_OFF);
      // strlcpy_P(buffer_value, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), BUFFER_SIZE);
      DT_mqtt_send(PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), "Arret");
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
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.V3);
    sauvegardeEEPROM();

    Serial.println(" ");
  }
  else if (strcmp_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set")) == 0) // C4
  {
    str_buffer = buffer;
    eeprom_config.C4 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C4);
    sauvegardeEEPROM();
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
    str_buffer = buffer;
    eeprom_config.C8 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C8);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set") == 0) // C9
  {
    str_buffer = buffer;
    eeprom_config.C9 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C9);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set") == 0) // C10
  {
    str_buffer = buffer;
    eeprom_config.C10 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C10);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set") == 0) // C11
  {
    str_buffer = buffer;
    eeprom_config.C11 = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C11);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set") == 0) // C_PCBT_MIN
  {
    str_buffer = buffer;
    eeprom_config.C_PCBT_MIN = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C_PCBT_MIN);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set") == 0) // C_PCBT_MAX
  {
    str_buffer = buffer;
    eeprom_config.C_PCBT_MAX = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set") == 0) // C_MCBT_MIN
  {
    str_buffer = buffer;
    eeprom_config.C_MCBT_MIN = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C_MCBT_MIN);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set") == 0) // C_MCBT_MAX
  {
    str_buffer = buffer;
    eeprom_config.C_MCBT_MAX = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.C_MCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set") == 0) // KP_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KP(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KP());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set") == 0) // KI_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KI(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KI());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set") == 0) // KD_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KD(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KD());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set") == 0) // KT_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KT(str_buffer.toInt());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_PCBT_get_KT());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set") == 0) // KP_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KP(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KP());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set") == 0) // KI_MCBT
  {
    str_buffer = buffer;

    DT_3voies_MCBT_set_KI(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KI());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set") == 0) // KD_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KD(str_buffer.toFloat());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KD());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set") == 0) // KT_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KT(str_buffer.toInt());
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, DT_3voies_MCBT_get_KT());
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
    str_buffer = buffer;
    eeprom_config.ratio_PCBT = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.ratio_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set") == 0) // RATIO_PCBT
  {
    str_buffer = buffer;
    eeprom_config.ratio_MCBT = str_buffer.toFloat();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.ratio_MCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set") == 0) // OFFSET_PCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_offset_PCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.out_offset_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set") == 0) // OFFSET_MCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_offset_MCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.out_offset_MCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set") == 0) // OFFSET_PCBT_IN
  {
    str_buffer = buffer;
    eeprom_config.in_offset_PCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.in_offset_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set") == 0) // OFFSET_MCBT_IN
  {
    str_buffer = buffer;
    eeprom_config.in_offset_MCBT = str_buffer.toInt();
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), BUFFER_SIZE);
    DT_mqtt_send(buffer, eeprom_config.in_offset_MCBT);
    sauvegardeEEPROM();
  }

#endif                                                 // VANNES
  else if (strcmp(topic, "homeassistant/status") == 0) // Home Assistant Online / Offline
  {
    if (strcmp(buffer, "online") == 0)
    {
      // homeassistant(true);
      mqtt_publish(true);
      mem_config.MQTT_online = true;
    }
    else if (strcmp(buffer, "offline") == 0)
    {
      mem_config.MQTT_online = false;
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
  memory();
  // auto Serial.println("starting board version " BOARD_SW_VERSION_PRINT);

  Serial.println("Load eeprom");
  chargeEEPROM();
  memory();

  Wire.begin();
  memory();
  // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
  // Wire.write(MCP_CHANNEL); // channel 1
  // Wire.endTransmission();

  Serial.println("init mcp");
  DT_mcp_init();

  memory();

#ifdef MQTT
  // Serial.print(millis());
  Serial.println(F("starting mqtt"));
  DT_mqtt_init();
  Serial.println(F("mqtt initialized"));
  DT_mqtt_set_subscribe_callback(mqtt_subscribe);
  DT_mqtt_set_receve_callback(mqtt_receve);
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
  // Serial.print(millis());
  Serial.println(F("Board started"));
  Serial.print(F("version: "));
  Serial.println(F(BOARD_SW_VERSION_PRINT));
}

void loop()
{
  // debug(AT);
  uint32_t now = millis();

  wdt_reset();

#ifdef MQTT
  DT_mqtt_loop();
  mqtt_publish(false);
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
  static uint32_t load_1s_count = 0;
  static uint32_t load_1s_time = 0;
  load_1s_count += 1;
  if (now - load_1s_time >= 1000)
  {
    float load = ((now - load_1s_time) / 20.0) / load_1s_count;

    load_1s_count = load * 100;

#ifdef MQTT
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1s"), BUFFER_SIZE);
    debug(AT);
    DT_mqtt_send(buffer, (float)(load_1s_count / 100.0));
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
    strlcpy_P(buffer, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1m"), BUFFER_SIZE);
    debug(AT);
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
      // Serial.print(millis());
      Serial.print(F("new dynamic alocation heap = "));
      Serial.println((uint16_t)old_heap, HEX);

      // Serial.print(millis());
      Serial.print(F("heap size= "));
      Serial.println((uint16_t)(old_heap - __malloc_heap_start));

      Serial.print(F("malloc margin= "));
      Serial.println((uint16_t)(__malloc_margin));

      Serial.print(F("stack size= "));
      Serial.println((uint16_t)(RAMEND - SP));

      Serial.print(F("brkval= "));
      Serial.println((uint16_t)(__brkval));

      Serial.print(F("old_heap= "));
      Serial.println((uint16_t)(old_heap));

      Serial.print(F("SP= "));
      Serial.println((uint16_t)(SP));

      Serial.print(F("free memory= "));
      Serial.println((uint16_t)((uint16_t)SP - (uint16_t)old_heap));
    }
    free(heap);
  }
  memory();
}
