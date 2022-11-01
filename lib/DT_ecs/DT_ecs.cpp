#include <DT_ecs.h>
#include <DT_eeprom.h>
#include <DT_PT100.h>
#include <DT_relay.h>

void DT_ecs_loop()
{
    if (mem_config.MQTT_online == false)
    {
        if (DT_pt100_get(PT100_ECS1) < 45.0 )
        {
            DT_relay(RELAY_ECS1, true);
        }
        if (DT_pt100_get(PT100_ECS1) > 50.0 )
        {
             DT_relay(RELAY_ECS1, false);
        }

        if (DT_pt100_get(PT100_ECS2) < 45.0 )
        {
            DT_relay(RELAY_ECS2, true);
        }
        if (DT_pt100_get(PT100_ECS2) > 50.0 )
        {
             DT_relay(RELAY_ECS2, false);
        }
    }
}