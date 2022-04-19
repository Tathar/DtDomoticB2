#pragma once


#include <avr/wdt.h> //watchdog

#include <config.h>
#include <DT_mqtt.h>

#ifdef MQTT

bool homeassistant(bool start);

#endif //MQTT