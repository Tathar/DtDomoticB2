#include "pinout.h"
#include "config.h"
#include "DT_cover.h"
#include "DT_eeprom.h"
#include "DT_relay.h"

enum cover_step
{
    cover_step_none,
    cover_step_stoped,
    cover_step_off,
    cover_step_start,
    cover_step_up,
    cover_step_over_up,
    cover_step_down,
    cover_step_over_down
};

struct cover_struct
{
    // uint8_t interlock; //adresse
    cover_step step;
    uint8_t old_pos;      // in percente
    uint8_t go_pos;       // in percent
    uint8_t pos;          // in percent
    uint32_t mouve_start; // statt move time for percent calcule (in ms)
};

#if COVER_NUM > 0
cover_struct cover[COVER_NUM];
void (*_cover_callback)(const uint8_t num, const uint8_t percent, const cover_state state);
#endif // COVER_NUM

void DT_cover_init()
{
#if DIMMER_COVER_NUM > 0
    for (uint8_t num = 0; num < (DIMMER_COVER_NUM * 2); ++num)
    {
        uint8_t pin = pgm_read_byte(OPT_ARRAY + DIMMER_LIGHT_NUM_OPT + DIMMER_HEAT_NUM + num);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW); // extinction du dimmer
    }
    for (uint8_t num = 0; num < DIMMER_COVER_NUM; ++num) // init variables
    {
        cover[num].step = cover_step_off;
        cover[num].pos = 0;
        cover[num].go_pos = 0;
        cover[num].mouve_start = 0;
    }
#endif // DIMMER_COVER_NUM > 0

#if RELAY_COVER_NUM > 0
    for (uint8_t num = DIMMER_COVER_NUM; num < (COVER_NUM); ++num) // init variables
    {
        cover[num].step = cover_step_off;
        cover[num].pos = 0;
        cover[num].go_pos = 0;
        cover[num].mouve_start = 0;
    }
#endif // RELAY_COVER_NUM > 0

    _cover_callback = nullptr;
}

// move cover at postiotion "percent"
void DT_cover_set(uint8_t num, uint8_t percent)
{
    cover[num].go_pos = percent;
    cover[num].step = cover_step_start;
}

// stop cover
void DT_cover_stop(uint8_t num)
{
    cover[num].step = cover_step_off;
}

uint8_t DT_cover_get(uint8_t num)
{
    return cover[num].pos;
}

void _cover_write(uint8_t num, bool val)
{
#if DIMMER_COVER_NUM > 0
    if (num < DIMMER_COVER_NUM * 2)
    {
        uint8_t pin = pgm_read_byte(OPT_ARRAY + DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + num);
        digitalWrite(pin, val);
    }
#endif // DIMMER_COVER_NUM > 0

#if RELAY_COVER_NUM > 0
    if (num > DIMMER_COVER_NUM * 2 && num < ((COVER_NUM) * 2))
    {
        DT_relay(num - DIMMER_COVER_NUM, val);
    }
#endif // RELAY_COVER_NUM > 0
}

void DT_cover_loop()
{
#if COVER_NUM > 0
    for (uint8_t num = 0; num < DIMMER_COVER_NUM; ++num)
    {
        if (cover[num].step == cover_step_start) // demande de monté
        {
            if (cover[num].go_pos > cover[num].pos)
            {
                _cover_write((num * 2) + 1, LOW); // arret de la descente
                // _NOP();
                _cover_write(num * 2, HIGH); // marche de la montée

                cover[num].mouve_start = millis();
                cover[num].old_pos = cover[num].pos;
                cover[num].step = cover_step_up;

                if (_cover_callback != nullptr)
                {
                    _cover_callback(num, cover[num].pos, cover_opening);
                }
            }
            else if (cover[num].go_pos < cover[num].pos)
            {
                _cover_write((num * 2), LOW); // arret de la monté
                // _NOP();
                _cover_write((num * 2) + 1, HIGH); // marche de la descente
                cover[num].mouve_start = millis();
                cover[num].old_pos = cover[num].pos;
                cover[num].step = cover_step_down;

                if (_cover_callback != nullptr)
                {
                    _cover_callback(num, cover[num].pos, cover_closing);
                }
            }
        }
        else if (cover[num].step == cover_step_up) // Monté en cours
        {
            cover[num].pos = cover[num].old_pos + ((millis() - cover[num].mouve_start) * eeprom_config.dimmer_store[num].ratio_up);
            if (cover[num].pos == cover[num].go_pos)
            {
                if (cover[num].go_pos == 100)
                {
                    cover[num].mouve_start = millis();
                    cover[num].step = cover_step_over_up;
                }
                else
                {
                    _cover_write((num * 2), LOW); // arret de la monté
                    cover[num].step = cover_step_stoped;
                }

                if (_cover_callback != nullptr)
                {
                    _cover_callback(num, cover[num].pos, cover_open);
                }
            }
            if (_cover_callback != nullptr)
            {
                _cover_callback(num, cover[num].pos, cover_opening);
            }
        }
        else if (cover[num].step == cover_step_down) // descente en cours
        {
            cover[num].pos = cover[num].old_pos - ((millis() - cover[num].mouve_start) * eeprom_config.dimmer_store[num].ratio_down);

            if (cover[num].pos == cover[num].go_pos)
            {
                if (cover[num].go_pos == 0)
                {
                    cover[num].mouve_start = millis();
                    cover[num].step = cover_step_over_down;
                }
                else
                {
                    _cover_write((num * 2) + 1, LOW); // arret de la descente
                    cover[num].step = cover_step_stoped;
                }

                if (_cover_callback != nullptr)
                {
                    _cover_callback(num, cover[num].pos, cover_closed);
                }
            }
            else if (_cover_callback != nullptr)
            {
                _cover_callback(num, cover[num].pos, cover_closing);
            }
        }
        else if (cover[num].step == cover_step_over_up && millis() - cover[num].mouve_start >= 60000) // Monté en cours pendant 1 minute
        {
            cover[num].step = cover_step_off;
        }
        else if (cover[num].step == cover_step_over_down && millis() - cover[num].mouve_start >= 60000) //  descente en cours pendant 1 minute
        {
            cover[num].step = cover_step_off;
        }
        else if (cover[num].step == cover_step_off) //  descente en cours pendant 1 minute
        {
            _cover_write((num * 2), LOW);     // arret de la monté
            _cover_write((num * 2) + 1, LOW); // arret de la descente
            cover[num].step = cover_step_stoped;
            if (_cover_callback != nullptr)
            {
                _cover_callback(num, cover[num].pos, cover_stopped);
            }
        }
    }
#endif // DIMMER_COVER_NUM > 0
}

void DT_cover_set_callback(void (*callback)(const uint8_t num, const uint8_t percent, const cover_state state))
{
    _cover_callback = callback;
}