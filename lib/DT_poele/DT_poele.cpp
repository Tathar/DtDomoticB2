#include <DT_poele.h>
// #include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
// #include <DT_fake_ntc.h>

#include <QuickPID.h>
#include <DT_eeprom.h>

#include <config.h>
#ifdef POELE
void (*poele_mode_callback)(const DT_Poele_mode mode);
bool async_call_poele_mode;
// void (*poele_C1_callback)(const uint8_t C1);
//  void (*poele_T4_callback)(const float t4);

// bool ev1; // 0(Circuit ballon tampon + Ballon ECS) / 1(Circuit Ballon ECS)
// float T4; // Temperature envoyé au poêle
// float C1; // consigne temp Ballon
// T1 = Temp Ballon					T2 = Temp ECS							T3 = Temp ECS2
// T5 = Temp Extérieur					T6 = Temp Vanne 3V PCBT					T7 = Temp Vanne 3V MCBT				T8 = Temp Vanne 3V Jacuzzi

uint32_t temp_default_pt100_B = 0;
uint32_t temp_default_pt100_ECS = 0;

void DT_Poele_init()
{
    DT_relay(RELAY_EV1, false);
    poele_mode_callback = nullptr;
}

// decision de mise en service du poele en fonction des temperature
bool marche_poele_ballon_normal(bool actuel, uint32_t now)
{
    // mise en marche du poele en fonction de la consigne MCBT
    if (DT_pt100_get(PT100_H_BALON) > 0)
    {
        if (DT_pt100_get(PT100_H_BALON) < (mem_config.C3 + eeprom_config.V2)) // temp haut balon  < consigne MCBT + reserve temp
        {
            actuel = true;
        }
    }

    // mise en marche du poele en fonction de la consigne PCBT
    if (DT_pt100_get(PT100_M_BALON) > 0)
    {
        if (DT_pt100_get(PT100_M_BALON) < (mem_config.C2 + eeprom_config.V2)) // temp milieu balon  < consigne PCBT + reserve temp
        {
            actuel = true;
        }
    }

    // arret du poele en fonction de la temperature
    if (DT_pt100_get(PT100_B_BALON) > 0)
    {
        if (DT_pt100_get(PT100_B_BALON) > (max(mem_config.C2, mem_config.C3) + eeprom_config.C7)) // temp bas balon > maximum consigne PCBT ou consigne MCBT  + bande morte Poele
        {
            actuel = false;
        }
        temp_default_pt100_B = now;
    }
    else if (now - temp_default_pt100_B > TEMPS_DEFAULT_PT100_POELE)
    {
        actuel = false;
    }

    return actuel;
}

void DT_Poele_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;
    bool poele = DT_relay_get(MARCHE_POELE);
    static bool ev1 = false;

    if (now - old >= 1000)
    {
        // 220502  debug(F(AT));
        old = now;
        if (eeprom_config.poele_mode == DT_POELE_NORMAL)
        {
            // mode ECS + Chauffage
            ev1 = false;
            // marche poele
            poele = marche_poele_ballon_normal(poele, now);
        }
        else if (eeprom_config.poele_mode == DT_POELE_ARRET)
        {
            // mode ECS + Chauffage
            ev1 = false;
            poele = false;
        }
        else if (eeprom_config.poele_mode == DT_POELE_ECS) // mode ECS uniquement
        {
            ev1 = true;
            // temperature envoyer au poele

            if (DT_pt100_get(PT100_ECS1) > 0 && DT_pt100_get(PT100_ECS2) > 0)
            {
                // marche poele
                if (min(DT_pt100_get(PT100_ECS1), DT_pt100_get(PT100_ECS2)) < (eeprom_config.C5))
                {
                    poele = true;
                }
                else if (min(DT_pt100_get(PT100_ECS1), DT_pt100_get(PT100_ECS2)) > eeprom_config.C5)
                {
                    poele = true;
                    // eeprom_config.poele_mode = DT_POELE_NORMAL;
                    DT_Poele_set_mode(DT_POELE_NORMAL);
                }
                temp_default_pt100_ECS = now;
            }
            else if (DT_pt100_get(PT100_ECS1) > 0)
            {
                if (DT_pt100_get(PT100_ECS1) < (eeprom_config.C5))
                {
                    poele = true;
                }
                else if (DT_pt100_get(PT100_ECS1) > eeprom_config.C5)
                {
                    poele = true;
                    // eeprom_config.poele_mode = DT_POELE_NORMAL;
                    DT_Poele_set_mode(DT_POELE_NORMAL);
                }
                temp_default_pt100_ECS = now;
            }
            else if (DT_pt100_get(PT100_ECS2) > 0)
            {
                if (DT_pt100_get(PT100_ECS2) < (eeprom_config.C5))
                {
                    poele = true;
                }
                else if (DT_pt100_get(PT100_ECS2) > eeprom_config.C5)
                {
                    poele = true;
                    // eeprom_config.poele_mode = DT_POELE_NORMAL;
                    DT_Poele_set_mode(DT_POELE_NORMAL);
                }
                temp_default_pt100_ECS = now;
            }
            else if (now - temp_default_pt100_ECS >= TEMPS_DEFAULT_PT100_POELE)
            {
                // marche poele
                poele = true;
                DT_Poele_set_mode(DT_POELE_NORMAL);
            }
        }
        else if (eeprom_config.poele_mode == DT_POELE_FORCE)
        {
            // mode ECS uniquement
            ev1 = false;
            if (eeprom_config.V1 > POELE_MAX_TEMPERATURE)
            {
                eeprom_config.V1 = POELE_MAX_TEMPERATURE;
            }
            // marche / arret du poele en fonction de la temperature bas
            if (DT_pt100_get(PT100_B_BALON) > 0)
            {
                if (DT_pt100_get(PT100_B_BALON) < eeprom_config.V1) // temp bas balon < consigne mode force
                {
                    poele = true;
                }
                else if (DT_pt100_get(PT100_B_BALON) >= eeprom_config.V1) // temp bas balon > consigne mode force
                {
                    poele = false;
                    // eeprom_config.poele_mode = DT_POELE_NORMAL;
                    DT_Poele_set_mode(DT_POELE_NORMAL);
                }
                temp_default_pt100_B = now;
            }
            else if (now - temp_default_pt100_B > TEMPS_DEFAULT_PT100_POELE)
            {
                poele = false;
                DT_Poele_set_mode(DT_POELE_NORMAL);
            }
        }

        // securité
        if (DT_pt100_get(PT100_H_BALON) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }
        else if (DT_pt100_get(PT100_M_BALON) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }
        else if (DT_pt100_get(PT100_B_BALON) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }
        else if (DT_pt100_get(PT100_ECS1) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }
        else if (DT_pt100_get(PT100_ECS2) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }

        DT_relay(MARCHE_POELE, poele);
        DT_relay(RELAY_EV1, ev1);

        // 220502  debug(F(AT));
    }

    if (poele_mode_callback != nullptr && async_call_poele_mode == true)
    {
        async_call_poele_mode = false;
        poele_mode_callback(eeprom_config.poele_mode);
    }
}

void DT_Poele_set_mode(DT_Poele_mode mode)
{
    if (mode != eeprom_config.poele_mode)
    {
        eeprom_config.poele_mode = mode;
        async_call_poele_mode = true;
    }
    sauvegardeEEPROM();
}

DT_Poele_mode DT_Poele_get_mode(void)
{
    return eeprom_config.poele_mode;
}

void DT_Poele_set_mode_callback(void (*callback)(const DT_Poele_mode mode))
{
    poele_mode_callback = callback;
}

#endif // POELE