#include <DT_fake_ntc.h>
#include "Arduino.h"
#include <pinout.h>
#include <SPI.h>

const uint8_t NTC_R1[] PROGMEM = {
    145, 112, 91, 88, 109, 84, 94, 70, 98, 82,
    73, 58, 57, 83, 72, 49, 56, 60, 46, 43,
    39, 41, 35, 35, 31, 32, 32, 37, 33, 25,
    27, 23, 30, 24, 20, 19, 21, 18, 17, 26,
    17, 24, 20, 15, 15, 14, 13, 13, 12, 17,
    11, 11, 11, 10, 14, 10, 9, 9, 9, 10,
    8, 8, 13, 13, 7, 10, 8, 8, 7, 6,
    6, 6, 8, 6, 7, 5, 5, 5, 5, 5,
    5, 5, 5, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 5, 4, 3, 3, 3, 3, 3, 3,
    3};

const uint8_t NTC_R2[] PROGMEM = {
    149, 188, 255, 236, 140, 194, 139, 230, 108, 123,
    133, 204, 180, 83, 89, 173, 106, 85, 127, 132,
    155, 111, 162, 132, 188, 128, 107, 66, 74, 182,
    98, 180, 57, 90, 211, 242, 89, 184, 228, 37,
    111, 35, 44, 116, 89, 109, 155, 108, 172, 30,
    231, 133, 93, 199, 27, 83, 222, 122, 83, 37,
    165, 99, 16, 15, 191, 19, 31, 27, 42, 192,
    97, 64, 16, 38, 18, 231, 98, 62, 45, 35,
    29, 24, 21, 199, 83, 52, 38, 30, 24, 20,
    18, 15, 8, 12, 140, 63, 40, 29, 23, 19,
    16};

uint8_t fake_ntc_value = 0;
uint8_t fake_ntc_new_value;
void (*fake_ntc_callback)(const uint8_t value);

void DT_fake_ntc_init(uint8_t value)
{
    fake_ntc_callback = nullptr;
    pinMode(FAKE_NTC_CS, OUTPUT);
    digitalWrite(FAKE_NTC_CS, HIGH);
    SPI.begin();
    DT_fake_ntc_set(value);
}

void DT_fake_ntc_loop()
{
    static uint32_t old = 0;
    uint32_t now = millis();
    if (now - old > 1000)
    {
        old = now;
        if (fake_ntc_value < fake_ntc_new_value)
        {
            DT_fake_ntc_set((uint8_t)(fake_ntc_value + 1));
        }
        else if (fake_ntc_value > fake_ntc_new_value)
        {
            DT_fake_ntc_set((uint8_t)(fake_ntc_value - 1));
        }
    }
}

void DT_fake_ntc_set(uint8_t value)
{
    if (value <= 100 && fake_ntc_value != value)
    {
        uint8_t r1 = pgm_read_byte(NTC_R1 + value);
        uint8_t r2 = pgm_read_byte(NTC_R2 + value);
        digitalWrite(FAKE_NTC_CS, LOW);
        delay(1);
        SPI.transfer(FAKE_NTC_R1_ADDRESS);
        SPI.transfer(r1);
        digitalWrite(FAKE_NTC_CS, HIGH);
        delay(1);
        digitalWrite(FAKE_NTC_CS, LOW);
        delay(1);
        SPI.transfer(FAKE_NTC_R2_ADDRESS);
        SPI.transfer(r2);
        digitalWrite(FAKE_NTC_CS, HIGH);
        delay(1);
        fake_ntc_value = value;
        fake_ntc_new_value = value;
        if (fake_ntc_callback != nullptr)
        {
            fake_ntc_callback(value);
        }
    }
}

void DT_fake_ntc_slow_set(uint8_t value)
{
    if (value <= 100 && fake_ntc_value != value)
    {
        fake_ntc_new_value = value;
    }
}

uint8_t DT_fake_ntc_get()
{
    return fake_ntc_value;
};

void DT_fake_ntc_callback(void (*callback)(const uint8_t value))
{
    fake_ntc_callback = callback;
}