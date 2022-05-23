#include "DT_Dimmer.h"
#include "pinout.h"
#include "config.h"
#include "DT_eeprom.h"

#define EI_NOTINT0
#define EI_NOTINT1
// #include <EnableInterrupt.h>

#include <avr/interrupt.h>

bool default_0 = false;
volatile unsigned int default_0_count = 0;

struct dimmer_light
{
    uint16_t Dimmer_time;         // in millisecond
    uint32_t Dimmer_time_start;   // in millisecond
    uint8_t Dimmer_percent;       // in Percent
    uint8_t Dimmer_go_percent;    // in Percent
    uint8_t Dimmer_old_percent;   // in Percent
    uint8_t Dimmer_start_percent; // in Percent
    bool Dimmer_candle;
};

dimmer_light light[DIMMER_LIGHT_NUM];

#if DIMMER_HEAT_NUM > 0
heat_mode mode[DIMMER_HEAT_NUM];
#endif // DIMMER_HEAT_NUM


#define SCALE(val, in_min, in_max, out_min, out_max) (((double)val - (double)in_min) * ((double)out_max - (double)out_min) / ((double)in_max - (double)in_min)) + out_min

uint16_t to_ocrx(uint8_t dimmer_num, double percent)
{
    if (percent >= 100)
    {
        return 0;
    }
    else if (percent == 0)
    {
        if (dimmer_num < 13)
            return 65535;
        else
            return 255;
    }
    else
    {
        // if (dimmer_num < 12)
        // {
        return (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
        // }
        // else
        // {
        //     return (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num])); // low resolution
        // }
    }
}

inline void desativation_ocrx(uint8_t dimmer_num)
{

    switch (dimmer_num)
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
        TCCR3A &= 0b11001111; // OPT7 = 2 -> PE4 -> OC3B
        break;

    case 6:
        TCCR5A &= 0b11001111; // OPT8 = 45 -> PL4 -> OC5B
        break;

    case 7:
        TCCR3A &= 0b00111111; // OPT9 = 5 -> PE3 -> OC3A
        break;

    case 8:
        TCCR4A &= 0b11001111; // OPT10 = 7 -> PH4 -> OC4B
        break;

    case 9:
        TCCR1A &= 0b00111111; // OPT11 = 11 -> PB5 -> OC1A
        break;

    case 10:
        TCCR1A &= 0b11110011; // OPT12 = 13 -> PB7 -> OC1C
        break;

    case 11:
        TCCR5A &= 0b11110011; // OPT13 = 44 -> PL5 -> OC5C
        break;

    case 12:
        TCCR2A &= 0b11001111; // OPT14 = 9 -> PH6 -> OC2B
        break;

    case 13:
        TCCR2A &= 0b00111111; // OPT15 = 10 -> PB4 -> OC2A
        break;

    default:
        break;
    }
}

inline void activation_ocrx(uint8_t dimmer_num)
{

    switch (dimmer_num)
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
        TCCR3A |= 0b00110000; // OPT7 = 2 -> PE4 -> OC3B
        break;

    case 6:
        TCCR5A |= 0b00110000; // OPT8 = 45 -> PL4 -> OC5B
        break;

    case 7:
        TCCR3A |= 0b11000000; // OPT9 = 5 -> PE3 -> OC3A
        break;

    case 8:
        TCCR4A |= 0b00110000; // OPT10 = 7 -> PH4 -> OC4B
        break;

    case 9:
        TCCR1A |= 0b11000000; // OPT11 = 11 -> PB5 -> OC1A
        break;

    case 10:
        TCCR1A |= 0b00001100; // OPT12 = 13 -> PB7 -> OC1C
        break;

    case 11:
        TCCR5A |= 0b00001100; // OPT13 = 44 -> PL5 -> OC5C
        break;

    case 12:
        TCCR2A |= 0b00110000; // OPT14 = 9 -> PH6 -> OC2B
        break;

    case 13:
        TCCR2A |= 0b11000000; // OPT15 = 10 -> PB4 -> OC2A
        break;

    default:
        break;
    }
}

void set_ocrx(uint8_t dimmer_num, double percent)
{
    if (percent >= 100)
    {
        desativation_ocrx(dimmer_num);
        uint8_t pin = pgm_read_byte(OPT_ARRAY + dimmer_num + 1);
        digitalWrite(pin, HIGH);
        light[dimmer_num].Dimmer_percent = 100;
        // return 0;
    }
    else if (percent == 0)
    {
        desativation_ocrx(dimmer_num);
        uint8_t pin = pgm_read_byte(OPT_ARRAY + dimmer_num + 1);
        digitalWrite(pin, LOW);
        light[dimmer_num].Dimmer_percent = 0;
        // return ICR1;
    }
    else
    {
        switch (dimmer_num)
        {
        case 0:
            OCR5A = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 1:
            OCR3C = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 2:
            OCR4A = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 3:
            OCR4C = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 4:
            OCR1B = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 5:
            OCR3B = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 6:
            OCR5B = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 7:
            OCR3A = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 8:
            OCR4B = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 9:
            OCR1A = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 10:
            OCR1C = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 11:
            OCR5C = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 12:
            OCR2B = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        case 13:
            OCR2A = (SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]));
            break;

        default:
            break;
        }
        activation_ocrx(dimmer_num);
        light[dimmer_num].Dimmer_percent = percent;
        // return SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]);
    }

    Serial.print(F("Dimmer_percent["));
    Serial.print(dimmer_num);
    Serial.print(F("] = "));
    Serial.println(light[dimmer_num].Dimmer_percent);

    Serial.print(F("ICR5 = "));
    Serial.println(ICR5);

    Serial.print(F("TCCR5A = "));
    Serial.println(TCCR5A, BIN);

    Serial.print(F("TCCR5B = "));
    Serial.println(TCCR5B, BIN);
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

inline void force_opt(uint8_t dimmer_num, uint8_t level)
{

    switch (dimmer_num)
    {
    case 0: // OPT2 = 46 -> PL3 -> OC5A
        SET_PIN(PORTL, PL3, level);
        break;

    case 1: // OPT3 = 3 -> PE5 -> OC3C
        SET_PIN(PORTE, PE5, level);
        break;

    case 2: // OPT4 = 6 -> PH3 -> OC4A
        SET_PIN(PORTH, PH3, level);
        break;

    case 3: // OPT5 = 8 -> PH5 -> OC4C
        SET_PIN(PORTH, PH5, level);
        break;

    case 4: // OPT6 = 12 -> PB6 -> OC1B
        SET_PIN(PORTB, PB6, level);
        break;

    case 5: // OPT7 = 2 -> PE4 -> OC3B
        SET_PIN(PORTE, PE4, level);
        break;

    case 6: // OPT8 = 45 -> PL4 -> OC5B
        SET_PIN(PORTL, PL4, level);
        break;

    case 7: // OPT9 = 5 -> PE3 -> OC3A
        SET_PIN(PORTE, PE3, level);
        break;

    case 8:; // OPT10 = 7 -> PH4 -> OC4B
        SET_PIN(PORTH, PH4, level);
        break;

    case 9: // OPT11 = 11 -> PB5 -> OC1A
        SET_PIN(PORTB, PB5, level);
        break;

    case 10: // OPT12 = 13 -> PB7 -> OC1C
        SET_PIN(PORTB, PB7, level);
        break;

    case 11: // OPT13 = 44 -> PL5 -> OC5C
        SET_PIN(PORTL, PL5, level);
        break;

    case 12: // OPT14 = 9 -> PH6 -> OC2B
        SET_PIN(PORTH, PH6, level);
        break;

    case 13: // OPT15 = 10 -> PB4 -> OC2A
        SET_PIN(PORTB, PB4, level);
        break;

    default:
        break;
    }
}

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
    TCNT1 = 19995; // clear timer1
    TCNT2 = 255;   // clear timer2
    TCNT3 = 19995; // clear timer3
    TCNT4 = 19995; // celar timer4
    TCNT5 = 19995; // clear timer5

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

    for (uint8_t num = 1; num < DIMMER_LIGHT_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(OPT_ARRAY + num);
        pinMode(pin, OUTPUT);
        desativation_ocrx(num - 1);
        digitalWrite(pin, LOW); // extinction du dimmer
    }

    for (uint8_t i = 0; i < DIMMER_LIGHT_NUM; i++) // init variables
    {
        light[i].Dimmer_percent = 0;
        light[i].Dimmer_old_percent = 100;
        // Dimmer_value[i] = 65535;
        // Dimmer_new_value[i] = 65535;
        // Dimmer_time_interval[i] = 0;
        light[i].Dimmer_candle = false;
    }

#if DIMMER_HEAT_NUM > 0
    for (uint8_t num = 1; num < DIMMER_HEAT_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(OPT_ARRAY + DIMMER_LIGHT_NUM + num);
        pinMode(pin, OUTPUT);
        desativation_ocrx(num - 1);
        digitalWrite(pin, LOW); // extinction du dimmer
    }
    for (uint8_t num = 0; num < DIMMER_HEAT_NUM; ++num) // init variables
    {
        mode[num] = none;
    }
#endif // DIMMER_HEAT_NUM > 0

    noInterrupts(); // disable all interrupts

    // interuption PCINT1  (point zero)
#if DIMMER_LIGHT_NUM >= 1
    // EIMSK = 0b00000100;  // active INT2 interupt
    // EICRA = 0b00100000;
    // EIMSK = 1<<INT2;
    // EICRA = 1<<ISC21;
    pinMode(OPT_1, INPUT); // Point zero
    attachInterrupt(digitalPinToInterrupt(19), point_zero, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(19), point_zero, RISING);
    // PCMSK0 = 0b00000001; // active pcint at pin 8

    TCCR5A = 0b00000010; // PWM mode Fast PWM
    TCCR5B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK5 = 0b0000000;  // No interuption
    // ICR5 = 19840;        // Timer5 TOP
    ICR5 = 20000; // Timer5 TOP
#endif
#if DIMMER_LIGHT_NUM >= 2
    TCCR3A = 0b00000010; // PWM mode Fast PWM
    TCCR3B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK3 = 0b0000000;  // No interuption
    // ICR3 = 19840;        // Timer3 TOP
    ICR3 = 20000; // Timer3 TOP
#endif
#if DIMMER_LIGHT_NUM >= 3
    TCCR4A = 0b00000010; // PWM mode Fast PWM
    TCCR4B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK4 = 0b0000000;  // No interuption
    // ICR4 = 19840;        // Timer4 TOP
    ICR4 = 20000; // Timer4 TOP
#endif
#if DIMMER_LIGHT_NUM >= 5
    TCCR1A = 0b00000010; // PWM mode Fast PWM
    TCCR1B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK1 = 0b0000000;  // No interuption
    // ICR1 = 19840;        // Timer1 TOP
    ICR1 = 20000; // Timer1 TOP
#endif
#if DIMMER_LIGHT_NUM >= 5
    TCCR2A = 0b00000010; // PWM mode Fast PWM
    TCCR2B = 0b00000111; // prescale 1024
    TIMSK2 = 0b0000000;  // No interuption
#endif

    interrupts();

    // delay(20);
};

// demmarage / extinction du dimmer
// num : numero du dimmer
// percent : pourcentage du dimmer (0 pour arret)
// time : temps de passage a la nouvelle valleur
// candle : mode bougie (non implémanté)
void dimmer_set(uint8_t num, uint8_t percent, uint16_t time, bool candle)
{
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
    // uint8_t temp_percent;
    percent = percent >= 100 ? 100 : percent;
    Serial.print(F("dimmer "));
    Serial.print(num);
    Serial.print(F(" percent = "));
    Serial.println(percent);
    Serial.print(F(" candle = "));
    Serial.println(candle);
    // Dimmer_new_value[num] = to_ocrx(num, percent);
    // uint16_t ocrx = set_ocrx(num, percent);
    // Serial.print(F("Dimmer_new_value = "));
    // Serial.println(ocrx);

    // uint8_t interval;
    // if (percent > light[num].Dimmer_percent)
    // {
    //     interval = time / (percent - light[num].Dimmer_percent);
    // }
    // else
    // {
    //     interval = time / (light[num].Dimmer_percent - percent);
    // }

    if (percent != 0 && default_0)
    {
        // Dimmer_value[num] = Dimmer_new_value[num] = to_ocrx(num, percent);
        // light[num].Dimmer_percent = 100;
        set_ocrx(num, 100);
        light[num].Dimmer_go_percent = percent;
        light[num].Dimmer_time = 1;
    }
    else if (percent == 0 && default_0)
    {
        // Dimmer_value[num] = Dimmer_new_value[num];
        // light[num].Dimmer_percent = percent;
        set_ocrx(num, 0);
        light[num].Dimmer_go_percent = percent;
        light[num].Dimmer_time = 1;
    }
    else if (time == 0)
    {
        // Dimmer_value[num] = Dimmer_new_value[num];
        // light[num].Dimmer_percent = percent;
        set_ocrx(num, percent);
        light[num].Dimmer_go_percent = percent;
        light[num].Dimmer_time = 1;
    }
    else
    {
        light[num].Dimmer_time = time;           // in millisecond
        light[num].Dimmer_time_start = millis(); // in millisecond
        light[num].Dimmer_go_percent = percent;
        light[num].Dimmer_start_percent = light[num].Dimmer_percent;
    }

    if (percent != 0)
    {
        light[num].Dimmer_old_percent = percent;
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
}

void dimmer_set(uint8_t num, bool start, uint16_t time, bool candle)
{
    if (start)
        dimmer_set(num, light[num].Dimmer_old_percent, time, candle);
    else
        dimmer_set(num, (uint8_t)0, time, candle);
}

void dimmer_set_heat_mode(uint8_t num, heat_mode Mode)
{
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
}

// valeur en pourcentage de fonctionnement du dimmer
//  num: numero du dimmer
uint8_t get_dimmer(uint8_t num)
{
    // return light[num].Dimmer_percent;
    // if (Dimmer_new_value[num] == 65535)
    // {
    //     return 0;
    // }
    // else
    // {
    //     return light[num].Dimmer_old_percent;
    // }
    return light[num].Dimmer_go_percent;
}

// valeur en pourcentage de fonctionnement du dimmer
//  num: numero du dimmer
heat_mode get_heat_mode(uint8_t num)
{
    if (num < DIMMER_HEAT_NUM)
    {
        return (mode[num]);
    }
    return heat_mode(none);
}

// true si le dimmer est en mode bougie
bool get_dimmer_candle(uint8_t num)
{
    return light[num].Dimmer_candle;
};

// boucle d'execcution du dimmer
void dimmer_loop()
{
    // static unsigned long dimmed_speed_time[DIMMER_LIGHT_NUM];
    // static old_time = 0
    static unsigned long dimmer_candle_old_time = 0;
    static unsigned long candle_interval = random(MIN_CANDLE_TIME, MAX_CANDLE_TIME);
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
        }
    }

    for (uint8_t num = 0; num < DIMMER_LIGHT_NUM; num++)
    {
        uint32_t time_go = light[num].Dimmer_time_start + light[num].Dimmer_time;
        /// Dimmed start / stop
        if (millis() <= time_go)
        {
            // uint8_t percent = SCALE(millis(), light[num].Dimmer_time_start, Dimmer_time_go[num], light[num].Dimmer_start_percent, light[num].Dimmer_go_percent);
            uint8_t percent = ((millis() - light[num].Dimmer_time_start) * (light[num].Dimmer_go_percent - light[num].Dimmer_start_percent) / (time_go - light[num].Dimmer_time_start)) + light[num].Dimmer_start_percent;
            // light[num] = Dimmer_percent[num] >= 99 ? 100 : Dimmer_percent[num].Dimmer_percent + 1;
            // Dimmer_value[num] = to_ocrx(num, light[num].Dimmer_percent);

            Serial.print(F(" set_ocrx  = "));
            Serial.println(percent);
            set_ocrx(num, percent);
        }
        else if (light[num].Dimmer_time > 1)
        {
            set_ocrx(num, light[num].Dimmer_go_percent);
            light[num].Dimmer_time = 1;
        }
        // else
        // {
        //     set_ocrx(num, light[num].Dimmer_go_percent);
        // }

        /// Candle
        if (millis() - dimmer_candle_old_time >= candle_interval)
        {
            if (light[num].Dimmer_candle == true) // FIXME: candle: need test and update
            {
                dimmer_candle_old_time = millis();
                uint8_t candle_percent = random(CANDLE_OFSSET_PERCENTE_MIN, CANDLE_OFSSET_PERCENTE_MAX + 1);
                // dimmer_set(num, config.Dimmer_old_value[num] - (uint8_t)((double)config.Dimmer_old_value[num] * (double)candle_percent / (double)100), CANDLE_SPEED);
                // unsigned int tmp = light[num] - (uint8_t)((double)Dimmer_percent[num].Dimmer_percent * (double)candle_percent / (double)100);
                uint8_t percent_tmp = light[num].Dimmer_percent - candle_percent;
                uint16_t time_tmp = random(CANDLE_SPEED_MIN, CANDLE_SPEED_MAX + 1);
                // scale_percent = SCALE(tmp, 0, 100, DIMMER_SCALE_MIN, DIMMER_SCALE_MAX);
                // Dimmer_new_value[num] = to_ocrx(num, tmp);
                // set_ocrx(num, tmp);
                light[num].Dimmer_time = time_tmp;       // in millisecond
                light[num].Dimmer_time_start = millis(); // in millisecond
                light[num].Dimmer_start_percent = light[num].Dimmer_percent;
                light[num].Dimmer_go_percent = percent_tmp;

                // Dimmer_time_interval[num] = random(CANDLE_SPEED_MIN, CANDLE_SPEED_MAX + 1); // convert to 0.1ms
                candle_interval = random(MIN_CANDLE_TIME, MAX_CANDLE_TIME);
            }
        }
    }

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