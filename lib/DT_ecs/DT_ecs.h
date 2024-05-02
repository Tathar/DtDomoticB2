#ifndef DT_ECS
#define DT_ECS

#include <config.h>
// #if defined(RELAY_ECS1) ||defined(RELAY_ECS2)
#include <DT_eeprom.h>

DT_ECS_mode DT_ecs1_get_mode();
void DT_ecs1_set_mode(DT_ECS_mode mode);
DT_ECS_mode DT_ecs2_get_mode();
void DT_ecs2_set_mode(DT_ECS_mode mode);
void DT_ecs_loop();

// #endif
#endif