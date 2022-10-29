#ifndef DT_CPT_PULSE_INPUT
#define DT_CPT_PULSE_INPUT

#include <pinout.h>

void DT_cpt_pulse_input_init();
void DT_cpt_pulse_input_loop();
void DT_cpt_pulse_input_set_callback(void (*callback)(const uint8_t num, const uint32_t counter));
uint32_t DT_cpt_pulse_input_get(uint8_t num);




#endif //DT_CPT_PULSE_INPUT