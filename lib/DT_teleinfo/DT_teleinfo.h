#ifndef DT_TELEINFO
#define DT_TELEINFO

#include <config.h>
#include "Arduino.h"
void DT_teleinfo_init();
void DT_teleinfo_loop();
void DT_teleinfo_set_callback(void (*callback)(const char *name, const char *value));

#endif