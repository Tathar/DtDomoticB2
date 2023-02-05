#include <DT_cpt_pulse_input.h>
#include <DT_relay.h>
#include "Arduino.h"
#include "config.h"

#if CPT_PULSE_INPUT > 0
uint32_t counter[CPT_PULSE_INPUT];
void (*_cpt_pulse_input_callback)(const uint8_t num, const uint32_t counter) = nullptr;
#endif // CPT_PULSE_INPUT > 0

#if CPT_PULSE_INPUT_IF_OUT > 0
uint32_t counter_if_out[CPT_PULSE_INPUT_IF_OUT * 2];
void (*_cpt_pulse_input_if_out_callback)(const uint8_t num, const uint32_t counter, const bool cond) = nullptr;
#endif // CPT_PULSE_INPUT_IF_OUT > 0

#if CPT_PULSE_INPUT_IF_IN > 0
uint32_t counter_if_in[CPT_PULSE_INPUT_IF_IN * 2];
void (*_cpt_pulse_input_if_in_callback)(const uint8_t num, const uint32_t counter, const bool cond) = nullptr;
#endif // CPT_PULSE_INPUT_IF_IN > 0

void DT_cpt_pulse_input_init()
{

#if CPT_PULSE_INPUT > 0
    _cpt_pulse_input_callback = nullptr;
    for (uint8_t num = 0; num < CPT_PULSE_INPUT; ++num)
    {
        counter[num] = 0;
    }
#endif // CPT_PULSE_INPUT > 0

#if CPT_PULSE_INPUT_IF_OUT > 0
    _cpt_pulse_input_if_out_callback = nullptr;
    for (uint8_t num = 0; num < CPT_PULSE_INPUT_IF_OUT * 2; ++num)
    {
        counter_if_out[num] = 0;
    }
#endif // CPT_PULSE_INPUT_IF_OUT > 0

#if CPT_PULSE_INPUT_IF_IN > 0
    _cpt_pulse_input_if_in_callback = nullptr;
    for (uint8_t num = 0; num < CPT_PULSE_INPUT_IF_IN * 2; ++num)
    {
        counter_if_in[num] = 0;
    }
#endif // CPT_PULSE_INPUT_IF_IN > 0
};

void DT_cpt_pulse_input_loop_event(const uint8_t btn, const Bt_Action action)
{
    uint8_t btn_cycle;
#if CPT_PULSE_INPUT > 0
    for (uint8_t num = 0; num < CPT_PULSE_INPUT; ++num)
    {
        btn_cycle = pgm_read_byte(CPT_PULSE_INPUT_ARRAY + num);
        if (btn == btn_cycle && action == IN_PUSHED)
        {
            counter[num] += 1;
            if (_cpt_pulse_input_callback != nullptr)
            {
                _cpt_pulse_input_callback(num, counter[num]);
            }
        }
    }
#endif // CPT_PULSE_INPUT > 0

#if CPT_PULSE_INPUT_IF_OUT > 0
    // cpt pulse input if
    for (uint8_t num = 0; num < CPT_PULSE_INPUT_IF_OUT; ++num)
    {
        btn_cycle = pgm_read_byte(CPT_PULSE_INPUT_IF_OUT_ARRAY + num);
        if (btn == btn_cycle && action == IN_PUSHED)
        {
            bool relay = DT_relay_get(pgm_read_byte(CPT_PULSE_INPUT_IF_OUT_COND_ARRAY + num));
            if (relay)
            {
                counter_if_out[num * 2] += 1;
                if (_cpt_pulse_input_if_out_callback != nullptr)
                {
                    _cpt_pulse_input_if_out_callback(num, counter_if_out[num * 2], true);
                }
            }
            else
            {
                counter_if_out[(num * 2) + 1] += 1;
                if (_cpt_pulse_input_if_out_callback != nullptr)
                {
                    _cpt_pulse_input_if_out_callback(num, counter_if_out[(num * 2) + 1], false);
                }
            }
        }
    }
#endif // CPT_PULSE_INPUT_IF_OUT > 0

#if CPT_PULSE_INPUT_IF_IN > 0
    // cpt pulse input if
    for (uint8_t num = 0; num < CPT_PULSE_INPUT_IF_IN; ++num)
    {
        btn_cycle = pgm_read_byte(CPT_PULSE_INPUT_IF_IN_ARRAY + num);
        if (btn == btn_cycle && action == IN_PUSHED)
        {
            bool relay = DT_relay_get(pgm_read_byte(CPT_PULSE_INPUT_IF_IN_COND_ARRAY + num));
            if (relay)
            {
                counter_if_in[num * 2] += 1;
                if (_cpt_pulse_input_if_in_callback != nullptr)
                {
                    _cpt_pulse_input_if_in_callback(num, counter_if_in[num * 2], true);
                }
            }
            else
            {
                counter_if_in[(num * 2) + 1] += 1;
                if (_cpt_pulse_input_if_in_callback != nullptr)
                {
                    _cpt_pulse_input_if_in_callback(num, counter_if_in[(num * 2) + 1], false);
                }
            }
        }
    }
#endif // CPT_PULSE_INPUT_IF_IN > 0
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

void DT_cpt_pulse_input_if_out_reset(const uint8_t num)
{
#if CPT_PULSE_INPUT_IF_OUT > 0
    if ((num * 2) + 1 < CPT_PULSE_INPUT_IF_OUT * 2)
    {
        counter_if_out[num * 2] = 0;
        counter_if_out[(num * 2) + 1] = 0;
        if (_cpt_pulse_input_if_out_callback != nullptr)
        {
            _cpt_pulse_input_if_out_callback(num, counter_if_out[num * 2], true);
            _cpt_pulse_input_if_out_callback(num, counter_if_out[(num * 2) + 1], false);
        }
    }
#endif // CPT_PULSE_INPUT_IF_OUT > 0
}

void DT_cpt_pulse_input_if_in_reset(const uint8_t num)
{
#if CPT_PULSE_INPUT_IF_IN > 0
    if ((num * 2) + 1 < CPT_PULSE_INPUT_IF_IN * 2)
    {
        counter_if_in[num * 2] = 0;
        counter_if_in[(num * 2) + 1] = 0;
        if (_cpt_pulse_input_if_in_callback != nullptr)
        {
            _cpt_pulse_input_if_in_callback(num, counter_if_in[num * 2], true);
            _cpt_pulse_input_if_in_callback(num, counter_if_in[(num * 2) + 1], false);
        }
    }
#endif // CPT_PULSE_INPUT_IF_IN > 0
}

void DT_cpt_pulse_input_set_callback(void (*callback)(const uint8_t num, const uint32_t counter))
{
#if CPT_PULSE_INPUT > 0
    _cpt_pulse_input_callback = callback;
#endif // CPT_PULSE_INPUT > 0
}

void DT_cpt_pulse_input_if_out_set_callback(void (*callback)(const uint8_t num, const uint32_t counter, bool cond))
{
#if CPT_PULSE_INPUT_IF_OUT > 0
    _cpt_pulse_input_if_out_callback = callback;
#endif // CPT_PULSE_INPUT_IF_OUT > 0
}

void DT_cpt_pulse_input_if_in_set_callback(void (*callback)(const uint8_t num, const uint32_t counter, bool cond))
{
#if CPT_PULSE_INPUT_IF_IN > 0
    _cpt_pulse_input_if_in_callback = callback;
#endif // CPT_PULSE_INPUT_IF_IN > 0
}

uint32_t DT_cpt_pulse_input_get(uint8_t num)
{
#if CPT_PULSE_INPUT > 0
    return counter[num];
#else  // CPT_PULSE_INPUT_IF_OUT > 0
    return 0;
#endif // CPT_PULSE_INPUT > 0
}

uint32_t DT_cpt_pulse_input_if_out_get(uint8_t num, bool cond)
{
#if CPT_PULSE_INPUT_IF_OUT > 0
    if ((num * 2) + 1 < CPT_PULSE_INPUT_IF_OUT * 2)
    {
        if (cond)
            return counter_if_out[num * 2];
        else
            return counter_if_out[(num * 2) + 1];
    }
    else
        return 0;
#else  // CPT_PULSE_INPUT_IF_OUT > 0
    return 0;
#endif // CPT_PULSE_INPUT_IF_OUT > 0
}

uint32_t DT_cpt_pulse_input_if_in_get(uint8_t num, bool cond)
{
#if CPT_PULSE_INPUT_IF_IN > 0
    if ((num * 2) + 1 < CPT_PULSE_INPUT_IF_IN * 2)
    {
        if (cond)
            return counter_if_in[num * 2];
        else
            return counter_if_in[(num * 2) + 1];
    }
    else
        return 0;
#else  // CPT_PULSE_INPUT_IF_IN > 0
    return 0;
#endif // CPT_PULSE_INPUT_IF_IN > 0
}