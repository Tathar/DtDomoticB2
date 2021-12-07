#ifndef DT_RELAY
#define DT_RELAY

#include <pinout.h>

void DT_relay_init();
void DT_relay_loop();
void DT_relay(int num, bool active);
void DT_relay(int num, int time);
uint8_t DT_relay_get(int num);
void DT_relay_set_callback(void (*callback)(const uint8_t num, const bool action));
#endif