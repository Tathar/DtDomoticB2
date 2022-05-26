#include <DT_PT100.h>
#include "Arduino.h"
#include "config.h"

#include <Adafruit_MAX31865.h>

#define MIN_DEFAULT_PT100 -100 // si la temperature est inferieur, on considere que la PT100 est en default
#define MAX_DEFAULT_PT100 200  // si la temperature est superieur, on considere que la PT100 est en default

Adafruit_MAX31865 *max31865[TEMP_NUM];

float old_temp[TEMP_NUM];
void (*pt100_callback)(const uint8_t num, const float temp);

float _temp_get(int num)
{
    float tmp = max31865[num]->temperature(100, TEMP_RREF);
    if (tmp < MIN_DEFAULT_PT100 || tmp > MAX_DEFAULT_PT100)
    {
        return TEMP_DEFAULT_PT100;
    }
    int32_t digit = tmp * 100;
    return digit / 100.0;
}

void DT_pt100_init()
{
    pt100_callback = nullptr;
    for (uint8_t num = 0; num < TEMP_NUM; ++num)
    {
#if DIMMER_LIGHT_NUM >= 1                                                      // si dimmer
        uint8_t pin = pgm_read_byte(OPT_ARRAY + (1 + DIMMER_LIGHT_NUM + num)); // decallage de 1 lie au dimmer
#else
        uint8_t pin = pgm_read_byte(OPT_ARRAY + num); // pas de decallage sinon
#endif
        old_temp[num] = 0;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        max31865[num] = new Adafruit_MAX31865(pin);
        max31865[num]->begin(MAX31865_3WIRE);
    }
}

void DT_pt100_loop()
{
    static uint32_t old_time = 0;
    static uint32_t PT100_callback_time = 0;
    uint32_t now = millis();
    float tmp = 0;
    if (now - old_time > 1000)
    {
        for (uint8_t num = 0; num < TEMP_NUM; num++)
        {
            uint16_t rtd_value;
            if (max31865[num]->readRTDAsync(rtd_value))
            {
                tmp = max31865[num]->temperatureAsync(rtd_value, 100, TEMP_RREF);
                if (num == TEMP_NUM - 1)
                {
                    old_time = now;
                }
                old_temp[num] = tmp;
            }
        }
    }
    if (now - PT100_callback_time >= MQTT_REFRESH / TEMP_NUM)
    {
        PT100_callback_time = now;
        if (pt100_callback != nullptr)
        {

            static uint8_t num = 0;
            if (num < TEMP_NUM)
            {
                pt100_callback(num + 1, old_temp[num]);
                ++num;
            }
            if (num == TEMP_NUM)
            {
                num = 0;
            }
        }
    }
}

float DT_pt100_get(uint8_t num)
{
    if (num - 1 < TEMP_NUM)
    {
        return old_temp[num - 1];
    }
    else
    {
        return TEMP_DEFAULT_PT100;
    }
}

void DT_pt100_set_callback(void (*callback)(const uint8_t num, const float temp))
{
    pt100_callback = callback;
}