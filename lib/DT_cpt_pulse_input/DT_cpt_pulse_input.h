#ifndef DT_CPT_PULSE_INPUT
#define DT_CPT_PULSE_INPUT

#include <pinout.h>
#include <DT_input.h>

void DT_cpt_pulse_input_init();
void DT_cpt_pulse_input_loop_event(const uint8_t btn, const Bt_Action action);
void DT_cpt_pulse_input_reset(const uint8_t num);
void DT_cpt_pulse_input_if_out_reset(const uint8_t num);
void DT_cpt_pulse_input_if_in_reset(const uint8_t num);
void DT_cpt_pulse_input_set_callback(void (*callback)(const uint8_t num, const uint32_t counter));
void DT_cpt_pulse_input_if_out_set_callback(void (*callback)(const uint8_t num, const uint32_t counter, const bool cond));
void DT_cpt_pulse_input_if_in_set_callback(void (*callback)(const uint8_t num, const uint32_t counter, const bool cond));
uint32_t DT_cpt_pulse_input_get(uint8_t num);
uint32_t DT_cpt_pulse_input_if_out_get(uint8_t num, bool cond);
uint32_t DT_cpt_pulse_input_if_in_get(uint8_t num, bool cond);




#endif //DT_CPT_PULSE_INPUT