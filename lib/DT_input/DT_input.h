#ifndef DT_INPUT
#define DT_INPUT
#include <Arduino.h>

void DT_input_init();
void DT_input_loop();
uint8_t DT_input_get(int num);
void DT_input_set_callback(void (*callback)(const uint8_t num, const uint8_t action));
#endif