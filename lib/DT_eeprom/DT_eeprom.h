#ifndef EEPROM_CONFIG_H
#define EEPROM_CONFIG_H

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <DT_poele.h>

static const uint8_t STRUCT_MAGIC = 1;
static const uint8_t STRUCT_VERSION = 1;

// declaration de la structure de configuration
typedef struct Config
{
    uint8_t magic;
    uint8_t struct_version;

    DT_Poele_mode poele_mode;

    uint8_t V1; // Variable paramètre poêle (60°C)
    uint8_t V2; // Variable Reserve chaleur Ballon (20°C)
    uint8_t V3; // Variable Temp Demi plage Morte

} Config_t;

// structure de configuration
extern Config config;

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM(void);

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM(void);
#endif