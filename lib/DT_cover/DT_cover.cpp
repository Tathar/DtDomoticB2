#include "pinout.h"
#include "config.h"
#include "DT_cover.h"
#include "DT_eeprom.h"
#include "DT_relay.h"

enum cover_step
{
    cover_step_none,
    cover_step_stoped,
    cover_step_stoping,
    cover_step_delay_start,
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
    for (uint8_t num = 0; num < DIMMER_COVER_NUM; ++num) // init variables
    {
        cover[num].step = cover_step_stoped;
        cover[num].pos = 0;
        cover[num].go_pos = 0;
        cover[num].mouve_start = 0;
    }
#endif // DIMMER_COVER_NUM > 0

#if RELAY_COVER_NUM > 0
    for (uint8_t num = DIMMER_COVER_NUM; num < (COVER_NUM); ++num) // init variables
    {
        cover[num].step = cover_step_stoped;
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
    cover[num].step = cover_step_delay_start;
}

// stop cover
void DT_cover_stop(uint8_t num)
{
    cover[num].step = cover_step_stoping;
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
        DT_dimmer_relay(num, val);
    }
#endif // DIMMER_COVER_NUM > 0

#if RELAY_COVER_NUM > 0
    if (num > DIMMER_COVER_NUM * 2 && num < ((COVER_NUM)*2))
    {
        DT_relay(num - DIMMER_COVER_NUM, val);
    }
#endif // RELAY_COVER_NUM > 0
}

void DT_cover_loop()
{
#if COVER_NUM > 0
    for (uint8_t num = 0; num < COVER_NUM; ++num)
    {
        if (cover[num].step == cover_step_delay_start) // arret avent d effectuer une action
        {
            debug(F("cover_step_delay_start"));
            _cover_write((num * 2) + 1, LOW); // arret de la descente
            _cover_write(num * 2, LOW);       // arret de la montée
            cover[num].mouve_start = millis();
            cover[num].step = cover_step_start;
        }
        else if (cover[num].step == cover_step_start && millis() - cover[num].mouve_start > 250) // demande de monté
        {
            debug(F("cover_step_start"));
            if (cover[num].go_pos > cover[num].pos)
            {
                _cover_write((num * 2) + 1, LOW); // arret de la descente
                _cover_write(num * 2, HIGH);      // marche de la montée

                cover[num].mouve_start = millis();
                cover[num].old_pos = cover[num].pos;
                cover[num].step = cover_step_up;

                if (_cover_callback != nullptr)
                {
                    debug(F("cover_loop_callback"));
                    _cover_callback(num, cover[num].pos, cover_opening);
                }
            }
            else if (cover[num].go_pos < cover[num].pos)
            {
                _cover_write((num * 2), LOW);      // arret de la monté
                _cover_write((num * 2) + 1, HIGH); // marche de la descente
                cover[num].mouve_start = millis();
                cover[num].old_pos = cover[num].pos;
                cover[num].step = cover_step_down;

                if (_cover_callback != nullptr)
                {
                    debug(F("cover_loop_callback"));
                    _cover_callback(num, cover[num].pos, cover_closing);
                }
            }
        }
        else if (cover[num].step == cover_step_up) // Monté en cours
        {
            debug(F("cover_step_up"));
            cover[num].pos = cover[num].old_pos + ((millis() - cover[num].mouve_start) / (eeprom_config.cover[num].time_up / 100));
            if (cover[num].pos == cover[num].go_pos)
            {
                cover[num].old_pos = cover[num].pos;

                if (cover[num].go_pos == 100)
                {
                    // cover[num].mouve_start = millis();
                    cover[num].step = cover_step_over_up;
                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback"));
                        _cover_callback(num, cover[num].pos, cover_open);
                    }
                }
                else
                {
                    _cover_write((num * 2), LOW); // arret de la monté
                    cover[num].step = cover_step_stoped;
                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback"));
                        _cover_callback(num, cover[num].pos, cover_stopped);
                    }
                }
            }
            else if (_cover_callback != nullptr)
            {
                debug(F("cover_loop_callback"));
                _cover_callback(num, cover[num].pos, cover_opening);
            }
        }
        else if (cover[num].step == cover_step_down) // descente en cours
        {
            debug(F("cover_step_down"));
            cover[num].pos = cover[num].old_pos - ((millis() - cover[num].mouve_start) / (eeprom_config.cover[num].time_down / 100));

            if (cover[num].pos == cover[num].go_pos)
            {
                cover[num].old_pos = cover[num].pos;

                if (cover[num].go_pos == 0)
                {
                    // cover[num].mouve_start = millis();
                    cover[num].step = cover_step_over_down;

                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback"));
                        _cover_callback(num, cover[num].pos, cover_closed);
                    }
                }
                else
                {
                    _cover_write((num * 2) + 1, LOW); // arret de la descente
                    cover[num].step = cover_step_stoped;

                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback"));
                        _cover_callback(num, cover[num].pos, cover_stopped);
                    }
                }
            }
            else if (_cover_callback != nullptr)
            {
                debug(F("cover_loop_callback"));
                _cover_callback(num, cover[num].pos, cover_closing);
            }
        }
        else if (cover[num].step == cover_step_over_up && millis() - cover[num].mouve_start >= 60000) // Monté en cours pendant 1 minute
        {
            debug(F("cover_step_over_up"));
            _cover_write((num * 2), LOW); // arret de la monté
            cover[num].step = cover_step_stoped;
        }
        else if (cover[num].step == cover_step_over_down && millis() - cover[num].mouve_start >= 60000) //  descente en cours pendant 1 minute
        {
            debug(F("cover_step_over_down"));
            _cover_write((num * 2) + 1, LOW); // arret de la descente
            cover[num].step = cover_step_stoped;
        }
        else if (cover[num].step == cover_step_stoping) //  descente en cours pendant 1 minute
        {
            debug(F("cover_step_stoping"));
            _cover_write((num * 2), LOW);     // arret de la monté
            _cover_write((num * 2) + 1, LOW); // arret de la descente
            cover[num].step = cover_step_stoped;
            if (_cover_callback != nullptr)
            {
                debug(F("cover_loop_callback"));
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