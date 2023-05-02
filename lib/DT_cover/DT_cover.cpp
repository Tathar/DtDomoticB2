#include "pinout.h"
#include "config.h"
#include "DT_cover.h"
#include "DT_eeprom.h"
#include "DT_relay.h"
#include "DT_Dimmer.h"

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
    cover_step_over_down,
    cover_step_register_delay_up,
    cover_step_register_delay_down,
    cover_step_register_up,
    cover_step_register_down,
    cover_step_register_opening,
    cover_step_register_closing,
    cover_step_register_end,
    cover_step_backup_delay,
    cover_step_backup,
    cover_step_backup_up,
    cover_step_backup_down,
};

struct cover_struct
{
    // uint8_t interlock; //adresse
    cover_step step;
    int32_t old_pos; // in milisecond
    int8_t go_pos;   // in percent
    int32_t pos;     // in milisecond
    // int8_t pos;           // in percent
    uint32_t mouve_start; // start move time for percent calcule (in ms)
};

#if COVER_NUM > 0
cover_struct cover[COVER_NUM];
void (*_cover_callback)(const uint8_t num, const int8_t percent, const cover_state state);
#endif // COVER_NUM

void DT_cover_init()
{
#if COVER_NUM > 0
    for (uint8_t num = 0; num < COVER_NUM; ++num) // init variables
    {
        cover[num].step = cover_step_stoped;
        cover[num].pos = 0;
        cover[num].go_pos = 0;
        cover[num].mouve_start = 0;
    }
    _cover_callback = nullptr;

#if DIMMER_COVER_NUM > 0
    for (uint8_t num = 0; num < DIMMER_COVER_NUM * 2; ++num)
    {
        uint8_t pin = pgm_read_byte(DIMMER_COVER_ARRAY + num);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW); // extinction du dimmer
    }
#endif // DIMMER_COVER_NUM > 0
#endif // COVER_NUM > 0
}

// move cover at postiotion "percent"
void DT_cover_set(uint8_t num, uint8_t percent)
{
#if COVER_NUM > 0
    cover[num].go_pos = percent;
    cover[num].step = cover_step_delay_start;
#endif // COVER_NUM > 0
}

// move cover at postiotion "percent"
void DT_cover_up(uint8_t num)
{
#if COVER_NUM > 0
    cover[num].go_pos = 100;
    cover[num].step = cover_step_delay_start;
#endif // COVER_NUM > 0
}

// move cover at postiotion "percent"
void DT_cover_down(uint8_t num)
{
#if COVER_NUM > 0
    cover[num].go_pos = 0;
    cover[num].step = cover_step_delay_start;
#endif // COVER_NUM > 0
}

// stop cover
void DT_cover_stop(uint8_t num)
{
#if COVER_NUM > 0
    cover[num].step = cover_step_stoping;
#endif // COVER_NUM > 0
}

inline int8_t position(uint8_t num)
{
    if (cover[num].pos <= 0)
    {
        return 0;
    }
    else if (cover[num].pos >= eeprom_config.cover[num].time_up)
    {
        return 100;
    }
    else
    {
        // return (32.614 * log((double)cover[num].pos / (double)eeprom_config.cover[num].time_up * (double)100) - 15.814);
        return (double)cover[num].pos / (double)eeprom_config.cover[num].time_up * (double)100;
    }
    // y= 32,614 ln(x) - 55,815 // 0 second pour 0 %
}

void DT_cover_backup_pos(uint8_t num)
{
#if COVER_NUM > 0
    if (DT_cover_get_state(num) < 8) // the cover is stoped
    {
        eeprom_config.cover[num].backup_pos = position(num);
        sauvegardeEEPROM();
        if (position(num) <= 50)
        {
            cover[num].go_pos = position(num) + 5;
            cover[num].step = cover_step_backup_delay;
        }
        else if (position(num) > 50)
        {
            cover[num].go_pos = position(num) - 5;
            cover[num].step = cover_step_backup_delay;
        }
    }
#endif // COVER_NUM > 0
}

void DT_cover_restore_pos(uint8_t num)
{
#if COVER_NUM > 0
    if (DT_cover_get_state(num) < 8 && position(num) != eeprom_config.cover[num].backup_pos) // the cover is stoped
    {
        DT_cover_set(num, eeprom_config.cover[num].backup_pos);
    }
#endif // COVER_NUM > 0
}

// stop cover
void DT_cover_start_register(uint8_t num, bool up)
{
#if COVER_NUM > 0
    if (up)
    {
        cover[num].step = cover_step_register_delay_up;
        cover[num].go_pos = 100;
    }
    else
    {
        cover[num].step = cover_step_register_delay_down;
        cover[num].go_pos = 0;
    }
#endif // COVER_NUM > 0
}

// stop cover
void DT_cover_stop_register(uint8_t num)
{
#if COVER_NUM > 0
    if (cover[num].step == cover_step_register_opening || cover[num].step == cover_step_register_closing)
        cover[num].step = cover_step_register_end;
#endif // COVER_NUM > 0
}

uint8_t DT_cover_get(uint8_t num)
{
#if COVER_NUM > 0
    return position(num);
#endif // COVER_NUM > 0
}

cover_state DT_cover_get_state(uint8_t num)
{
#if COVER_NUM > 0
    switch (cover[num].step)
    {
    case cover_step_stoped:
    case cover_step_stoping:
    case cover_step_register_end:
    case cover_step_over_up:
    case cover_step_over_down:
        if (position(num) == 0)
            return cover_closed;
        else if (position(num) == 100)
            return cover_open;
        else
            return cover_stopped;
        break;
    case cover_step_delay_start:
    case cover_step_start:
        if (position(num) < cover[num].go_pos)
            return cover_opening;
        else
            return cover_closing;
        break;
    case cover_step_up:
    case cover_step_register_delay_up:
    case cover_step_register_up:
        return cover_opening;
        break;
    case cover_step_down:
    case cover_step_register_delay_down:
    case cover_step_register_down:
        return cover_closing;
        break;
    default:
        return cover_stopped;
        break;
    }
#endif // COVER_NUM > 0
}

void _cover_write(uint8_t num, bool state)
{
#if COVER_NUM > 0
    // debug(F("_cover_write"));
    // Serial.print("_cover_write(");
    // Serial.print(num);
    // Serial.print(",");
    // Serial.print(val);
    // Serial.println(")");
#if DIMMER_COVER_NUM > 0
    if (num < DIMMER_COVER_NUM * 2)
    {
        uint8_t pin = pgm_read_byte(DIMMER_COVER_ARRAY + num);
        uint8_t other;
        if (num % 2 == 0)
        {
            other = pgm_read_byte(DIMMER_COVER_ARRAY + num + 1);
        }
        else
        {
            other = pgm_read_byte(DIMMER_COVER_ARRAY + num - 1);
        }

        if (state)
        {
            if (digitalRead(other) == LOW)
            {
                digitalWrite(pin, HIGH);
            }
        }
        else
        {
            digitalWrite(pin, LOW);
            // async_call[num] = true;
        }
    }
#endif // DIMMER_COVER_NUM > 0

#if RELAY_COVER_NUM > 0
    if (num >= (DIMMER_COVER_NUM * 2) && num < (COVER_NUM * 2))
    {
        // debug(F("RELAY_COVER_NUM"));
        // Serial.print("DT_relay(");
        // Serial.print(num - (DIMMER_COVER_NUM * 2));
        // Serial.print(",");
        // Serial.print(val);
        // Serial.println(")");
        DT_relay(num - (DIMMER_COVER_NUM * 2), state);
    }
#endif // RELAY_COVER_NUM > 0
#endif // COVER_NUM > 0
}

void DT_cover_loop()
{
#if COVER_NUM > 0
    for (uint8_t num = 0; num < COVER_NUM; ++num)
    {
        if (cover[num].step == cover_step_delay_start || cover[num].step == cover_step_register_delay_up || cover[num].step == cover_step_register_delay_down ||
            cover[num].step == cover_step_backup_delay) // arret avent d effectuer une action
        {
            debug(F("cover_step_delay_start"));
            _cover_write((num * 2) + 1, LOW); // arret de la descente
            _cover_write(num * 2, LOW);       // arret de la montée
            cover[num].mouve_start = millis();
            switch (cover[num].step)
            {
            case cover_step_delay_start:
                cover[num].step = cover_step_start;
                break;
            case cover_step_register_delay_up:
                cover[num].step = cover_step_register_up;
                break;
            case cover_step_register_delay_down:
                cover[num].step = cover_step_register_down;
                break;
            case cover_step_backup_delay:
                cover[num].step = cover_step_backup;
                break;
            default:
                break;
            }
        }
        else if ((cover[num].step == cover_step_start || cover[num].step == cover_step_backup) && millis() - cover[num].mouve_start > COVER_SECURE_DELAY) // demande de monté
        {
            debug(F("cover_step_start"));
            if (cover[num].go_pos == 100 || cover[num].go_pos > position(num) )
            {
                _cover_write((num * 2) + 1, LOW); // arret de la descente
                _cover_write(num * 2, HIGH);      // marche de la montée

                cover[num].mouve_start = millis();
                cover[num].old_pos = cover[num].pos;
                if (cover[num].step == cover_step_start)
                {
                    debug(F("cover_step_up"));
                    cover[num].step = cover_step_up;
                }
                else if (cover[num].step == cover_step_backup)
                {
                    cover[num].step = cover_step_backup_up;
                }

                if (_cover_callback != nullptr)
                {
                    debug(F("cover_loop_callback"));
                    _cover_callback(num, position(num), cover_opening);
                }
            }
            else if (cover[num].go_pos == 0 || cover[num].go_pos < position(num))
            {
                _cover_write((num * 2), LOW);      // arret de la monté
                _cover_write((num * 2) + 1, HIGH); // marche de la descente
                cover[num].mouve_start = millis();
                cover[num].old_pos = cover[num].pos;

                if (cover[num].step == cover_step_start)
                {
                    debug(F("cover_step_down"));
                    cover[num].step = cover_step_down;
                }
                if (cover[num].step == cover_step_backup)
                {
                    cover[num].step = cover_step_backup_down;
                }

                if (_cover_callback != nullptr)
                {
                    debug(F("cover_loop_callback"));
                    _cover_callback(num, position(num), cover_closing);
                }
            }
            else
            {
                cover[num].step = cover_step_stoping;
            }
        }
        else if (cover[num].step == cover_step_up || cover[num].step == cover_step_backup_up) // Monté en cours
        {
            // debug(F("cover_step_up"));
            cover[num].pos = cover[num].old_pos + (millis() - cover[num].mouve_start);

            // Serial.print(F("cover time = "));
            // Serial.println(cover[num].pos);
            // Serial.print(F("cover pos = "));
            // Serial.println(position(num));

            if (position(num) >= cover[num].go_pos)
            {
                Serial.println(F("> Stop"));

                if (cover[num].go_pos >= 100)
                {
                    cover[num].pos = eeprom_config.cover[num].time_up;
                    cover[num].mouve_start = millis();
                    cover[num].step = cover_step_over_up;

                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback_step_up_100"));
                        _cover_callback(num, position(num), cover_open);
                    }
                }
                else
                {
                    _cover_write((num * 2), LOW); // arret de la monté

                    if (cover[num].step == cover_step_backup_up)
                    {
                        cover[num].step = cover_step_delay_start;
                        cover[num].go_pos = eeprom_config.cover[num].backup_pos;
                    }
                    else
                    {
                        cover[num].step = cover_step_stoped;
                    }

                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback_step_up_other"));
                        _cover_callback(num, position(num), cover_stopped);
                    }
                }

                // cover[num].old_pos = cover[num].pos;
            }
            else if (_cover_callback != nullptr && ((millis() - cover[num].mouve_start) % 250) == 0)
            {
                debug(F("cover_loop_callback_step_up"));
                _cover_callback(num, position(num), cover_opening);
            }
        }
        else if (cover[num].step == cover_step_down || cover[num].step == cover_step_backup_down) // descente en cours
        {
            // debug(F("cover_step_down"));
            cover[num].pos = cover[num].old_pos - ((millis() - cover[num].mouve_start) * ((double)eeprom_config.cover[num].time_down / (double)eeprom_config.cover[num].time_up));

            // Serial.print(F("cover time = "));
            // Serial.println(cover[num].pos);
            // Serial.print(F("cover pos = "));
            // Serial.println(position(num));

            if (position(num) <= cover[num].go_pos)
            {

                if (position(num) <= 0)
                {

                    Serial.println(F("< Stop"));
                    cover[num].pos = 0;
                    cover[num].mouve_start = millis();
                    cover[num].step = cover_step_over_down;

                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback_step_down_0"));
                        _cover_callback(num, position(num), cover_closed);
                    }
                }
                else
                {
                    _cover_write((num * 2) + 1, LOW); // arret de la descente

                    if (cover[num].step == cover_step_backup_down)
                    {
                        cover[num].step = cover_step_delay_start;
                        cover[num].go_pos = eeprom_config.cover[num].backup_pos;
                    }
                    else
                    {
                        cover[num].step = cover_step_stoped;
                    }

                    if (_cover_callback != nullptr)
                    {
                        debug(F("cover_loop_callback_step_down_other"));
                        _cover_callback(num, position(num), cover_stopped);
                    }
                }

                // cover[num].old_pos = cover[num].pos;
            }
            else if (_cover_callback != nullptr && ((millis() - cover[num].mouve_start) % 250) == 0)
            {
                debug(F("cover_loop_callback_step_down"));
                _cover_callback(num, position(num), cover_closing);
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
        else if (cover[num].step == cover_step_register_up && cover[num].go_pos == 100 && millis() - cover[num].mouve_start > COVER_SECURE_DELAY) // demande de configuration du temp de monté
        {
            debug(F("cover_step_register_up"));
            _cover_write((num * 2) + 1, LOW); // arret de la descente
            _cover_write(num * 2, HIGH);      // marche de la montée

            cover[num].mouve_start = millis();
            cover[num].old_pos = 0;
            cover[num].go_pos = 50;

            cover[num].step = cover_step_register_opening;

            if (_cover_callback != nullptr)
            {
                debug(F("cover_loop_callback"));
                _cover_callback(num, position(num), cover_opening);
            }
        }
        else if (cover[num].step == cover_step_register_opening) // configuration du temp de monté
        {
            debug(F("cover_step_register_opening"));
            eeprom_config.cover[num].time_up = millis() - cover[num].mouve_start;
        }
        else if (cover[num].step == cover_step_register_down && cover[num].go_pos == 0 && millis() - cover[num].mouve_start > COVER_SECURE_DELAY) // demande de configuration du temp de monté
        {
            debug(F("cover_step_register_up"));
            _cover_write((num * 2), LOW);      // arret de la monté
            _cover_write((num * 2) + 1, HIGH); // marche de la descente

            cover[num].mouve_start = millis();
            cover[num].old_pos = 100;
            cover[num].go_pos = 50;

            cover[num].step = cover_step_register_closing;

            if (_cover_callback != nullptr)
            {
                debug(F("cover_loop_callback"));
                _cover_callback(num, position(num), cover_closing);
            }
        }
        else if (cover[num].step == cover_step_register_closing) // configuration du temp de descente
        {
            debug(F("cover_step_register_closing"));
            eeprom_config.cover[num].time_down = millis() - cover[num].mouve_start;
        }
        else if (cover[num].step == cover_step_register_end) // demande de configuration du temp de monté
        {
            debug(F("cover_step_register_end"));
            _cover_write((num * 2), LOW);     // arret de la monté
            _cover_write((num * 2) + 1, LOW); // arret de la descente

            sauvegardeEEPROM();

            cover[num].step = cover_step_stoped;
            if (_cover_callback != nullptr)
            {
                debug(F("cover_loop_callback"));
                _cover_callback(num, position(num), cover_stopped);
            }
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
                _cover_callback(num, position(num), cover_stopped);
            }
        }
    }
#endif // COVER_NUM > 0
}

void DT_cover_set_callback(void (*callback)(const uint8_t num, const int8_t percent, const cover_state state))
{
#if COVER_NUM > 0
    _cover_callback = callback;
#endif // COVER_NUM > 0
}
