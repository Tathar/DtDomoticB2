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

const uint8_t NTC_R2[] PROGMEM = {145, 112, 91, 88, 109, 84, 94, 70, 98, 82,
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
uint8_t fake_ntc_value = 0;

void (*fake_ntc_callback)(const uint8_t value);

void DT_fake_ntc_init(uint8_t value)
{
    fake_ntc_callback = nullptr;
    pinMode(FAKE_NTC_CS, OUTPUT);
    digitalWrite(FAKE_NTC_CS, HIGH);
    SPI.begin();
    DT_fake_ntc_set(value);
}

void DT_fake_ntc_set(uint8_t value)
{
    uint8_t r1 = pgm_read_byte(NTC_R1 + value);
    uint8_t r2 = pgm_read_byte(NTC_R2 + value);
    digitalWrite(FAKE_NTC_CS, LOW);
    SPI.transfer(FAKE_NTC_R1_ADDRESS);
    SPI.transfer(r1);
    SPI.transfer(FAKE_NTC_R2_ADDRESS);
    SPI.transfer(r2);
    digitalWrite(FAKE_NTC_CS, HIGH);
    fake_ntc_value = value;
    if (fake_ntc_callback != nullptr)
    {
        fake_ntc_callback(value);
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