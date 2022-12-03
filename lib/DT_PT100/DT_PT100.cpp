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
    uint8_t fault = max31865[num]->readFault();
    if (tmp < MIN_DEFAULT_PT100 || tmp > MAX_DEFAULT_PT100)
    if (fault)
    {
        Serial.print(F("fault PT100 "));
        Serial.print(num);
        Serial.print(F(" "));
        if (fault & MAX31865_FAULT_HIGHTHRESH)
        {
            Serial.println(F("RTD High Threshold"));
        }
        if (fault & MAX31865_FAULT_LOWTHRESH)
        {
            Serial.println(F("RTD Low Threshold"));
        }
        if (fault & MAX31865_FAULT_REFINLOW)
        {
            Serial.println(F("REFIN- > 0.85 x Bias"));
        }
        if (fault & MAX31865_FAULT_REFINHIGH)
        {
            Serial.println(F("REFIN- < 0.85 x Bias - FORCE- open"));
        }
        if (fault & MAX31865_FAULT_RTDINLOW)
        {
            Serial.println(F("RTDIN- < 0.85 x Bias - FORCE- open"));
        }
        if (fault & MAX31865_FAULT_OVUV)
        {
            Serial.println(F("Under/Over voltage"));
        }
        max31865[num]->clearFault();

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
#if DIMMER_RELAY_LAST_NUM > 0                                                       // si dimmer
        uint8_t pin = pgm_read_byte(OPT_ARRAY + (DIMMER_RELAY_LAST_NUM + 1 + num)); // decallage de 1 lie au point zero des dimmers
#else
        uint8_t pin = pgm_read_byte(OPT_ARRAY + num); // pas de decallage sinon
#endif
        old_temp[num] = TEMP_DEFAULT_PT100;
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

                if (tmp > MIN_DEFAULT_PT100 && tmp < MAX_DEFAULT_PT100)
                {
                    old_temp[num] = tmp;
                }
            }
        }
    }
    if (now - PT100_callback_time >= MQTT_REFRESH / TEMP_NUM)
    {
        PT100_callback_time = now;
        // if (pt100_callback != nullptr)
        {

            static uint8_t num = 0;
            if (num < TEMP_NUM)
            {
                Serial.print(F("PT100 "));
                Serial.print(num);
                Serial.print(F(" = "));
                Serial.println(old_temp[num]);

                if (pt100_callback != nullptr)
                    pt100_callback(num, old_temp[num]);
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
    if (num < TEMP_NUM)
    {
        return old_temp[num];
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