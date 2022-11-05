#include <DT_ecs.h>
#include <DT_eeprom.h>
#include <DT_PT100.h>
#include <DT_relay.h>

void DT_ecs_loop()
{
    if (mem_config.MQTT_online == false)
    {
#ifdef RELAY_ECS1
        if (DT_pt100_get(PT100_ECS2) < 45.0)
        {
            debug(F(AT "Marche ECS1") );
            DT_relay(RELAY_ECS1, true);
        }
        else if (DT_pt100_get(PT100_ECS2) > 50.0)
        {
            debug(F(AT "Arret ECS1"));
            DT_relay(RELAY_ECS1, false);
        }
#endif

#ifdef RELAY_ECS2
        if (DT_pt100_get(PT100_ECS1) < 45.0)
        {
            debug(F(AT "Marche ECS2"));
            DT_relay(RELAY_ECS2, true);
        }
        else if (DT_pt100_get(PT100_ECS1) > 50.0)
        {
            debug(F(AT "Arret ECS2"));
            DT_relay(RELAY_ECS2, false);
        }
#endif
    }
}