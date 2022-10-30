#include <DT_cpt_pulse_input.h>
#include <DT_input.h>
#include "Arduino.h"
#include "config.h"

uint32_t counter[CPT_PULSE_INPUT];
uint8_t cpt_front[CPT_PULSE_INPUT];

void (*_cpt_pulse_input_callback)(const uint8_t num, const uint32_t counter) = nullptr;

void DT_cpt_pulse_input_init()
{
    _cpt_pulse_input_callback = nullptr;
    for (uint8_t i = 0; i < CPT_PULSE_INPUT; ++i)
    {
        counter[i] = 0;
        cpt_front[i] = LOW;
    }
};

void DT_cpt_pulse_input_loop()
{
    for (uint8_t i = 0; i < CPT_PULSE_INPUT; ++i)
    {
        if (cpt_front[i] == LOW && DT_input_get_pin_stats(pgm_read_byte(CPT_PULSE_INPUT_ARRAY + i)) == HIGH)
        {
            cpt_front[i] = HIGH;
            counter[i] += 1;
            // Serial.print(F("cpt_pulse_input_callback("));
            // Serial.print(i);
            // Serial.print(F(", "));
            // Serial.print(counter[i]);
            // Serial.println(F(")"));
            if (_cpt_pulse_input_callback != nullptr)
            {
                _cpt_pulse_input_callback(i, counter[i]);
            }
        }
        else if (cpt_front[i] == HIGH && DT_input_get_pin_stats(pgm_read_byte(CPT_PULSE_INPUT_ARRAY + i)) == LOW)
        {
            // Serial.print("cpt_pulse_input front off");
            cpt_front[i] = LOW;
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