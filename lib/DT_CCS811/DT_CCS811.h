#ifndef DT_CCS811
#define DT_CCS811

#include "Arduino.h"
void DT_CCS811_init();
void DT_CCS811_loop();
void DT_CCS811_set_callback_co2(void (*callback)(const uint8_t num, const float co2));
void DT_CCS811_set_callback_cov(void (*callback)(const uint8_t num, const float hmidity));
float DT_CCS811_get_co2(const uint8_t num);
float DT_CCS811_get_cov(const uint8_t num);
void DT_CCS811_set_environmental_data(uint8_t num, float humidity, float temperature);

#endif