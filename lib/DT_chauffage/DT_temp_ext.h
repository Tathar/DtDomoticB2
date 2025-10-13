#ifndef DT_TEMP_EXT
#define DT_TEMP_EXT
#include <Arduino.h>

void DT_get_avg_temp_init();
void DT_get_avg_temp_loop();

void DT_3voies_set_callback_avg_temp(void (*callback_avg_temp)(const float temp));
float DT_get_temp_moyen();

#endif //DT_TEMP_EXT
