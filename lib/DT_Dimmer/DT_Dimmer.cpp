
#include "DT_Dimmer.h"
#include "pinout.h"
#include "config.h"
#include "DT_eeprom.h"

//#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + DIMMER_RELAY_NUM > 0

#define EI_NOTINT0
#define EI_NOTINT1
// #include <EnableInterrupt.h>

#include <avr/interrupt.h>

bool default_0 = false;
volatile unsigned int default_0_count = 0;

struct dimmer_light
{
    uint16_t Dimmer_time;       // in millisecond
    uint32_t Dimmer_time_start; // in millisecond
    uint8_t Dimmer_value;       // in Percent
    uint8_t Dimmer_go_value;    // in Percent
    uint8_t Dimmer_old_value;   // in Percent
    uint8_t Dimmer_start_value; // in Percent
    bool Dimmer_candle;
    uint16_t ocr;
};

dimmer_light light[DIMMER_LIGHT_NUM];

#if DIMMER_HEAT_NUM > 0
heat_mode mode[DIMMER_HEAT_NUM];
#endif // DIMMER_HEAT_NUM

#if DIMMER_RELAY_NUM > 0
uint32_t num_dimmer_delay[DIMMER_RELAY_NUM];
#endif

/*
uint16_t to_ocrx(uint8_t num, double value)
{
    if (value == 255)
    {
        return 0;
    }
    else if (value == 0)
    {
        if (num < 13)
            return 65535;
        else
            return 255;
    }
    else
    {
        return (SCALE(value, 1, 254, eeprom_config.Dimmer_scale_min[num], eeprom_config.Dimmer_scale_max[num]));
    }
}
*/
inline void desativation_ocrx(uint8_t num)
{

    switch (num)
    {
    case 0:
        TCCR5A &= 0b00111111; // OPT2 = 46 -> PL3 -> OC5A
        break;

    case 1:
        TCCR3A &= 0b11110011; // OPT3 = 3 -> PE5 -> OC3C
        break;

    case 2:
        TCCR4A &= 0b00111111; // OPT4 = 6 -> PH3 -> OC4A
        break;

    case 3:
        TCCR4A &= 0b11110011; // OPT5 = 8 -> PH5 -> OC4C
        break;

    case 4:
        TCCR1A &= 0b11001111; // OPT6 = 12 -> PB6 -> OC1B
        break;

    case 5:
        TCCR5A &= 0b11001111; // OPT8 = 45 -> PL4 -> OC5B
        break;

    case 6:
        TCCR3A &= 0b00111111; // OPT9 = 5 -> PE3 -> OC3A
        break;

    case 7:
        TCCR4A &= 0b11001111; // OPT10 = 7 -> PH4 -> OC4B
        break;

    case 8:
        TCCR1A &= 0b00111111; // OPT11 = 11 -> PB5 -> OC1A
        break;

    case 9:
        TCCR1A &= 0b11110011; // OPT12 = 13 -> PB7 -> OC1C
        break;

    case 10:
        TCCR5A &= 0b11110011; // OPT13 = 44 -> PL5 -> OC5C
        break;

    case 11:
        TCCR2A &= 0b11001111; // OPT14 = 9 -> PH6 -> OC2B
        break;

    case 12:
        TCCR2A &= 0b00111111; // OPT15 = 10 -> PB4 -> OC2A
        break;

    default:
        break;
    }
}

inline void activation_ocrx(uint8_t num)
{

    switch (num)
    {
    case 0:
        TCCR5A |= 0b11000000; // OPT2 = 46 -> PL3 -> OC5A
        break;

    case 1:
        TCCR3A |= 0b00001100; // OPT3 = 3 -> PE5 -> OC3C
        break;

    case 2:
        TCCR4A |= 0b11000000; // OPT4 = 6 -> PH3 -> OC4A
        break;

    case 3:
        TCCR4A |= 0b00001100; // OPT5 = 8 -> PH5 -> OC4C
        break;

    case 4:
        TCCR1A |= 0b00110000; // OPT6 = 12 -> PB6 -> OC1B
        break;

    case 5:
        TCCR5A |= 0b00110000; // OPT8 = 45 -> PL4 -> OC5B
        break;

    case 6:
        TCCR3A |= 0b11000000; // OPT9 = 5 -> PE3 -> OC3A
        break;

    case 7:
        TCCR4A |= 0b00110000; // OPT10 = 7 -> PH4 -> OC4B
        break;

    case 8:
        TCCR1A |= 0b11000000; // OPT11 = 11 -> PB5 -> OC1A
        break;

    case 9:
        TCCR1A |= 0b00001100; // OPT12 = 13 -> PB7 -> OC1C
        break;

    case 10:
        TCCR5A |= 0b00001100; // OPT13 = 44 -> PL5 -> OC5C
        break;

    case 11:
        TCCR2A |= 0b00110000; // OPT14 = 9 -> PH6 -> OC2B
        break;

    case 12:
        TCCR2A |= 0b11000000; // OPT15 = 10 -> PB4 -> OC2A
        break;

    default:
        break;
    }
}

#define SET_PIN(port, pin, level) \
    if (level == LOW)             \
    {                             \
        port &= ~_BV(pin);        \
    }                             \
    else                          \
    {                             \
        port |= _BV(pin);         \
    }

inline void force_low_opt(uint8_t num)
{

    switch (num)
    {
    case 0: // OPT2 = 46 -> PL3 -> OC5A
        SET_PIN(PORTL, PL3, LOW);
        // SET_PIN(DDRL, PL3, LOW);
        break;

    case 1: // OPT3 = 3 -> PE5 -> OC3C
        SET_PIN(PORTE, PE5, LOW);
        // SET_PIN(DDRE, PE5, LOW);
        break;

    case 2: // OPT4 = 6 -> PH3 -> OC4A
        SET_PIN(PORTH, PH3, LOW);
        // SET_PIN(DDRH, PH3, LOW);
        break;

    case 3: // OPT5 = 8 -> PH5 -> OC4C
        SET_PIN(PORTH, PH5, LOW);
        // SET_PIN(DDRH, PH5, LOW);
        break;

    case 4: // OPT6 = 12 -> PB6 -> OC1B
        SET_PIN(PORTB, PB6, LOW);
        //SET_PIN(DDRB, PB6, LOW);
        break;

    case 5: // OPT8 = 45 -> PL4 -> OC5B
        SET_PIN(PORTL, PL4, LOW);
        // SET_PIN(DDRL, PL4, LOW);
        break;

    case 6: // OPT9 = 5 -> PE3 -> OC3A
        SET_PIN(PORTE, PE3, LOW);
        // SET_PIN(DDRE, PE3, LOW);
        break;

    case 7: // OPT10 = 7 -> PH4 -> OC4B
        SET_PIN(PORTH, PH4, LOW);
        // SET_PIN(DDRH, PH4, LOW);
        break;

    case 8: // OPT11 = 11 -> PB5 -> OC1A
        SET_PIN(PORTB, PB5, LOW);
        // SET_PIN(DDRB, PB5, LOW);
        break;

    case 9: // OPT12 = 13 -> PB7 -> OC1C
        SET_PIN(PORTB, PB7, LOW);
        // SET_PIN(DDRB, PB7, LOW);
        break;

    case 10: // OPT13 = 44 -> PL5 -> OC5C
        SET_PIN(PORTL, PL5, LOW);
        // SET_PIN(DDRL, PL5, LOW);
        break;

    case 11: // OPT14 = 9 -> PH6 -> OC2B
        SET_PIN(PORTH, PH6, LOW);
        // SET_PIN(DDRH, PH6, LOW);
        break;

    case 12: // OPT15 = 10 -> PB4 -> OC2A
        SET_PIN(PORTB, PB4, LOW);
        // SET_PIN(DDRB, PB4, LOW);
        break;

    default:
        break;
    }
}

inline void force_high_opt(uint8_t num)
{

    switch (num)
    {
    case 0: // OPT2 = 46 -> PL3 -> OC5A
        // SET_PIN(DDRL, PL3, HIGH);
        SET_PIN(PORTL, PL3, HIGH);
        break;

    case 1: // OPT3 = 3 -> PE5 -> OC3C
        // SET_PIN(DDRE, PE5, HIGH);
        SET_PIN(PORTE, PE5, HIGH);
        break;

    case 2: // OPT4 = 6 -> PH3 -> OC4A
        // SET_PIN(DDRH, PH3, HIGH);
        SET_PIN(PORTH, PH3, HIGH);
        break;

    case 3: // OPT5 = 8 -> PH5 -> OC4C
        // SET_PIN(DDRH, PH5, HIGH);
        SET_PIN(PORTH, PH5, HIGH);
        break;

    case 4: // OPT6 = 12 -> PB6 -> OC1B
        // SET_PIN(DDRB, PB6, HIGH);
        SET_PIN(PORTB, PB6, HIGH);
        break;

    case 5: // OPT8 = 45 -> PL4 -> OC5B
        // SET_PIN(DDRL, PL4, HIGH);
        SET_PIN(PORTL, PL4, HIGH);
        break;

    case 6: // OPT9 = 5 -> PE3 -> OC3A
        // SET_PIN(DDRE, PE3, HIGH);
        SET_PIN(PORTE, PE3, HIGH);
        break;

    case 7: // OPT10 = 7 -> PH4 -> OC4B
        // SET_PIN(DDRH, PH4, HIGH);
        SET_PIN(PORTH, PH4, HIGH);
        break;

    case 8: // OPT11 = 11 -> PB5 -> OC1A
        // SET_PIN(DDRB, PB5, HIGH);
        SET_PIN(PORTB, PB5, HIGH);
        break;

    case 9: // OPT12 = 13 -> PB7 -> OC1C
        // SET_PIN(DDRB, PB7, HIGH);
        SET_PIN(PORTB, PB7, HIGH);
        break;

    case 10: // OPT13 = 44 -> PL5 -> OC5C
        // SET_PIN(DDRL, PL5, HIGH);
        SET_PIN(PORTL, PL5, HIGH);
        break;

    case 11: // OPT14 = 9 -> PH6 -> OC2B
        // SET_PIN(DDRH, PH6, HIGH);
        SET_PIN(PORTH, PH6, HIGH);
        break;

    case 12: // OPT15 = 10 -> PB4 -> OC2A
        // SET_PIN(DDRB, PB4, HIGH);
        SET_PIN(PORTB, PB4, HIGH);
        break;

    default:
        break;
    }
}

inline void set_ocrx(uint8_t num)
{
    if (light[num].ocr == 0)
    {
        desativation_ocrx(num);
        force_high_opt(num);
    }
    else if (light[num].ocr == 65535)
    {
        desativation_ocrx(num);
        force_low_opt(num);
    }
    else
    {
        switch (num)
        {
        case 0: // OPT2 = 46 -> PL3 -> OC5A
            OCR5A = light[num].ocr;
            break;

        case 1: // OPT3 = 3 -> PE5 -> OC3C
            OCR3C = light[num].ocr;
            break;

        case 2: // OPT4 = 6 -> PH3 -> OC4A
            OCR4A = light[num].ocr;
            break;

        case 3: // OPT5 = 8 -> PH5 -> OC4C
            OCR4C = light[num].ocr;
            break;

        case 4: // OPT6 = 12 -> PB6 -> OC1B
            OCR1B = light[num].ocr;
            break;

        case 5: // OPT8 = 45 -> PL4 -> OC5B
            OCR5B = light[num].ocr;
            break;

        case 6: // OPT9 = 5 -> PE3 -> OC3A
            OCR3A = light[num].ocr;
            break;

        case 7: // OPT10 = 7 -> PH4 -> OC4B
            OCR4B = light[num].ocr;
            break;

        case 8: // OPT11 = 11 -> PB5 -> OC1A
            OCR1A = light[num].ocr;
            break;

        case 9: // OPT12 = 13 -> PB7 -> OC1C
            OCR1C = light[num].ocr;
            break;

        case 10: // OPT13 = 44 -> PL5 -> OC5C
            OCR5C = light[num].ocr;
            break;

        case 11: // OPT14 = 9 -> PH6 -> OC2B
            OCR2B = light[num].ocr;
            break;

        case 12: // OPT15 = 10 -> PB4 -> OC2A
            OCR2A = light[num].ocr;
            break;

        default:
            break;
        }
        activation_ocrx(num);
    }
}

#if DIMMER_LIGHT_NUM > 0
void calc_ocr(uint8_t num, double value)
{
    if (value == 255)
    {
        // desativation_ocrx(num);
        // uint8_t pin = pgm_read_byte(OPT_ARRAY + num + 1);
        // digitalWrite(pin, HIGH);
        light[num].ocr = 0;
        light[num].Dimmer_value = 255;
        // return 0;
    }
    else if (value == 0)
    {
        // desativation_ocrx(num);
        // uint8_t pin = pgm_read_byte(OPT_ARRAY + num + 1);
        // digitalWrite(pin, LOW);
        light[num].ocr = 65535;
        light[num].Dimmer_value = 0;
        // return ICR1;
    }
    else
    {
        light[num].ocr = (SCALE(value, 1, 254, eeprom_config.Dimmer_scale_min[num], eeprom_config.Dimmer_scale_max[num]));
        light[num].Dimmer_value = value;
    }

    // Serial.print(F("Dimmer_value["));
    // Serial.print(num);
    // Serial.print(F("] = "));
    // Serial.println(light[num].Dimmer_value);

    // Serial.print(F("ICR5 = "));
    // Serial.println(ICR5);

    // Serial.print(F("TCCR5A = "));
    // Serial.println(TCCR5A, BIN);

    // Serial.print(F("TCCR5B = "));
    // Serial.println(TCCR5B, BIN);

    // set_ocrx(num);
}
#endif // DIMMER_LIGHT_NUM > 0

void (*_callback_dimmer)(const uint8_t num, const uint8_t percent, const bool candle);

// timer 2 Overflow interrupt service routine
// ISR(TIMER2_OVF_vect)
// {
//     TOV2_count += 1;
// }

// timer compare interrupt service routine (Dimmer 3)
// ISR(TIMER2_COMPA_vect)
// {
//     if (TOV2_count == OCR2A_COUNT)
//     {
//         SET_DIMMER15;
//     }
// }

// timer compare interrupt service routine (Dimmer 4)
// ISR(TIMER2_COMPB_vect)
// {
//     // ATOMIC_BLOCK(ATOMIC_FORCEON)
//     // {
//     if (TOV2_count == OCR2B_COUNT)
//     {
//         SET_DIMMER16;
//     }
//     // }
// }

// PCINT 0-7 interrupt service routine
// ISR(INT2_vect)
void point_zero()
{
    // if (!(PINB & _BV(PB0))) // PCINT0 interrupts on FALLING
    // {
    TCNT2 = 110;   // clear timer2
    TCNT1 = 19995; // clear timer1
    TCNT3 = 19995; // clear timer3
    TCNT4 = 19995; // celar timer4
    TCNT5 = 19995; // clear timer5

    // Serial.println("point 0");

#if DIMMER_LIGHT_NUM > 0
    for (uint8_t num = 0; num < DIMMER_LIGHT_NUM; num++)
    {
        set_ocrx(num);
    }
#endif // DIMMER_LIGHT_NUM

#if DIMMER_HEAT_NUM > 0
    if (PIND & _BV(PD2)) // PCINT0 interrupts on RAISING
    {
        for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num)
        {
            switch (mode[num])
            {
            case off:
                force_opt(num + DIMMER_LIGHT_NUM, HIGH);
                break;

            case sleep:
                force_opt(num + DIMMER_LIGHT_NUM, LOW);
                break;

            default:
                break;
            }
        }
    }
    else // PCINT0 interrupts on FAILLING
    {
        for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num)
        {
            switch (mode[num])
            {
            case off:
                force_opt(num + DIMMER_LIGHT_NUM, LOW);
                break;

            case sleep:
                force_opt(num + DIMMER_LIGHT_NUM, HIGH);
                break;

            default:
                break;
            }
        }
    }
#endif
    ++default_0_count;
}

// initialisation des dimmer
void Dimmer_init(void)
{

    _callback_dimmer = nullptr;

#if DIMMER_LIGHT_NUM > 0
    for (uint8_t num = 0; num < DIMMER_LIGHT_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(DIMMER_ARRAY + num);

        pinMode(pin, OUTPUT);
        desativation_ocrx(num);
        digitalWrite(pin, LOW); // extinction du dimmer
    }

    // pinMode(10, OUTPUT);
    // digitalWrite(10, LOW); // extinction du dimmer

    for (uint8_t i = 0; i < DIMMER_LIGHT_NUM; i++) // init variables
    {
        light[i].Dimmer_value = 0;
        light[i].Dimmer_old_value = 255;
        light[i].ocr = 65535;
        // Dimmer_value[i] = 65535;
        // Dimmer_new_value[i] = 65535;
        // Dimmer_time_interval[i] = 0;
        light[i].Dimmer_candle = false;
    }
#endif

#if DIMMER_HEAT_NUM > 0
    for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num)
    {

        uint8_t pin = pgm_read_byte(DIMMER_ARRAY + DIMMER_LIGHT_NUM + num);

        pinMode(pin, OUTPUT);
        desativation_ocrx(num);
        digitalWrite(pin, LOW); // extinction du dimmer
    }
    for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num) // init variables
    {
        mode[num] = none;
    }
#endif // DIMMER_HEAT_NUM > 0

    noInterrupts(); // disable all interrupts

    // interuption PCINT1  (point zero)
#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM >= 1
    // EIMSK = 0b00000100;  // active INT2 interupt
    // EICRA = 0b00100000;
    // EIMSK = 1<<INT2;
    // EICRA = 1<<ISC21;
    // pinMode(OPT_1, INPUT); // Point zero
    // attachInterrupt(digitalPinToInterrupt(OPT_1), point_zero, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(19), point_zero, RISING);

    pinMode(OPT_7, INPUT); // Point zero
    attachInterrupt(digitalPinToInterrupt(OPT_7), point_zero, RISING);
    // PCMSK0 = 0b00000001; // active pcint at pin 8

    TCCR5A = 0b00000010; // PWM mode Fast PWM
    TCCR5B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK5 = 0b0000000;  // No interuption
    // ICR5 = 19840;        // Timer5 TOP
    ICR5 = 20000; // Timer5 TOP
#endif

#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM >= 2
    TCCR3A = 0b00000010; // PWM mode Fast PWM
    TCCR3B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK3 = 0b0000000;  // No interuption
    // ICR3 = 19840;        // Timer3 TOP
    ICR3 = 20000; // Timer3 TOP
#endif
#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM >= 3
    TCCR4A = 0b00000010; // PWM mode Fast PWM
    TCCR4B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK4 = 0b0000000;  // No interuption
    // ICR4 = 19840;        // Timer4 TOP
    ICR4 = 20000; // Timer4 TOP
#endif
#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM >= 5
    TCCR1A = 0b00000010; // PWM mode Fast PWM
    TCCR1B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK1 = 0b0000000;  // No interuption
    // ICR1 = 19840;        // Timer1 TOP
    ICR1 = 20000; // Timer1 TOP
#endif
#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM >= 12
    TCCR2A = 0b00000011; // PWM mode Fast PWM
    TCCR2B = 0b00000111; // prescale 1024
    TIMSK2 = 0b0000000;  // No interuption
#endif

#if DIMMER_RELAY_NUM > 0
    for (uint8_t num = 0; num < DIMMER_RELAY_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(DIMMER_RELAY_ARRAY + num);

        pinMode(pin, OUTPUT);
        num_dimmer_delay[num] = 0;
    }
#endif

    interrupts();

    // delay(20);
};

// demmarage / extinction du dimmer
// num : numero du dimmer
// power : 255 == 100% | 0 == 0%)
// time : temps pour ataindre la puissance
// candle : mode bougie (non implémanté)
void dimmer_set(uint8_t num, uint8_t value, uint16_t time, bool candle)
{
#if DIMMER_LIGHT_NUM > 0
    /*Serial.print(F("dimmer_set("));
    Serial.print(num);
    Serial.print(F(", "));
    Serial.print(percent);
    Serial.print(F(", "));
    Serial.print(time);
    Serial.print(F(", "));
    Serial.print(candle);
    Serial.println(F(")"));*/
    light[num].Dimmer_candle = candle;
    Serial.print(F("dimmer "));
    Serial.print(num);
    Serial.print(F(" value = "));
    Serial.print(value);
    Serial.print(F(" time = "));
    Serial.print(time);
    Serial.print(F(" candle = "));
    Serial.println(candle);
    // Dimmer_new_value[num] = to_ocrx(num, percent);
    // uint16_t ocrx = set_ocrx(num, percent);
    // Serial.print(F("Dimmer_new_value = "));
    // Serial.println(ocrx);

    // uint8_t interval;
    // if (percent > light[num].Dimmer_value)
    // {
    //     interval = time / (percent - light[num].Dimmer_value);
    // }
    // else
    // {
    //     interval = time / (light[num].Dimmer_value - percent);
    // }

    if (value != 0 && default_0)
    {
        desativation_ocrx(num);
        force_high_opt(num);
        calc_ocr(num, 255);
        light[num].Dimmer_go_value = value;
        light[num].Dimmer_time = 1;
    }
    else if (value == 0 && default_0)
    {
        desativation_ocrx(num);
        force_low_opt(num);
        // Dimmer_value[num] = Dimmer_new_value[num];
        // light[num].Dimmer_value = percent;
        calc_ocr(num, 0);
        light[num].Dimmer_go_value = value;
        light[num].Dimmer_time = 1;
    }
    else if (time == 0)
    {
        // Dimmer_value[num] = Dimmer_new_value[num];
        // light[num].Dimmer_value = percent;
        calc_ocr(num, value);
        light[num].Dimmer_go_value = value;
        light[num].Dimmer_time = 1;
    }
    else
    {
        // uint16_t calc_time;
        // if (value > light[num].Dimmer_value)
        // {
        //     calc_time = (value - light[num].Dimmer_value) * time
        // }
        // else
        // {
        //     calc_time = (light[num].Dimmer_value - value) * time
        // }

        light[num].Dimmer_time = time;           // in millisecond
        light[num].Dimmer_time_start = millis(); // in millisecond
        light[num].Dimmer_go_value = value;
        light[num].Dimmer_start_value = light[num].Dimmer_value;
    }

    if (value != 0)
    {
        light[num].Dimmer_old_value = value;
    }

    // update[num] = true;

    // config.Dimmer_old_value[num] = percent <= DIMMER_MIN ? DIMMER_MIN : percent;

    // if (light[num].Dimmer_candle)
    //     DtCan_Board_send(config.address, num + 9, CAN_BOARD_DIMMER_CANDLE, percent);
    // else
    //     DtCan_Board_send(config.address, num + 9, CAN_BOARD_DIMMER, percent);

    // Serial.print(F("dimmer_set("));
    // Serial.print(num);
    // Serial.print(F(","));
    // Serial.print(percent);
    // Serial.print(F(","));
    // Serial.print(speed);
    // Serial.print(F(") scale = "));
    // Serial.print(scale_percent);
    // Serial.print(F(" value = "));
    // Serial.println(Dimmer_value[num]);

    // Serial.print(F(" calc time  = "));
    // Serial.println(Dimmer_time_interval[num]);
#endif // DIMMER_LIGHT_NUM > 0
}

void dimmer_set(uint8_t num, bool start, uint16_t time, bool candle)
{
#if DIMMER_LIGHT_NUM > 0
    if (start)
        dimmer_set(num, light[num].Dimmer_old_value, time, candle);
    else
        dimmer_set(num, (uint8_t)0, time, candle);
#endif // DIMMER_LIGHT_NUM > 0
}

void dimmer_stop(uint8_t num)
{
#if DIMMER_LIGHT_NUM > 0
    dimmer_set(num, light[num].Dimmer_value, 0, light[num].Dimmer_candle);
#endif // DIMMER_LIGHT_NUM > 0
}

void dimmer_set_heat_mode(uint8_t num, heat_mode Mode)
{
#if DIMMER_HEAT_NUM > 0
    if (num < DIMMER_HEAT_NUM)
    {
        mode[num] = Mode;
        switch (Mode)
        {
        case comfort:
            force_opt(num + DIMMER_LIGHT_NUM, LOW);
            break;

        case eco:
            force_opt(num + DIMMER_LIGHT_NUM, HIGH);
            break;

        default:
            break;
        }
    }
#endif // DIMMER_HEAT_NUM > 0
}

// valeur pour 255 de fonctionnement du dimmer
//  num: numero du dimmer
uint8_t get_dimmer(uint8_t num)
{
    // return light[num].Dimmer_value;
    // if (Dimmer_new_value[num] == 65535)
    // {
    //     return 0;
    // }
    // else
    // {
    //     return light[num].Dimmer_old_value;
    // }
    return light[num].Dimmer_go_value;
}

// valeur en pourcentage de fonctionnement du dimmer
//  num: numero du dimmer
#if DIMMER_HEAT_NUM > 0
heat_mode get_heat_mode(uint8_t num)
{
    if (num < DIMMER_HEAT_NUM)
    {
        return (mode[num]);
    }
    return heat_mode(none);
}
#endif // DIMMER_HEAT_NUM > 0

// true si le dimmer est en mode bougie
bool get_dimmer_candle(uint8_t num)
{
    return light[num].Dimmer_candle;
};

#if DIMMER_RELAY_NUM > 0
// utilisation des dimmer comme relay
void DT_dimmer_relay(uint8_t num, bool state)
{

    Serial.print(F("DT_dimmer_relay( "));
    Serial.print(num);
    Serial.print(F(", "));
    Serial.print(state);
    Serial.println(F(")"));
    
    uint8_t pin = pgm_read_byte(DIMMER_RELAY_ARRAY + num);

    if (state)
    {
        digitalWrite(pin, HIGH);
        // async_call[num] = true;
    }
    else
    {
        digitalWrite(pin, LOW);
        // async_call[num] = true;
    }
}

bool DT_dimmer_relay_get(uint8_t num)
{
    uint8_t pin = pgm_read_byte(DIMMER_RELAY_ARRAY + num);
    return digitalRead(pin);
}

void DT_dimmer_relay(uint8_t num, uint32_t time)
{
    Serial.print(F("DT_dimmer_relay( "));
    Serial.print(num);
    Serial.print(F(", "));
    Serial.print(time);
    Serial.println(F(")"));
    if (num < DIMMER_RELAY_NUM)
    {
        if (time > 0)
        {
            num_dimmer_delay[num] = time;
            DT_dimmer_relay(num, true);
        }
        else
        {
            num_dimmer_delay[num] = 0;
            DT_dimmer_relay(num, false);
        }
    }
}
#endif // DIMMER_RELAY_NUM > 0

// boucle d'execcution du dimmer
void dimmer_loop()
{
    // static unsigned long dimmed_speed_time[DIMMER_LIGHT_NUM];
    // static old_time = 0
    // static unsigned long dimmer_candle_old_time = 0;
    // static unsigned long candle_interval = random(MIN_CANDLE_TIME, MAX_CANDLE_TIME);
    // uint16_t Dimmer_time_interval[DIMMER_LIGHT_NUM];
    static unsigned long default_0_time = 0;

    if (millis() - default_0_time >= 1000 && !default_0)
    {
        default_0_time = millis();
        if (default_0_count != 0)
        {
            default_0_count = 0;
        }
        else
        {
            default_0 = true;
            for (uint8_t num = 0; num < DIMMER_LIGHT_NUM; num++)
            {
                dimmer_set(num, light[num].Dimmer_go_value);
            }
        }
    }

#if DIMMER_LIGHT_NUM > 0
    for (uint8_t num = 0; num < DIMMER_LIGHT_NUM; num++)
    {
        if (light[num].Dimmer_time <=1 )
        {
            continue;
        }
        
        uint32_t time_go = light[num].Dimmer_time_start + (uint32_t)light[num].Dimmer_time;
        /// Dimmed start / stop
        if (millis() < time_go)
        {
            uint8_t value = SCALE(millis(), light[num].Dimmer_time_start, time_go, light[num].Dimmer_start_value, light[num].Dimmer_go_value);

            // Serial.print(F(" calc_ocr  = "));
            // Serial.println(value);
            calc_ocr(num, value);
        }
        else if (light[num].Dimmer_time > 1)
        {
            calc_ocr(num, light[num].Dimmer_go_value);
            light[num].Dimmer_time = 1;
        }
        // else
        // {
        //     calc_ocr(num, light[num].Dimmer_go_value);
        // }

        /// Candle
        if (light[num].Dimmer_candle == true)
        {

            if (light[num].Dimmer_go_value == light[num].Dimmer_value)
            {
                uint8_t candle_percent = random(CANDLE_OFSSET_PERCENTE_MIN, CANDLE_OFSSET_PERCENTE_MAX + 1);
                uint8_t percent_tmp;
                if (candle_percent < light[num].Dimmer_old_value)
                    percent_tmp = light[num].Dimmer_old_value - candle_percent;
                else
                    percent_tmp = 0;

                uint16_t time_tmp = random(CANDLE_SPEED_MIN, CANDLE_SPEED_MAX + 1);
                light[num].Dimmer_time = time_tmp;       // in millisecond
                light[num].Dimmer_time_start = millis(); // in millisecond
                light[num].Dimmer_start_value = light[num].Dimmer_value;
                light[num].Dimmer_go_value = percent_tmp;
            }
        }
    }
#endif // DIMMER_LIGHT_NUM

    // HEAT
    /*
    gestion des modes confort - 1  et confort - 2
    */
#if DIMMER_HEAT_NUM > 0
    static uint32_t heat_interval = 0;
    static uint8_t heat_step = 0;
    if (heat_step == 0 && millis() - heat_interval >= 300000) // toute les 5 minutes (debut de cycle)
    {
        heat_interval = millis();
        heat_step = 1;
        for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num)
        {
            switch (mode[num])
            {
            case confort_1:
            case confort_2:
                force_opt(num + DIMMER_LIGHT_NUM, HIGH);
                break;

            default:
                break;
            }
        }
    }
    else if (heat_step == 1 && millis() - heat_interval >= 3000) // 3 seconde
    {

        heat_step = 2;
        for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num)
        {
            switch (mode[num])
            {
            case confort_1:
                force_opt(num + DIMMER_LIGHT_NUM, LOW);
                break;

            default:
                break;
            }
        }
    }
    else if (heat_step == 2 && millis() - heat_interval >= 7000) // 7 seconde
    {

        heat_step = 0;
        for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num)
        {
            switch (mode[num])
            {
            case confort_2:
                force_opt(num + DIMMER_LIGHT_NUM, LOW);
                break;

            default:
                break;
            }
        }
    }
#endif // DIMMER_HEAT_NUM > 0

#if DIMMER_RELAY_NUM > 0
    static uint32_t last = 0;
    uint32_t now = millis();
    uint32_t elapse = now - last;
    // static uint8_t async_num_callback = 0;
    last = now;

    for (uint8_t num = 0; num < DIMMER_RELAY_NUM; ++num)
    {
        if (num_dimmer_delay[num] == 0)
        {
            continue;
        }
        else if (num_dimmer_delay[num] <= elapse)
        {
            num_dimmer_delay[num] = 0;
            DT_dimmer_relay(num, false);
        }
        else
        {
            num_dimmer_delay[num] -= elapse;
        }
    }
#endif // DIMER_RELAY_NUM > 0

    // appel du callback
    static uint8_t async_num = 0;
    if (_callback_dimmer != nullptr)
    {
        if (light[async_num].Dimmer_time == 1)
        {
            _callback_dimmer(async_num, get_dimmer(async_num), light[async_num].Dimmer_candle);
            light[async_num].Dimmer_time = 0;
        }

        if (++async_num == DIMMER_LIGHT_NUM)
        {
            async_num = 0;
        }
    }
}

void set_dimmer_callback(void (*callback)(const uint8_t num, const uint8_t percent, const bool candle))
{
    _callback_dimmer = callback;
}

//#endif // DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + DIMMER_RELAY_NUM