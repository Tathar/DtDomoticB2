#pragma once


#include <avr/wdt.h> //watchdog

#include <config.h>
#include <DT_mqtt.h>
#include <ArduinoJson.h>


extern StaticJsonDocument<256> doc;
extern char buffer[BUFFER_SIZE];
extern char buffer_value[BUFFER_VALUE_SIZE];


void homeassistant(bool start);