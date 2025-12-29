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
        }

        // Valeurs par défaut struct_version == 2
        if (eeprom_config.struct_version < 2 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 2"));
                eeprom_config.struct_version = 2;

        }

        // Valeurs par défaut struct_version == 3
        if (eeprom_config.struct_version < 3 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 3"));
                eeprom_config.struct_version = 3;
        }

        // Valeurs par défaut struct_version == 4
        if (eeprom_config.struct_version < 4 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 4"));
                eeprom_config.struct_version = 4;
        }

        // Valeurs par défaut struct_version == 5
        if (eeprom_config.struct_version < 5 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 5"));
                eeprom_config.struct_version = 5;
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
                                eeprom_config.Dimmer_scale_max[num] = 7000;  // Mise a l echelle
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
        if (eeprom_config.struct_version < 7 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 7"));
                eeprom_config.struct_version = 7;

        }
        if (eeprom_config.struct_version < 8 || erreur)
        {
                need_save = true;
                Serial.println(F("EEPROM version < 8"));
                eeprom_config.struct_version = 8;

#ifdef DT_3VOIES_PCBT_RAPH
                eeprom_config.SetPoint_manual_3voies_PCBT_raph = 0;
                eeprom_config.SetPoint_auto_1_3voies_PCBT_raph = 29;            // consigne Temp PCBT a -10°C
                eeprom_config.SetPoint_auto_2_3voies_PCBT_raph = 21;            // consigne Temp PCBT a +10°C
                eeprom_config.SetPoint_3voies_min_PCBT_raph = 40;               // consigne Temp PCBT minimum
                eeprom_config.SetPoint_3voies_max_PCBT_raph = TMP_EAU_PCBT_MAX; // consigne Temp PCBT maximum
                eeprom_config.pid_3voies_PCBT_raph.KP = 1300;
                eeprom_config.pid_3voies_PCBT_raph.KI = 0;
                eeprom_config.pid_3voies_PCBT_raph.KD = 100000;
                eeprom_config.pid_3voies_PCBT_raph.KT = 20000;
                eeprom_config.pid_3voies_PCBT_raph.action = QuickPID::Action::direct;
                eeprom_config.pid_3voies_PCBT_raph.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_3voies_PCBT_raph.dmode = QuickPID::dMode::dOnMeas;
                eeprom_config.pid_3voies_PCBT_raph.iawmode = QuickPID::iAwMode::iAwCondition;
                eeprom_config.in_offset_3voies_PCBT_raph = 0;
                eeprom_config.out_inhib_3voies_PCBT_raph = 400;
                eeprom_config.mode_3voies_PCBT_raph = DT_3voies_PCBT_raph_OFF;
#endif
#ifdef DT_3VOIES_MCBT_RAPH
                eeprom_config.SetPoint_manual_3voies_MCBT_raph = 0;
                eeprom_config.SetPoint_auto_1_3voies_MCBT_raph = 48;            // consigne Temp MCBT a -10°C
                eeprom_config.SetPoint_auto_2_3voies_MCBT_raph = 32;            // consigne Temp MCBT a +10°C
                eeprom_config.SetPoint_3voies_min_MCBT_raph = 22;               // consigne Temp MCBT minimum
                eeprom_config.SetPoint_3voies_max_MCBT_raph = TMP_EAU_MCBT_MAX; // consigne Temp MCBT maximum
                eeprom_config.pid_3voies_MCBT_raph.KP = 500;
                eeprom_config.pid_3voies_MCBT_raph.KI = 0;
                eeprom_config.pid_3voies_MCBT_raph.KD = 0;
                eeprom_config.pid_3voies_MCBT_raph.KT = 60000;
                eeprom_config.pid_3voies_MCBT_raph.action = QuickPID::Action::direct;
                eeprom_config.pid_3voies_MCBT_raph.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_3voies_MCBT_raph.dmode = QuickPID::dMode::dOnMeas;
                eeprom_config.pid_3voies_MCBT_raph.iawmode = QuickPID::iAwMode::iAwCondition;
                eeprom_config.out_inhib_3voies_MCBT_raph = 400;
                eeprom_config.in_offset_3voies_MCBT_raph = 0;
                eeprom_config.mode_3voies_MCBT_raph = DT_3voies_MCBT_raph_OFF;
#endif
#ifdef POELE
                eeprom_config.poele_mode = DT_POELE_OFF;
                eeprom_config.V1 = 60; // consigne poêle en mode force (70°C)
                eeprom_config.V2 = 2;  // Variable Reserve chaleur Ballon (20°C)
                eeprom_config.V3 = 0;  // Variable Temp Demi plage Morte
                eeprom_config.C4 = 48; // consigne Jacuzzi
                eeprom_config.C7 = 90; // Reserve arret poele
#endif
#ifdef DT_3VOIES_1_NATH
                eeprom_config.SetPoint_manual_3voies_1_nath = 25;
                eeprom_config.SetPoint_auto_1_3voies_1_nath = 60;            // consigne Temp MCBT a -10°C
                eeprom_config.SetPoint_auto_2_3voies_1_nath = 35;            // consigne Temp MCBT a +10°C
                eeprom_config.SetPoint_3voies_min_1_nath = 20;               // consigne Temp MCBT minimum
                eeprom_config.SetPoint_3voies_max_1_nath = DT_3VOIES_1_NATH_MAX_TMP_EAU; // consigne Temp MCBT maximum
                eeprom_config.pid_3voies_1_nath.KP = 600;
                eeprom_config.pid_3voies_1_nath.KI = 0;
                eeprom_config.pid_3voies_1_nath.KD = 0;
                eeprom_config.pid_3voies_1_nath.KT = 400000;
                eeprom_config.pid_3voies_1_nath.action = QuickPID::Action::direct;
                eeprom_config.pid_3voies_1_nath.pmode = QuickPID::pMode::pOnError;
                eeprom_config.pid_3voies_1_nath.dmode = QuickPID::dMode::dOnMeas;
                eeprom_config.pid_3voies_1_nath.iawmode = QuickPID::iAwMode::iAwCondition;
                eeprom_config.out_inhib_3voies_1_nath = 400;
                eeprom_config.in_offset_3voies_1_nath = 0;
                eeprom_config.mode_3voies_1_nath = DT_3voies_1_nath_OFF;
#endif
        }

        // Sauvegarde les nouvelles données
        if (need_save)
                sauvegardeEEPROM();
};
