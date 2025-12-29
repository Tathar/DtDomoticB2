#include <DT_ecs_raph.h>
#include <DT_relay.h>
#include <DT_PT100.h>
#include <DT_mqtt_send.h>
#include <DT_clock.h>
#include <DT_poele.h>

#ifdef ECS_RAPH

float start_ecs_temp = 0.0;
uint32_t start_ecs_time = 0;

void DT_ECS_RAPH_init() {
};

void DT_ECS_RAPH_loop()
{
  static bool started = false;
  static bool hour_rise = false;
  static uint32_t last_check_time_1 = 0;

  if (millis() - last_check_time_1 > 1000)
  {
    last_check_time_1 = millis();
    float current_temp = DT_pt100_get(PT100_ECS);
    if (eeprom_config.ecs_raph_mode != DT_ECS_MANUAL)
    {
      if (!started && current_temp > 0 && current_temp < DT_ECS_RAPH_get_secours()) // mode secours
      {
        started = ecs_start();
      }
      if (started && current_temp > 0 && current_temp >= DT_ECS_RAPH_get_consigne()) // arret de l'ecs
      {
        started = ecs_stop();
      }
    }

    switch (eeprom_config.ecs_raph_mode)
    {
    case DT_ECS_MANUAL: // on ne fait rien en mode manuel (relais disponible HA)
      break;
    case DT_ECS_WINTER: // pas de chauffage electrique en hiver, seulement le mode secours
      break;
    case DT_ECS_BETWEEN:
    case DT_ECS_ETE:
      if (!hour_rise && !DT_Poele_started() && rtcNtp.ToD() > (DT_Chauffage_get_debut_cycle() + 60))
      {
        started = ecs_start();
        hour_rise = true;
      }
      else if (hour_rise && rtcNtp.ToD() < DT_Chauffage_get_debut_cycle() + 60)
      {
        hour_rise = false;
      }
      break;
    }
  }
  else if (started && millis() - start_ecs_time > 2 * 60 * 60 * 1000)  //verification du fonctionnement de la resistane
  {
    start_ecs_time = millis();
    if (DT_pt100_get(PT100_ECS) - start_ecs_temp < 2.0)
    {
      started = ecs_start; // changement de resistance
    }   
  }
}

uint8_t get_next_resistor()
{
  static uint8_t old_resistor = 254;
  old_resistor = (old_resistor + 1) % 3;
  return old_resistor;
}

bool ecs_start()
{
  switch (get_next_resistor())
  {
  case 0:
    DT_relay(RELAY_ECS1, true);
    DT_relay(RELAY_ECS2, false);
    DT_relay(RELAY_ECS3, false);
    break;
  case 1:
    DT_relay(RELAY_ECS1, false);
    DT_relay(RELAY_ECS2, true);
    DT_relay(RELAY_ECS3, false);
    break;
  case 2:
    DT_relay(RELAY_ECS1, false);
    DT_relay(RELAY_ECS2, false);
    DT_relay(RELAY_ECS3, true);
    break;
  }
  start_ecs_temp = DT_pt100_get(PT100_ECS);
  start_ecs_time = millis();
  return true;
}

bool ecs_stop()
{
  DT_relay(RELAY_ECS1, false);
  DT_relay(RELAY_ECS2, false);
  DT_relay(RELAY_ECS3, false);
  return false;
}

DT_ECS_RAPH_mode DT_ECS_RAPH_get_mode()
{
  return eeprom_config.ecs_raph_mode;
}

void DT_ECS_RAPH_set_mode(DT_ECS_RAPH_mode mode)
{
  eeprom_config.ecs_raph_mode = mode;
  sauvegardeEEPROM();
  DT_ECS_RAPH_mode_callback(mode);
}

// envoi de donné MQTT quand le Mode de fonctionnement de l'ECS change
void DT_ECS_RAPH_mode_callback(const DT_ECS_RAPH_mode mode)
{
  const __FlashStringHelper *payload;
  Serial.println(F("ecs1_mode_callback"));
  // mode poele
  switch (mode)
  {
  case DT_ECS_MANUAL:
    payload = F("Manuel");
    break;
  case DT_ECS_WINTER:
    payload = F("Hiver");
    break;
  case DT_ECS_BETWEEN:
    payload = F("Intersaison");
    break;
  case DT_ECS_ETE:
    payload = F("Ete");
    break;
  }
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/mode/state"), payload);
}

float DT_ECS_RAPH_get_init_consigne()
{
  return eeprom_config.ecs_raph_init_consigne;
};

void DT_ECS_RAPH_set_init_consigne(float cons)
{
  eeprom_config.ecs_raph_init_consigne = cons;
  sauvegardeEEPROM();
  DT_ECS_RAPH_init_consigne_callback(cons);
  DT_ECS_RAPH_consigne_callback(DT_ECS_RAPH_get_consigne());
};

void DT_ECS_RAPH_init_consigne_callback(float cons)
{
  char payload[16];
  snprintf(payload, sizeof(payload), "%.2f", cons);
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/init_cons/state"), payload);
}

uint8_t DT_ECS_RAPH_get_nb_personne()
{
  return eeprom_config.ecs_raph_nb_personne;
}

void DT_ECS_RAPH_set_nb_personne(uint8_t nb)
{
  eeprom_config.ecs_raph_nb_personne = nb;
  sauvegardeEEPROM();
  DT_ECS_RAPH_nb_personne_callback(nb);
  DT_ECS_RAPH_consigne_callback(DT_ECS_RAPH_get_consigne());
}

void DT_ECS_RAPH_nb_personne_callback(uint8_t nb)
{
  char payload[4];
  snprintf(payload, sizeof(payload), "%d", nb);
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/nb_pers/state"), payload);
}

float DT_ECS_RAPH_get_coef()
{
  return eeprom_config.ecs_raph_coef;
}

void DT_ECS_RAPH_set_coef(float coef)
{
  eeprom_config.ecs_raph_coef = coef;
  sauvegardeEEPROM();
  DT_ECS_RAPH_nb_personne_callback(coef);
  DT_ECS_RAPH_consigne_callback(DT_ECS_RAPH_get_consigne());
}

void DT_ECS_RAPH_coef_callback(float coef)
{
  char payload[16];
  snprintf(payload, sizeof(payload), "%.2f", coef);
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/coef/state"), payload);
}

float DT_ECS_RAPH_get_secours()
{
  return eeprom_config.ecs_raph_secours;
}

void DT_ECS_RAPH_set_secours(float secours)
{
  eeprom_config.ecs_raph_secours = secours;
  sauvegardeEEPROM();
  DT_ECS_RAPH_secours_callback(secours);
}

void DT_ECS_RAPH_secours_callback(float secours)
{
  char payload[16];
  snprintf(payload, sizeof(payload), "%.2f", secours);
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/secours/state"), payload);
}

float DT_ECS_RAPH_get_consigne() // consigne d'arret de l'ECS
{
  float init_consigne = DT_ECS_RAPH_get_init_consigne();
  uint8_t nb_personne = DT_ECS_RAPH_get_nb_personne();
  float coef = DT_ECS_RAPH_get_coef();

  float consigne = init_consigne + (nb_personne * coef);
  return consigne;
}

void DT_ECS_RAPH_consigne_callback(float cons)
{
  char payload[16];
  snprintf(payload, sizeof(payload), "%.2f", cons);
  DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/cons/state"), payload);
}

bool DT_ECS_RAPH_mqtt_subscribe(MQTTClient &mqtt, uint8_t sequance)
{
  if (sequance == 0)
    mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/mode/set");
  else if (sequance == 1)
    mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/coef/set");
  else if (sequance == 2)
    mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/nb_pers/set");
  else if (sequance == 3)
    mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/init_cons/set");
  else if (sequance == 4)
    mqtt.subscribe(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/secours/set");
  else
    return false;
  return true;
}

bool DT_ECS_RAPH_mqtt_receve(const char topic[], const char buffer[])
{
  if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/mode/set") == 0)
  {
    if (strcmp(buffer, "Manuel") == 0)
      DT_ECS_RAPH_set_mode(DT_ECS_MANUAL);
    else if (strcmp(buffer, "Hiver") == 0)
      DT_ECS_RAPH_set_mode(DT_ECS_WINTER);
    else if (strcmp(buffer, "Intersaison") == 0)
      DT_ECS_RAPH_set_mode(DT_ECS_BETWEEN);
    else if (strcmp(buffer, "Ete") == 0)
      DT_ECS_RAPH_set_mode(DT_ECS_ETE);

    return true;
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/coef/set") == 0)
  {
    float coef = atof(buffer);
    DT_ECS_RAPH_set_coef(coef);
    return true;
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/nb_pers/set") == 0)
  {
    uint8_t nb = (uint8_t)atoi(buffer);
    DT_ECS_RAPH_set_nb_personne(nb);
    return true;
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/init_cons/set") == 0)
  {
    float init_cons = atof(buffer);
    DT_ECS_RAPH_set_init_consigne(init_cons);
    return true;
  }
  else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/ecs/secours/set") == 0)
  {
    float secours = atof(buffer);
    DT_ECS_RAPH_set_secours(secours);
    return true;
  }
  return false;
}

bool DT_ECS_RAPH_mqtt_publish(uint8_t sequance)
{
  if (sequance == 0)
  {
    DT_ECS_RAPH_mode_callback(DT_ECS_RAPH_get_mode());
    return true;
  }
  else if (sequance == 1)
  {
    DT_ECS_RAPH_init_consigne_callback(DT_ECS_RAPH_get_init_consigne());
    return true;
  }
  else if (sequance == 2)
  {
    DT_ECS_RAPH_nb_personne_callback(DT_ECS_RAPH_get_nb_personne());
    return true;
  }
  else if (sequance == 3)
  {
    DT_ECS_RAPH_coef_callback(DT_ECS_RAPH_get_coef());
    return true;
  }
  else if (sequance == 4)
  {
    DT_ECS_RAPH_secours_callback(DT_ECS_RAPH_get_secours());
    return true;
  }
  else if (sequance == 5)
  {
    DT_ECS_RAPH_consigne_callback(DT_ECS_RAPH_get_consigne());
    return true;
  }
  else
  {
    return false;
  }
}

#endif // ECS_RAPH
