#ifndef DT_SCD4X
#define DT_SCD4X

#include "Arduino.h"
void DT_SCD4X_init();
void DT_SCD4X_loop();
void DT_SCD4X_set_callback_temperature(void (*callback)(const uint8_t num, const float temperature));
void DT_SCD4X_set_callback_humidity(void (*callback)(const uint8_t num, const float hmidity));
void DT_SCD4X_set_callback_co2(void (*callback)(const uint8_t num, const float pressure));
float DT_SCD4X_get_temperature(const uint8_t num);
float DT_SCD4X_get_humidity(const uint8_t num);
float DT_SCD4X_get_co2(const uint8_t num);

#endif