#include <DT_opt_relay.h>
#include "Arduino.h"
#include <config.h>

uint32_t opt_num_delay[RELAY_NUM];
bool opt_async_call[RELAY_NUM];
void (*_callback_opt_relay)(const uint8_t num, const bool action);

void DT_opt_relay_init()
{
    _callback_opt_relay = NULL;

    for (uint8_t num = 0; num < RELAY_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(OPT_RELAY_ARRAY + num);

        pinMode(pin, OUTPUT);
        DT_opt_relay(num, false);
        opt_num_delay[num] = 0;
        opt_async_call[num] = false;
    }
}

void DT_opt_relay(uint8_t num, bool state)
{
    uint8_t pin = pgm_read_byte(OPT_RELAY_ARRAY + num);
    bool revert = pgm_read_byte(OPT_RELAY_REVERT + num);
    if ((state && !revert) || (!state && revert))
    {
        if (digitalRead(pin) != HIGH)
        {
            digitalWrite(pin, HIGH);
            opt_async_call[num] = true;
        }
    }
    else
    {
        if (digitalRead(pin) != LOW)
        {
            digitalWrite(pin, LOW);
            opt_async_call[num] = true;
        }
    }
}

bool DT_opt_relay_get(uint8_t num)
{
    uint8_t pin = pgm_read_byte(OPT_RELAY_ARRAY + num);
    bool revert = pgm_read_byte(OPT_RELAY_REVERT + num);
    bool ret;
    ret = digitalRead(pin);
    return revert ? !ret : ret;
}

void DT_opt_relay(uint8_t num, uint32_t time)
{
    if (time > 0)
    {
        opt_num_delay[num] = time;
        DT_opt_relay(num, true);
    }
    else
    {
        opt_num_delay[num] = 0;
        DT_opt_relay(num, false);
    }
}

void DT_opt_relay_loop()
{
    static uint32_t last = 0;
    uint32_t now = millis();
    uint32_t elapse = now - last;
    static uint8_t async_num_callback_opt_relay = 0;
    last = now;

    for (uint8_t num = 0; num < OPT_RELAY_NUM; ++num)
    {
        if (opt_num_delay[num] == 0)
        {
            continue;
        }
        else if (opt_num_delay[num] <= elapse)
        {
            opt_num_delay[num] = 0;
            DT_opt_relay(num, false);
        }
        else
        {
            opt_num_delay[num] -= elapse;
        }
    }

    if (_callback_opt_relay != NULL)
    {
        if (async_num_callback_opt_relay < OPT_RELAY_NUM - 1)
        {
            ++async_num_callback_opt_relay;
        }
        else
        {
            async_num_callback_opt_relay = 0;
        }

        if (opt_async_call[async_num_callback_opt_relay] == true)
        {
            opt_async_call[async_num_callback_opt_relay] = false;
            _callback_opt_relay(async_num_callback_opt_relay, DT_opt_relay_get(async_num_callback_opt_relay));
        }
    }
}

void DT_opt_relay_set_callback(void (*callback)(const uint8_t num, const bool action))
{
    _callback_opt_relay = callback;
}