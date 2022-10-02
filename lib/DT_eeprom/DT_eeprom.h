#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include "../lib/DT_poele/DT_poele.h"
#include "../lib/DT_3voies/DT_3voies.h"

#include <QuickPID.h>
#include <config.h>

static const uint8_t STRUCT_MAGIC = 2;
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
    // uint32_t backup_pos; // ms per percent
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

// declaration de la structure de configuration
struct Mem_Config
{
    float C2;            // consigne Temp PCBT
    float C3;            // consigne MCBT
    bool MQTT_online;    // connecte au broker MQTT
    bool HA_online;      // Home Assistant
    bool HA_MQTT_CONFIG; // Home Assistant MQTT configuration
#if DIMMER_LIGHT_NUM >= 1
    uint8_t Dimmer_old_value[DIMMER_LIGHT_NUM]; // Mise a l echelle
#endif
};
struct Eeprom_Config
{
    uint8_t magic;
    uint8_t struct_version;

    DT_Poele_mode poele_mode;
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
    uint16_t out_offset_PCBT;   // en ms
    uint16_t out_offset_MCBT;   // en ms
    float in_offset_PCBT;       // en °c
    float in_offset_MCBT;       // en °c
    uint8_t in_offset_avg_temp; // en °C

#if DIMMER_LIGHT_NUM >= 1
    uint16_t Dimmer_scale_min[DIMMER_LIGHT_NUM]; // Mise a l echelle
    uint16_t Dimmer_scale_max[DIMMER_LIGHT_NUM]; // Mise a l echelle
#endif

#if COVER_NUM >= 1
    Cover cover[COVER_NUM];
#endif

#if RADIATOR_NUM > 0
    Radiator radiator[RADIATOR_NUM];
#endif
};

// structure de configuration
extern Eeprom_Config eeprom_config;
extern Mem_Config mem_config;

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM(void);

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM(void);
#endif