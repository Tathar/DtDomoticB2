#ifndef DT_PT100
#define DT_PT100

#include <pinout.h>

#define TEMP_DEFAULT_PT100 -273.5 // temperature renvoyer en cas de default (zero absolut)

void DT_pt100_init();
void DT_pt100_loop();
void DT_pt100_set_callback(void (*callback)(const uint8_t num, const float temp) = nullptr);
float DT_pt100_get(uint8_t num);

#endif