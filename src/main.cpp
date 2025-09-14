#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>

#include <DT_ha.h>

#include <DT_relay.h>
#include <DT_opt_relay.h>
#include <DT_Dimmer.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_mqtt.h>
#include <DT_mqtt_send.h>
#include <DT_ha.h>
#include <DT_BME280.h>
#include <DT_CCS811.h>
#include <DT_SCD4X.h>
#include <DT_HDC1080.h>
#include <DT_mcp.h>
#include <DT_poele.h>
#include <DT_chauffage.h>
#include <DT_eeprom.h>
#include <DT_cover.h>
#include <DT_portal.h>
#include <DT_radiator.h>
#include <DT_cpt_pulse_input.h>
#include <DT_ecs.h>
#include <DT_teleinfo.h>
#include <DT_clock.h>

#include <DT_interaction.h>

#include <avr/wdt.h> //watchdog

// #include <pinout.h>
#include <config.h>

#include <boost/preprocessor/slot/counter.hpp>

#include <memdebug.h>

long int lastReconnectAttempt = 0;

extern void *__brkval;
// calcule de la memoire disponible
void memory(bool print)
{
  /*
  static unsigned int min_free_memory = 65535;
  unsigned int free_memory = (uint16_t)SP - (uint16_t)__brkval;

  if (free_memory < min_free_memory)
  {
    min_free_memory = free_memory;
    Serial.print(F("new minimum free memory = "));
    Serial.println(min_free_memory);
    Serial.print(F("constant memory = "));
    Serial.println((uint16_t)__malloc_heap_start);
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
  return free_memory;
  */
}

// calcule de la charge system
//  sur une base de 1 pour un temps de cycle moyen de 20ms
void load()
{
  memory(false);
  uint32_t now = millis();
  static uint32_t load_1s_count = 0;
  static uint32_t load_1s_time = 0;
  load_1s_count += 1;
  if (now - load_1s_time >= 1000)
  {
    // 220502  debug(F(AT));
    float load = ((now - load_1s_time) / 20.0) / load_1s_count;

    load_1s_count = load * 100;

#ifdef MQTT
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1s"), (float)(load_1s_count / 100.0));
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/memory/used"), getMemoryUsed());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/memory/free"), getFreeMemory());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/memory/large"), getLargestAvailableMemoryBlock());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/memory/number"), getNumberOfBlocksInFreeList());
#else
    Serial.print(F("Load 1s = "));
    Serial.println((float)(load_1s_count / 100.0));
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
    load_10s[load_10s_num] = ((now - load_10s_time) / 20.0) / load_10s_count;

    float temp = 0;
    for (uint8_t num = 0; num < 6; ++num)
    {
      temp += load_10s[num];
    }

    load_10s_count = (temp / 6.0) * 100;

#ifdef MQTT
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/load_1m"), (float)(load_10s_count / 100.0));
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
  memory(false);
}

#ifdef MQTT
inline bool can_send()
{
  if (mem_config.MQTT_online)
  {
    return true;
  }
  return false;
}

// Relay Callback
// envoie de donnée MQTT quand un relais est activé / désactivé
void relay_callback(const uint8_t num, const bool action)
{

  // char topic[56];
  const __FlashStringHelper *payload;
  debug(F(AT));
  memory(false);
  // debug_wdt_reset();
  if (can_send())
  {
    if (action)
    {
      payload = F("ON");
    }
    else
    {
      payload = F("OFF");
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/state"), num + 1, payload);
  }
  memory(false);
}

#if OPT_RELAY_NUM > 0
// Opt Relay Callback
// envoie de donnée MQTT quand un relais est activé / désactivé
void opt_relay_callback(const uint8_t num, const bool action)
{

  // char topic[56];
  const __FlashStringHelper *payload;
  debug(F(AT));
  memory(false);
  // debug_wdt_reset();
  if (can_send())
  {
    if (action)
    {
      payload = F("ON");
    }
    else
    {
      payload = F("OFF");
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/opt-relay-%02d/state"), num + 1, payload);
  }
  memory(false);
}
#endif // OPT_RELAY_NUM > 0
#endif // MQTT

#if DIMMER_LIGHT_NUM > 0
void dimmer_callback(const uint8_t num, const uint8_t percent, const bool candle)
{
  if (can_send())
  {
    const __FlashStringHelper *payload;
    if (num < DIMMER_LIGHT_NUM)
    {
      if (percent == 0)
        payload = F("OFF");
      else
        payload = F("ON");
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/state"), num + 1, payload);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/bri_state"), num + 1, percent);
      if (candle)
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_state"), num + 1, F("CANDLE"));
      else
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_state"), num + 1, F("NONE"));
    }
  }
}
#endif // NUM_DIMMER

#if COVER_NUM > 0
void cover_callback(const uint8_t num, const int8_t percent, const cover_state state)
{
  if (can_send())
  {
    switch (state)
    {
    case cover_stopped:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num + 1, F("stopped"));
      break;

    case cover_open:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num + 1, F("open"));
      break;

    case cover_closed:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num + 1, F("closed"));
      break;

    case cover_opening:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num + 1, F("opening"));
      break;

    case cover_closing:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num + 1, F("closing"));
      break;

    default:
      break;
    }
  }

  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/pos_state"), num + 1, percent);
}
#endif // COVER_NUM

#if PORTAL_NUM > 0
void portal_callback(const uint8_t num, const portal_state state)
{
  if (can_send())
  {
    switch (state)
    {
    case portal_stopped:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d/state"), num + 1, F("stopped"));
      break;

    case portal_open:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d/state"), num + 1, F("open"));
      break;

    case portal_closed:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d/state"), num + 1, F("closed"));
      break;

    case portal_opening:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d/state"), num + 1, F("opening"));
      break;

    case portal_closing:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d/state"), num + 1, F("closing"));
      break;

    default:
      break;
    }
  }
  debug(F(AT));
}
#endif // PORTAL_NUM

// envoi de donné MQTT quand une entrée est activée / desactivée
void input_mqtt(const uint8_t num, const Bt_Action action)
{
  debug(F(AT));
  memory(false);

  // switch (action)
  // {
  // case IN_PUSHED:
  //   Serial.println(F("IN_PUSHED"));
  //   break;
  // case IN_RELEASE:
  //   Serial.println(F("IN_RELEASE"));
  //   break;
  // case IN_PUSH:
  //   Serial.println(F("IN_PUSH"));
  //   break;
  // case IN_LPUSH:
  //   Serial.println(F("IN_LPUSH"));
  //   break;
  // case IN_LLPUSH:
  //   Serial.println(F("IN_LLPUSH"));
  //   break;
  // case IN_XLLPUSH:
  //   Serial.println(F("IN_XLLPUSH"));
  //   break;
  // case IN_2PUSH:
  //   Serial.println(F("IN_2PUSH"));
  //   break;
  // case IN_L2PUSH:
  //   Serial.println(F("IN_L2PUSH"));
  //   break;
  // case IN_LL2PUSH:
  //   Serial.println(F("IN_LL2PUSH"));
  //   break;
  // case IN_XLL2PUSH:
  //   Serial.println(F("IN_XLL2PUSH"));
  //   break;
  // }
#if CPT_PULSE_INPUT > 0 || CPT_PULSE_INPUT_IF_OUT > 0 || CPT_PULSE_INPUT_IF_IN > 0
  DT_cpt_pulse_input_loop_event(num, action);
#endif // CPT_PULSE_INPUT > 0 || CPT_PULSE_IF_INPUT > 0

  if (can_send())
  {
#ifdef MQTT
    // snprintf_P(topic, 56, F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num);
#endif
    switch (action)
    {
    case IN_PUSHED:
      // Serial.println("ON");
#ifdef MQTT
      // DT_mqtt_send(topic, "ON");
      // payload = F("ON");
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num + 1, F("ON"));
#endif
      break;

    case IN_RELEASE:
      // Serial.println("OFF");
#ifdef MQTT
      // DT_mqtt_send(topic, "OFF");
      // payload = F("OFF");
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num + 1, F("OFF"));
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
  memory(false);
  debug(F(AT));
}

void input_callback(const uint8_t num, const Bt_Action action)
{
  DT_interation_input_action(num, action);
  input_mqtt(num, action);
}

#ifdef MQTT
#if PT100_NUM > 0
// envoi de donné MQTT quand une temperature issue d'une PT100 à changée
void pt100_callback(const uint8_t num, const float temp)
{
  //  debug(F(AT));
  // memory(false);
  // Serial.println("PT100_CALLBACK ");

  if (can_send())
  {
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d"), num + 1, temp);
    if (num == PT100_EXT)
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-state"), (float)get_temp_ext());
  }
  memory(false);
}
#endif // PT100

#if BME280_NUM > 0
// envoi de donné MQTT quand une temperature issue d'une carte BME280 à changée
void bme280_callback_temperature(const uint8_t num, const float temperature)
{
  debug(F(AT));
  // memory(false);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/temp"), num + 1, temperature);
    }
  }
  // memory(false);
}

// envoi de donné MQTT quand une valeur d'humidité issue d'une carte BME280 à changée
void bme280_callback_humidity(const uint8_t num, const float humidity)
{
  debug(F(AT));
  // memory(true);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/humidity"), num + 1, humidity);
    }
  }
  // memory(false);
}

// envoi de donné MQTT quand la pression issue d'une carte BME280 à changée
void bme280_callback_pressure(const uint8_t num, const float pressure)
{
  debug(F(AT));
  // memory(true);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      debug(F(AT));
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/pressure"), num + 1, pressure);
    }
  }
  // memory(false);
  debug(F(AT));
}
#endif // BME280_NUM

#if CCS811_NUM > 0
// envoi de donné MQTT quand une valeur de CO2 issue d'une carte CCS811 à changée
void ccs811_callback_co2(const uint8_t num, const float co2)
{
  debug(F(AT));
  memory(false);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/co2"), num + 1, co2);
    }
  }
  memory(false);
}

// envoi de donné MQTT quand une valeur de COV issue d'une carte CCS811 à changée
void ccs811_callback_cov(const uint8_t num, const float cov)
{
  // debug(AT);
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/cov"), num + 1, cov);
    }
  }
  memory(false);
}
#endif

#if SCD4X_NUM > 0
// envoi de donné MQTT quand une temperature issue d'une carte SCD4X à changée
void scd4x_callback_temperature(const uint8_t num, const float temperature)
{
  debug(F(AT));
  // memory(false);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/scd4x-%02d/temperature"), num + 1, temperature);
    }
  }
  // memory(false);
}

// envoi de donné MQTT quand une valeur d'humidité issue d'une carte SCD4X à changée
void scd4x_callback_humidity(const uint8_t num, const float humidity)
{
  debug(F(AT));
  // memory(true);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/scd4x-%02d/humidity"), num + 1, humidity);
    }
  }
  // memory(false);
}

// envoi de donné MQTT quand le CO2 issue d'une carte SCD4X à changée
void scd4x_callback_co2(const uint8_t num, const float pressure)
{
  debug(F(AT));
  // memory(true);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/scd4x-%02d/co2"), num + 1, pressure);
    }
  }
  // memory(false);
}
#endif // SCD4X_NUM

#if HDC1080_NUM > 0
// envoi de donné MQTT quand une temperature issue d'une carte HDC1080 à changée
void hdc1080_callback_temperature(const uint8_t num, const float temperature)
{
  debug(F(AT));
  // memory(false);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/hdc1080-%02d/temp"), num + 1, temperature);
    }
  }
  // memory(false);
}

// envoi de donné MQTT quand une valeur d'humidité issue d'une carte HDC1080 à changée
void hdc1080_callback_humidity(const uint8_t num, const float humidity)
{
  debug(F(AT));
  // memory(true);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/hdc1080-%02d/humidity"), num + 1, humidity);
    }
  }
  // memory(false);
}
#endif // HDC1080_NUM

#if CPT_PULSE_INPUT > 0
// envoi de donné MQTT quand compteur a évolué
void cpt_pulse_input_callback(const uint8_t num, const uint32_t counter)
{
  debug(F(AT));
  // memory(true);
  if (can_send())
  {
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-%02d"), num + 1, counter);
  }
  // memory(false);
}
#endif // CPT_PULSE_INPUT

#if CPT_PULSE_INPUT_IF_OUT > 0
// envoi de donné MQTT quand compteur a évolué
void cpt_pulse_input_if_out_callback(const uint8_t num, const uint32_t counter, bool cond)
{
  debug(F(AT));
  // memory(true);
  if (can_send())
  {
    if (cond)
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-out-%02d/true"), num + 1, counter);
    else
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-out-%02d/false"), num + 1, counter);
  }
  // memory(false);
}
#endif // CPT_PULSE_INPUT_IF_OUT

#if CPT_PULSE_INPUT_IF_IN > 0
// envoi de donné MQTT quand compteur a évolué
void cpt_pulse_input_if_in_callback(const uint8_t num, const uint32_t counter, bool cond)
{
  debug(F(AT));
  // memory(true);
  if (can_send())
  {
    if (cond)
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-in-%02d/true"), num + 1, counter);
    else
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-in-%02d/false"), num + 1, counter);
  }
  // memory(false);
}
#endif // CPT_PULSE_INPUT_IF_IN

// envoi de donné MQTT quand le Mode de fonctionnement du poele change
#ifdef POELE
void poele_mode_callback(const DT_Poele_mode mode)
{
  debug(F(AT));
  memory(false);
  const __FlashStringHelper *payload;
  // mode poele
  if (can_send())
  {
    // strlcpy_P(topic, F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), 56);
    switch (mode)
    {
    case DT_POELE_ARRET:
      payload = F("Arret");
      break;
    case DT_POELE_NORMAL:
      payload = F("Normal");
      break;
    case DT_POELE_FORCE:
      payload = F("Forcé");
      break;
    case DT_POELE_STANDBY:
      payload = F("Veille");
      break;
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), payload);
  }
  memory(false);
}
#endif // POELE

#ifdef CHAUFFAGE
void chauffage_mode_callback(const DT_Chauffage_mode mode)
{
  debug(F(AT));
  memory(false);
  const __FlashStringHelper *payload;
  // mode poele
  if (can_send())
  {
    // strlcpy_P(topic, F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), 56);
    switch (mode)
    {
    case DT_CHAUFFAGE_ETE:
      payload = F("Eté");
      break;
    case DT_CHAUFFAGE_INTERSAISON:
      payload = F("Inter-saison");
      break;
    case DT_CHAUFFAGE_HIVER:
      payload = F("Hiver");
      break;
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/mode/state"), payload);
  }
  memory(false);
}

void temperature_arret_poele_hiver(const float temperature)
{
  debug(F(AT));

  if (can_send())
  {
    if (mem_config.MQTT_online)
    {
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPH/state"), temperature);
    }
  }
}

void arret_meteo_callback(const bool action)
{

  // char topic[56];
  const __FlashStringHelper *payload;
  debug(F(AT));
  memory(false);
  // debug_wdt_reset();
  if (can_send())
  {
    if (action)
    {
      payload = F("ON");
    }
    else
    {
      payload = F("OFF");
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/AM/state"), payload);
  }
  memory(false);
}
#endif // CHAUFFAGE

// envoi de donné MQTT quand la variable C2 ou C3 change
#ifdef VANNES
void dt3voies_callback(const float C2, const float C3)
{
  debug(F(AT));
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      // 220502  debug(F(AT));
      // send_buffer.reserve(2);
      int32_t digit = C2 * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/state"), (float)(digit / 100.0));

      digit = C3 * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/state"), (float)(digit / 100.0));
    }
  }
  memory(false);
}

// retour des valleur du PID PCBT
void dt3voies_callback_pid_pcbt(const float P, const float I, const float D, const float OUT)
{
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      // 220502  debug(F(AT));
      // send_buffer.reserve(4);

      int32_t digit = P * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/P"), (float)(digit / 100.0));
      digit = I * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/I"), (float)(digit / 100.0));
      digit = D * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/D"), (float)(digit / 100.0));
      digit = OUT * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/OUT"), (float)(digit / 100.0));
      // 220502  debug(F(AT));
      memory(false);
    }
  }
}

// retour des valleur du PID MCBT
void dt3voies_callback_pid_mcbt(const float P, const float I, const float D, const float OUT)
{
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      // 220502  debug(F(AT));
      refresh = now;
      // send_buffer.reserve(4);

      int32_t digit = P * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/P"), (float)(digit / 100.0));
      digit = I * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/I"), (float)(digit / 100.0));
      digit = D * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/D"), (float)(digit / 100.0));
      digit = OUT * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/OUT"), (float)(digit / 100.0));
      // 220502  debug(F(AT));
    }
  }
  memory(false);
}

// retour des valeurs de la temperature moyenné
void dt3voies_callback_avg_temp(const float temp)
{
  memory(false);
  if (can_send())
  {
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/state"), (float)temp);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-state"), (float)get_temp_ext());
  }
  memory(false);
}
#endif // VANNES

// envoi de donné MQTT quand le Mode de fonctionnement de l'ECS1 change
#ifdef RELAY_ECS1
void ecs1_mode_callback(const DT_ECS_mode mode)
{
  debug(F(AT));
  memory(false);
  const __FlashStringHelper *payload;
  Serial.println(F("ecs1_mode_callback"));
  // mode poele
  if (can_send())
  {
    switch (mode)
    {
    case DT_ECS_ARRET:
      payload = F("Arret");
      break;
    case DT_ECS_MARCHE:
      payload = F("Marche");
      break;
    case DT_ECS_STANDBY:
      payload = F("Veille");
      break;
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs1/state"), payload);
  }
  memory(false);
}
#endif // RELAY_ECS1

// envoi de donné MQTT quand le Mode de fonctionnement de l'ECS1 change
#ifdef RELAY_ECS2
void ecs2_mode_callback(const DT_ECS_mode mode)
{
  debug(F(AT));
  memory(false);
  const __FlashStringHelper *payload;
  Serial.println(F("ecs2_mode_callback"));
  // mode poele
  if (can_send())
  {
    switch (mode)
    {
    case DT_ECS_ARRET:
      payload = F("Arret");
      break;
    case DT_ECS_MARCHE:
      payload = F("Marche");
      break;
    case DT_ECS_STANDBY:
      payload = F("Veille");
      break;
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs2/state"), payload);
  }
  memory(false);
}
#endif // RELAY_ECS2

#if RADIATOR_NUM > 0
// envoi du pourcentage de fonctionnement et de la valeur de l'Integral en MQTT
void dt_radiator_callback(const uint8_t num, const float out, const float I)
{
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (can_send())
  {
    if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
    {
      refresh = now;
      // send_buffer.reserve(3);

      if (eeprom_config.radiator[num].mode == Radiator_Mode_Off)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/state"), num, F("off"));
      }
      else if (eeprom_config.radiator[num].mode == Radiator_Mode_Heating && out != 0)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/state"), num, F("heating"));
      }
      else if (eeprom_config.radiator[num].mode == Radiator_Mode_Heating)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/state"), num, F("idle"));
      }

      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/OUT"), num, out);
      int32_t digit = I * 100;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/I"), num, (float)(digit / 100.0));
    }
  }
  memory(false);
}
#endif // RADIATOR_NUM

#ifdef TIC
void teleinfo_callback(const char *name, const char *value)
{
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/TIC"), value);
}
#endif // TIC

// envoie en MQTT de l'ensamble des donnée de la carte
bool mqtt_publish(bool start)
{
  memory(false);
  const __FlashStringHelper *payload;
  static uint16_t sequance = 65534;
  static uint8_t num = 0;
  static uint32_t time = 0;
  if (start)
  {
    sequance = BOOST_PP_COUNTER;
    return false;
  }
  else if (sequance == BOOST_PP_COUNTER)
    Serial.println(F("mqtt_publish"));

  if (mem_config.ha_mqtt_config == false && mem_config.ha_mqtt_subscribe) // on attand la fin de l envoie de la configuration home assistant
  {
    return false;
  }

  uint32_t now = millis();
  if (now - time >= 50)
  {
    time = now;
    switch (sequance)
    {
    case BOOST_PP_COUNTER:
      DT_mqtt_send(F(MQTT_WILL_TOPIC), F(MQTT_NOT_WILL_MESSAGE));
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < RELAY_NUM)
      {
        relay_callback(num, DT_relay_get(num));
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
    case BOOST_PP_COUNTER:
      if (num < OPT_RELAY_NUM)
      {
        opt_relay_callback(num, DT_opt_relay_get(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // OPT_RELAY_NUM > 0

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < INPUT_NUM)
      {
        input_mqtt(num, DT_input_get_stats(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

#if DIMMER_LIGHT_NUM > 0 // dimmer
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < DIMMER_LIGHT_NUM)
      {
        dimmer_callback(num, get_dimmer(num), get_dimmer_candle(num));
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
      if (num < DIMMER_LIGHT_NUM)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_state"), num + 1, eeprom_config.Dimmer_scale_min[num]);
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
      if (num < DIMMER_LIGHT_NUM)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_state"), num + 1, eeprom_config.Dimmer_scale_max[num]);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // DIMMER_LIGHT_NUM

#if RADIATOR_NUM > 0 // RADIATOR
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER: // RADIATOR consigne
      if (num < DIMMER_LIGHT_NUM)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/temp_state"), num + 1, DT_radiator_get_consigne(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER: // RADIATOR mode (off/heating)
      if (num < DIMMER_LIGHT_NUM)
      {
        if (eeprom_config.radiator[num].mode == Radiator_Mode_Off)
        {
          DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/mode_state"), num + 1, F("off"));
        }
        else if (eeprom_config.radiator[num].mode == Radiator_Mode_Heating)
        {
          DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/mode_state"), num + 1, F("heat"));
        }
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER: // RADIATOR cycle
      if (num < DIMMER_LIGHT_NUM)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/cycle-state"), num + 1, DT_radiator_get_cycle(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER: // RADIATOR M10
      if (num < DIMMER_LIGHT_NUM)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/m10-state"), num + 1, DT_radiator_get_M10(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER: // RADIATOR P10
      if (num < DIMMER_LIGHT_NUM)
      {
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d//KI-state"), num + 1, DT_radiator_get_KI(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

#endif // RADIATOR

#if PT100_NUM > 0 // PT100
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < PT100_NUM)
      {
        pt100_callback(num, DT_pt100_get(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif

#if CPT_PULSE_INPUT > 0 // input pulse counter
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < CPT_PULSE_INPUT)
      {
        cpt_pulse_input_callback(num, DT_cpt_pulse_input_get(num));
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif

#if CPT_PULSE_INPUT_IF_OUT > 0 // input pulse counter
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < CPT_PULSE_INPUT_IF_OUT)
      {
        cpt_pulse_input_if_out_callback(num, DT_cpt_pulse_input_if_out_get(num, true), true);
        cpt_pulse_input_if_out_callback(num, DT_cpt_pulse_input_if_out_get(num, false), false);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // CPT_PULSE_INPUT_IF_OUT

#if CPT_PULSE_INPUT_IF_IN > 0 // input pulse counter
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < CPT_PULSE_INPUT_IF_IN)
      {
        cpt_pulse_input_if_in_callback(num, DT_cpt_pulse_input_if_in_get(num, true), true);
        cpt_pulse_input_if_in_callback(num, DT_cpt_pulse_input_if_in_get(num, false), false);
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
      // mode poele
      poele_mode_callback(DT_Poele_get_mode());
      // EEPROM
      //  V1
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/state"), eeprom_config.V1);

      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // V2
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/state"), eeprom_config.V2);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // V3
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), eeprom_config.V3);
      break;
#endif // POELE

#ifdef CHAUFFAGE
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // mode chauffage
      chauffage_mode_callback(DT_Chauffage_get_mode());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // Temperature arret poele hiver
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPH"), eeprom_config.temperature_arret_poele_hiver);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // Temperature arret poele hiver
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPH"), eeprom_config.temperature_arret_poele_intersaison);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // Temperature arret poele hiver
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMa"), eeprom_config.temperature_balon_max);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // Temperature arret poele hiver
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMi"), eeprom_config.temperature_balon_min);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // Temperature arret poele hiver
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TID"), eeprom_config.temp_inter_demmarage);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // Temperature arret poele hiver
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/DRV"), eeprom_config.date_retour_vacance);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // Temperature arret poele hiver
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/AM"), mem_config.ha_arret_meteo);
      break;

#endif // CHAUFFAGE

#ifdef VANNES
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // 3 voies PCBT mode
      switch (DT_3voies_PCBT_get_mode())
      {
      case DT_3VOIES_DEMMARAGE:
        // DT_mqtt_send(topic, "Demmarage");
        payload = F("Demmarage");
        break;
      case DT_3VOIES_NORMAL:
        // DT_mqtt_send(topic, "Normal");
        payload = F("Normal");
        break;
      case DT_3VOIES_MANUAL:
        // DT_mqtt_send(topic, "Manuel");
        payload = F("Manuel");
        break;
      case DT_3VOIES_OFF:
        // DT_mqtt_send(topic, "Arret");
        payload = F("Arret");
        break;
      case DT_3VOIES_STANDBY:
        // DT_mqtt_send(topic, "Arret");
        payload = F("Veille");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // 3 voies MCBT mode
      switch (DT_3voies_MCBT_get_mode())
      {
      case DT_3VOIES_DEMMARAGE:
        payload = F("Demmarage");
        break;
      case DT_3VOIES_NORMAL:
        payload = F("Normal");
        break;
      case DT_3VOIES_MANUAL:
        payload = F("Manuel");
        break;
      case DT_3VOIES_OFF:
        payload = F("Arret");
        break;
      case DT_3VOIES_STANDBY:
        payload = F("Veille");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // consigne vanne 3 voies PCBT & MCBT
      dt3voies_callback(DT_3voies_get_C2(), DT_3voies_get_C3());

      break;
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C4
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), eeprom_config.C4);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C5
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), eeprom_config.C5);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C6
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), eeprom_config.C6);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C7
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), eeprom_config.C7);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C8
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/state"), eeprom_config.C8);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C9
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/state"), eeprom_config.C9);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C10
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), eeprom_config.C10);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C11
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/state"), eeprom_config.C11);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), eeprom_config.C_PCBT_MIN);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C_PCBT_MAX
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), eeprom_config.C_PCBT_MAX);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C_MCBT_MIN
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), eeprom_config.C_MCBT_MIN);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C_MCBT_MAX
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), eeprom_config.C_MCBT_MAX);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KP_PCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), DT_3voies_PCBT_get_KP());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KI_PCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), DT_3voies_PCBT_get_KI());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KD_PCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), DT_3voies_PCBT_get_KD());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KT_PCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), DT_3voies_PCBT_get_KT());

      break;
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KP_MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), DT_3voies_MCBT_get_KP());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KI_MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), DT_3voies_MCBT_get_KI());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KD_MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), DT_3voies_MCBT_get_KD());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KT_MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), DT_3voies_MCBT_get_KT());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID PCBT Action
      switch (eeprom_config.pid_pcbt.action)
      {
      case QuickPID::Action::direct:
        payload = F("direct");
        break;
      case QuickPID::Action::reverse:
        payload = F("reverse");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID PCBT pMode
      switch (eeprom_config.pid_pcbt.pmode)
      {
      case QuickPID::pMode::pOnError:
        payload = F("pOnError");
        break;
      case QuickPID::pMode::pOnMeas:
        payload = F("pOnMeas");
        break;
      case QuickPID::pMode::pOnErrorMeas:
        payload = F("pOnErrorMeas");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID PCBT dMode
      switch (eeprom_config.pid_pcbt.dmode)
      {
      case QuickPID::dMode::dOnError:
        payload = F("dOnError");
        break;
      case QuickPID::dMode::dOnMeas:
        payload = F("dOnMeas");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID PCBT iAwMode
      switch (eeprom_config.pid_pcbt.iawmode)
      {
      case QuickPID::iAwMode::iAwCondition:
        payload = F("iAwCondition");
        break;
      case QuickPID::iAwMode::iAwClamp:
        payload = F("iAwClamp");
        break;
      case QuickPID::iAwMode::iAwOff:
        payload = F("iAwOff");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID MCBT Action
      switch (eeprom_config.pid_mcbt.action)
      {
      case QuickPID::Action::direct:
        payload = F("direct");
        break;
      case QuickPID::Action::reverse:
        payload = F("reverse");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID MCBT pMode
      switch (eeprom_config.pid_mcbt.pmode)
      {
      case QuickPID::pMode::pOnError:
        payload = F("pOnError");
        break;
      case QuickPID::pMode::pOnMeas:
        payload = F("pOnMeas");
        break;
      case QuickPID::pMode::pOnErrorMeas:
        payload = F("pOnErrorMeas");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID MCBT dMode
      switch (eeprom_config.pid_mcbt.dmode)
      {
      case QuickPID::dMode::dOnError:
        payload = F("dOnError");
        break;
      case QuickPID::dMode::dOnMeas:
        payload = F("dOnMeas");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID MCBT iAwMode
      switch (eeprom_config.pid_mcbt.iawmode)
      {
      case QuickPID::iAwMode::iAwCondition:
        payload = F("iAwCondition");
        break;
      case QuickPID::iAwMode::iAwClamp:
        payload = F("iAwClamp");
        break;
      case QuickPID::iAwMode::iAwOff:
        payload = F("iAwOff");
        break;
      }
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), payload);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // RATIO PCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), eeprom_config.ratio_PCBT);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // RATIO MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), eeprom_config.ratio_MCBT);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // OFFSET_PCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/inhib-out/state"), eeprom_config.out_inhib_PCBT);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // OFFSET_MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/inhib-out/state"), eeprom_config.out_inhib_MCBT);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // OFFSET_PCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), eeprom_config.in_offset_PCBT);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // OFFSET_MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), eeprom_config.in_offset_MCBT);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // TEMPERATURE MOYENNE
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/state"), DT_3voies_get_temp_moyen());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // TEMPERATURE MOYENNE DECALEE
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-state"), (float)get_temp_ext());
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // TEMPERATURE MOYENNE
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-sup/state"), eeprom_config.in_offset_avg_temp_sup);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // TEMPERATURE MOYENNE
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-inf/state"), eeprom_config.in_offset_avg_temp_inf);
      break;

#endif // vanne

#ifdef RELAY_ECS1
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // mode poele
      ecs1_mode_callback(DT_ecs1_get_mode());
      // EEPROM
      //  V1
      break;
#endif // RELAY_ECS1

#ifdef RELAY_ECS2
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // mode poele
      ecs2_mode_callback(DT_ecs2_get_mode());
      // EEPROM
      //  V1
      break;
#endif // RELAY_ECS1

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // ONLINE
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), F("online"));
      break;

#ifdef WATCHDOG_TIME
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/debug_str/state"), eeprom_config.debug_str);
      strncpy(eeprom_config.debug_str, "@", 1);
      // memcpy(eeprom_config.debug_str, "@", 1);
      break;
#endif // WATCHDOG_TIME

    default:
      return true;
      break;
    }

    ++sequance;
    memory(false);
  }
  return false;
}

// inscription au topic MQTT (necessaire a la recption des données par la carte)
bool mqtt_subscribe(MQTTClient &mqtt, bool start)
{
  //  debug(F(AT));
  char topic[56];
  memory(false);
  uint32_t now = millis();
  static uint16_t sequance = 65534;
  static uint8_t num = 0;
  static uint32_t time = 0;

  if (start)
  {
    sequance = BOOST_PP_COUNTER;
    return false;
  }
  else if (sequance == BOOST_PP_COUNTER)
    Serial.println(F("mqtt_subscribe"));

  if (now - time >= 50)
  {
    time = now;
    switch (sequance)
    {
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/mode_set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/temp_set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/FG1/away_set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
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
      break;

#if OPT_RELAY_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < OPT_RELAY_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/opt-relay-%02d/set"), num + 1);
        mqtt.subscribe(topic);
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
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < DIMMER_LIGHT_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < DIMMER_LIGHT_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/bri_set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < DIMMER_LIGHT_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < DIMMER_LIGHT_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < DIMMER_LIGHT_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_set"), num + 1);
        mqtt.subscribe(topic);
        num++;
        sequance--;
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
      if (num < COVER_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/set"), num + 1);
        mqtt.subscribe(topic);
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/pos_set"), num + 1);
        mqtt.subscribe(topic);
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/up_set"), num + 1);
        mqtt.subscribe(topic);
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/down_set"), num + 1);
        mqtt.subscribe(topic);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // COVER

#if PORTAL_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < PORTAL_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d/set"), num + 1);
        mqtt.subscribe(topic);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // PORTAL_NUM

#if RADIATOR_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < RADIATOR_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/temp_set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < RADIATOR_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/mode_set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < RADIATOR_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/cycle-set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < RADIATOR_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/m10-set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < RADIATOR_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/p10-set"), num + 1);
        mqtt.subscribe(topic);
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
      if (num < RADIATOR_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/KI-set"), num + 1);
        mqtt.subscribe(topic);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // RADIATOR_NUM

#if CPT_PULSE_INPUT > 0 // listen reset mqtt event
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < CPT_PULSE_INPUT)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-%02d/btn"), num + 1);
        mqtt.subscribe(topic);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // CPT_PULSE_INPUT

#if CPT_PULSE_INPUT_IF_OUT > 0 // listen reset mqtt event
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < CPT_PULSE_INPUT_IF_OUT)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-out_%02d/btn"), num + 1);
        mqtt.subscribe(topic);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // CPT_PULSE_INPUT_IF_OUT

#if CPT_PULSE_INPUT_IF_IN > 0 // listen reset mqtt event
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < CPT_PULSE_INPUT_IF_IN)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-in_%02d/btn"), num + 1);
        mqtt.subscribe(topic);
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
      // Poele
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // 3 voies PCBT mode
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // EEPROM
      //  V1
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/set");
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C7
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set");
      break;
#endif // POELE

#ifdef CHAUFFAGE
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // mode chauffage
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/mode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // temperature a poele intersaison
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPI/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // temperature a poele intersaison
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMa/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // temperature a poele intersaison
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMi/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // temperature a poele intersaison
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TID/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // temperature a poele intersaison
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/DRV/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // temperature a poele intersaison
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/AM/set");
      break;
#endif // CHAUFFAGE

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // 3 voies PCBT consigne
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/set");

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // 3 voies MCBT mode
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/set");

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // 3 voies MCBT consigne
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // V2
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // V3
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C4
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C5
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C6
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C8
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C9
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C10
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C11
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C_PCBT_MIN
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C_PCBT_MAX
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C_MCBT_MIN
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C_MCBT_MAX
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KP_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KI_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KD_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KT_PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KP_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KI_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KD_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // KT_MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID PCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // PID MCBT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // RATIO
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // OFFSET OUT
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/inhib-out/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/inhib-out/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // OFFSET IN
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set");
      break;

#ifdef RELAY_ECS1
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // RELAY_ECS1
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs1/set");
      break;
#endif // RELAY_ECS1

#ifdef RELAY_ECS2
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // RELAY_ECS1
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs2/set");
      break;
#endif // RELAY_ECS1

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-sup/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-inf/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // HomeAssistant
      mqtt.subscribe("homeassistant/status");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mem_config.ha_mqtt_publish = mqtt_publish(true);
      break;

    default:
      return true;
      break;
    }

    ++sequance;
    memory(false);
  }
  return false;
}

// ensemble des action a effectuée a la reception d'un message MQTT
void __attribute__((optimize("O0"))) mqtt_receve(MQTTClient *client, const char topic[], const char payload[], const int length)
{

  debug(F(AT));
  char buffer[64];
  char _topic[64];
  memory(false);
  String str_buffer;

  // Copy the payload to the new buffer
  if (length < 64)
  {
    memcpy(buffer, payload, length);
    buffer[length] = '\0';

    // Serial.print(F("receve topic "));
    // Serial.print(topic);
    // Serial.print(" = ");
    // Serial.println(buffer);
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
      DT_relay(num - 1, true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_relay(num - 1, false);
  }
#if OPT_RELAY_NUM > 0
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/opt-relay-%02d/set"), &num) == 1) // relais
  {
    // auto Serial.print("sscanf = ");
    // auto Serial.println(num);

    if (strcmp(buffer, "ON") == 0)
      DT_opt_relay(num - 1, true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_opt_relay(num - 1, false);
  }
#endif // OPT_RELAY_NUM > 0

#if DIMMER_LIGHT_NUM > 0
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d"), &num) == 1) // dimmer
  {
    bool bool_value = false;
    // Serial.println("dimmer");
    if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/set"), num) > 0 && strncmp(topic, _topic, 64) == 0)
    {
      // Serial.println("set");
      bool_value = get_dimmer_candle(num);
      if (strcmp(buffer, "ON") == 0)
      {
        dimmer_set(num - 1, true, DIMMER_ON_OFF_SPEED, bool_value);
      }
      else if (strcmp(buffer, "OFF") == 0)
        dimmer_set(num - 1, false, DIMMER_ON_OFF_SPEED);
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/bri_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("bri_set");
      if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
      {
        bool_value = get_dimmer_candle(num - 1);
        dimmer_set(num - 1, u8t_value, DIMMER_ON_OFF_SPEED, bool_value);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("fx_set");
      u8t_value = get_dimmer(num - 1);
      if (strcmp(buffer, "NONE") == 0)
      {
        dimmer_set(num - 1, u8t_value, DIMMER_ON_OFF_SPEED, false);
      }
      else if (strcmp(buffer, "CANDLE") == 0)
      {
        dimmer_set(num - 1, u8t_value, DIMMER_ON_OFF_SPEED, true);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("min_set");
      str_buffer = buffer;
      eeprom_config.Dimmer_scale_min[num - 1] = (uint16_t)str_buffer.toDouble();
      if (eeprom_config.Dimmer_scale_min[num - 1] <= eeprom_config.Dimmer_scale_max[num - 1])
        eeprom_config.Dimmer_scale_min[num - 1] = eeprom_config.Dimmer_scale_max[num - 1] + 20;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_state"), num, (uint16_t)eeprom_config.Dimmer_scale_min[num - 1]);
      dimmer_set(num - 1, true, 0);
      sauvegardeEEPROM();
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("max_set");
      str_buffer = buffer;
      eeprom_config.Dimmer_scale_max[num - 1] = (uint16_t)str_buffer.toDouble();
      if (eeprom_config.Dimmer_scale_max[num - 1] >= eeprom_config.Dimmer_scale_min[num - 1])
        eeprom_config.Dimmer_scale_max[num - 1] = eeprom_config.Dimmer_scale_min[num - 1] - 20;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_state"), num, (uint16_t)eeprom_config.Dimmer_scale_max[num - 1]);
      dimmer_set(num - 1, true, 0);
      sauvegardeEEPROM();
    }
  }
#endif // DIMMER_LIGHT_NUM

#if COVER_NUM > 0
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d"), &num) == 1) // cover
  {
    Serial.println(F("cover"));
    if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // cover
    {
      Serial.println(F("set"));
      if (strcmp(buffer, "STOP") == 0)
      {
        DT_cover_stop(num - 1);
      }
      else if (strcmp(buffer, "OPEN") == 0)
      {
        DT_cover_set(num - 1, 100);
      }
      else if (strcmp(buffer, "CLOSE") == 0)
      {
        DT_cover_set(num - 1, 0);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/pos_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // cover
    {
      Serial.println(F("pos_set"));
      if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
      {
        DT_cover_set(num - 1, u8t_value);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/up_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // time cover up
    {
      Serial.print(F("up_set = "));
      str_buffer = buffer;
      eeprom_config.cover[num - 1].time_up = (uint16_t)(str_buffer.toDouble());
      Serial.println(eeprom_config.cover[num - 1].time_up);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/up_state"), num, (eeprom_config.cover[num - 1].time_up));
      sauvegardeEEPROM();
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/down_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // time cover down
    {
      Serial.print(F("down_set = "));
      str_buffer = buffer;
      eeprom_config.cover[num - 1].time_down = (uint16_t)(str_buffer.toDouble());
      Serial.println(eeprom_config.cover[num - 1].time_down);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/down_state"), num, (eeprom_config.cover[num - 1].time_down));
      sauvegardeEEPROM();
    }
  }
#endif // COVER_NUM > 0

#if PORTAL_NUM > 0
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d"), &num) == 1) // cover
  {
    // Serial.println(F("portal"));
    if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/portal-%02d/set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // portal
    {
      // Serial.println(F("set"));
      if (strcmp(buffer, "STOP") == 0)
      {
        DT_portal_stop(num - 1);
      }
      else if (strcmp(buffer, "OPEN") == 0)
      {
        DT_portal_open(num - 1);
      }
      else if (strcmp(buffer, "CLOSE") == 0)
      {
        DT_portal_close(num - 1);
      }
    }
  }
#endif // PORTAL_NUM > 0

#if RADIATOR_NUM > 0
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d"), &num) == 1) // radiator
  {
    Serial.println(F("radiator"));
    if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/temp_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // radiator temp_set
    {
      Serial.println(F("temp_set"));
      str_buffer = buffer;
      DT_radiator_set_consigne(num - 1, str_buffer.toFloat());
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/temp_state"), num, DT_radiator_get_consigne(num - 1));
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/mode_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // radiator mode
    {
      Serial.println(F("mode_set"));
      if (strcmp(buffer, "off") == 0)
      {
        DT_radiator_set_mode(num - 1, Radiator_Mode_Off);
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/mode_state"), num, F("off"));
      }
      else if (strcmp(buffer, "heat") == 0)
      {
        DT_radiator_set_mode(num - 1, Radiator_Mode_Heating);
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/mode_state"), num, F("heat"));
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/cycle-set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // radiator cycle
    {
      Serial.print(F("cycle-set = "));
      str_buffer = buffer;
      DT_radiator_set_cycle(num - 1, (uint32_t)(str_buffer.toInt()));
      Serial.println(DT_radiator_get_cycle(num - 1));
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/cycle-state"), num, DT_radiator_get_cycle(num));
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/m10-set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // radiator M10
    {
      Serial.print(F("m10-set = "));
      str_buffer = buffer;
      DT_radiator_set_M10(num - 1, str_buffer.toFloat());
      Serial.println(DT_radiator_get_M10(num - 1));
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/m10-state"), num, DT_radiator_get_M10(num));
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/p10-set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // radiator P10
    {
      Serial.print(F("p10-set = "));
      str_buffer = buffer;
      DT_radiator_set_P10(num - 1, str_buffer.toFloat());
      Serial.println(DT_radiator_get_P10(num - 1));
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/m10-state"), num, DT_radiator_get_P10(num));
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/KI-set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // radiator P10
    {
      Serial.print(F("KI-set = "));
      str_buffer = buffer;
      DT_radiator_set_KI(num - 1, str_buffer.toFloat());
      Serial.println(DT_radiator_get_KI(num - 1));
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/radiator-%02d/m10-state"), num, DT_radiator_get_KI(num));
    }
  }

#endif // RADIATOR_NUM > 0

#if CPT_PULSE_INPUT > 0                                                                                // listen mqtt reset event
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-%02d/btn"), &num) == 1) // counter reset
  {
    // Serial.println(F("counter reset"));
    DT_cpt_pulse_input_reset(num - 1);
  }
#endif // CPT_PULSE_INPUT > 0

#if CPT_PULSE_INPUT_IF_OUT > 0                                                                                // listen mqtt reset event
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-out-%02d/btn"), &num) == 1) // counter reset
  {
    // Serial.println(F("counter if out reset"));
    DT_cpt_pulse_input_if_out_reset(num - 1);
  }
#endif // CPT_PULSE_INPUT_IF_OUT > 0

#if CPT_PULSE_INPUT_IF_IN > 0                                                                                // listen mqtt reset event
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/counter-if-in-%02d/btn"), &num) == 1) // counter reset
  {
    Serial.println(F("counter if in reset"));
    DT_cpt_pulse_input_if_in_reset(num - 1);
  }
#endif // CPT_PULSE_INPUT_IF_IN > 0

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
    else if (strcmp(buffer, "Forcé") == 0)
    {
      DT_Poele_set_mode(DT_POELE_FORCE);
    }
    else if (strcmp(buffer, "Veille") == 0)
    {
      DT_Poele_set_mode(DT_POELE_STANDBY);
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/set") == 0) // V1
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.V1 = u8t_value;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V1/state"), u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/set") == 0) // V2
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.V2 = u8t_value;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V2/state"), u8t_value);
      sauvegardeEEPROM();
    }
  }
#endif // CHAUFFAGE
#ifdef CHAUFFAGE
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/mode/set") == 0) // Mode du chauffage
  {
    if (strcmp(buffer, "Eté") == 0)
    {
      DT_Chauffage_set_mode(DT_CHAUFFAGE_ETE);
    }
    else if (strcmp(buffer, "Inter-saison") == 0)
    {
      DT_Chauffage_set_mode(DT_CHAUFFAGE_INTERSAISON);
    }
    else if (strcmp(buffer, "Hiver") == 0)
    {
      DT_Chauffage_set_mode(DT_CHAUFFAGE_HIVER);
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPI/set") == 0) // Temperature arret poele intersaison
  {
    str_buffer = buffer;
    eeprom_config.temperature_arret_poele_intersaison = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPI/state"), eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMa/set") == 0) // temperature_balon_max
  {
    str_buffer = buffer;
    eeprom_config.temperature_balon_max = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMa/state"), eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMi/set") == 0) // temperature_balon_max
  {
    str_buffer = buffer;
    eeprom_config.temperature_balon_min = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMi/state"), eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TID/set") == 0) // temp_inter_demmarage
  {
    str_buffer = buffer;
    eeprom_config.temp_inter_demmarage = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TID/state"), eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/DRV/set") == 0) // date_retour_vacance
  {
    str_buffer = buffer;
    eeprom_config.temp_inter_demmarage = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/DRV/state"), eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/AM/set") == 0) // ha_arret_meteo
  {
    if (strcmp(buffer, "ON") == 0)
      DT_Chauffage_set_arret_meteo(true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_Chauffage_set_arret_meteo(false);
  }
#endif // CHAUFFAGE
#ifdef VANNES
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/set") == 0) // Mode de la vannes 3 voie PCBT
  {

    if (strcmp(buffer, "Normal") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_NORMAL);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), F("Normal"));
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_DEMMARAGE);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), F("Demmarage"));
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_MANUAL);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), F("Manuel"));
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_OFF);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), F("Arret"));
    }
    else if (strcmp(buffer, "Veille") == 0)
    {
      DT_3voies_PCBT_set_mode(DT_3VOIES_STANDBY);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/mode/state"), F("Veille"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/set") == 0) // Mode de la vannes 3 voie PCBT
  {
    str_buffer = buffer;
    DT_3voies_set_C2(str_buffer.toFloat());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/state"), DT_3voies_get_C2());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    if (strcmp(buffer, "Normal") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_NORMAL);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), F("Normal"));
    }
    else if (strcmp(buffer, "Demmarage") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_DEMMARAGE);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), F("Demmarage"));
    }
    else if (strcmp(buffer, "Manuel") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_MANUAL);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), F("Manuel"));
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_OFF);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), F("Arret"));
    }
    else if (strcmp(buffer, "Veille") == 0)
    {
      DT_3voies_MCBT_set_mode(DT_3VOIES_STANDBY);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/mode/state"), F("Veille"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/set") == 0) // Mode de la vannes 3 voie MCBT
  {
    str_buffer = buffer;
    DT_3voies_set_C3(str_buffer.toFloat());
  }
  else if (strcmp_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/set")) == 0) // V3
  {
    // Serial.print(F("set V3 = "));
    str_buffer = buffer;
    eeprom_config.V3 = str_buffer.toFloat();
    Serial.print(eeprom_config.V3);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), eeprom_config.V3);
    sauvegardeEEPROM();

    // Serial.println(" ");
  }
  else if (strcmp_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/set")) == 0) // C4
  {
    str_buffer = buffer;
    eeprom_config.C4 = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C4/state"), eeprom_config.C4);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/set") == 0) // C5
  {
    str_buffer = buffer;
    eeprom_config.C5 = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), eeprom_config.C5);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set") == 0) // C6
  {
    str_buffer = buffer;
    eeprom_config.C6 = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), eeprom_config.C7);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set") == 0) // C7
  {
    // Serial.println(F("C7/set"));
    // if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    // {
    //   eeprom_config.C7 = u8t_value;
    //   DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), eeprom_config.C7);
    //   sauvegardeEEPROM();
    // }
    str_buffer = buffer;
    eeprom_config.C7 = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), eeprom_config.C7);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/set") == 0) // C8
  {
    str_buffer = buffer;
    eeprom_config.C8 = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C8/state"), eeprom_config.C8);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/set") == 0) // C9
  {
    str_buffer = buffer;
    eeprom_config.C9 = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C9/state"), eeprom_config.C9);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/set") == 0) // C10
  {
    str_buffer = buffer;
    eeprom_config.C10 = str_buffer.toFloat();
    // Serial.print("C10 = ");
    // Serial.println(eeprom_config.C10);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), eeprom_config.C10);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set") == 0) // C11
  {
    str_buffer = buffer;
    eeprom_config.C11 = str_buffer.toFloat();
    // Serial.print("C11 = ");
    // Serial.println(eeprom_config.C11);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/state"), eeprom_config.C11);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/set") == 0) // C_PCBT_MIN
  {
    str_buffer = buffer;
    eeprom_config.C_PCBT_MIN = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/min_temp/state"), eeprom_config.C_PCBT_MIN);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/set") == 0) // C_PCBT_MAX
  {
    str_buffer = buffer;
    eeprom_config.C_PCBT_MAX = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/max_temp/state"), eeprom_config.C_PCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/set") == 0) // C_MCBT_MIN
  {
    str_buffer = buffer;
    eeprom_config.C_MCBT_MIN = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/min_temp/state"), eeprom_config.C_MCBT_MIN);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/set") == 0) // C_MCBT_MAX
  {
    str_buffer = buffer;
    eeprom_config.C_MCBT_MAX = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/max_temp/state"), eeprom_config.C_MCBT_MAX);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/set") == 0) // KP_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KP(str_buffer.toFloat());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KP/state"), DT_3voies_PCBT_get_KP());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/set") == 0) // KI_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KI(str_buffer.toFloat());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KI/state"), DT_3voies_PCBT_get_KI());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/set") == 0) // KD_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KD(str_buffer.toFloat());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KD/state"), DT_3voies_PCBT_get_KD());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/set") == 0) // KT_PCBT
  {
    str_buffer = buffer;
    DT_3voies_PCBT_set_KT(str_buffer.toInt());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/KT/state"), DT_3voies_PCBT_get_KT());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/set") == 0) // KP_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KP(str_buffer.toFloat());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KP/state"), DT_3voies_MCBT_get_KP());
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/set") == 0) // KI_MCBT
  {
    str_buffer = buffer;

    DT_3voies_MCBT_set_KI(str_buffer.toFloat());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KI/state"), DT_3voies_MCBT_get_KI());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/set") == 0) // KD_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KD(str_buffer.toFloat());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KD/state"), DT_3voies_MCBT_get_KD());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/set") == 0) // KT_MCBT
  {
    str_buffer = buffer;
    DT_3voies_MCBT_set_KT(str_buffer.toInt());
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/KT/state"), DT_3voies_MCBT_get_KT());
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/set") == 0) // PCBT Action
  {
    if (strcmp(buffer, "direct") == 0)
    {
      DT_3voies_PCBT_set_action(QuickPID::Action::direct);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), F("direct"));
    }
    if (strcmp(buffer, "reverse") == 0)
    {
      DT_3voies_PCBT_set_action(QuickPID::Action::reverse);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_action/state"), F("reverse"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/set") == 0) // PCBT pMode
  {
    if (strcmp(buffer, "pOnError") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnError);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), F("pOnError"));
    }
    if (strcmp(buffer, "pOnErrorMeas") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnErrorMeas);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), F("pOnErrorMeas"));
    }
    if (strcmp(buffer, "pOnMeas") == 0)
    {
      DT_3voies_PCBT_set_pmode(QuickPID::pMode::pOnMeas);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_pmode/state"), F("pOnMeas"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/set") == 0) // PCBT dMode
  {
    if (strcmp(buffer, "dOnError") == 0)
    {
      DT_3voies_PCBT_set_dmode(QuickPID::dMode::dOnError);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), F("dOnError"));
    }
    if (strcmp(buffer, "dOnMeas") == 0)
    {
      DT_3voies_PCBT_set_dmode(QuickPID::dMode::dOnMeas);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_dmode/state"), F("dOnMeas"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/set") == 0) // PCBT iawmode
  {
    if (strcmp(buffer, "iAwClamp") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwClamp);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), F("iAwClamp"));
    }
    if (strcmp(buffer, "iAwCondition") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwCondition);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), F("iAwCondition"));
    }
    if (strcmp(buffer, "iAwOff") == 0)
    {
      DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode::iAwOff);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/pid_iawmode/state"), F("iAwOff"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/set") == 0) // MCBT Action
  {
    if (strcmp(buffer, "direct") == 0)
    {
      DT_3voies_MCBT_set_action(QuickPID::Action::direct);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), F("direct"));
    }
    if (strcmp(buffer, "reverse") == 0)
    {
      DT_3voies_MCBT_set_action(QuickPID::Action::reverse);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_action/state"), F("reverse"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/set") == 0) // MCBT pMode
  {
    if (strcmp(buffer, "pOnError") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnError);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), F("pOnError"));
    }
    if (strcmp(buffer, "pOnErrorMeas") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnErrorMeas);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), F("pOnErrorMeas"));
    }
    if (strcmp(buffer, "pOnMeas") == 0)
    {
      DT_3voies_MCBT_set_pmode(QuickPID::pMode::pOnMeas);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_pmode/state"), F("pOnMeas"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/set") == 0) // MCBT dMode
  {
    if (strcmp(buffer, "dOnError") == 0)
    {
      DT_3voies_MCBT_set_dmode(QuickPID::dMode::dOnError);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), F("dOnError"));
    }
    if (strcmp(buffer, "dOnMeas") == 0)
    {
      DT_3voies_MCBT_set_dmode(QuickPID::dMode::dOnMeas);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_dmode/state"), F("dOnMeas"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/set") == 0) // MCBT iawmode
  {
    if (strcmp(buffer, "iAwClamp") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwClamp);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), F("iAwClamp"));
    }
    if (strcmp(buffer, "iAwCondition") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwCondition);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), F("iAwCondition"));
    }
    if (strcmp(buffer, "iAwOff") == 0)
    {
      DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode::iAwOff);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/pid_iawmode/state"), F("iAwOff"));
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/set") == 0) // RATIO_PCBT
  {
    str_buffer = buffer;
    eeprom_config.ratio_PCBT = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/ratio/state"), eeprom_config.ratio_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/set") == 0) // RATIO_PCBT
  {
    str_buffer = buffer;
    eeprom_config.ratio_MCBT = str_buffer.toFloat();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/ratio/state"), eeprom_config.ratio_MCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/inhib-out/set") == 0) // OFFSET_PCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_inhib_PCBT = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/inhib-out/state"), eeprom_config.out_inhib_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/inhib-out/set") == 0) // OFFSET_MCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_inhib_MCBT = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/inhib-out/state"), eeprom_config.out_inhib_MCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/set") == 0) // OFFSET_PCBT_IN
  {
    str_buffer = buffer;
    eeprom_config.in_offset_PCBT = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-in/state"), eeprom_config.in_offset_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/set") == 0) // OFFSET_MCBT_IN
  {
    str_buffer = buffer;
    eeprom_config.in_offset_MCBT = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-in/state"), eeprom_config.in_offset_MCBT);
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-sup/set") == 0) // OFFSET_AVG_TEMP_MAX
  {
    str_buffer = buffer;
    eeprom_config.in_offset_avg_temp_sup = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-sup/state"), eeprom_config.in_offset_avg_temp_sup);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-state"), (float)get_temp_ext());
    sauvegardeEEPROM();
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-inf/set") == 0) // OFFSET_AVG_TEMP_MIN
  {
    str_buffer = buffer;
    eeprom_config.in_offset_avg_temp_inf = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-inf/state"), eeprom_config.in_offset_avg_temp_inf);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/avg-temp/offset-state"), (float)get_temp_ext());
    sauvegardeEEPROM();
  }

#endif // VANNES
#ifdef RELAY_ECS1
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs1/set") == 0) //
  {
    if (strcmp(buffer, "Marche") == 0)
    {
      DT_ecs1_set_mode(DT_ECS_MARCHE);
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_ecs1_set_mode(DT_ECS_ARRET);
    }
    else if (strcmp(buffer, "Veille") == 0)
    {
      DT_ecs1_set_mode(DT_ECS_STANDBY);
    }
    ecs1_mode_callback(DT_ecs1_get_mode());
    sauvegardeEEPROM();
  }
#endif // RELAY_ECS1
#ifdef RELAY_ECS2
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs2/set") == 0) //
  {
    if (strcmp(buffer, "Marche") == 0)
    {
      DT_ecs2_set_mode(DT_ECS_MARCHE);
    }
    else if (strcmp(buffer, "Arret") == 0)
    {
      DT_ecs2_set_mode(DT_ECS_ARRET);
    }
    else if (strcmp(buffer, "Veille") == 0)
    {
      DT_ecs2_set_mode(DT_ECS_STANDBY);
    }
    ecs2_mode_callback(DT_ecs2_get_mode());
    sauvegardeEEPROM();
  }
#endif                                                 // RELAY_ECS2
  else if (strcmp(topic, "homeassistant/status") == 0) // Home Assistant Online / Offline
  {
    if (strcmp(buffer, "online") == 0)
    {
      mem_config.ha_mqtt_publish = mqtt_publish(true);
      mem_config.HA_online = true; // TODO : ne fonctionne plus
      mem_config.ha_mqtt_config = homeassistant(true);
    }
    else if (strcmp(buffer, "offline") == 0)
    {
      mem_config.HA_online = false; // TODO : ne fonctionne plus
    }
  }

  memory(false);
}

void mqtt_connection_lost()
{
#ifdef POELE
  if (DT_Poele_get_mode() == DT_POELE_STANDBY)
  {
    DT_Poele_set_mode(DT_POELE_NORMAL);
  }
#endif // POELE

#ifdef VANNES
  if (DT_3voies_PCBT_get_mode() == DT_3VOIES_STANDBY)
  {
    DT_3voies_PCBT_set_mode(DT_3VOIES_NORMAL);
  }

  if (DT_3voies_MCBT_get_mode() == DT_3VOIES_STANDBY)
  {
    DT_3voies_MCBT_set_mode(DT_3VOIES_NORMAL);
  }
  eeprom_config.in_offset_PCBT = 0;
  eeprom_config.in_offset_MCBT = 0;
#endif // VANNES
#ifdef RELAY_ECS1
  if (DT_ecs1_get_mode() == DT_ECS_STANDBY)
  {
    DT_ecs1_set_mode(DT_ECS_MARCHE);
  }
#endif // RELAY_ECS1
#ifdef RELAY_ECS2
  if (DT_ecs2_get_mode() == DT_ECS_STANDBY)
  {
    DT_ecs2_set_mode(DT_ECS_MARCHE);
  }
#endif // RELAY_ECS2
}
#endif // MQTT

// initialisation de la carte a la mise sous tention
void setup()
{
  // wdt_disable();
  init_wdt();
  // Serial.begin(9600);
  Serial.begin(57600);
  Serial.println(F("starting board"));
  memory(true);
  // auto Serial.println("starting board version " BOARD_SW_VERSION_PRINT);

  Serial.println(F("Load eeprom"));
  chargeEEPROM();
  memory(false);

  TWCR = 0;
  Wire.begin();
  memory(false);
  // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
  // Wire.write(MCP_CHANNEL); // channel 1
  // Wire.endTransmission();
#ifdef I2C_Multiplexeur
  Serial.println(F("init mcp"));
  DT_mcp_init();
#endif

  memory(false);

#ifdef MQTT
  // Serial.print(millis());
  Serial.println(F("starting mqtt"));
  DT_mqtt_init();
  Serial.println(F("mqtt initialized"));
  DT_mqtt_set_subscribe_callback(mqtt_subscribe);
  DT_mqtt_set_receve_callback(mqtt_receve);
  DT_mqtt_set_publish_callback(mqtt_publish);
  DT_mqtt_set_connection_lost_callback(mqtt_connection_lost);
#endif // MQTT

  Serial.println(F("starting relay"));
  DT_relay_init();
#ifdef MQTT
  DT_relay_set_callback(relay_callback);
#endif // MQTT

#if OPT_RELAY_NUM > 0
  Serial.println(F("starting opt-relay"));
  DT_opt_relay_init();
#ifdef MQTT
  DT_opt_relay_set_callback(opt_relay_callback);
#endif // MQTT
#endif // OPT_RELAY_NUM > 0

#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM > 0
  Serial.println(F("starting dimmer"));
  Dimmer_init();
#endif // DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM > 0
#ifdef MQTT
#if DIMMER_LIGHT_NUM > 0
  set_dimmer_callback(dimmer_callback);
#endif // DIMMER_LIGHT_NUM
#endif // MQTT

#if COVER_NUM > 0
  Serial.println(F("starting cover"));
  DT_cover_init();
#ifdef MQTT
  DT_cover_set_callback(cover_callback);
#endif // MQTT
#endif // COVER_NUM

#if PORTAL_NUM > 0
  Serial.println(F("starting portal"));
  DT_portal_init();
#ifdef MQTT
  DT_portal_set_callback(portal_callback);
#endif // MQTT
#endif // PORTAL_NUM

#if RADIATOR_NUM > 0
  Serial.println(F("starting radiator"));
  DT_radiator_init();
#ifdef MQTT
  DT_radiator_set_callback(dt_radiator_callback);
#endif // MQTT
#endif // VANNE

  Serial.println(F("starting input"));
  DT_input_init();
  DT_input_set_callback(input_callback);

#if PT100_NUM > 0
  Serial.println(F("starting PT100"));
  DT_pt100_init();
#ifdef MQTT
  DT_pt100_set_callback(pt100_callback);
#endif // MQTT
#endif // PT100

#if BME280_NUM > 0

  Serial.println(F("starting BME280"));
  DT_BME280_init();
#ifdef MQTT
  DT_BME280_set_callback_temperature(bme280_callback_temperature);
  DT_BME280_set_callback_humidity(bme280_callback_humidity);
  DT_BME280_set_callback_pressure(bme280_callback_pressure);
#endif // MQTT
#endif // BME280_NUM

#if CCS811_NUM > 0
  Serial.println(F("starting CCS811"));
  DT_CCS811_init();
#ifdef MQTT
  DT_CCS811_set_callback_co2(ccs811_callback_co2);
  DT_CCS811_set_callback_cov(ccs811_callback_cov);
#endif // MQTT
#endif // BCC811_NUM

#if SCD4X_NUM > 0

  Serial.println(F("starting SCD4X"));
  DT_SCD4X_init();
#ifdef MQTT
  DT_SCD4X_set_callback_temperature(scd4x_callback_temperature);
  DT_SCD4X_set_callback_humidity(scd4x_callback_humidity);
  DT_SCD4X_set_callback_co2(scd4x_callback_co2);
#endif // MQTT
#endif // SCD4X_NUM

#if HDC1080_NUM > 0

  Serial.println(F("starting HDC1080"));
  DT_HDC1080_init();
#ifdef MQTT
  DT_HDC1080_set_callback_temperature(hdc1080_callback_temperature);
  DT_HDC1080_set_callback_humidity(hdc1080_callback_humidity);
#endif // MQTT
#endif // HDC1080_NUM

#if CPT_PULSE_INPUT > 0 || CPT_PULSE_INPUT_IF_OUT > 0 || CPT_PULSE_INPUT_IF_IN > 0
  Serial.println(F("starting cpt_pulse_input"));
  DT_cpt_pulse_input_init();
#ifdef MQTT
#if CPT_PULSE_INPUT > 0
  DT_cpt_pulse_input_set_callback(cpt_pulse_input_callback);
#endif // CPT_PULSE_INPUT
#if CPT_PULSE_INPUT_IF_OUT > 0
  DT_cpt_pulse_input_if_out_set_callback(cpt_pulse_input_if_out_callback);
#endif // CPT_PULSE_INPUT_IF_OUT
#if CPT_PULSE_INPUT_IF_IN > 0
  DT_cpt_pulse_input_if_in_set_callback(cpt_pulse_input_if_in_callback);
#endif // CPT_PULSE_INPUT_IF_IN
#endif // MQTT
#endif // CPT_PULSE_INPUT > 0 || CPT_PULSE_INPUT_IF_OUT > 0 || CPT_PULSE_INPUT_IF_IN > 0

  // auto Serial.println("starting fake_NTC");
  // DT_fake_ntc_init();
  // DT_fake_ntc_callback(fake_ntc_callback);

#ifdef POELE
  Serial.println(F("starting Poele"));
  DT_Poele_init();
#ifdef MQTT
  DT_Poele_set_mode_callback(poele_mode_callback);
#endif // MQTT
#endif // POELE

#ifdef CHAUFFAGE
  Serial.println(F("starting Chauffage"));
  DT_Chauffage_init();
#ifdef MQTT
  DT_Chauffage_set_mode_callback(chauffage_mode_callback);
  DT_Chauffage_set_temperature_arret_poele_hiver_callback(temperature_arret_poele_hiver);  
  DT_Chauffage_set_arret_meteo_callback(arret_meteo_callback);
#endif // MQTT
#endif // CHAUFFAGE

#ifdef VANNES
  Serial.println(F("starting 3 voies"));
  DT_3voies_init();
#ifdef MQTT
  DT_3voies_set_callback(dt3voies_callback);
  DT_3voies_pcbt_set_callback_pid(dt3voies_callback_pid_pcbt);
  DT_3voies_mcbt_set_callback_pid(dt3voies_callback_pid_mcbt);
  DT_3voies_set_callback_avg_temp(dt3voies_callback_avg_temp);
#endif // MQTT
#endif // VANNE

#ifdef TIC
  DT_teleinfo_init();
  DT_teleinfo_set_callback(teleinfo_callback);
#endif // TIC

  DT_interation_init();

// wdt_enable(WATCHDOG_TIME);
#ifdef MQTT
  homeassistant(true);
#endif

#ifdef CLOCK
  debug_wdt_reset(F(AT));
  Serial.println(F("starting RTC/NTP"));
  rtcNtp.begin();
  rtcNtp.syncOnce();
#endif

  // Serial.print(millis());
  Serial.println(F("Board started"));
  Serial.print(F("version: "));
  Serial.println(F(BOARD_SW_VERSION_PRINT));
  memory(true);

  // init_wdt();
}

// boucle principale
void loop()
{
  debug_wdt_reset(F(AT));
  uint32_t now = millis();
  static uint16_t interlock = BOOST_PP_COUNTER;
  if (interlock == 0)
  {
    interlock = BOOST_PP_COUNTER;
  }

#ifdef MQTT
  DT_mqtt_loop();
#endif

  DT_input_loop();

#if DIMMER_LIGHT_NUM > 0
  dimmer_loop();
#endif

  switch (interlock++)
  {
  case BOOST_PP_COUNTER:
    DT_relay_loop();
    break;

#if OPT_RELAY_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_opt_relay_loop();
    break;
#endif // OPT_RELAY_NUM

#if PT100_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_pt100_loop();
    break;
#endif

#if BME280_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_BME280_loop();
    break;
#endif // BME280_NUM

#if CCS811_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_CCS811_loop();
    break;
#endif

#if SCD4X_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_SCD4X_loop();
    break;
#endif // SCD4X_NUM

#if HDC1080_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_HDC1080_loop();
    break;
#endif // HDC1080_NUM

#ifdef POELE
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_Poele_loop();
    break;
#endif

#ifdef CHAUFFAGE
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_Chauffage_loop();
    break;
#endif

#ifdef VANNES
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_3voies_loop();
    break;
#endif

#if COVER_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_cover_loop();
    break;
#endif

#if PORTAL_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_portal_loop();
    break;
#endif // PORTAL_NUM

#if RADIATOR_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_radiator_loop();
    break;
#endif

#ifdef DT_INTERACTION
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_interation_loop();
    break;
#endif

#if defined(RELAY_ECS1) || defined(RELAY_ECS2)
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    // DT_ecs_loop();
    break;
#endif // RELAY_ECS1 || RELAY_ECS2

#ifdef TIC
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_teleinfo_loop();
    break;
#endif // TIC

#ifdef CLOCK
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    rtcNtp.loop();
    break;
#endif // CLOCK

  default:
    interlock = 0;
    break;
  }

  load();
  memory(false);

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
  // static bool up = true;
  if (now - old > 5000)
  {
    old = now;
    DT_relay(RELAY_ECS1, !DT_relay_get(RELAY_ECS1));
  }
}
