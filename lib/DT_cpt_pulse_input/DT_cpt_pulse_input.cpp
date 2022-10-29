#include <DT_cpt_pulse_input.h>
#include <DT_input.h>
#include "Arduino.h"
#include "config.h"

uint32_t counter[CPT_PULSE_INPUT];
bool cpt_front[CPT_PULSE_INPUT];

void (*_cpt_pulse_input_callback)(const uint8_t num, const uint32_t counter) = nullptr;

void DT_cpt_pulse_input_init()
{
    _cpt_pulse_input_callback = nullptr;
    for (uint8_t i = 0; i < CPT_PULSE_INPUT; ++i)
    {
        counter[i] = 0;
    }
};

void DT_cpt_pulse_input_loop()
{
    bool value = false;
    for (uint8_t i = 0; i < CPT_PULSE_INPUT; ++i)
    {
        if (cpt_front[i] == false && DT_input_get_pin_stats(pgm_read_byte(CPT_PULSE_INPUT_ARRAY + i) == true))
        {
            cpt_front[i] = true;
            counter[i] += 1;
            if (_cpt_pulse_input_callback != nullptr)
            {
                _cpt_pulse_input_callback(i,counter[i]);
            }
        }
        else if (cpt_front[i] == true && DT_input_get_pin_stats(pgm_read_byte(CPT_PULSE_INPUT_ARRAY + i) == false))
        {
            cpt_front[i] = false;
        }
    }
}

void DT_cpt_pulse_input_set_callback(void (*callback)(const uint8_t num, const uint32_t counter))
{
    _cpt_pulse_input_callback = callback;
}

uint32_t DT_cpt_pulse_input_get(uint8_t num)
{
    return counter[num];
}