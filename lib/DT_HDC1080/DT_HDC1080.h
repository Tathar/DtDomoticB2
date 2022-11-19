#ifndef DT_HDC1080
#define DT_HDC1080

#include "Arduino.h"
void DT_HDC1080_init();
void DT_HDC1080_loop();
void DT_HDC1080_set_callback_temperature(void (*callback)(const uint8_t num, const float temperature));
void DT_HDC1080_set_callback_humidity(void (*callback)(const uint8_t num, const float hmidity));
float DT_HDC1080_get_temperature(const uint8_t num);
float DT_HDC1080_get_humidity(const uint8_t num);

#endif