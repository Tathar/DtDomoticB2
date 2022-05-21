#ifndef DtDimmer_H
#define DtDimmer_H

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

enum heat_mode
{
    none,
    off,
    comfort,
    confort_1,
    confort_2,
    eco,
    sleep
};

// init all input/output and values
void Dimmer_init(void);

void dimmer_loop();

// update OCR1A counter
void dimmer_debug(void);

// void set_dimmer1(uint8_t percent);
// void set_dimmer2(uint8_t percent);
// void set_dimmer3(uint8_t percent);
// void set_dimmer4(uint8_t percent);

void dimmer_set(uint8_t num, uint8_t percent, uint16_t time = 0, bool candle = false);
void dimmer_set(uint8_t num, bool start, uint16_t time = 0, bool candle = false);
void dimmer_heat(uint8_t num, heat_mode mode);
// void dimmer_on(uint8_t num);
// void dimmer_off(uint8_t num);
// void dimmer_slow_set(uint8_t num, uint8_t percent);
// void dimmer_slow_on(uint8_t num);
// void dimmer_slow_off(uint8_t num);
// void dimmer_candle_on(uint8_t num);
// void dimmer_candle_off(uint8_t num);
// uint8_t dimmer_up(uint8_t num);
// uint8_t dimmer_down(uint8_t num);

// //get dimmer 1 value on percent
// uint8_t get_dimmer1();

// //get dimmer 2 value on percent
// uint8_t get_dimmer2();

// //get dimmer 3 value on percent
// uint8_t get_dimmer3();

// //get dimmer 4 value on percent
// uint8_t get_dimmer4();

// get "num" dimmer value on percent

uint8_t get_dimmer(uint8_t num);
// uint8_t get_dimmer_old_value(uint8_t num);
bool get_dimmer_candle(uint8_t num);

void set_dimmer_callback(void (*callback)(const uint8_t num, const uint8_t percent, const bool candle));

#endif