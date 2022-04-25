#ifndef DT_POELE
#define DT_POELE
#include <Arduino.h>

enum __attribute__((__packed__)) DT_Poele_mode
{
    DT_POELE_ARRET,
    DT_POELE_NORMAL,
    DT_POELE_ECS,
    DT_POELE_FORCE,
};

void DT_Poele_init();
void DT_Poele_loop();
void DT_Poele_set_mode(DT_Poele_mode mode);
DT_Poele_mode DT_Poele_get_mode(void);
void DT_Poele_set_mode_callback(void (*mode_callback)(const DT_Poele_mode mode));

#endif