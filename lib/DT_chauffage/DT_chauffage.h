#ifndef DT_CHAUFFAGE
#define DT_CHAUFFAGE
#include <Arduino.h>


struct MQTTClient;

enum __attribute__((__packed__)) DT_Chauffage_mode
{
    DT_CHAUFFAGE_ETE,
    DT_CHAUFFAGE_BETWEEN,
    DT_CHAUFFAGE_WINTER,
};

void DT_Chauffage_init();
void DT_Chauffage_loop();

void DT_Chauffage_set_mode(DT_Chauffage_mode mode);
DT_Chauffage_mode DT_Chauffage_get_mode(void);
void DT_Chauffage_mode_callback(const DT_Chauffage_mode mode);

void DT_Chauffage_set_temperature_arret_poele_hiver(float data);
float DT_Chauffage_get_temperature_arret_poele_hiver(void);
void DT_Chauffage_temperature_arret_poele_hiver_callback(const float data);

void DT_Chauffage_set_temperature_arret_poele_intersaison(float data);
float DT_Chauffage_get_temperature_arret_poele_intersaison(void);
void DT_Chauffage_temperature_arret_poele_intersaison_callback(const float data);

void DT_Chauffage_set_temperature_between_start(float data);
float DT_Chauffage_get_temperature_between_start(void);
void DT_Chauffage_temperature_between_start_callback(const float data);

void DT_Chauffage_set_temperature_ballon_max(float data);
float DT_Chauffage_get_temperature_ballon_max(void);
void DT_Chauffage_temperature_ballon_max_callback(const float data);

void DT_Chauffage_set_temperature_ballon_min(float data);
float DT_Chauffage_get_temperature_ballon_min(void);
void DT_Chauffage_temperature_ballon_min_callback(const float data);

void DT_Chauffage_set_temps_inter_demmarage(uint32_t timestemp);
uint8_t DT_Chauffage_get_temps_inter_demmarage(void);
void DT_Chauffage_temps_inter_demmarage_callback(const uint32_t timestemp);

void DT_Chauffage_set_date_retour_vacance(uint32_t timestemp);
uint32_t DT_Chauffage_get_date_retour_vacance(void);
void DT_Chauffage_date_retour_vacance_callback(const uint32_t timestemp);

void DT_Chauffage_set_debut_cycle(uint32_t timestemp);
uint32_t DT_Chauffage_get_debut_cycle(void);
void DT_Chauffage_debut_cycle_callback(const uint32_t timestemp);

void DT_Chauffage_set_arret_meteo(bool data);
bool DT_Chauffage_get_arret_meteo(void);
void DT_Chauffage_arret_meteo_callback(const bool data);

//MQTT
bool DT_Chauffage_mqtt_subscribe(MQTTClient &mqtt, uint8_t sequance);
bool DT_Chauffage_mqtt_receve(const char topic[], const char buffer[]);
bool DT_Chauffage_mqtt_publish(uint8_t sequance);

#endif