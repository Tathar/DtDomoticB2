#ifndef DT_RELAY
#define DT_RELAY

#include <pinout.h>

void DT_relay_init();
void DT_relay_loop();
void DT_relay(uint8_t num, bool active);
void DT_relay(uint8_t num, uint32_t time);
uint8_t DT_relay_get(uint8_t num);
void DT_relay_set_callback(void (*callback)(const uint8_t num, const bool action));
#endif