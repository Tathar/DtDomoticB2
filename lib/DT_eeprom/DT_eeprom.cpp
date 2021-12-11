#include "DT_eeprom.h"
#include "config.h"
#include <EEPROM.h>

// declaration de la structure de configuration
Eeprom_Config eeprom_config;
Mem_Config mem_config;

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM()
{
        // Met à jour le nombre magic et le numéro de version avant l'écriture
        eeprom_config.magic = STRUCT_MAGIC;
        eeprom_config.struct_version = STRUCT_VERSION;
        EEPROM.put(0, eeprom_config);
}

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM()
{
        // uint8_t i = 0; // for loop

        // Lit la mémoire EEPROM
        EEPROM.get(0, eeprom_config);

        // Détection d'une mémoire non initialisée
        byte erreur = eeprom_config.magic != STRUCT_MAGIC;

        // Valeurs par défaut struct_version == 1
        if (eeprom_config.struct_version < 1 || erreur)
        {
                eeprom_config.struct_version = 1;
                //TODO: mqtt home assistant
                eeprom_config.poele_mode = DT_POELE_OFF;
                eeprom_config.mode_3voies = DT_3VOIES_OFF;

                eeprom_config.V1 = 60; // Variable paramètre poêle (60°C)
                eeprom_config.V2 = 20; // Variable Reserve chaleur Ballon (20°C)
                eeprom_config.V3 = 0;  // Variable Temp Demi plage Morte
                // eeprom_config.C2 = 30; // consigne Temp PCBT
                // eeprom_config.C3 = 60; // consigne MCBT
                eeprom_config.C4 = 80;  // consigne Jacuzzi
                eeprom_config.C5 = 60;  // consigne ECS1 & ECS2
                eeprom_config.C7 = 0;   // Variable Temp Demi plage Morte
                eeprom_config.C8 = 35;  // consigne Temp PCBT a -10°C
                eeprom_config.C9 = 20;  // consigne Temp PCBT a +10°C
                eeprom_config.C10 = 50; // consigne Temp MCBT a -10°C
                eeprom_config.C11 = 35; // consigne Temp MCBT a +10°C
        }

        // Sauvegarde les nouvelles données
        sauvegardeEEPROM();
};
