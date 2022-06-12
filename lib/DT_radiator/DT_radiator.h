#pragma once

void DT_radiator_init();
void DT_radiator_loop();

float DT_radiator_get_consigne(uint8_t num);
void DT_radiator_set_consigne(uint8_t num, float consigne);

Radiator_Mode DT_radiator_get_mode(uint8_t num);
void DT_radiator_set_mode(uint8_t num, Radiator_Mode mode);

uint32_t DT_radiator_get_cycle(uint8_t num);
void DT_radiator_set_cycle(uint8_t num, uint32_t cycle);

float DT_radiator_get_M10(uint8_t num);
void DT_radiator_set_M10(uint8_t num, float m10);

float DT_radiator_get_P10(uint8_t num);
void DT_radiator_set_P10(uint8_t num, float p10);

float DT_radiator_get_KI(uint8_t num);
void DT_radiator_set_KI(uint8_t num, float KI);

void DT_radiator_set_callback(void (*callback)(const uint8_t num, const float out, const float I) = nullptr);
