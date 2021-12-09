#ifndef DT_3VOIES
#define DT_3VOIES
#include <Arduino.h>

enum DT_3voies_mode
{
    DT_3VOIES_OFF,
    DT_3VOIES_DEMMARAGE,
    DT_3VOIES_NORMAL
};

void DT_3voies_init();
void DT_3voies_loop();
void DT_3voies_set_mode(DT_3voies_mode mode);
DT_3voies_mode DT_3voies_get_mode(void);

#endif