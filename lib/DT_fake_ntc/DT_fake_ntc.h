#ifndef DT_FAKE_NTC
#define DT_FAKE_NTC
#include <Arduino.h>

void DT_fake_ntc_init(uint8_t value = 20);
void DT_fake_ntc_loop();
void DT_fake_ntc_set(uint8_t value);
void DT_fake_ntc_slow_set(uint8_t value);
__attribute__((always_inline)) inline void DT_fake_ntc_set(float value)
{
    if (value > 0)
        DT_fake_ntc_set((uint8_t)value);
}
__attribute__((always_inline)) inline void DT_fake_ntc_slow_set(float value)
{
    if (value > 0)
        DT_fake_ntc_set((uint8_t)value);
}
uint8_t DT_fake_ntc_get();
void DT_fake_ntc_callback(void (*callback)(const uint8_t value));

#endif