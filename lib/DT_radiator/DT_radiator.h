#pragma once

void DT_radiator_init();
void DT_radiator_loop();

void DT_radiator_set_consigne(uint8_t num, float consigne);

void DT_radiator_set_cycle(uint8_t num, uint32_t cycle);
void DT_radiator_set_M10(uint8_t num, float m10);
void DT_radiator_set_P10(uint8_t num, float p10);
void DT_radiator_set_KI(uint8_t num, float KI);

void DT_radiator_set_callback(void (*callback)(const uint8_t num, const float out, const float I) = nullptr);
