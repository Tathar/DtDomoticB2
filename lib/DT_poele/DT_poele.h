#ifndef DT_POELE
#define DT_POELE
#include <Arduino.h>

enum DT_Poele_mode
{
    DT_POELE_OFF,
    DT_POELE_ARRET,
    DT_POELE_SECOURS,
    DT_POELE_NORMAL,
    DT_POELE_ECS,
    DT_POELE_BOOST,
    DT_POELE_MANUAL,
};

void DT_Poele_init();
void DT_Poele_loop();
void DT_Poele_set_mode(DT_Poele_mode mode);
DT_Poele_mode DT_Poele_get_mode(void);
void DT_Poele_set_mode_callback(void (*mode_callback)(const DT_Poele_mode mode));

void DT_Poele_set_C1_callback(void (*callback)(const uint8_t C1));
void DT_Poele_set_C1(const float c1);
float DT_Poele_get_C1();

#endif