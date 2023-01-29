#include <DT_cpt_pulse_input.h>
#include <DT_input.h>
#include "Arduino.h"
#include "config.h"

#if CPT_PULSE_INPUT > 0
uint32_t counter[CPT_PULSE_INPUT];
uint8_t cpt_front[CPT_PULSE_INPUT];

void (*_cpt_pulse_input_callback)(const uint8_t num, const uint32_t counter) = nullptr;
#endif // CPT_PULSE_INPUT > 0

void DT_cpt_pulse_input_init()
{
#if CPT_PULSE_INPUT > 0
    _cpt_pulse_input_callback = nullptr;
    for (uint8_t num = 0; num < CPT_PULSE_INPUT; ++num)
    {
        counter[num] = 0;
        cpt_front[num] = LOW;
    }
#endif // CPT_PULSE_INPUT > 0
};

void DT_cpt_pulse_input_loop()
{
#if CPT_PULSE_INPUT > 0
    for (uint8_t num = 0; num < CPT_PULSE_INPUT; ++num)
    {
        if (cpt_front[num] == LOW && DT_input_get_pin_stats(pgm_read_byte(CPT_PULSE_INPUT_ARRAY + num)) == HIGH)
        {
            cpt_front[num] = HIGH;
            counter[num] += 1;
            // Serial.print(F("cpt_pulse_input_callback("));
            // Serial.print(i);
            // Serial.print(F(", "));
            // Serial.print(counter[num]);
            // Serial.println(F(")"));
            if (_cpt_pulse_input_callback != nullptr)
            {
                _cpt_pulse_input_callback(num, counter[num]);
            }
        }
        else if (cpt_front[num] == HIGH && DT_input_get_pin_stats(pgm_read_byte(CPT_PULSE_INPUT_ARRAY + num)) == LOW)
        {
            // Serial.print("cpt_pulse_input front off");
            cpt_front[num] = LOW;
        }
    }
#endif // CPT_PULSE_INPUT > 0
}

void DT_cpt_pulse_input_reset(const uint8_t num)
{
#if CPT_PULSE_INPUT > 0
    counter[num] = 0;
    if (_cpt_pulse_input_callback != nullptr)
    {
        _cpt_pulse_input_callback(num, counter[num]);
    }
#endif // CPT_PULSE_INPUT > 0
}

void DT_cpt_pulse_input_set_callback(void (*callback)(const uint8_t num, const uint32_t counter))
{
#if CPT_PULSE_INPUT > 0
    _cpt_pulse_input_callback = callback;
#endif // CPT_PULSE_INPUT > 0
}

uint32_t DT_cpt_pulse_input_get(uint8_t num)
{
#if CPT_PULSE_INPUT > 0
    return counter[num];
#else
    return 0;
#endif // CPT_PULSE_INPUT > 0
}