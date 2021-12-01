#include <DT_input.h>
#include "Arduino.h"

#include <../../src/pinout.h>
#include <../../src/config.h>

uint32_t debounce_start_time[INPUT_NUM];
uint8_t old_pin_stats[INPUT_NUM];

void (*input_callback)(const uint8_t num, const bool action);

void DT_input_init()
{
    input_callback = NULL;

    for (uint8_t num = 0; num < INPUT_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(INPUT_ARRAY + num);
        pinMode(pin, INPUT);
    }
}

uint8_t DT_input_get(int num)
{
    return old_pin_stats[num - 1];
}

void DT_input_loop()
{
    uint32_t now = millis();

    for (uint8_t num = 0; num < INPUT_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(INPUT_ARRAY + num);
        bool revert = pgm_read_byte(INPUT_REVERT + num);
        uint8_t pin_stats = digitalRead(pin);

        if (revert)
            pin_stats = !pin_stats;

        if (pin_stats != old_pin_stats[num])
        {
            if (pin_stats == HIGH && now - debounce_start_time[num] >= DEBOUNCE_TIME) // Raise UP no debounced
            {
                old_pin_stats[num] = pin_stats;
                debounce_start_time[num] = now; // demmarage du timer de debounce
                if (input_callback != nullptr)
                {
                    input_callback(num, pin_stats);
                }
            }

            else if (pin_stats == LOW && now - debounce_start_time[num] >= DEBOUNCE_TIME) // Raise DOWN no debounced
            {
                old_pin_stats[num] = pin_stats;
                debounce_start_time[num] = now; // demmarage du timer de debounce
                if (input_callback != nullptr)
                {
                    input_callback(num, pin_stats);
                }
            }
        }
    }
}

void DT_input_set_callback(void (*callback)(const uint8_t num, const bool action))
{
    input_callback = callback;
}