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
                eeprom_config.poele_mode = DT_POELE_ARRET;
                eeprom_config.mode_3voies_PCBT = DT_3VOIES_OFF;
                eeprom_config.mode_3voies_MCBT = DT_3VOIES_OFF;

                eeprom_config.V1 = 60; // consigne poêle en mode force (70°C)
                eeprom_config.V2 = 2; // Variable Reserve chaleur Ballon (20°C)
                eeprom_config.V3 = 0;  // Variable Temp Demi plage Morte
                eeprom_config.C4 = 60; // consigne Jacuzzi
                eeprom_config.C5 = 55; // consigne ECS1 & ECS2
                eeprom_config.C7 = 90; // Valeur renvoyer au poele pour le mode silance (Fake NTC) //TODO: plus utilisée ?
#ifdef VANNES
                eeprom_config.C8 = 29;                       // consigne Temp PCBT a -10°C
                eeprom_config.C9 = 21;                       // consigne Temp PCBT a +10°C
                eeprom_config.C10 = 48;                      // consigne Temp MCBT a -10°C
                eeprom_config.C11 = 32;                      // consigne Temp MCBT a +10°C
                eeprom_config.C_PCBT_MIN = 17;               // consigne Temp PCBT minimum
                eeprom_config.C_PCBT_MAX = TMP_EAU_PCBT_MAX; // consigne Temp PCBT maximum
                eeprom_config.C_MCBT_MIN = 18;               // consigne Temp MCBT minimum
                eeprom_config.C_MCBT_MAX = TMP_EAU_MCBT_MAX; // consigne Temp MCBT maximum
#endif                                                       // VANNES
        }

        // Valeurs par défaut struct_version == 2
        if (eeprom_config.struct_version < 2 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 2"));
                eeprom_config.struct_version = 2;

                eeprom_config.pid_pcbt.KP = 1300;
                eeprom_config.pid_pcbt.KI = 0;
                eeprom_config.pid_pcbt.KD = 100000;
                eeprom_config.pid_pcbt.KT = 20000;
                eeprom_config.pid_pcbt.action = QuickPID::Action::direct;
                eeprom_config.pid_pcbt.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_pcbt.dmode = QuickPID::dMode::dOnMeas;
                eeprom_config.pid_pcbt.iawmode = QuickPID::iAwMode::iAwCondition;

                eeprom_config.pid_mcbt.KP = 500;
                eeprom_config.pid_mcbt.KI = 0;
                eeprom_config.pid_mcbt.KD = 100000;
                eeprom_config.pid_mcbt.KT = 20000;
                eeprom_config.pid_mcbt.action = QuickPID::Action::direct;
                eeprom_config.pid_mcbt.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_mcbt.dmode = QuickPID::dMode::dOnMeas;
                eeprom_config.pid_mcbt.iawmode = QuickPID::iAwMode::iAwCondition;
        }

        // Valeurs par défaut struct_version == 3
        if (eeprom_config.struct_version < 3 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 3"));
                eeprom_config.struct_version = 3;

                eeprom_config.ratio_PCBT = 1;
                eeprom_config.ratio_MCBT = 1;
        }

        // Valeurs par défaut struct_version == 4
        if (eeprom_config.struct_version < 4 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 4"));
                eeprom_config.struct_version = 4;

                eeprom_config.out_inhib_PCBT = 400;
                eeprom_config.out_inhib_MCBT = 400;
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

#if DIMMER_LIGHT_NUM > 0
                for (uint8_t num; num < DIMMER_LIGHT_NUM; ++num)
                {
                        if (num < 12)
                        {
                                eeprom_config.Dimmer_scale_min[num] = 17000; // Mise a l echelle
                                eeprom_config.Dimmer_scale_max[num] = 7000; // Mise a l echelle
                        }
                        else
                        {
                                eeprom_config.Dimmer_scale_min[num] = 255; // Mise a l echelle
                                eeprom_config.Dimmer_scale_max[num] = 156; // Mise a l echelle
                        }
                }
#endif

#if COVER_NUM > 0
                for (uint8_t num; num < COVER_NUM; ++num)
                {
                        eeprom_config.cover[num].time_down = 300; // ratio temp / 100 a la descente
                        eeprom_config.cover[num].time_up = 300;   // ratio temp / 100 a la monté
                }
#endif

#if RADIATOR_NUM > 0
                for (uint8_t num; num < RADIATOR_NUM; ++num)
                {
                        eeprom_config.radiator[num].cycle = 600000; // temp de cycle en ms
                        eeprom_config.radiator[num].KI = 0;         // coeficient d integral
                        eeprom_config.radiator[num].m10 = 80;       // temp de fonctionnement a -10°c, en pourcentage
                        eeprom_config.radiator[num].p10 = 20;       // temp de fonctionnement a +10°c, en pourcentage
                }
#endif
        }

        // Sauvegarde les nouvelles données
        if (need_save)
                sauvegardeEEPROM();
};
