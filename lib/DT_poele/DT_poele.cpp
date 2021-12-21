#include <DT_poele.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <DT_fake_ntc.h>

#include <DT_eeprom.h>

#include <config.h>

void (*poele_callback)(const uint8_t C1);

//bool ev1; // 0(Circuit ballon tampon + Ballon ECS) / 1(Circuit Ballon ECS)
float T4; // Temperature envoyé au poêle
float C1; // consigne temp Ballon
// T1 = Temp Ballon					T2 = Temp ECS							T3 = Temp ECS2
// T5 = Temp Extérieur					T6 = Temp Vanne 3V PCBT					T7 = Temp Vanne 3V MCBT				T8 = Temp Vanne 3V Jacuzzi

void DT_Poele_init()
{
    DT_relay(RELAY_EV1, true);
    C1 = 0;
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
            DT_relay(RELAY_EV1, true);
            // calcule consigne balon
            C1 = max(mem_config.C2, mem_config.C3);
            C1 = max(C1, eeprom_config.C4);
            C1 += eeprom_config.V2;
            if (C1 > MAX_C1)
            {
                C1 = MAX_C1;
            }
            // temperature envoyer au poele
            T4 = (eeprom_config.V1 + DT_pt100_get(PT100_BALON)) - C1;
            // Serial.print("C1 = ");
            // Serial.println(C1);
            // Serial.print("C2 = ");
            // Serial.println(mem_config.C2);
            // Serial.print("C3 = ");
            // Serial.println(mem_config.C3);
            // Serial.print("C4 = ");
            // Serial.println(eeprom_config.C4);
            // Serial.print("V1 = ");
            // Serial.println(eeprom_config.V1);
            // Serial.print("V2 = ");
            // Serial.println(eeprom_config.V2);
            // Serial.print("T4 = ");
            // Serial.println(T4);
        }
        else if (eeprom_config.poele_mode == DT_POELE_SILENCE)
        {
            // mode ECS + Chauffage
            DT_relay(RELAY_EV1, true);
            T4 = eeprom_config.C7;
        }
        else if (eeprom_config.poele_mode == DT_POELE_SECOURS)
        {
            // mode ECS + Chauffage
            DT_relay(RELAY_EV1, true);
            // temperature envoyer au poele
            T4 = DT_pt100_get(PT100_BALON);
        }
        else if (eeprom_config.poele_mode == DT_POELE_ECS)
        {
            // mode ECS uniquement
            DT_relay(RELAY_EV1, false);
            // temperature envoyer au poele
            float _min = min(DT_pt100_get(PT100_ECS1), DT_pt100_get(PT100_ECS2));
            T4 = eeprom_config.V1 - eeprom_config.V2 + _min - eeprom_config.C5;
        }
        else if (eeprom_config.poele_mode == DT_POELE_BOOST)
        {
            // mode ECS uniquement
            DT_relay(RELAY_EV1, true);
            // temperature envoyer au poele
            T4 = eeprom_config.V1 + DT_pt100_get(PT100_BALON) - eeprom_config.C6;
        }

        // securité
        if (DT_pt100_get(PT100_BALON) > 85)
        {
            // temperature envoyer au poele
            T4 = DT_pt100_get(PT100_BALON);
        }

        if (T4 < MIN_T4)
            T4 = MIN_T4;

        if (eeprom_config.poele_mode != DT_POELE_MANUAL)
        {
            DT_fake_ntc_set(T4);
        }

        if (poele_callback != nullptr && old_C1 != C1)
        {
            poele_callback(C1);
        }
    }
}

void DT_Poele_set_mode(DT_Poele_mode mode)
{
    eeprom_config.poele_mode = mode;
    // sauvegardeEEPROM();
}

DT_Poele_mode DT_Poele_get_mode(void)
{
    return eeprom_config.poele_mode;
}

void DT_Poele_set_callback(void (*callback)(const uint8_t C1))
{
    poele_callback = callback;
}

void DT_Poele_set_C1(const float c1)
{
    C1 = c1;
}
