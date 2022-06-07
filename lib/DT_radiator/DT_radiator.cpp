
#include "Arduino.h"
#include <DT_eeprom.h>
#include <DT_PT100.h>
#include <DT_Dimmer.h>
#include <DT_relay.h>

#if RADIATOR_NUM > 0

#define SCALE(val, in_min, in_max, out_min, out_max) (((double)val - (double)in_min) * ((double)out_max - (double)out_min) / ((double)in_max - (double)in_min)) + out_min

struct DT_radiator
{
    uint32_t start_cycle;
    float cumule;
};

DT_radiator radiator[RADIATOR_NUM];
void (*_callback_radiator)(const uint8_t num, const float out, const float I);

void DT_radiator_init()
{
    for (uint8_t num = 0; num < RADIATOR_NUM; ++num)
    {
        radiator[num].start_cycle = 0;
        radiator[num].cumule = 0;
    }
    _callback_radiator = nullptr;
};

void DT_radiator_loop()
{
    uint32_t now = millis();
    static uint8_t num = 0;
    // for (uint8_t num = 0; num < RADIATOR_NUM; ++num)
    if (num < RADIATOR_NUM)
    {
        if (now - radiator[num].start_cycle >= eeprom_config.radiator[num].cycle)
        {
            radiator[num].start_cycle = now;
            float percent = SCALE(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.radiator[num].m10, eeprom_config.radiator[num].m10);

            // calcule de l integral
            if (eeprom_config.radiator[num].KI != 0)
            {
                uint8_t pt100;
                if (num < DIMMER_RADIATOR_NUM)
                {
                    pt100 = pgm_read_byte(DIMMER_RADIATOR_PT100_ARRAY + num);
                }
                else
                {
                    pt100 = pgm_read_byte(RELAY_RADIATOR_PT100_ARRAY + (num - DIMMER_RADIATOR_NUM));
                }

                radiator[num].cumule += eeprom_config.radiator[num].consigne - DT_pt100_get(pt100);
                percent += radiator[num].cumule * eeprom_config.radiator[num].KI;
                if (_callback_radiator != nullptr)
                    _callback_radiator(num, percent, radiator[num].cumule * eeprom_config.radiator[num].KI);
            }
            else
            {
                radiator[num].cumule = 0;
                if (_callback_radiator != nullptr)
                    _callback_radiator(num, percent, 0);
            }

            if (num < DIMMER_RADIATOR_NUM)
            {
                DT_dimmer_relay(DIMMER_RADIATOR_FIRST_NUM + num, (uint32_t)SCALE(percent, 0, 100, 0, eeprom_config.radiator[num].cycle));
            }
            else
            {
                DT_relay(RELAY_RADIATOR_FIRST_NUM + (num - DIMMER_RADIATOR_NUM), (uint32_t)SCALE(percent, 0, 100, 0, eeprom_config.radiator[num].cycle));
            }
        }
    }

    if (++num >= RADIATOR_NUM)
    {
        num = 0;
    }
};

void DT_radiator_set_consigne(uint8_t num, float consigne)
{
    eeprom_config.radiator[num].consigne = consigne;
};

void DT_radiator_set_cycle(uint8_t num, uint32_t cycle)
{
    eeprom_config.radiator[num].cycle = cycle; // temp de cycle en ms
};

void DT_radiator_set_M10(uint8_t num, float m10)
{
    eeprom_config.radiator[num].m10 = m10; // temp de fonctionnement a -10°c, en pourcentage
};

void DT_radiator_set_P10(uint8_t num, float p10)
{
    eeprom_config.radiator[num].p10 = p10; // temp de fonctionnement a +10°c, en pourcentage
};

void DT_radiator_set_KI(uint8_t num, float KI)
{
    eeprom_config.radiator[num].KI = KI; // coeficient d integral
};

void DT_radiator_set_callback(void (*callback)(const uint8_t num, const float out, const float I) = nullptr)
{
    _callback_radiator = callback;
};

#endif