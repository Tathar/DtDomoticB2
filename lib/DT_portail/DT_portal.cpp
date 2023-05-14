#include "pinout.h"
#include "config.h"
#include "DT_portal.h"
// #include "DT_eeprom.h"
#include "DT_relay.h"
// #include "DT_Dimmer.h"

enum portal_step
{
    portal_step_none,
    portal_step_stop,
    portal_step_open,
    portal_step_closed,
    portal_step_up,
    portal_step_down,
};

struct portal_struct
{
    // uint8_t interlock; //adresse
    portal_step step;
    portal_step action;
    uint32_t push_start;
};

#if PORTAL_NUM > 0
portal_struct portal[PORTAL_NUM];
void (*_portal_callback)(const uint8_t num, const portal_state state);
#endif // PORTAL_NUM

void DT_portal_init()
{
#if PORTAL_NUM > 0
    for (uint8_t num = 0; num < PORTAL_NUM; ++num) // init variables
    {
        portal[num].step = portal_step_none;
        portal[num].action = portal_step_none;
        portal[num].push_start = 0;
    }
    _portal_callback = nullptr;

#endif // PORTAL_NUM > 0
}

// move portal at postiotion "percent"
void DT_portal_open(uint8_t num)
{
#if PORTAL_NUM > 0
    portal[num].action = portal_step_up;
#endif // PORTAL_NUM > 0
}

// move portal at postiotion "percent"
void DT_portal_close(uint8_t num)
{
#if PORTAL_NUM > 0
    portal[num].action = portal_step_down;
#endif // PORTAL_NUM > 0
}

void DT_portal_stop(uint8_t num)
{
#if PORTAL_NUM > 0
    portal[num].action = portal_step_stop;
#endif // PORTAL_NUM > 0
}

portal_state DT_portal_get_state(uint8_t num)
{
#if PORTAL_NUM > 0
    switch (portal[num].step)
    {
    case portal_step_open:
        return portal_open;
        break;
    case portal_step_closed:
        return portal_closed;
        break;
    case portal_step_up:
        return portal_opening;
        break;
    case portal_step_down:
        return portal_closing;
        break;
    default:
        return portal_stopped;
        break;
    }
#endif // PORTAL_NUM > 0
}

void _portal_write(uint8_t num, bool state)
{
#if PORTAL_NUM > 0
    // debug(F("RELAY_COVER_NUM"));
    // Serial.print("DT_relay(");
    // Serial.print(num - (DIMMER_COVER_NUM * 2));
    // Serial.print(",");
    // Serial.print(val);
    // Serial.println(")");
    uint8_t relay = pgm_read_byte(RELAY_PORTAL_ARRAY + num);
    uint8_t linked_relay;

    if (num % 2 == 0)
    {
        linked_relay = pgm_read_byte(RELAY_PORTAL_ARRAY + num + 1);
    }
    else
    {
        linked_relay = pgm_read_byte(RELAY_PORTAL_ARRAY + num - 1);
    }

    if (state)
    {
        if (DT_relay_get(linked_relay) == false)
        {
            DT_relay(relay, true);
        }
    }
    else
    {
        DT_relay(relay, false);
        // async_call[num] = true;
    }

#endif // COVER_NUM > 0
}

void DT_portal_loop()
{
#if PORTAL_NUM > 0
    for (uint8_t num = 0; num < COVER_NUM; ++num)
    {
        if (portal[num].action == portal_step_up && portal[num].step != portal_step_up) // demande d'ouverture
        {
            debug(F("cover_step_up"));
            portal[num].push_start = millis();
            _portal_write(num * 2, true);
            portal[num].step = portal_step_up;
        }
        else if (portal[num].action == portal_step_down && portal[num].step != portal_step_down) // demande de fermeture
        {
            portal[num].push_start = millis();
            debug(F("cover_step_down"));
            _portal_write((num * 2) + 1, true);
            portal[num].step = portal_step_down;
        }
        else if (portal[num].action == portal_step_stop && (portal[num].step == portal_step_up || portal[num].step == portal_step_down )) // demande d'arret
        {
            portal[num].push_start = millis();
            debug(F("cover_step_stop"));
            _portal_write(num * 2, true);
            portal[num].step = portal_step_stop;
        }
        else if (millis() - portal[num].push_start >= 1000 && portal[num].action != portal_step_none)
        {
            _portal_write(num * 2, false);
            _portal_write((num * 2) + 1, false);
            portal[num].action = portal_step_none;
        }
        else if (millis() - portal[num].push_start >= PORTAL_OPEN_CLOSE_TIME && portal[num].step == portal_step_up)
        {
            portal[num].step = portal_step_open;
        }
        else if (millis() - portal[num].push_start >= PORTAL_OPEN_CLOSE_TIME && portal[num].step == portal_step_down)
        {
            portal[num].step = portal_step_closed;
        }
    }
#endif // PORTAL_NUM > 0
}

void DT_portal_set_callback(void (*callback)(const uint8_t num, const portal_state state))
{
#if PORTAL_NUM > 0
    _portal_callback = callback;
#endif // COVER_NUM > 0
}
