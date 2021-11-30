#ifndef DT_TEMP
#define DT_TEMP

#include <../../src/pinout.h>

void DT_temp_init();
void DT_temp_loop();
void DT_temp_set_callback(void (*callback)(const uint8_t num, const float temp) = nullptr);
float DT_temp_get(int num);

#endif