#include "DT_eeprom.h"
#include "config.h"
#include <EEPROM.h>

// declaration de la structure de configuration
Eeprom_Config eeprom_config;
Mem_Config mem_config;

/** Sauvegarde en mémoire EEPROM le contenu actuel de la structure */
void sauvegardeEEPROM()
{
        Serial.println(F("Save on EEPROM"));
        // Met à jour le nombre magic et le numéro de version avant l'écriture
        // eeprom_config.magic = STRUCT_MAGIC;
        // eeprom_config.struct_version = STRUCT_VERSION;
        EEPROM.put(0, eeprom_config);
}

/** Charge le contenu de la mémoire EEPROM dans la structure */
void chargeEEPROM()
{
        // uint8_t i = 0; // for loop
        bool need_save = false;
        // Lit la mémoire EEPROM
        EEPROM.get(0, eeprom_config);

        // Détection d'une mémoire non initialisée
        byte erreur = eeprom_config.magic != STRUCT_MAGIC;
        if (erreur)
        {
                Serial.println(F("EEPROM error"));
                eeprom_config.magic = STRUCT_MAGIC;
        }

        // Valeurs par défaut struct_version == 1
        if (eeprom_config.struct_version < 1 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 1"));
                eeprom_config.struct_version = 1;
                // TODO: mqtt home assistant
                eeprom_config.poele_mode = DT_POELE_ARRET;
                eeprom_config.mode_3voies_PCBT = DT_3VOIES_OFF;
                eeprom_config.mode_3voies_MCBT = DT_3VOIES_OFF;

                eeprom_config.V1 = 70; // consigne poêle en mode force (70°C)
                eeprom_config.V2 = 20; // Variable Reserve chaleur Ballon (20°C)
                eeprom_config.V3 = 0;  // Variable Temp Demi plage Morte
                eeprom_config.C4 = 80; // consigne Jacuzzi
                eeprom_config.C5 = 60; // consigne ECS1 & ECS2
                eeprom_config.C7 = 90; // Valeur renvoyer au poele pour le mode silance (Fake NTC)
#ifdef VANNES
                eeprom_config.C8 = 35;                       // consigne Temp PCBT a -10°C
                eeprom_config.C9 = 20;                       // consigne Temp PCBT a +10°C
                eeprom_config.C10 = 50;                      // consigne Temp MCBT a -10°C
                eeprom_config.C11 = 35;                      // consigne Temp MCBT a +10°C
                eeprom_config.C_PCBT_MIN = 20;               // consigne Temp PCBT minimum
                eeprom_config.C_PCBT_MAX = TMP_EAU_PCBT_MAX; // consigne Temp PCBT maximum
                eeprom_config.C_MCBT_MIN = 25;               // consigne Temp MCBT minimum
                eeprom_config.C_MCBT_MAX = TMP_EAU_MCBT_MAX; // consigne Temp MCBT maximum
#endif                                                       // VANNES
        }

        // Valeurs par défaut struct_version == 2
        if (eeprom_config.struct_version < 2 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 2"));
                eeprom_config.struct_version = 2;

                eeprom_config.pid_pcbt.KP = 2000;
                eeprom_config.pid_pcbt.KI = 4;
                eeprom_config.pid_pcbt.KD = 4;
                eeprom_config.pid_pcbt.KT = 10000;
                eeprom_config.pid_pcbt.action = QuickPID::Action::direct;
                eeprom_config.pid_pcbt.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_pcbt.dmode = QuickPID::dMode::dOnError;
                eeprom_config.pid_pcbt.iawmode = QuickPID::iAwMode::iAwCondition;

                eeprom_config.pid_mcbt.KP = 2000;
                eeprom_config.pid_mcbt.KI = 4;
                eeprom_config.pid_mcbt.KD = 4;
                eeprom_config.pid_mcbt.KT = 10000;
                eeprom_config.pid_mcbt.action = QuickPID::Action::direct;
                eeprom_config.pid_mcbt.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_mcbt.dmode = QuickPID::dMode::dOnError;
                eeprom_config.pid_mcbt.iawmode = QuickPID::iAwMode::iAwCondition;
        }

        // Valeurs par défaut struct_version == 3
        if (eeprom_config.struct_version < 3 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 3"));
                eeprom_config.struct_version = 3;

                eeprom_config.ratio_PCBT = 4;
                eeprom_config.ratio_MCBT = 4;
        }

        // Valeurs par défaut struct_version == 4
        if (eeprom_config.struct_version < 4 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 4"));
                eeprom_config.struct_version = 4;

                eeprom_config.out_offset_PCBT = 0;
                eeprom_config.out_offset_MCBT = 0;
        }

        // Valeurs par défaut struct_version == 5
        if (eeprom_config.struct_version < 5 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 5"));
                eeprom_config.struct_version = 5;

                eeprom_config.in_offset_PCBT = 0;
                eeprom_config.in_offset_MCBT = 0;
        }

        // Valeurs par défaut struct_version == 6
        if (eeprom_config.struct_version < 6 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 6"));
                eeprom_config.struct_version = 6;

#if DIMMER_NUM >= 1
                for (uint8_t num; num < DIMMER_NUM; ++num)
                {
                        if (num < 13)
                        {
                                eeprom_config.Dimmer_scale_min[num] = 100;   // Mise a l echelle
                                eeprom_config.Dimmer_scale_max[num] = 15000; // Mise a l echelle
                        }
                        else
                        {
                                eeprom_config.Dimmer_scale_min[num] = 10;  // Mise a l echelle
                                eeprom_config.Dimmer_scale_max[num] = 200; // Mise a l echelle
                        }
                }
#endif
        }

        // Sauvegarde les nouvelles données
        if (need_save)
                sauvegardeEEPROM();
};
