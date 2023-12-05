#include <DT_ecs.h>
#include <DT_relay.h>





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