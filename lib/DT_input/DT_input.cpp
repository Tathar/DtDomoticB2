#include <DT_input.h>
#include "Arduino.h"

#include <pinout.h>
#include <config.h>

#include <DT_mcp.h>
#include <Wire.h>

uint32_t debounce_start_time[INPUT_NUM];
uint8_t old_pin_stats[INPUT_NUM];

void (*input_callback)(const uint8_t num, const uint8_t action);

void DT_input_init()
{
    input_callback = nullptr;

    pinMode(MCP_PIN_INTERUPT, INPUT_PULLUP);

    Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
    Wire.write(MCP_CHANNEL);
    Wire.endTransmission();

    // Wire.requestFrom(I2C_MULTIPLEXER_ADDRESS, 1);
    // if (!Wire.available())
    // {
    //     Serial.println(F("Multiplexer error"));
    //     return; // Error
    // }

    // Serial.print(F("Multiplexer settings = "));
    // Serial.println(Wire.read(), BIN);

    for (uint8_t num = 0; num < INPUT_NUM; ++num)
    {
        uint16_t pin = pgm_read_word(INPUT_ARRAY + num);
        if (pin >= 100)
        {
            uint8_t i2c = pin / 100;
            pin -= i2c * 100;
            i2c -= 1;
            mcp[i2c].pinMode(pin, INPUT);
            mcp[i2c].setupInterrupts(false, true, HIGH);
            mcp[i2c].setupInterruptPin(pin, CHANGE);
        }
        else
        {
            pinMode(pin, INPUT);
        }
    }
}

uint8_t DT_input_get(int num)
{
    return old_pin_stats[num - 1];
}

void DT_input_loop()
{
    uint32_t now = millis();

    Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
    Wire.write(MCP_CHANNEL);
    Wire.endTransmission();

    bool as_interrupt = false;

    if (!digitalRead(MCP_PIN_INTERUPT))
    {
        as_interrupt = true;
        // Serial.println(F("MCP Interrupt"));
    }

    for (uint8_t num = 0; num < INPUT_NUM; ++num)
    {
        uint16_t pin = pgm_read_word(INPUT_ARRAY + num);
        bool revert = pgm_read_byte(INPUT_REVERT + num);

        ////auto Serial.println(pin);
        ////auto Serial.println(revert);
        uint8_t pin_stats;

        if (pin >= 100)
        {
            if (!as_interrupt) // si pas d interuption (pullup)
                continue;

            // Serial.println(F("MCP2308 input not implemented"));
            // Serial.println(pin);
            uint8_t i2c = pin / 100;
            pin -= i2c * 100;
            i2c -= 1;
            pin_stats = mcp[i2c].digitalRead(pin);
            // Serial.println(pin);
            // Serial.println(i2c);
        }
        else
        {
            pin_stats = digitalRead(pin);
        }

        if (revert)
            pin_stats = !pin_stats;

        if (pin_stats != old_pin_stats[num])
        {
            if (pin_stats == HIGH && now - debounce_start_time[num] >= DEBOUNCE_TIME) // Raise UP no debounced
            {
                Serial.print("input");
                Serial.print(num);
                Serial.print(" pin = ");
                Serial.println(pin);

                old_pin_stats[num] = pin_stats;
                debounce_start_time[num] = now; // demmarage du timer de debounce
                if (input_callback != nullptr)
                {
                    input_callback(num + 1, pin_stats);
                }
            }

            else if (pin_stats == LOW && now - debounce_start_time[num] >= DEBOUNCE_TIME) // Raise DOWN no debounced
            {
                old_pin_stats[num] = pin_stats;
                debounce_start_time[num] = now; // demmarage du timer de debounce
                if (input_callback != nullptr)
                {
                    input_callback(num + 1, pin_stats);
                }
            }
        }
    }
}

void DT_input_set_callback(void (*callback)(const uint8_t num, const uint8_t action))
{
    input_callback = callback;
}