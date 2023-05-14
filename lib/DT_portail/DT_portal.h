#pragma once

#include <Arduino.h>

enum portal_state
{
    portal_stopped,
    portal_closed,
    portal_open,
    portal_opening = 8,
    portal_closing,
};

void DT_portal_init();
void DT_portal_loop();

void DT_portal_open(uint8_t num);
void DT_portal_close(uint8_t num);
void DT_portal_stop(uint8_t num);
portal_state DT_portal_get_state(uint8_t num);

void DT_portal_set_callback(void (*callback)(const uint8_t num, const portal_state state));
