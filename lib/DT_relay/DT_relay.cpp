#include <DT_relay.h>
#include "Arduino.h"
#include <Adafruit_MCP23X08.h>
#include <config.h>

uint32_t num_delay[RELAY_NUM];
void (*_callback)(const uint8_t num, const bool action);
Adafruit_MCP23X08 mcp[2];

void DT_relay_init()
{
    uint8_t i2c = 0;
    _callback = NULL;

    for (uint8_t mcp_num = 0; mcp_num < 2; ++mcp_num)
    {
        i2c = pgm_read_byte(MCP_ADDRESS + mcp_num);
        if (!mcp[mcp_num].begin_I2C(i2c))
        {
            {
               //auto Serial.println("MCP23008 Init Error.");
            }
        }
    }

    for (uint8_t num = 0; num < RELAY_NUM; ++num)
    {

        uint8_t pin = pgm_read_byte(RELAY_ARRAY + num);

        if (pin >= 100)
        {
            pin -= 100;
            i2c = pin / 10;
            pin %= 10;
            mcp[i2c].pinMode(pin, OUTPUT);
        }
        else
        {
            pinMode(pin, OUTPUT);
        }

        DT_relay(num + 1, false);
        num_delay[num] = 0;
    }
}

void DT_relay(uint8_t num, bool state)
{
    uint8_t pin = pgm_read_byte(RELAY_ARRAY + (num - 1));
    bool revert = pgm_read_byte(RELAY_REVERT + (num - 1));

    bool old_state = DT_relay_get(num);

#ifdef VANNES
    //interverouillage
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
#endif //VANNES

    if (pin >= 100)
    {
        pin -= 100;
        uint8_t i2c = pin / 10;
        pin %= 10;
        if ((state && !revert) || (!state && revert))
        {
            mcp[i2c].digitalWrite(pin, HIGH);
        }
        else
        {
            mcp[i2c].digitalWrite(pin, LOW);
        }
    }
    else
    {
        if ((state && !revert) || (!state && revert))
        {
            digitalWrite(pin, HIGH);
        }
        else
        {
            digitalWrite(pin, LOW);
        }
    }

    if (_callback != NULL && old_state != state)
    {
        _callback(num, state);
    }
}

bool DT_relay_get(uint8_t num)
{
    uint8_t pin = pgm_read_byte(RELAY_ARRAY + (num - 1));
    bool revert = pgm_read_byte(RELAY_REVERT + (num - 1));
    bool ret;
    if (pin >= 100)
    {
        pin -= 100;
        uint8_t i2c = pin / 10;
        pin %= 10;
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