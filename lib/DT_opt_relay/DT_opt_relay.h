#ifndef DT_OPT_RELAY
#define DT_OPT_RELAY

#include <config.h>

void DT_opt_relay_init();
void DT_opt_relay_loop();
void DT_opt_relay(uint8_t num, bool active);
void DT_opt_relay(uint8_t num, uint32_t time);
bool DT_opt_relay_get(uint8_t num);
void DT_opt_relay_set_callback(void (*callback)(const uint8_t num, const bool action));
#endif