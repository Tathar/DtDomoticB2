#ifndef DT_POELE
#define DT_POELE
#include <Arduino.h>

enum DT_Poele_mode
{
    DT_POELE_OFF,
    DT_POELE_SILANCE,
    DT_POELE_SECOURS,
    DT_POELE_NORMAL,
    DT_POELE_ECS,
    DT_POELE_BOOST,
};

void DT_Poele_init();
void DT_Poele_loop();
void DT_Poele_set_mode(DT_Poele_mode mode);

#endif