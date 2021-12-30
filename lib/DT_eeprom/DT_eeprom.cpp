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
                // TODO: mqtt home assistant
                eeprom_config.poele_mode = DT_POELE_OFF;
                eeprom_config.mode_3voies_PCBT = DT_3VOIES_OFF;
                eeprom_config.mode_3voies_MCBT = DT_3VOIES_OFF;

                eeprom_config.V1 = 60;                       // Variable paramètre poêle (60°C)
                eeprom_config.V2 = 20;                       // Variable Reserve chaleur Ballon (20°C)
                eeprom_config.V3 = 0;                        // Variable Temp Demi plage Morte
                eeprom_config.C4 = 80;                       // consigne Jacuzzi
                eeprom_config.C5 = 60;                       // consigne ECS1 & ECS2
                eeprom_config.C7 = 90;                       // Valeur renvoyer au poele pour le mode silance (Fake NTC)
                eeprom_config.C8 = 35;                       // consigne Temp PCBT a -10°C
                eeprom_config.C9 = 20;                       // consigne Temp PCBT a +10°C
                eeprom_config.C10 = 50;                      // consigne Temp MCBT a -10°C
                eeprom_config.C11 = 35;                      // consigne Temp MCBT a +10°C
                eeprom_config.C_PCBT_MIN = 20;               // consigne Temp PCBT minimum
                eeprom_config.C_PCBT_MAX = TMP_EAU_PCBT_MAX; // consigne Temp PCBT maximum
                eeprom_config.C_MCBT_MIN = 25;               // consigne Temp MCBT minimum
                eeprom_config.C_MCBT_MAX = TMP_EAU_MCBT_MAX; // consigne Temp MCBT maximum

                // eeprom_config.KP_PCBT = 500;  // pid KP MCBT
                // eeprom_config.KI_PCBT = 0;    // pid KI MCBT
                // eeprom_config.KD_PCBT = 0;    // pid KD MCBT
                // eeprom_config.KT_PCBT = 1000; // pid interval MCBT (en ms)

                // eeprom_config.KP_MCBT = 500;  // pid KP MCBT
                // eeprom_config.KI_MCBT = 0;    // pid KI MCBT
                // eeprom_config.KD_MCBT = 0;    // pid KD MCBT
                // eeprom_config.KT_MCBT = 1000; // pid interval MCBT (en ms)
        }

        // Valeurs par défaut struct_version == 2
        if (eeprom_config.struct_version < 2 || erreur)
        {
                eeprom_config.struct_version = 2;
                // TODO: mqtt home assistant
                eeprom_config.pid_pcbt.KP = 1000;
                eeprom_config.pid_pcbt.KI = 0;
                eeprom_config.pid_pcbt.KD = 0;
                eeprom_config.pid_pcbt.KT = 10000;
                eeprom_config.pid_pcbt.action = QuickPID::Action::direct;
                eeprom_config.pid_pcbt.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_pcbt.dmode = QuickPID::dMode::dOnError;
                eeprom_config.pid_pcbt.iawmode = QuickPID::iAwMode::iAwCondition;

                eeprom_config.pid_mcbt.KP = 1000;
                eeprom_config.pid_mcbt.KI = 0;
                eeprom_config.pid_mcbt.KD = 0;
                eeprom_config.pid_mcbt.KT = 10000;
                eeprom_config.pid_mcbt.action = QuickPID::Action::direct;
                eeprom_config.pid_mcbt.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_mcbt.dmode = QuickPID::dMode::dOnError;
                eeprom_config.pid_mcbt.iawmode = QuickPID::iAwMode::iAwCondition;
        }

        // Sauvegarde les nouvelles données
        sauvegardeEEPROM();
};
