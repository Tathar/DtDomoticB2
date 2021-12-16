#include <DT_PT100.h>
#include "Arduino.h"

#include <Adafruit_MAX31865.h>

#define MIN_DEFAULT_PT100 -100 // si la temperature est inferieur, on considere que la PT100 est en default
#define MAX_DEFAULT_PT100 200  // si la temperature est superieur, on considere que la PT100 est en default

Adafruit_MAX31865 temp[2] = {Adafruit_MAX31865(11), Adafruit_MAX31865(12)};

float old_temp[TEMP_NUM];
void (*pt100_callback)(const uint8_t num, const float temp);

float _temp_get(int num)
{
    Serial.print("Temperature N°");
    Serial.print(num + 1);

    uint8_t fault = temp[num].readFault();

    if (fault)
    {

        Serial.print(" Fault 0x");
        Serial.println(fault, HEX);
        // sprintf(buffer, "with %%p:  x    = %p\n", &capteur);
        // Serial.print(buffer);
        if (fault & MAX31865_FAULT_HIGHTHRESH)
        {
            Serial.println("RTD High Threshold");
        }
        if (fault & MAX31865_FAULT_LOWTHRESH)
        {
            Serial.println("RTD Low Threshold");
        }
        if (fault & MAX31865_FAULT_REFINLOW)
        {
            Serial.println("REFIN- > 0.85 x Bias");
        }
        if (fault & MAX31865_FAULT_REFINHIGH)
        {
            Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
        }
        if (fault & MAX31865_FAULT_RTDINLOW)
        {
            Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
        }
        if (fault & MAX31865_FAULT_OVUV)
        {
            Serial.println("Under/Over voltage");
        }
        temp[num].clearFault();
        return TEMP_DEFAULT_PT100;
    }
    else
    {
        Serial.print(" return ");
        float tmp = temp[num].temperature(100, TEMP_RREF);
        Serial.print(tmp);
        Serial.println("°C");
        // sprintf(buffer, "with %%p:  x    = %p\n", &capteur);
        // Serial.print(buffer);
        if (tmp < MIN_DEFAULT_PT100 || tmp > MAX_DEFAULT_PT100)
            return TEMP_DEFAULT_PT100;

        return tmp;
    }
}

void DT_pt100_init()
{
    pt100_callback = nullptr;
    for (uint8_t num = 0; num < TEMP_NUM; ++num)
    {
        old_temp[num] = 0;
        // uint8_t pin = pgm_read_byte(TEMP_ARRAY + num);
        // pinMode(pin, OUTPUT);
        // digitalWrite(pin, HIGH);
        temp[num].begin(MAX31865_3WIRE);
    }
}

void DT_pt100_loop()
{
    // delay(50);
    static uint32_t old_time = 0;
    uint32_t now = millis();
    float tmp = 0;
    if (now - old_time > 1000)
    {
        old_time = now;
        for (uint8_t num = 0; num < 2; num++)
        {
            tmp = _temp_get(num);
            old_temp[num] = tmp;
            if (pt100_callback != nullptr)
            {
                pt100_callback(num + 1, tmp);
            }
        }
    }
}

float DT_pt100_get(uint8_t num)
{
    return old_temp[num - 1];
}

void DT_pt100_set_callback(void (*callback)(const uint8_t num, const float temp))
{
    pt100_callback = callback;
}