#include "DT_eeprom.h"
#include "config.h"
#include <EEPROM.h>

// declaration de la structure de configuration
Config config;

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM()
{
        // Met à jour le nombre magic et le numéro de version avant l'écriture
        config.magic = STRUCT_MAGIC;
        config.struct_version = STRUCT_VERSION;
        EEPROM.put(0, config);
}

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM()
{
        // uint8_t i = 0; // for loop

        // Lit la mémoire EEPROM
        EEPROM.get(0, config);

        // Détection d'une mémoire non initialisée
        byte erreur = config.magic != STRUCT_MAGIC;

        // Valeurs par défaut struct_version == 1
        if (config.struct_version < 1 || erreur)
        {
                config.struct_version = 1;
                //TODO: mqtt home assistant
                config.poele_mode = DT_POELE_OFF;
                config.mode_3voies = DT_3VOIES_OFF;

                config.V1 = 60; // Variable paramètre poêle (60°C)
                config.V2 = 20; // Variable Reserve chaleur Ballon (20°C)
                config.V3 = 0;  // Variable Temp Demi plage Morte
                config.C2 = 30; // consigne Temp PCBT
                config.C3 = 60; // consigne MCBT
                config.C4 = 80; // consigne Jacuzzi
                config.C5 = 60; // consigne ECS1 & ECS2
                config.C7 = 0;  // Variable Temp Demi plage Morte
        }

        // Sauvegarde les nouvelles données
        sauvegardeEEPROM();
};
