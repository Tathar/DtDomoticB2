#include <DT_relay.h>
#include "Arduino.h"
#include <Adafruit_MCP23X08.h>
#include <config.h>
#include <DT_mcp.h>

uint32_t num_delay[RELAY_NUM];
bool async_call[RELAY_NUM];
void (*_callback)(const uint8_t num, const bool action);

void DT_relay_init()
{
    _callback = NULL;

    for (uint8_t num = 0; num < RELAY_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(RELAY_ARRAY + num);

        if (pin >= 100)
        {
            uint8_t i2c = pin / 100;
            pin -= i2c * 100;
            i2c -= 1;
            mcp[i2c].pinMode(pin, OUTPUT);
        }
        else
        {
            pinMode(pin, OUTPUT);
        }

        DT_relay(num + 1, false);
        num_delay[num] = 0;
        async_call[num] = false;
    }
}

void DT_relay(uint8_t num, bool state)
{
    uint8_t pin = pgm_read_byte(RELAY_ARRAY + (num - 1));
    bool revert = pgm_read_byte(RELAY_REVERT + (num - 1));

#ifdef VANNES
    // interverouillage
    if (num == VANNE_PCBT_HOT && state == true && DT_relay_get(VANNE_PCBT_COLD) == true)
    {
        return;
    }
    else if (num == VANNE_PCBT_COLD && state == true && DT_relay_get(VANNE_PCBT_HOT) == true)
    {
        return;
    }
    if (num == VANNE_MCBT_HOT && state == true && DT_relay_get(VANNE_MCBT_COLD) == true)
    {
        return;
    }
    else if (num == VANNE_MCBT_COLD && state == true && DT_relay_get(VANNE_MCBT_HOT) == true)
    {
        return;
    }
#endif // VANNES

    if (pin >= 100)
    {
        uint8_t i2c = pin / 100;
        pin -= i2c * 100;
        i2c -= 1;
        if ((state && !revert) || (!state && revert))
        {
            if (mcp[i2c].digitalRead(pin) != HIGH)
            {
                mcp[i2c].digitalWrite(pin, HIGH);
                async_call[num - 1] = true;
            }
        }
        else
        {
            if (mcp[i2c].digitalRead(pin) != LOW)
            {
                mcp[i2c].digitalWrite(pin, LOW);
                async_call[num - 1] = true;
            }
        }
    }
    else
    {
        if ((state && !revert) || (!state && revert))
        {
            if (digitalRead(pin) != HIGH)
            {
                digitalWrite(pin, HIGH);
                async_call[num - 1] = true;
            }
        }
        else
        {
            if (digitalRead(pin) != LOW)
            {
                digitalWrite(pin, LOW);
                async_call[num - 1] = true;
            }
        }
    }
}

bool DT_relay_get(uint8_t num)
{
    uint8_t pin = pgm_read_byte(RELAY_ARRAY + (num - 1));
    bool revert = pgm_read_byte(RELAY_REVERT + (num - 1));
    bool ret;
    if (pin >= 100)
    {
        uint8_t i2c = pin / 100;
        pin -= i2c * 100;
        i2c -= 1;
        ret = mcp[i2c].digitalRead(pin);
    }
    else
    {
        ret = digitalRead(pin);
    }
    return revert ? !ret : ret;
}

void DT_relay(uint8_t num, uint32_t time)
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
    static uint8_t async_num_callback = 0;
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

    if (_callback != NULL)
    {
        if (async_num_callback < RELAY_NUM - 1)
        {
            ++async_num_callback;
        }
        else
        {
            async_num_callback = 0;
        }

        if (async_call[async_num_callback] == true)
        {
            async_call[async_num_callback] = false;
            _callback(async_num_callback + 1, DT_relay_get(async_num_callback + 1));
        }
    }
}

void DT_relay_set_callback(void (*callback)(const uint8_t num, const bool action))
{
    _callback = callback;
}