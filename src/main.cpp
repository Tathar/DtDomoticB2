#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>

#include <DT_ha.h>

#include <DT_relay.h>
#include <DT_Dimmer.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_mqtt.h>
#include <DT_mqtt_send.h>
#include <DT_ha.h>
#include <DT_BME280.h>
#include <DT_CCS811.h>
#include <DT_mcp.h>
#include <DT_poele.h>
#include <DT_eeprom.h>
#include <DT_cover.h>

#include <avr/wdt.h> //watchdog

// #include <pinout.h>
#include <config.h>

#include <boost/preprocessor/slot/counter.hpp>

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
// calcule de la memoire disponible
void memory(bool print)
{
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

// Relay Callback
// envoie de donnée MQTT quand un relais est activé / désactivé
void relay_callback(const uint8_t num, const bool action)
{
  // char topic[56];
  const __FlashStringHelper *payload;
  debug(F(AT));
  memory(false);
  // debug_wdt_reset();
  if (mem_config.MQTT_online)
  {
    if (action)
    {
      payload = F("ON");
    }
    else
    {
      payload = F("OFF");
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/state"), num, payload);
  }
  memory(false);
}
#endif // MQTT

#if DIMMER_LIGHT_NUM > 0
void dimmer_callback(const uint8_t num, const uint8_t percent, const bool candle)
{
  const __FlashStringHelper *payload;
  if (num < DIMMER_LIGHT_NUM)
  {
    if (percent == 0)
      payload = F("OFF");
    else
      payload = F("ON");
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/state"), num, payload);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/bri_state"), num, percent);
    if (candle)
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_state"), num, F("CANDLE"));
    else
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_state"), num, F("NONE"));
  }
}
#endif // NUM_DIMMER

#if COVER_NUM > 0
void cover_callback(const uint8_t num, const uint8_t percent, const cover_state state)
{
  switch (state)
  {
  case cover_stopped:
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num, F("stopped"));
    break;

  case cover_open:
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num, F("open"));
    break;

  case cover_closed:
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num, F("closed"));
    break;

  case cover_opening:
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num, F("opening"));
    break;

  case cover_closing:
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/state"), num, F("closing"));
    break;

  default:
    break;
  }

  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/pos_state"), num, percent);
}
#endif
// envoi de donné MQTT quand une entrée est activée / desactivée
void input_callback(const uint8_t num, const Bt_Action action)
{
  debug(F(AT));
  const __FlashStringHelper *payload;
  memory(false);

  if (mem_config.MQTT_online)
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
      payload = F("ON");
#endif
      break;

    case IN_RELEASE:
      // Serial.println("OFF");
#ifdef MQTT
      // DT_mqtt_send(topic, "OFF");
      payload = F("OFF");
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
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/input-%02d/state"), num, payload);
  memory(false);
}

#ifdef MQTT
#if TEMP_NUM > 0
// envoi de donné MQTT quand une temperature issue d'une PT100 à changée
void pt100_callback(const uint8_t num, const float temp)
{
  debug(F(AT));
  memory(false);
  Serial.println("PT100_CALLBACK ");

  if (mem_config.MQTT_online)
  {
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pt100-%02d/temperature"), num, temp);
  }
  memory(false);
}
#endif // PT100

// envoi de donné MQTT quand une temperature issue d'une carte BME280 à changée
void bme280_callback_temperature(const uint8_t num, const float temperature)
{
  debug(F(AT));
  memory(false);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/temperature"), num, temperature);
  }
  memory(false);
}

// envoi de donné MQTT quand une valeur d'humidité issue d'une carte BME280 à changée
void bme280_callback_humidity(const uint8_t num, const float humidity)
{
  debug(F(AT));
  memory(true);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/humidity"), num, humidity);
  }
  memory(false);
}

// envoi de donné MQTT quand la pression issue d'une carte BME280 à changée
void bme280_callback_pressure(const uint8_t num, const float pressure)
{
  debug(F(AT));
  memory(true);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/bme280-%02d/pressure"), num, pressure);
  }
  memory(false);
}

// envoi de donné MQTT quand une valeur de CO2 issue d'une carte CSS811 à changée
void ccs811_callback_co2(const uint8_t num, const float co2)
{
  debug(F(AT));
  memory(false);

  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/co2"), num, co2);
  }
  memory(false);
}

// envoi de donné MQTT quand une valeur de COV issue d'une carte CSS811 à changée
void ccs811_callback_cov(const uint8_t num, const float cov)
{
  // debug(AT);
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ccs811-%02d/cov"), num, cov);
  }
  memory(false);
}

// envoi de donné MQTT quand le Mode de fonctionnement du poele change
#ifdef POELE
void poele_mode_callback(const DT_Poele_mode mode)
{
  debug(F(AT));
  memory(false);
  const __FlashStringHelper *payload;
  // mode poele
  if (mem_config.MQTT_online)
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
    case DT_POELE_ECS:
      payload = F("ECS");
      break;
    case DT_POELE_FORCE:
      payload = F("Forcé");
      break;
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), payload);
  }
  memory(false);
}
#endif // POELE

// envoi de donné MQTT quand la variable C2 ou C3 change
#ifdef VANNES
void dt3voies_callback(const float C2, const float C3)
{
  debug(F(AT));
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // 220502  debug(F(AT));
    send_buffer.reserve(2);
    int32_t digit = C2 * 100;
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/C2/state"), (float)(digit / 100.0));

    digit = C3 * 100;
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C3/state"), (float)(digit / 100.0));
  }
  memory(false);
}

// retour des valleur du PID PCBT
void dt3voies_callback_pid_pcbt(const float P, const float I, const float D, const float OUT)
{
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    refresh = now;
    // 220502  debug(F(AT));
    send_buffer.reserve(4);

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

// retour des valleur du PID MCBT
void dt3voies_callback_pid_mcbt(const float P, const float I, const float D, const float OUT)
{
  memory(false);
  static uint32_t refresh = 0;
  uint32_t now = millis();
  if (now - refresh >= MQTT_REFRESH && mem_config.MQTT_online)
  {
    // 220502  debug(F(AT));
    refresh = now;
    send_buffer.reserve(4);

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
  memory(false);
}
#endif // VANNES

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

  uint32_t now = millis();
  if (now - time >= 50)
  {
    time = now;
    switch (sequance)
    {
    case BOOST_PP_COUNTER:
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/availability"), F("online"));
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

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < INPUT_NUM)
      {
        input_callback(num, DT_input_get_stats(num));
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
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_state"), num, eeprom_config.Dimmer_scale_min[num]);
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
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_state"), num, eeprom_config.Dimmer_scale_max[num]);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;
#endif // DIMMER_LIGHT_NUM

#if TEMP_NUM > 0 // PT100
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < TEMP_NUM)
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
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C8/state"), eeprom_config.C8);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C9
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C9/state"), eeprom_config.C9);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C10
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C10/state"), eeprom_config.C10);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // C11
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C11/state"), eeprom_config.C11);
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
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), eeprom_config.out_offset_PCBT);
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // OFFSET_MCBT
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), eeprom_config.out_offset_MCBT);
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
#endif // vanne

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // ONLINE
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"), F("online"));

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

// inscription au topic MQTT (necessair a la recption des données par la carte)
bool mqtt_subscribe(MQTTClient &mqtt, bool start)
{
  debug(F(AT));
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/set"), num);
        mqtt.subscribe(topic);
        num++;
        sequance--;
      }
      else
      {
        num = 0;
      }
      break;

#ifdef DIMMER_LIGHT_NUM
#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      if (num < DIMMER_LIGHT_NUM)
      {
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/set"), num);
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/bri_set"), num);
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_set"), num);
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_set"), num);
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
        snprintf_P(topic, 56, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_set"), num);
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
#endif // POELE

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

#ifdef POELE
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
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set");
      break;

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set");
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

#include BOOST_PP_UPDATE_COUNTER()
    case BOOST_PP_COUNTER:
      // HomeAssistant
      mqtt.subscribe("homeassistant/status");
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
void mqtt_receve(MQTTClient *client, const char topic[], const char payload[], const int length)
{

  debug(F(AT));
  char buffer[64];
  char _topic[64];
  memory(false);
  String str_buffer;
  Serial.print("receve topic ");
  Serial.println(topic);

  // Copy the payload to the new buffer
  if (length < 64)
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
  bool bool_value = false;
  if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/relay-%02d/set"), &num) == 1) // relais
  {
    // auto Serial.print("sscanf = ");
    // auto Serial.println(num);

    if (strcmp(buffer, "ON") == 0)
      DT_relay(num, true);
    else if (strcmp(buffer, "OFF") == 0)
      DT_relay(num, false);
  }
#if DIMMER_LIGHT_NUM > 0
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d"), &num) == 1) // dimmer
  {
    // Serial.println("dimmer");
    if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/set"), num) > 0 && strncmp(topic, _topic, 64) == 0)
    {
      // Serial.println("set");
      bool_value = get_dimmer_candle(num);
      if (strcmp(buffer, "ON") == 0)
      {
        dimmer_set(num, true, DIMMER_SPEED, bool_value);
      }
      else if (strcmp(buffer, "OFF") == 0)
        dimmer_set(num, false, DIMMER_SPEED);
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/bri_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("bri_set");
      if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
      {
        bool_value = get_dimmer_candle(num);
        dimmer_set(num, u8t_value, DIMMER_SPEED, bool_value);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/fx_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("fx_set");
      u8t_value = get_dimmer(num);
      if (strcmp(buffer, "NONE") == 0)
      {
        dimmer_set(num, u8t_value, DIMMER_SPEED, false);
      }
      else if (strcmp(buffer, "CANDLE") == 0)
      {
        dimmer_set(num, u8t_value, DIMMER_SPEED, true);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("min_set");
      str_buffer = buffer;
      eeprom_config.Dimmer_scale_min[num] = (uint16_t)str_buffer.toDouble();
      if (eeprom_config.Dimmer_scale_min[num] <= eeprom_config.Dimmer_scale_max[num])
        eeprom_config.Dimmer_scale_min[num] = eeprom_config.Dimmer_scale_max[num] + 20;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/min_state"), num, (uint16_t)eeprom_config.Dimmer_scale_min[num]);
      dimmer_set(num, true, 0);
      sauvegardeEEPROM();
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // dimmer
    {
      // Serial.println("max_set");
      str_buffer = buffer;
      eeprom_config.Dimmer_scale_max[num] = (uint16_t)str_buffer.toDouble();
      if (eeprom_config.Dimmer_scale_max[num] >= eeprom_config.Dimmer_scale_min[num])
        eeprom_config.Dimmer_scale_max[num] = eeprom_config.Dimmer_scale_min[num] - 20;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/max_state"), num, (uint16_t)eeprom_config.Dimmer_scale_max[num]);
      dimmer_set(num, true, 0);
      sauvegardeEEPROM();
    }
  }
#endif // DIMMER_LIGHT_NUM

#if COVER_NUM > 0
  else if (sscanf_P(topic, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d"), &num) == 1) // cover
  {
    // Serial.println("dimmer");
    if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/cover-%02d/set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // cover
    {
      // Serial.println("set");
      if (strcmp(buffer, "STOP") == 0)
      {
        DT_cover_stop(num);
      }
      else if (strcmp(buffer, "OPEN") == 0)
      {
        DT_cover_set(num, 100);
      }
      else if (strcmp(buffer, "CLOSE") == 0)
      {
        DT_cover_set(num, 0);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/pos_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // cover
    {
      if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
      {
        DT_cover_set(num, u8t_value);
      }
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/up_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // time cover up
    {
      // Serial.println("max_set");
      str_buffer = buffer;
      eeprom_config.cover[num].ratio_up = (uint16_t)(str_buffer.toDouble() / 100);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/up_state"), num, (uint16_t)(eeprom_config.cover[num].ratio_up * 100));
      sauvegardeEEPROM();
    }
    else if (snprintf_P(_topic, 64, PSTR(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/down_set"), num) > 0 && strncmp(topic, _topic, 64) == 0) // time cover down
    {
      // Serial.println("max_set");
      str_buffer = buffer;
      eeprom_config.cover[num].ratio_down = (uint16_t)(str_buffer.toDouble() / 100);
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/dimmer-%02d/down_state"), num, (uint16_t)(eeprom_config.cover[num].ratio_down * 100));
      sauvegardeEEPROM();
    }
  }

#endif // COVER_NUM > 0

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
#endif // Poele
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
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/V3/state"), eeprom_config.V3);
    sauvegardeEEPROM();

    Serial.println(" ");
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
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C5 = u8t_value;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C5/state"), u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/set") == 0) // C6
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C6 = u8t_value;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C6/state"), u8t_value);
      sauvegardeEEPROM();
    }
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/set") == 0) // C7
  {
    if (sscanf_P(buffer, PSTR("%" SCNu8), &u8t_value) == 1)
    {
      eeprom_config.C7 = u8t_value;
      DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/C7/state"), u8t_value);
      sauvegardeEEPROM();
    }
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
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C10/state"), eeprom_config.C10);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/C11/set") == 0) // C11
  {
    str_buffer = buffer;
    eeprom_config.C11 = str_buffer.toFloat();
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

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/set") == 0) // OFFSET_PCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_offset_PCBT = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/pcbt/offset-out/state"), eeprom_config.out_offset_PCBT);
    sauvegardeEEPROM();
  }

  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/set") == 0) // OFFSET_MCBT_OUT
  {
    str_buffer = buffer;
    eeprom_config.out_offset_MCBT = str_buffer.toInt();
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/mcbt/offset-out/state"), eeprom_config.out_offset_MCBT);
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

#endif                                                 // VANNES
  else if (strcmp(topic, "homeassistant/status") == 0) // Home Assistant Online / Offline
  {
    if (strcmp(buffer, "online") == 0)
    {
      mqtt_publish(true);
      mem_config.HA_online = true; // TODO : ne fonctionne plus
      mem_config.HA_MQTT_CONFIG = homeassistant(true);
    }
    else if (strcmp(buffer, "offline") == 0)
    {
      mem_config.HA_online = false; // TODO : ne fonctionne plus
    }
  }

  memory(false);
}
#endif // MQTT

// initialisation de la carte a la mise sous tention
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

  Serial.println("starting dimmer");
  Dimmer_init();
#ifdef MQTT
  set_dimmer_callback(dimmer_callback);
#endif // MQTT

  Serial.println("starting cover");
  DT_cover_init();
#ifdef MQTT
  DT_cover_set_callback(cover_callback);
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

  wdt_enable(WATCHDOG_TIME);
  homeassistant(true);
  // Serial.print(millis());
  Serial.println(F("Board started"));
  Serial.print(F("version: "));
  Serial.println(F(BOARD_SW_VERSION_PRINT));
  memory(true);
}

// boucle principale
void loop()
{
  // debug(AT);
  uint32_t now = millis();
  static uint16_t interlock = BOOST_PP_COUNTER;
  if (interlock == 0)
  {
    interlock = BOOST_PP_COUNTER;
  }
  debug_wdt_reset(F(AT));
  // debug_wdt_reset();

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

#if TEMP_NUM > 0
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_pt100_loop();
    break;
#endif

#ifdef BME_280
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_BME280_loop();
    break;
#endif

#ifdef CCS811
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_CCS811_loop() break;
#endif

#ifdef POELE
#include BOOST_PP_UPDATE_COUNTER()
  case BOOST_PP_COUNTER:
    DT_Poele_loop();
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
  if (now - old > 1000)
  {
    old = now;
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
}
