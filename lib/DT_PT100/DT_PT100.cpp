#include <DT_PT100.h>
#include "Arduino.h"
#include "config.h"

#include <Adafruit_MAX31865.h>

#define MIN_DEFAULT_PT100 -100 // si la temperature est inferieur, on considere que la PT100 est en default
#define MAX_DEFAULT_PT100 200  // si la temperature est superieur, on considere que la PT100 est en default

Adafruit_MAX31865 *max31865[TEMP_NUM];
// Adafruit_MAX31865 max31865[20] = {Adafruit_MAX31865(23),
//                                   Adafruit_MAX31865(22),
//                                   Adafruit_MAX31865(25),
//                                   Adafruit_MAX31865(24),
//                                   Adafruit_MAX31865(27),
//                                   Adafruit_MAX31865(26),
//                                   Adafruit_MAX31865(29),
//                                   Adafruit_MAX31865(28),
//                                   Adafruit_MAX31865(31),
//                                   Adafruit_MAX31865(32),
//                                   Adafruit_MAX31865(33),
//                                   Adafruit_MAX31865(30),
//                                   Adafruit_MAX31865(35),
//                                   Adafruit_MAX31865(34),
//                                   Adafruit_MAX31865(37),
//                                   Adafruit_MAX31865(36),
//                                   Adafruit_MAX31865(39),
//                                   Adafruit_MAX31865(38),
//                                   Adafruit_MAX31865(41),
//                                   Adafruit_MAX31865(40)};

float old_temp[TEMP_NUM];
// uint32_t PT100_callback_time[TEMP_NUM];
void (*pt100_callback)(const uint8_t num, const float temp);

float _temp_get(int num)
{
    ////auto Serial.print("Temperature N°");
    ////auto Serial.print(num + 1);

    float tmp = max31865[num]->temperature(100, TEMP_RREF);
    // sprintf(buffer, "with %%p:  x    = %p\n", &capteur);
    ////auto Serial.print(buffer);

    ////auto Serial.print(" return ");
    if (tmp < MIN_DEFAULT_PT100 || tmp > MAX_DEFAULT_PT100)
    {
        ////auto Serial.print(TEMP_DEFAULT_PT100);
        ////auto Serial.println("°C");
        // max31865[num]->reconfigure();
        // max31865[num]->reconfigure();
        return TEMP_DEFAULT_PT100;
    }

    if (tmp == 0)
    {
        ////auto Serial.print(TEMP_DEFAULT_PT100);
        ////auto Serial.println("°C");
        // max31865[num]->reconfigure();
        // max31865[num]->reconfigure();
    }
    ////auto Serial.println("°C");
    int32_t digit = tmp * 100;
    return digit / 100.0;
}

void DT_pt100_init()
{
    // auto Serial.print("alocation memoire = ");
    //  temp = (Adafruit_MAX31865 **)malloc(sizeof(Adafruit_MAX31865 *) * TEMP_NUM);
    // auto Serial.println(sizeof(Adafruit_MAX31865) * TEMP_NUM);
    // auto Serial.println("set callback an nullptr");
    pt100_callback = nullptr;
    for (uint8_t num = 0; num < TEMP_NUM; ++num)
    {

        ////auto Serial.print("pgm read pin = ");
#if DIMMER_NUM >= 1                                                  // si dimmer
        uint8_t pin = pgm_read_byte(OPT_ARRAY + (DIMMER_NUM + num)); // decallage de 1 lie au dimmer
#else
        uint8_t pin = pgm_read_byte(OPT_ARRAY + num); // pas de decallage sinon
#endif
        // auto Serial.println(pin);
        ////auto Serial.println("creation object");
        // temp[num] = new Adafruit_MAX31865(pin);
        ////auto Serial.println("old_temp = 0");
        old_temp[num] = 0;
        // uint8_t pin = pgm_read_byte(TEMP_ARRAY + num);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);

        max31865[num] = new Adafruit_MAX31865(pin);
        ////auto Serial.println("adafruit.begin()");
        max31865[num]->begin(MAX31865_3WIRE);
        ////auto Serial.println("fin begin");
    }
}

void DT_pt100_loop()
{
    // delay(50);
    static uint32_t old_time = 0;
    static uint32_t PT100_callback_time = 0;
    uint32_t now = millis();
    float tmp = 0;
    if (now - old_time > 1000)
    {
        debug(F(AT));

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

        debug(F(AT));
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