#include <DT_poele.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
// #include <DT_fake_ntc.h>

#include <QuickPID.h>
#include <DT_eeprom.h>

#include <config.h>

void (*poele_mode_callback)(const DT_Poele_mode mode);
void (*poele_C1_callback)(const uint8_t C1);
// void (*poele_T4_callback)(const float t4);

// bool ev1; // 0(Circuit ballon tampon + Ballon ECS) / 1(Circuit Ballon ECS)
// float T4; // Temperature envoyé au poêle
float C1; // consigne temp Ballon
// T1 = Temp Ballon					T2 = Temp ECS							T3 = Temp ECS2
// T5 = Temp Extérieur					T6 = Temp Vanne 3V PCBT					T7 = Temp Vanne 3V MCBT				T8 = Temp Vanne 3V Jacuzzi

uint32_t temp_default_pt100 = 0;

void DT_Poele_init()
{
    DT_relay(RELAY_EV1, false);
    C1 = 0;
    poele_mode_callback = nullptr;
    poele_C1_callback = nullptr;
    // poele_T4_callback = nullptr;
}

bool marche_poele_ballon(uint32_t now)
{
    static bool ret = false;
    // mise en marche du poele
    if (DT_pt100_get(PT100_H_BALON) > 0 && DT_pt100_get(PT100_B_BALON) > 0)
    {
        if (((DT_pt100_get(PT100_H_BALON) + DT_pt100_get(PT100_B_BALON)) / 2) < (C1))
        {
            ret = true;
        }
        else if (((DT_pt100_get(PT100_H_BALON) + DT_pt100_get(PT100_B_BALON)) / 2) > (C1 + eeprom_config.C7))
        {
            ret = false;
        }
        temp_default_pt100 = 0;
    }
    else if (temp_default_pt100 != 0 && now - temp_default_pt100 >= TEMP_DEFAULT_PT100_POELE)
    {

        if (DT_pt100_get(PT100_H_BALON) > 0 && DT_pt100_get(PT100_H_BALON) < (C1))
        {
            ret = true;
        }
        else if (DT_pt100_get(PT100_B_BALON) > 0 && DT_pt100_get(PT100_B_BALON) < (C1))
        {
            ret = true;
        }
        else if (DT_pt100_get(PT100_H_BALON) > 0 && DT_pt100_get(PT100_H_BALON) > (C1 + eeprom_config.C7))
        {
            ret = false;
        }
        else if (DT_pt100_get(PT100_B_BALON) > 0 && DT_pt100_get(PT100_B_BALON) > (C1 + eeprom_config.C7))
        {
            ret = false;
        }
        else
        {
            ret = false;
        }
    }
    else if (temp_default_pt100 == 0)
    {
        temp_default_pt100 = now;
    }

    return ret;
}

void DT_Poele_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;

    if (now - old >= 1000)
    {
        old = now;
        float old_C1 = C1;
        // bool old_ev1 = ev1;
        if (eeprom_config.poele_mode == DT_POELE_NORMAL)
        {
            // mode ECS + Chauffage
            DT_relay(RELAY_EV1, false);
            // calcule consigne balon
            C1 = max(mem_config.C2, mem_config.C3);
            C1 = max(C1, eeprom_config.C4);
            C1 += eeprom_config.V2;
            if (C1 > MAX_C1)
            {
                C1 = MAX_C1;
            }
            // marche poele
            DT_relay(MARCHE_POELE, marche_poele_ballon(now));
        }
        else if (eeprom_config.poele_mode == DT_POELE_ARRET)
        {
            // mode ECS + Chauffage
            DT_relay(RELAY_EV1, false);
            DT_relay(MARCHE_POELE, false);
        }
        // else if (eeprom_config.poele_mode == DT_POELE_SECOURS)
        // {
        //     // mode ECS + Chauffage
        //     DT_relay(RELAY_EV1, false);
        //     // temperature envoyer au poele

        //     // marche poele
        //     DT_relay(MARCHE_POELE, marche_poele_ballon(now));
        // }
        else if (eeprom_config.poele_mode == DT_POELE_ECS)
        {
            // mode ECS uniquement
            DT_relay(RELAY_EV1, true);
            // temperature envoyer au poele

            if (DT_pt100_get(PT100_ECS1) > 0 && DT_pt100_get(PT100_ECS2) > 0)
            {
                // marche poele
                C1 = eeprom_config.C5;
                if (min(DT_pt100_get(PT100_ECS1), DT_pt100_get(PT100_ECS2)) < (C1))
                {
                    DT_relay(MARCHE_POELE, true);
                }
                else if (min(DT_pt100_get(PT100_ECS1), DT_pt100_get(PT100_ECS2)) > (C1 + eeprom_config.C7))
                {
                    DT_relay(MARCHE_POELE, false);
                }
                temp_default_pt100 = 0;
            }
            else if (DT_pt100_get(PT100_ECS1) > 0)
            {
                C1 = eeprom_config.C5;
                if (DT_pt100_get(PT100_ECS1) < (C1))
                {
                    DT_relay(MARCHE_POELE, true);
                }
                else if (DT_pt100_get(PT100_ECS1) > (C1 + eeprom_config.C7))
                {
                    DT_relay(MARCHE_POELE, false);
                }
                temp_default_pt100 = 0;
            }
            else if (DT_pt100_get(PT100_ECS2) > 0)
            {
                C1 = eeprom_config.C5;
                if (DT_pt100_get(PT100_ECS2) < (C1))
                {
                    DT_relay(MARCHE_POELE, true);
                }
                else if (DT_pt100_get(PT100_ECS2) > (C1 + eeprom_config.C7))
                {
                    DT_relay(MARCHE_POELE, false);
                }
                temp_default_pt100 = 0;
            }
            else if (temp_default_pt100 != 0 && now - temp_default_pt100 >= TEMP_DEFAULT_PT100_POELE)
            {
                // marche poele
                DT_relay(MARCHE_POELE, false);
            }
            else if (temp_default_pt100 == 0)
            {
                temp_default_pt100 = now;
            }
        }
        else if (eeprom_config.poele_mode == DT_POELE_BOOST)
        {
            // mode ECS uniquement
            DT_relay(RELAY_EV1, false);
            // temperature envoyer au poele
            C1 = eeprom_config.C6;
            DT_relay(MARCHE_POELE, marche_poele_ballon(now));
        }

        // securité
        if (DT_pt100_get(PT100_B_BALON) > 85)
        {
            // temperature envoyer au poele
            DT_relay(MARCHE_POELE, false);
        }
        else if (DT_pt100_get(PT100_H_BALON) > 85)
        {
            // temperature envoyer au poele
            DT_relay(MARCHE_POELE, false);
        }
        else if (DT_pt100_get(PT100_ECS1) > 85)
        {
            // temperature envoyer au poele
            DT_relay(MARCHE_POELE, false);
        }
        else if (DT_pt100_get(PT100_ECS2) > 85)
        {
            // temperature envoyer au poele
            DT_relay(MARCHE_POELE, false);
        }

        if (poele_C1_callback != nullptr && old_C1 != C1)
        {
            poele_C1_callback(C1);
        }
    }
}

void DT_Poele_set_mode(DT_Poele_mode mode)
{
    if (mode != eeprom_config.poele_mode)
    {
        eeprom_config.poele_mode = mode;
        if (poele_mode_callback != nullptr)
            poele_mode_callback(mode);
    }
    // sauvegardeEEPROM();
}

DT_Poele_mode DT_Poele_get_mode(void)
{
    return eeprom_config.poele_mode;
}

void DT_Poele_set_mode_callback(void (*callback)(const DT_Poele_mode mode))
{
    poele_mode_callback = callback;
}

void DT_Poele_set_C1_callback(void (*callback)(const uint8_t C1))
{
    poele_C1_callback = callback;
}

void DT_Poele_set_C1(const float c1)
{
    C1 = c1;
}

float DT_Poele_get_C1()
{
    return C1;
}
