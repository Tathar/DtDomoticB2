#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include "../lib/DT_poele/DT_poele.h"
#include <DT_3voies.h>

static const uint8_t STRUCT_MAGIC = 1;
static const uint8_t STRUCT_VERSION = 1;

// declaration de la structure de configuration
typedef struct Config
{
    uint8_t magic;
    uint8_t struct_version;

    DT_Poele_mode poele_mode;
    DT_3voies_mode mode_3voies;

    uint8_t V1; // Variable paramètre poêle (60°C)
    uint8_t V2; // Variable Reserve chaleur Ballon (20°C)
    uint8_t V3; // Variable Temp Demi plage Morte
    float C2;   // consigne Temp PCBT
    float C3;   // consigne MCBT
    float C4;   // consigne Jacuzzi
    uint8_t C5; // consigne ECS1 & ECS2
    uint8_t C6; // consigne mode boost
    uint8_t C7; // consigne Mode Silence
    float C8;  // consigne Temp PCBT a -10°C
    float C9;  // consigne Temp PCBT a +10°C
    float C10; // consigne Temp MCBT a -10°C
    float C11; // consigne Temp MCBT a +10°C

} Config_t;

// structure de configuration
extern Config config;

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM(void);

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM(void);
#endif