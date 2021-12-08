#include <DT_poele.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <DT_fake_ntc.h>

#include <DT_eeprom.h>

#include <config.h>

// 0(Circuit ballon tampon + Ballon ECS) / 1(Circuit Ballon ECS)
bool ev1;
// Temp envoyé au poêle
float T4;
// consigne temp Ballon
float C1;
// consigne Temp PCBT
float C2;
// consigne MCBT
float C3;
// consigne Jacuzzi
float C4;
// consigne Temp PCBT a -10°C
float C8;
// consigne Temp PCBT a +10°C
float C9;
// consigne Temp MCBT a -10°C
float C10;
// consigne Temp MCBT a +10°C
float C11;

// T1 = Temp Ballon					T2 = Temp ECS							T3 = Temp ECS2
// T5 = Temp Extérieur					T6 = Temp Vanne 3V PCBT					T7 = Temp Vanne 3V MCBT				T8 = Temp Vanne 3V Jacuzzi

void DT_Poele_init()
{
    ev1 = true;
    C1 = 60;
}

void DT_Poele_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;

    if (now - old >= 1000)
    {
        old = now;
        if (config.poele_mode == DT_POELE_NORMAL)
        {
            // mode ECS + Chauffage
            ev1 = true;
            // calcule consigne balon
            C1 = max(C2, C3);
            C1 = max(C1, C4);
            C1 += config.V2;
            // temperature envoyer au poele
            T4 = config.V1 + DT_pt100_get(PT100_BALON) - C1;
        }
        else if (config.poele_mode == DT_POELE_SILENCE)
        {
            // mode ECS + Chauffage
            ev1 = true;
            T4 = config.C7;
        }
        else if (config.poele_mode == DT_POELE_SECOURS)
        {
            // mode ECS + Chauffage
            ev1 = true;
            // temperature envoyer au poele
            T4 = DT_pt100_get(PT100_BALON);
        }
        else if (config.poele_mode == DT_POELE_ECS)
        {
            // mode ECS uniquement
            ev1 = false;
            // temperature envoyer au poele
            float _min = min(DT_pt100_get(PT100_ECS1), DT_pt100_get(PT100_ECS2));
            T4 = config.V1 - config.V2 + _min - config.C5;
        }
        else if (config.poele_mode == DT_POELE_BOOST)
        {
            // mode ECS uniquement
            ev1 = true;
            // temperature envoyer au poele
            T4 = config.V1 + DT_pt100_get(PT100_BALON) - config.C6;
        }

        // securité
        if (DT_pt100_get(PT100_BALON) > 85)
        {

            // temperature envoyer au poele
            T4 = DT_pt100_get(PT100_BALON);
        }

        DT_fake_ntc_set(T4);
    }
}

void DT_Poele_set_mode(DT_Poele_mode mode)
{
    config.poele_mode = mode;
}

DT_Poele_mode DT_Poele_get_mode(void)
{
    return config.poele_mode;
}
