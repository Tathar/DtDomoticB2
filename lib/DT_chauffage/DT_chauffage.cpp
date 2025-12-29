// #include <DT_chauffage.h>
#include <DT_poele.h>
// #include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <DT_mqtt_send.h>

#include <QuickPID.h>
#include <DT_eeprom.h>

#include <config.h>
#ifdef CHAUFFAGE

bool async_call_chauffage_mode;
bool async_call_temperature_arret_poele_hiver;
bool async_call_arret_meteo;

void DT_Chauffage_init()
{
    // DT_relay(RELAY_EV1, false);

    async_call_chauffage_mode = false;
    async_call_temperature_arret_poele_hiver = false;
    async_call_arret_meteo = false;
}

void DT_Chauffage_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;

    if (now - old >= 1000)
    {
        // 220502  debug(F(AT));
        old = now;
        if (eeprom_config.chauffage_mode == DT_CHAUFFAGE_ETE)
        {
        }
        else if (eeprom_config.chauffage_mode == DT_CHAUFFAGE_BETWEEN)
        {
        }
        else if (eeprom_config.chauffage_mode == DT_CHAUFFAGE_WINTER)
        {
            DT_relay(9, false);
            DT_relay(12, false);
        }
    }
}

// getters and setters
DT_Chauffage_mode DT_Chauffage_get_mode(void)
{
    return eeprom_config.chauffage_mode;
}

void DT_Chauffage_set_mode(DT_Chauffage_mode mode)
{
    if (mode != eeprom_config.chauffage_mode)
    {
        eeprom_config.chauffage_mode = mode;
        async_call_chauffage_mode = true;
        sauvegardeEEPROM();
        DT_Chauffage_mode_callback(mode);
    }
}

void DT_Chauffage_mode_callback(const DT_Chauffage_mode mode)
{
    const __FlashStringHelper *payload;
    switch (mode)
    {
    case DT_CHAUFFAGE_ETE:
        payload = F("Eté");
        break;
    case DT_CHAUFFAGE_BETWEEN:
        payload = F("Inter-saison");
        break;
    case DT_CHAUFFAGE_WINTER:
        payload = F("Hiver");
        break;
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/mode/state"), payload);
}

void DT_Chauffage_set_temperature_arret_poele_hiver(float data)
{
    if (data != eeprom_config.temperature_arret_poele_hiver)
    {
        eeprom_config.temperature_arret_poele_hiver = data;
        async_call_temperature_arret_poele_hiver = true;
        sauvegardeEEPROM();
        DT_Chauffage_temperature_arret_poele_hiver_callback(data);
    }
}

float DT_Chauffage_get_temperature_arret_poele_hiver(void)
{
    return eeprom_config.temperature_arret_poele_hiver;
};

void DT_Chauffage_temperature_arret_poele_hiver_callback(const float data)
{
    char payload[10];
    dtostrf(data, 4, 1, payload);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPH/state"), payload);
}

void DT_Chauffage_set_temperature_arret_poele_intersaison(float data)
{
    if (data != eeprom_config.temperature_arret_poele_intersaison)
    {
        eeprom_config.temperature_arret_poele_intersaison = data;
        async_call_temperature_arret_poele_hiver = true;
        sauvegardeEEPROM();
        DT_Chauffage_temperature_arret_poele_intersaison_callback(data);
    }
}

float DT_Chauffage_get_temperature_arret_poele_intersaison(void)
{
    return eeprom_config.temperature_arret_poele_intersaison;
}

void DT_Chauffage_temperature_arret_poele_intersaison_callback(const float data)
{
    char payload[10];
    dtostrf(data, 4, 1, payload);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPI/state"), payload);
}

void DT_Chauffage_set_temperature_ballon_max(float data)
{
    if (data != eeprom_config.temperature_balon_max)
    {
        eeprom_config.temperature_balon_max = data;
        sauvegardeEEPROM();
        DT_Chauffage_temperature_ballon_max_callback(data);
    }
}

float DT_Chauffage_get_temperature_ballon_max(void)
{
    return eeprom_config.temperature_balon_max;
}

void DT_Chauffage_temperature_ballon_max_callback(const float data)
{

    char payload[10];
    dtostrf(data, 4, 1, payload);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMa/state"), payload);
}

void DT_Chauffage_set_temperature_ballon_min(float data)
{
    if (data != eeprom_config.temperature_balon_min)
    {
        eeprom_config.temperature_balon_min = data;
        sauvegardeEEPROM();
        DT_Chauffage_temperature_ballon_min_callback(data);
    }
}

float DT_Chauffage_get_temperature_ballon_min(void)
{
    return eeprom_config.temperature_balon_min;
}

void DT_Chauffage_temperature_ballon_min_callback(const float data)
{
    char payload[10];
    dtostrf(data, 4, 1, payload);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMi/state"), payload);
}

void DT_Chauffage_set_temps_inter_demmarage(uint8_t heures)
{
    if (heures != eeprom_config.temp_inter_demmarage)
    {
        eeprom_config.temp_inter_demmarage = heures;
        sauvegardeEEPROM();
        DT_Chauffage_temps_inter_demmarage_callback(heures);
    }
}

uint8_t DT_Chauffage_get_temps_inter_demmarage(void)
{
    return eeprom_config.temp_inter_demmarage;
}

void DT_Chauffage_temps_inter_demmarage_callback(const uint8_t heures)
{
    char payload[6];
    snprintf(payload, sizeof(payload), "%d", heures);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TID/state"), payload);
}

void DT_Chauffage_set_date_retour_vacance(uint32_t timestemp)
{
    if (timestemp != eeprom_config.date_retour_vacance)
    {
        eeprom_config.date_retour_vacance = timestemp;
        sauvegardeEEPROM();
        DT_Chauffage_date_retour_vacance_callback(timestemp);
    }
}

uint32_t DT_Chauffage_get_date_retour_vacance(void)
{
    return eeprom_config.date_retour_vacance;
}

void DT_Chauffage_date_retour_vacance_callback(const uint32_t timestemp)
{
    char payload[16];
    snprintf(payload, sizeof(payload), "%lu", timestemp);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/DRV/state"), payload);
}

void DT_Chauffage_set_debut_cycle(uint32_t timestemp)
{
    if (timestemp != eeprom_config.heure_debut_cycle)
    {
        eeprom_config.heure_debut_cycle = timestemp;
        sauvegardeEEPROM();
        DT_Chauffage_debut_cycle_callback(timestemp);
    }
}

uint32_t DT_Chauffage_get_debut_cycle(void)
{
    return eeprom_config.heure_debut_cycle;
}

void DT_Chauffage_debut_cycle_callback(const uint32_t timestemp)
{
    char payload[16];
    snprintf(payload, sizeof(payload), "%lu", timestemp);
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/EDC/state"), payload);
}

void DT_Chauffage_set_arret_meteo(bool data)
{
    if (data != mem_config.ha_arret_meteo)
    {
        mem_config.ha_arret_meteo = data;
        async_call_arret_meteo = true;
    }
};

bool DT_Chauffage_get_arret_meteo(void)
{
    return mem_config.ha_arret_meteo;
};

void DT_Chauffage_arret_meteo_callback(const bool data)
{
    const __FlashStringHelper *payload;
    if (data == true)
    {
        payload = F("ON");
    }
    else
    {
        payload = F("OFF");
    }
    DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/AM/state"), payload);
}

// MQTT
bool DT_Chauffage_mqtt_subscribe(MQTTClient &mqtt, uint8_t sequance)
{
    char topic[64];

    if (sequance == 0)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/mode/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 1)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPH/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 2)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPI/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 3)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMa/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 4)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMi/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 5)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TID/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 6)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/DRV/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 7)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/EDC/set");
        mqtt.subscribe(topic);
    }
    else if (sequance == 8)
    {
        snprintf(topic, sizeof(topic), MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/AM/set");
        mqtt.subscribe(topic);
    }
    else
    {
        return false;
    }
    return true;
}

bool __attribute__((optimize("O0"))) DT_Chauffage_mqtt_receve(const char topic[], const char buffer[])
{
    if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/mode/set") == 0)
    {
        if (strcmp(buffer, "Eté") == 0)
        {
            DT_Chauffage_set_mode(DT_CHAUFFAGE_ETE);
        }
        else if (strcmp(buffer, "Inter-saison") == 0)
        {
            DT_Chauffage_set_mode(DT_CHAUFFAGE_BETWEEN);
        }
        else if (strcmp(buffer, "Hiver") == 0)
        {
            DT_Chauffage_set_mode(DT_CHAUFFAGE_WINTER);
        }
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPH/set") == 0)
    {
        float temp = atof(buffer);
        DT_Chauffage_set_temperature_arret_poele_hiver(temp);
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TAPI/set") == 0)
    {
        float temp = atof(buffer);
        DT_Chauffage_set_temperature_arret_poele_intersaison(temp);
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMa/set") == 0)
    {
        float temp = atof(buffer);
        DT_Chauffage_set_temperature_ballon_max(temp);
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TBMi/set") == 0)
    {
        float temp = atof(buffer);
        DT_Chauffage_set_temperature_ballon_min(temp);
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/TID/set") == 0)
    {
        uint8_t heures = (uint8_t)atoi(buffer);
        DT_Chauffage_set_temps_inter_demmarage(heures);
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/DRV/set") == 0)
    {
        uint32_t timestemp = (uint32_t)atol(buffer);
        DT_Chauffage_set_date_retour_vacance(timestemp);
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/EDC/set") == 0)
    {
        uint32_t timestemp = (uint32_t)atol(buffer);
        DT_Chauffage_set_debut_cycle(timestemp);
        return true;
    }
    else if (strcmp(topic, MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/chauffage/AM/set") == 0)
    {
        if (strcmp(buffer, "ON") == 0)
        {
            DT_Chauffage_set_arret_meteo(true);
        }
        else if (strcmp(buffer, "OFF") == 0)
        {
            DT_Chauffage_set_arret_meteo(false);
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool DT_Chauffage_mqtt_publish(uint8_t sequance)
{
    if (sequance == 0)
    {
        DT_Chauffage_mode_callback(DT_Chauffage_get_mode());
        return true;
    }
    else if (sequance == 1)
    {
        DT_Chauffage_temperature_arret_poele_hiver_callback(DT_Chauffage_get_temperature_arret_poele_hiver());
        return true;
    }
    else if (sequance == 2)
    {
        DT_Chauffage_temperature_arret_poele_intersaison_callback(DT_Chauffage_get_temperature_arret_poele_intersaison());
        return true;
    }
    else if (sequance == 3)
    {
        DT_Chauffage_temperature_ballon_max_callback(DT_Chauffage_get_temperature_ballon_max());
        return true;
    }
    else if (sequance == 4)
    {
        DT_Chauffage_temperature_ballon_min_callback(DT_Chauffage_get_temperature_ballon_min());
        return true;
    }
    else if (sequance == 5)
    {
        DT_Chauffage_temps_inter_demmarage_callback(DT_Chauffage_get_temps_inter_demmarage());
        return true;
    }
    else if (sequance == 6)
    {
        DT_Chauffage_date_retour_vacance_callback(DT_Chauffage_get_date_retour_vacance());
        return true;
    }
    else if (sequance == 7)
    {
        DT_Chauffage_debut_cycle_callback(DT_Chauffage_get_debut_cycle());
        return true;
    }
    else if (sequance == 8)
    {
        DT_Chauffage_arret_meteo_callback(DT_Chauffage_get_arret_meteo());
        return true;
    }
    else
    {
        return false;
    }
}

#endif // CHAUFFAGE