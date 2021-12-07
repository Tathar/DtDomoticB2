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
uint8_t T4;
// consigne temp Ballon
float C1;
// consigne Temp PCBT
float C2;
// consigne MCBT
float C3;
// consigne Jacuzzi
float C4;
// consigne ECS1 & ECS2
float C5;
// consigne mode boost
float C6;
// consigne Mode Silence
float C7;
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
    ev1 = false;
    C1 = 60;
}
void DT_Poele_loop()
{
    if (config.poele_mode == DT_POELE_NORMAL)
    {
        // mode ECS + Chauffage
        ev1 = false;
        // calcule consigne balon
        C1 = max(C2, C3);
        C1 = max(C1, C4);
        C1 += config.V2;
        // temperature envoyer au poele
        T4 = config.V1 + DT_pt100_get(PT100_BALON) - C1;
    }

    if (DT_pt100_get(PT100_BALON) > 85)
    {
        T4 = T1;
    }

    DT_fake_ntc_set(T4);
}
void DT_Poele_set_mode(DT_Poele_mode mode)
{
    config.poele_mode = mode;
}
