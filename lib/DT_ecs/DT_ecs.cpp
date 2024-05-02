#include <DT_ecs.h>
#include <DT_relay.h>


DT_ECS_mode DT_ecs1_get_mode()
{
#ifdef RELAY_ECS1
    return eeprom_config.ecs1_mode;
#endif
}

void DT_ecs1_set_mode(DT_ECS_mode mode)
{
#ifdef RELAY_ECS1
    eeprom_config.ecs1_mode = mode;
#endif
}

DT_ECS_mode DT_ecs2_get_mode()
{
#ifdef RELAY_ECS2
    return eeprom_config.ecs2_mode;
#endif
}

void DT_ecs2_set_mode(DT_ECS_mode mode)
{
#ifdef RELAY_ECS2
    eeprom_config.ecs2_mode = mode;
#endif
}



void DT_ecs_loop()
{
#ifdef RELAY_ECS1
switch (eeprom_config.ecs1_mode)
{
case DT_ECS_MARCHE:
      DT_relay(RELAY_ECS1, true);
    break;
case DT_ECS_STANDBY:
      DT_relay(RELAY_ECS1, false);
    break;
case DT_ECS_ARRET:
      DT_relay(RELAY_ECS1, false);
    break;
default:
    DT_relay(RELAY_ECS1, true);
    break;
}
#endif //RELAY_ECS1

#ifdef RELAY_ECS2
switch (eeprom_config.ecs2_mode)
{
case DT_ECS_MARCHE:
      DT_relay(RELAY_ECS2, true);
    break;
case DT_ECS_STANDBY:
      DT_relay(RELAY_ECS2, false);
    break;
case DT_ECS_ARRET:
      DT_relay(RELAY_ECS2, false);
    break;
default:
    DT_relay(RELAY_ECS2, true);
    break;
}
#endif  //RELAY_ECS2
}