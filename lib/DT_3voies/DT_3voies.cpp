#include <DT_3voies.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>

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

void DT_3voies_init()
{
    ev1 = true;
    C1 = 60;
}

void DT_3voies_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;

    if (now - old >= 1000)
    {
        old = now;
    }
}

void DT_3voies_set_mode(DT_3voies_mode mode)
{
    config.mode_3voies = mode;
}

DT_3voies_mode DT_3voies_get_mode(void)
{
    return config.mode_3voies;
}
