#include <DT_input.h>
#include "Arduino.h"

#include <pinout.h>
#include <config.h>

#include <DT_mcp.h>
#include <Wire.h>

#define F_LP _BV(0)   // numero du bit LONG_PUSH dans l'octet
#define F_LLP _BV(1)  // numero du bit LONG_LONG_PUSH dans l'octet
#define F_XLLP _BV(2) // numero du bit XL_LONG_PUSH dans l'octet
#define F_all_long_push (F_LP | F_LLP | F_XLLP)

uint32_t debounce_start_time[INPUT_NUM];
uint32_t long_push_start_time[INPUT_NUM];
uint32_t multiple_push_start_time[INPUT_NUM];
uint8_t push_count[INPUT_NUM];
uint8_t input_front[INPUT_NUM];
uint8_t old_pin_stats[INPUT_NUM];
Bt_Action stats[INPUT_NUM];

void (*input_callback)(const uint8_t num, const Bt_Action action);

void DT_input_init()
{
    input_callback = nullptr;

    pinMode(MCP_PIN_INTERUPT, INPUT_PULLUP);

#ifdef INTERNAL_INPUT_I2C
    Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
    Wire.write(MCP_CHANNEL);
    Wire.endTransmission();
#endif

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
        bool revert = pgm_read_byte(INPUT_REVERT + num);
        if (pin >= 100)
        {
#ifdef INTERNAL_INPUT_I2C
            uint8_t i2c = pin / 100;
            pin -= i2c * 100;
            i2c -= 1;
            // mcp[i2c].pinMode(pin, INPUT);
            // mcp[i2c].setupInterrupts(false, true, HIGH);
            // mcp[i2c].setupInterruptPin(pin, CHANGE);

            DT_mcp_pinMode(i2c, pin, INPUT);
            DT_mcp_setupInterrupts(i2c, false, true, HIGH);
            DT_mcp_setupInterruptPin(i2c, pin, CHANGE);
#endif
        }
        else
        {
            pinMode(pin, INPUT);
        }

        debounce_start_time[num] = 0;
        long_push_start_time[num] = 0;
        multiple_push_start_time[num] = 0;
        push_count[num] = 0;
        input_front[num] = 0;

        // initialisation
        uint8_t pin_stats;

        if (pin >= 100)
        {
#ifdef INTERNAL_INPUT_I2C
            uint8_t i2c = pin / 100;
            pin -= i2c * 100;
            i2c -= 1;
            // pin_stats = mcp[i2c].digitalRead(pin);
            pin_stats = DT_mcp_digitalRead(i2c, pin);
            // Serial.println(pin);
            // Serial.println(i2c);
#endif
        }
        else
        {
            pin_stats = digitalRead(pin);
        }

        if (revert)
            pin_stats = !pin_stats;

        stats[num] = pin_stats == HIGH ? IN_PUSHED : IN_RELEASE;
        old_pin_stats[num] = pin_stats;
    }
}

uint8_t DT_input_get_pin_stats(int num)
{
    return old_pin_stats[num];
}

Bt_Action DT_input_get_stats(int num)
{
    return stats[num];
}

void DT_input_loop()
{
    debug(F(AT));
    uint32_t now = millis();

    // #ifdef INTERNAL_INPUT_I2C
    //     Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
    //     Wire.write(MCP_CHANNEL);
    //     Wire.endTransmission();
    // #endif

    bool as_interrupt = false;

#ifdef INTERNAL_INPUT_I2C
    // if (!digitalRead(MCP_PIN_INTERUPT))
    // {
    //     as_interrupt = true;
    //     // Serial.println(F("MCP Interrupt"));
    // }

    uint8_t datas[8] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
#endif

    for (uint8_t num = 0; num < INPUT_NUM; ++num)
    {
        uint16_t pin = pgm_read_word(INPUT_ARRAY + num);
        bool revert = pgm_read_byte(INPUT_REVERT + num);

        ////auto Serial.println(pin);
        ////auto Serial.println(revert);
        uint8_t pin_stats;

        if (pin >= 100)
        {
#ifdef INTERNAL_INPUT_I2C
            // if (as_interrupt) // si interuption (pullup)
            // {
            //     uint8_t i2c = pin / 100;
            //     pin -= i2c * 100;
            //     i2c -= 1;
            //     pin_stats = DT_mcp_digitalRead(i2c, pin);
            // }

            if (pin % 100 == 0)
            {
                uint8_t i2c = pin / 100;
                // pin -= i2c * 100;
                i2c -= 1;
                // // Serial.println("ici 1");
                DT_mcp_digitalReads(i2c, &datas); //lecture des 8 entrées
            }

            if (pin % 100 < 8) {
                // Serial.println("ici 2");
                pin_stats = datas[pin % 100];
            }

#endif
        }
        else
        {
            pin_stats = digitalRead(pin);
        }

        if (revert)
            pin_stats = !pin_stats;

        if (pin_stats != old_pin_stats[num]) // front
        {
            old_pin_stats[num] = pin_stats;
            debounce_start_time[num] = now;
        }
        else if (debounce_start_time[num] != 0 && now - debounce_start_time[num] >= DEBOUNCE_TIME) // debounced front
        {
            if (pin_stats == HIGH) // Raise UP
            {
                debounce_start_time[num] = 0;
                long_push_start_time[num] = now;   // demarage du timer d appuie long
                multiple_push_start_time[num] = 0; // arret du timer d appuie multiple
                push_count[num] += 1;
                stats[num] = IN_PUSHED; // 1
                if (input_callback != nullptr)
                {
                    input_callback(num, stats[num]);
                }
            }

            else if (pin_stats == LOW) // Raise DOWN
            {
                // Serial.println(F("Relese"));
                debounce_start_time[num] = 0;
                if (input_front[num] & F_all_long_push)
                    push_count[num] = 0;

                input_front[num] = 0;                               // mise a zero du front d appuie long
                multiple_push_start_time[num] = now == 0 ? 1 : now; // demarage du timer d appuie multiple
                stats[num] = IN_RELEASE;                            // 2
                if (input_callback != nullptr)
                {
                    input_callback(num, stats[num]);
                }
            }
        }
        else if (debounce_start_time[num] == 0) // debounced no change
        {
            if (multiple_push_start_time[num] != 0 && now - multiple_push_start_time[num] > MULTIPLE_PUSH_TIME) // multiple push timer raise
            {
                multiple_push_start_time[num] = 0;
                Bt_Action ret = IN_NULL;
                // Serial.println(F("Multiple Push"));
                if (push_count[num] > 0)
                {
                    // Serial.print(F(" = "));
                    // Serial.println(push_count);
                    ret = (Bt_Action)((int)IN_PUSH + ((push_count[num] - 1) * 4));
                }
                // Serial.println(F(""));

                push_count[num] = 0;

                if (ret != IN_NULL)
                {
                    stats[num] = ret;
                    if (input_callback != nullptr)
                    {
                        input_callback(num, ret);
                    }
                }
            }
            else if (pin_stats == HIGH &&
                     !(input_front[num] & F_LP) &&
                     long_push_start_time[num] != 0 &&
                     now - long_push_start_time[num] >= LONG_PUSH_TIME &&
                     now - long_push_start_time[num] < LONG_LONG_PUSH_TIME) // long push
            {
                // long_push_start_time = 0;
                input_front[num] = input_front[num] | F_LP; // mise a 1 du front LONG PUSH
                Bt_Action ret = IN_NULL;
                // Serial.println(F("Long Push"));
                if (push_count[num] > 0)
                {
                    // Serial.print(F(" = "));
                    // Serial.print(push_count);
                    ret = (Bt_Action)((int)IN_LPUSH + ((push_count[num] - 1) * 4));
                }
                // push_count = 0;
                // Serial.println(F(""));
                if (ret != IN_NULL)
                {
                    stats[num] = ret;
                    if (input_callback != nullptr)
                    {
                        input_callback(num, stats[num]);
                    }
                }
            }
            else if (pin_stats == HIGH &&
                     !(input_front[num] & F_LLP) &&
                     now - long_push_start_time[num] >= LONG_LONG_PUSH_TIME) // long push
            {
                input_front[num] = input_front[num] | F_LLP; // mise a 1 du front LONG LONG PUSH
                Bt_Action ret = IN_NULL;
                // Serial.println(F("Long Long Push"));
                if (push_count[num] > 0)
                {
                    // Serial.print(F(" = "));
                    // Serial.print(push_count);
                    ret = (Bt_Action)((int)IN_LLPUSH + ((push_count[num] - 1) * 4));
                }
                // push_count = 0;
                // Serial.println(F(""));

                if (ret != IN_NULL)
                {
                    stats[num] = ret;
                    if (input_callback != nullptr)
                    {
                        input_callback(num, stats[num]);
                    }
                }
            }
            else if (pin_stats == HIGH &&
                     !(input_front[num] & F_XLLP) &&
                     now - long_push_start_time[num] >= XL_LONG_PUSH_TIME) // long push
            {
                input_front[num] = input_front[num] | F_XLLP; // mise a 1 du front XL LONG PUSH
                Bt_Action ret = IN_NULL;
                // Serial.println(F("XL Long Push"));
                if (push_count[num] > 0)
                {
                    // Serial.print(F(" = "));
                    // Serial.print(push_count);
                    ret = (Bt_Action)((int)IN_XLLPUSH + ((push_count[num] - 1) * 4));
                }
                // push_count = 0;
                //  Serial.println(F(""));

                if (ret != IN_NULL)
                {
                    stats[num] = ret;
                    if (input_callback != nullptr)
                    {
                        input_callback(num, stats[num]);
                    }
                }
            }
        }
    }
    debug(F(AT));
}

void DT_input_set_callback(void (*callback)(const uint8_t num, const Bt_Action action))
{
    input_callback = callback;
}