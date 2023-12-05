#ifndef DT_ECS
#define DT_ECS

#include <config.h>
// #if defined(RELAY_ECS1) ||defined(RELAY_ECS2)
#include <DT_eeprom.h>

inline DT_ECS_mode DT_ecs1_get_mode()
{
#ifdef RELAY_ECS1
    return eeprom_config.ecs1_mode;
#endif
}

inline void DT_ecs1_set_mode(DT_ECS_mode mode)
{
#ifdef RELAY_ECS1
    eeprom_config.ecs1_mode = mode;
#endif
}


inline DT_ECS_mode DT_ecs2_get_mode()
{
#ifdef RELAY_ECS2
    return eeprom_config.ecs2_mode;
#endif
}

inline void DT_ecs2_set_mode(DT_ECS_mode mode)
{
#ifdef RELAY_ECS2
    eeprom_config.ecs2_mode = mode;
#endif
}

void DT_ecs_loop();

// #endif
#endif