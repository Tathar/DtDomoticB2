#ifndef DT_BME280
#define DT_BME280

#include "Arduino.h"
void DT_BME280_init();
void DT_BME280_loop();
void DT_BME280_set_callback_temperature(void (*callback)(const uint8_t num, const float temperature));
void DT_BME280_set_callback_humidity(void (*callback)(const uint8_t num, const float hmidity));
void DT_BME280_set_callback_pressure(void (*callback)(const uint8_t num, const float pressure));
float DT_BME280_get_temperature(const uint8_t num);
float DT_BME280_get_humidity(const uint8_t num);
float DT_BME280_get_pressure(const uint8_t num);

#endif