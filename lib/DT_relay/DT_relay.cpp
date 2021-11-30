#include <DT_relay.h>
#include "Arduino.h"

uint16_t num_delay[RELAY_NUM];
void (*_callback)(const uint8_t num, const bool action);

void DT_relay_init()
{
    _callback = NULL;
    for (uint8_t num = 0; num < RELAY_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(RELAY_ARRAY + num);
        pinMode(pin, OUTPUT);
        DT_relay(num + 1, false);
        num_delay[num] = 0;
    }
}

void DT_relay(int num, bool active)
{
    uint8_t pin = pgm_read_byte(RELAY_ARRAY + (num - 1));
    bool revert = pgm_read_byte(RELAY_REVERT + (num - 1));
    if ((active && !revert) || (!active && revert))
    {
        digitalWrite(pin, HIGH);
    }
    else
    {
        digitalWrite(pin, LOW);
    }

    if (_callback != NULL)
    {
        _callback(num, active);
    }
}

uint8_t DT_relay_get(int num)
{
    uint8_t pin = pgm_read_byte(RELAY_ARRAY + (num - 1));
    bool revert = pgm_read_byte(RELAY_REVERT + (num - 1));
    bool ret = digitalRead(pin);
    return revert ? !ret : ret;
}

void DT_relay(int num, int time)
{
    if (time > 0)
    {
        num_delay[num - 1] = time;
        DT_relay(num, true);
    }
    else
    {
        num_delay[num - 1] = 0;
        DT_relay(num, false);
    }
}

void DT_relay_loop()
{
    static uint32_t last = 0;
    uint32_t now = millis();
    uint32_t elapse = now - last;
    if (elapse > RELAY_MIN_TIME)
    {
        last = now;

        for (uint8_t num = 0; num < RELAY_NUM; ++num)
        {
            if (num_delay[num] == 0)
            {
                continue;
            }
            else if (num_delay[num] <= elapse)
            {
                num_delay[num] = 0;
                DT_relay(num + 1, false);
            }
            else
            {
                num_delay[num] -= elapse;
            }
        }
    }
}

void DT_relay_set_callback(void (*callback)(const uint8_t num, const bool action))
{
    _callback = callback;
}