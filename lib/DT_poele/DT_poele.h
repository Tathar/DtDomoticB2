#ifndef DT_POELE
#define DT_POELE
#include <Arduino.h>

enum __attribute__((__packed__)) DT_Poele_mode
{
    DT_POELE_OFF,
    DT_POELE_WINTER,
    DT_POELE_MAINTENANCE,
    DTP_POELE_BETWEEN,
};

void DT_Poele_init();
void DT_Poele_loop();
void DT_Poele_set_mode(DT_Poele_mode mode);
DT_Poele_mode DT_Poele_get_mode(void);
void DT_Poele_mode_callback(const DT_Poele_mode mode);

bool DT_Poele_started(void);


#endif