#ifndef DT_CHAUFFAGE
#define DT_CHAUFFAGE
#include <Arduino.h>

enum __attribute__((__packed__)) DT_Chauffage_mode
{
    DT_CHAUFFAGE_ETE,
    DT_CHAUFFAGE_INTERSAISON,
    DT_CHAUFFAGE_HIVER,
};

void DT_Chauffage_init();
void DT_Chauffage_loop();

void DT_Chauffage_set_mode(DT_Chauffage_mode mode);
DT_Chauffage_mode DT_Chauffage_get_mode(void);
void DT_Chauffage_set_mode_callback(void (*mode_callback)(const DT_Chauffage_mode mode));

void DT_Chauffage_set_temperature_arret_poele_hiver(float data);
float DT_Chauffage_get_temperature_arret_poele_hiver(void);
void DT_Chauffage_set_temperature_arret_poele_hiver_callback(void (*callback)(const float data));

void DT_Chauffage_set_arret_meteo(bool data);
bool DT_Chauffage_get_arret_meteo(void);
void DT_Chauffage_set_arret_meteo_callback(void (*callback)(const bool data));

#endif