#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <config.h>
#include <RTClib.h>

#include "../lib/DT_poele/DT_poele.h"
#include "../lib/DT_chauffage/DT_3voies_nath.h"
#include "../lib/DT_3voies/DT_3voies.h"
#include "../lib/DT_chauffage/DT_chauffage.h"
// #include "../lib/DT_clock/DT_clock.h"

#include <QuickPID.h>

static const uint8_t STRUCT_MAGIC = 9;
static const uint8_t STRUCT_VERSION = 1;

struct Pid
{
    float KP;    // pid KP
    float KI;    // pid KI
    float KD;    // pid KD
    uint32_t KT; // pid interval (en ms)
    QuickPID::Action action;
    QuickPID::pMode pmode;
    QuickPID::dMode dmode;
    QuickPID::iAwMode iawmode;
};

struct Cover
{
    uint16_t time_up;   // ms per percent
    uint16_t time_down; // ms per percent
    uint8_t backup_pos; // in percente
};

enum Radiator_Mode
{
    Radiator_Mode_Off,
    Radiator_Mode_Heating,
    Radiator_Mode_Cooling,
    Radiator_Mode_Drying,
    Radiator_Mode_Idle,
    Radiator_Mode_Fan
};

struct Radiator
{
    Radiator_Mode mode; // mode de fonctionnement de raditeur
    float m10;          // temp de fonctionnement a -10°c, en pourcentage
    float p10;          // temp de fonctionnement a +10°c, en pourcentage
    float KI;           // coeficient d integral
    float consigne;     // consigne
    uint32_t cycle;     //  temp de cycle en ms
};

enum __attribute__((__packed__)) DT_ECS_mode
{
    DT_ECS_ARRET,
    DT_ECS_MARCHE,
    DT_ECS_STANDBY,
};

// declaration de la structure de configuration
struct Mem_Config
{
    float C2;                       // consigne Temp PCBT
    float C3;                       // consigne MCBT
    bool MQTT_online = false;       // connecte au broker MQTT
    bool HA_online = false;         // Home Assistant
    bool ha_mqtt_config = false;    // Home Assistant MQTT configuration
    bool ha_mqtt_publish = false;   // Home Assistant MQTT configuration
    bool ha_mqtt_subscribe = false; // Home Assistant MQTT configuration

#ifdef CHAUFFAGE
    bool ha_arret_meteo = false;
#endif // CHAUFFAGE
#if DIMMER_LIGHT_NUM >= 1
    uint8_t Dimmer_old_value[DIMMER_LIGHT_NUM]; // Mise a l echelle
#endif
#ifdef CLOCK
    DateTime started; // date de demmarage
#endif
};
struct Eeprom_Config
{
    uint8_t magic;
    uint8_t struct_version;

    #ifdef POELE
    DT_Poele_mode poele_mode;
    #endif
    #ifdef VANNES
    DT_3voies_mode mode_3voies_PCBT;
    DT_3voies_mode mode_3voies_MCBT;

    uint8_t V1; // consigne poêle mode force (70°C)
    uint8_t V2; // Variable Reserve chaleur Ballon (20°C)
    float V3;   // Variable Temp Demi plage Morte
    // float C2;   // consigne Temp PCBT
    // float C3;   // consigne MCBT
    float C4;         // consigne Jacuzzi
    uint8_t C5;       // consigne ECS1 & ECS2
    uint8_t C6;       // consigne mode boost
    int8_t C7;        // Bande Morte Poele
    float C8;         // consigne Temp PCBT a -10°C
    float C9;         // consigne Temp PCBT a +10°C
    float C10;        // consigne Temp MCBT a -10°C
    float C11;        // consigne Temp MCBT a +10°C
    float C_PCBT_MIN; // consigne Temp PCBT minimum
    float C_PCBT_MAX; // consigne Temp PCBT maximum
    float C_MCBT_MIN; // consigne Temp MCBT minimum
    float C_MCBT_MAX; // consigne Temp MCBT maximum

    // float KP_PCBT;    // pid KP PCBT
    // float KI_PCBT;    // pid KI PCBT
    // float KD_PCBT;    // pid KD PCBT
    // uint32_t KT_PCBT; // pid interval PCBT (en ms)
    Pid pid_pcbt;

    // float KP_MCBT;    // pid KP MCBT
    // float KI_MCBT;    // pid KI MCBT
    // float KD_MCBT;    // pid KD MCBT
    // uint32_t KT_MCBT; // pid interval MCBT (en ms)
    Pid pid_mcbt;

    float ratio_PCBT;
    float ratio_MCBT;
    int16_t out_inhib_PCBT; // en ms
    int16_t out_inhib_MCBT; // en ms
    float in_offset_PCBT;   // en °c
    float in_offset_MCBT;   // en °c
    // uint8_t in_offset_avg_temp; // en °C //fourchette pour la quelle on utilise la temperature exterieur reel (non moyené)
    #endif //VANNE

#ifdef DT_PT100_EXT
    uint8_t in_offset_avg_temp_sup; // en °C //fourchette pour la quelle on utilise la temperature exterieur reel (non moyené)
    uint8_t in_offset_avg_temp_inf; // en °C //fourchette pour la quelle on utilise la temperature exterieur reel (non moyené)
#endif

#ifdef DT_3VOIES_1_NATH
    float SetPoint_manual_3voies_1_nath;
    DT_3voies_1_nath_mode mode_3voies_1_nath;
    float SetPoint_auto_1_3voies_1_nath;        // consigne Temp a -10°C
    float SetPoint_auto_2_3voies_1_nath;        // consigne Temp a +10°C
    float SetPoint_3voies_1_nath_min; // consigne Temp minimum
    float SetPoint_3voies_1_nath_max; // consigne Temp maximum
    Pid pid_3voies_1_nath;
    float ratio_3voies_1_nath;
    float in_offset_3voies_1_nath;   // en °c
    int16_t out_inhib_3voies_1_nath; // en ms
#endif // DT_3VOIES_1_NATH

#if DIMMER_LIGHT_NUM >= 1
    uint16_t Dimmer_scale_min[DIMMER_LIGHT_NUM]; // Mise a l echelle
    uint16_t Dimmer_scale_max[DIMMER_LIGHT_NUM]; // Mise a l echelle
#endif                                           // DIMMER_LIGHT_NUM

#ifdef RELAY_ECS1
    DT_ECS_mode ecs1_mode; // ecs1 mode
#endif

#if RELAY_ECS2
    DT_ECS_mode ecs2_mode; // ecs2 mode
#endif

#if COVER_NUM >= 1
    Cover cover[COVER_NUM];
#endif

#if RADIATOR_NUM > 0
    Radiator radiator[RADIATOR_NUM];
#endif

#ifdef WATCHDOG_TIME
    char debug_str[64];
#endif

#ifdef CHAUFFAGE
    DT_Chauffage_mode chauffage_mode;
    float temperature_arret_poele_hiver;
    float temperature_arret_poele_intersaison;
    float temperature_balon_max;
    float temperature_balon_min;
    uint8_t temp_inter_demmarage;
    uint32_t date_retour_vacance;

#endif // #CHAUFFAGE
};

// structure de configuration
extern Eeprom_Config eeprom_config;
extern Mem_Config mem_config;
// extern volatile Eeprom_Config eeprom_config;
// extern volatile Mem_Config mem_config;

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM(void);

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM(void);
#endif