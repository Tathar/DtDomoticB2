#include "DT_Dimmer.h"
#include "pinout.h"
#include "config.h"
#include "DT_eeprom.h"

#define EI_NOTINT0
#define EI_NOTINT1
// #include <EnableInterrupt.h>

#include <avr/interrupt.h>

// #define SET_DIMMER15 PORTC |= _BV(PINC0);
// #define SET_DIMMER16 PORTC |= _BV(PINC1);

// #define CLEAR_DIMMER13 PORTC &= 0b11111110;    // extinction dimmer 3 & 4
// #define CLEAR_DIMMER14 PORTC &= 0b11111101;    // extinction dimmer 3 & 4
// #define CLEAR_DIMMER13_14 PORTC &= 0b11111100; // extinction dimmer 3 & 4

#define SCALE(val, in_min, in_max, out_min, out_max) (((double)val - (double)in_min) * ((double)out_max - (double)out_min) / ((double)in_max - (double)in_min)) + out_min

// volatile uint8_t OCR2A_COUNT = 0;
// volatile uint8_t OCR2B_COUNT = 0;
// volatile uint8_t TOV2_count = 0;

// const PROGMEM uint16_t ocrx[] = {65535, 16498, 16210, 16090, 15997, 15920, 15849, 15786, 15729, 15674,
//                                  15636, 15572, 15525, 15480, 15436, 15394, 15339, 15315, 15276, 15238,
//                                  15201, 15163, 15129, 15083, 15058, 15024, 14990, 14957, 14926, 14892,
//                                  14868, 14827, 14799, 14766, 14737, 14705, 14676, 14646, 14616, 14571,
//                                  14555, 14527, 14496, 14468, 14439, 14408, 14379, 14356, 14315, 14292,
//                                  14263, 14236, 14207, 14178, 14149, 14120, 14100, 14059, 14031, 14003,
//                                  13972, 13944, 13913, 13883, 13853, 13803, 13794, 13762, 13733, 13700,
//                                  13669, 13639, 13607, 13588, 13542, 13509, 13475, 13441, 13405, 13370,
//                                  13336, 13291, 13261, 13223, 13184, 13146, 13105, 13076, 13019, 12974,
//                                  12927, 12878, 12825, 12770, 12713, 12650, 12579, 12502, 12409, 12308,
//                                  0};

// const PROGMEM uint16_t ocrx[] = {65535, 16498, 15441, 15002, 14662, 14374, 14119, 13887, 13673, 13472, 13281, 13101, 12930, 12763, 12605, 12449, 12308, 12154, 12011, 11873, 11735, 11602, 11471, 11342, 11217, 11091, 10968, 10848, 10727, 10610, 10475, 10377, 10262, 10148, 10035, 9923, 9810, 9701, 9591, 9492, 9373, 9263, 9155, 9049, 8939, 8835, 8727, 8622, 8514, 8408, 8302, 8212, 8091, 7985, 7877, 7772, 7659, 7557, 7450, 7344, 7236, 7126, 7019, 6891, 6798, 6689, 6576, 6464, 6351, 6237, 6122, 6006, 5908, 5772, 5652, 5531, 5408, 5282, 5157, 5028, 4897, 4764, 4628, 4487, 4331, 4199, 4050, 3894, 3736, 3563, 3398, 3218, 3027, 2836, 2612, 2380, 2125, 1837, 1497, 1058, 0};

// #define OCRX(value) pgm_read_word_near(ocrx + value)

// inline uint16_t to_ocrx(uint8_t dimmer_num, uint8_t percent) __attribute__((always_inline));
uint16_t to_ocrx(uint8_t dimmer_num, uint8_t percent)
{
    if (percent >= 100)
    {
        return 0;
    }
    else if (percent == 0)
    {
        return 65535;
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

volatile unsigned int Timer1_count0 = 0;
volatile unsigned int debug_max_count = 0;
volatile unsigned int debug_min_count = 512;

volatile uint8_t Dimmer_percent[DIMMER_NUM];    // in Percent
volatile uint16_t Dimmer_value[DIMMER_NUM];     // in Slice
volatile uint16_t Dimmer_new_value[DIMMER_NUM]; // in Slice
volatile uint16_t Dimmer_time_interval[DIMMER_NUM];
volatile bool Dimmer_candle[DIMMER_NUM];

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
ISR(PCINT0_vect)
{
    if (!(PINB & _BV(PB0))) // PCINT0 interrupts on FALLING
    {
        TCNT1 = 0; // clear timer1
        TCNT2 = 0; // cearr timer2
        TCNT3 = 0; // clear timer3
        TCNT4 = 0; // cearr timer4
        TCNT5 = 0; // clear timer5

        // if (OCR2A & 0b11111111) // if OCR2A != 0
        //     CLEAR_DIMMER13;
        // if (OCR2B & 0b11111111) // if OCR2B != 0
        //     CLEAR_DIMMER14;

        // TOV2_count = 0;

#if DIMMER_NUM >= 1
        OCR5C = Dimmer_value[0];
#endif
#if DIMMER_NUM >= 2
        OCR3B = Dimmer_value[1];
#endif
#if DIMMER_NUM >= 3
        OCR4A = Dimmer_value[2];
#endif
#if DIMMER_NUM >= 4
        OCR4C = Dimmer_value[3];
#endif
#if DIMMER_NUM >= 5
        OCR1B = Dimmer_value[4];
#endif
#if DIMMER_NUM >= 6
        OCR3B = Dimmer_value[5];
#endif
#if DIMMER_NUM >= 7
        OCR5B = Dimmer_value[6];
#endif
#if DIMMER_NUM >= 8
        OCR3A = Dimmer_value[7];
#endif
#if DIMMER_NUM >= 9
        OCR4B = Dimmer_value[8];
#endif
#if DIMMER_NUM >= 10
        OCR1A = Dimmer_value[9];
#endif
#if DIMMER_NUM >= 11
        OCR1C = Dimmer_value[10];
#endif
#if DIMMER_NUM >= 12
        OCR5A = Dimmer_value[11];
#endif
#if DIMMER_NUM >= 13
        OCR2A = Dimmer_value[12];
#endif
#if DIMMER_NUM >= 14
        OCR2B = Dimmer_value[13];
#endif
    }
}

void Dimmer_init(void)
{

    pinMode(OPT_1, INPUT); // Point zero

    for (uint8_t num = 0; num < DIMMER_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(OPT_ARRAY + num);
        pinMode(pin, OUTPUT);
    }

    for (uint8_t i = 0; i < DIMMER_NUM; i++) // init variables
    {
        Dimmer_percent[i] = 0;
        Dimmer_value[i] = 65535;
        Dimmer_new_value[i] = 65535;
        Dimmer_time_interval[i] = 0;
        Dimmer_candle[i] = false;
    }

    noInterrupts(); // disable all interrupts

    // Timer 1 Dimmer OPt_11 & OPT_6 & OPT_12)
#if DIMMER_NUM >= 10
    TCCR1A = TCCR1A | 0b11000010; // bit change at OCR1A
#endif
#if DIMMER_NUM >= 5
    TCCR1A = TCCR1A | 0b00110010; // bit change at OCR1B
#endif
#if DIMMER_NUM >= 11
    TCCR1A = TCCR1A | 0b00001110; // bit change at OCR1C
#endif
    TCCR1B = 0b00011010; // prescale 8
    TIMSK1 = 0b0000000;  // No interuption
    OCR1A = 65535;       // Dimmer OPT_11 OFF
    OCR1B = 65535;       // Dimmer OPT_6 OFF
    OCR1C = 65535;       // Dimmer OPT_12 OFF
    ICR1 = 19840;        // Timer1 TOP

    // Timmer 2 (Dimmer OPT_14 et OPT_15)
#if DIMMER_NUM >= 13
    TCCR2A = TCCR2A | 0b11000010; // bit change at OCR2A
#endif
#if DIMMER_NUM >= 14
    TCCR2A = TCCR2A | 0b00110010; // bit change at OCR2B
#endif
    // TCCR2A = 0b00000000; // normal mode
    TCCR2B = 0b00000010; // prescale 8
    TIMSK2 = 0b00000000; // interuption TOV2 OCR2A OCR2B
    OCR2A = 255;         // Dimmer OPT_14 OFF
    OCR2B = 255;         // Dimmer OPT_15 OFF

    // Timer 3 Dimmer OPT_9 & OPT_7 & OPT_3)
#if DIMMER_NUM >= 8
    TCCR3A = TCCR3A | 0b11000010; // bit change at OCR3A
#endif
#if DIMMER_NUM >= 6
    TCCR3A = TCCR3A | 0b00110010; // bit change at OCR3B
#endif
#if DIMMER_NUM >= 2
    TCCR3A = TCCR3A | 0b00001110; // bit change at OCR3C
#endif
    // TCCR3A = 0b11111110; // bit change at OCR1A OCR1B OCR1C
    TCCR3B = 0b00011010; // prescale 8
    TIMSK3 = 0b0000000;  // No interuption
    OCR3A = 65535;       // Dimmer OPT_9 OFF
    OCR3B = 65535;       // Dimmer OPT_7 OFF
    OCR3B = 65535;       // Dimmer OPT_3 OFF
    ICR3 = 19840;        // Timer3 TOP

    // Timer 4 Dimmer OPT_4 & OPT_10 & OPT_5)
#if DIMMER_NUM >= 3
    TCCR4A = TCCR4A | 0b11000010; // bit change at OCR4A
#endif
#if DIMMER_NUM >= 9
    TCCR4A = TCCR4A | 0b00110010; // bit change at OCR4B
#endif
#if DIMMER_NUM >= 4
    TCCR4A = TCCR4A | 0b00001110; // bit change at OCR4C
#endif
    // TCCR4A = 0b11111110; // bit change at OCR1A OCR1B OCR1C
    TCCR4B = 0b00011010; // prescale 8
    TIMSK4 = 0b0000000;  // No interuption
    OCR4A = 65535;       // Dimmer OPT_4 OFF
    OCR4B = 65535;       // Dimmer OPT_10 OFF
    OCR4C = 65535;       // Dimmer OPT_5 OFF
    ICR4 = 19840;        // Timer4 TOP

    // Timer 5 Dimmer OPT_13 & OPT_8 & OPT_2)
#if DIMMER_NUM >= 12
    TCCR5A = TCCR5A | 0b11000010; // bit change at OCR5A
#endif
#if DIMMER_NUM >= 7
    TCCR5A = TCCR5A | 0b00110010; // bit change at OCR5B
#endif
#if DIMMER_NUM >= 1
    TCCR5A = TCCR5A | 0b00001110; // bit change at OCR5C
#endif
    // TCCR5A = 0b11111110; // bit change at OCR5A OCR5B OCR5C
    TCCR5B = 0b00011010; // prescale 8
    TIMSK5 = 0b0000000;  // No interuption
    OCR5A = 65535;       // Dimmer OPT_13 OFF
    OCR5B = 65535;       // Dimmer OPT_8 OFF
    OCR5C = 65535;       // Dimmer OPT_2 OFF
    ICR1 = 19840;        // Timer5 TOP

    // interuption PCINT1  (point zero)
#if DIMMER_NUM >= 1
    PCICR = 0b00000001;  // active PCINT1 interupt
    PCMSK0 = 0b00000001; // active pcint at pin 8
#endif
    interrupts();

    // delay(20);
};

void dimmer_set(uint8_t num, uint8_t percent, uint8_t time, bool candle)
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
    Dimmer_candle[num] = candle;
    // uint8_t temp_percent;
    percent = percent >= 100 ? 100 : percent;
    Serial.print(F("dimmer "));
    Serial.print(num);
    Serial.print(F(" percent = "));
    Serial.println(percent);
    Dimmer_new_value[num] = to_ocrx(num, percent);
    Serial.print(F("Dimmer_new_value = "));
    Serial.println(Dimmer_new_value[num]);

    if (time == 0)
    {
        Dimmer_value[num] = Dimmer_new_value[num];
        Dimmer_percent[num] = percent;
    }
    else
    {
        Dimmer_time_interval[num] = time * 10; // convert to microsecond
    }

    // config.Dimmer_old_value[num] = percent <= DIMMER_MIN ? DIMMER_MIN : percent;

    // if (Dimmer_candle[num])
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
}

// uint8_t dimmer_up(uint8_t num)
// {
//     // double scale_percent = SCALE(Dimmer_percent[num] + 1, 0, 100, DIMMER_SCALE_MIN, DIMMER_SCALE_MAX);
//     // uint8_t temp_percent;
//     Dimmer_percent[num] = Dimmer_percent[num] >= 100 ? 100 : Dimmer_percent[num] + 1;
//     Dimmer_value[num] = Dimmer_new_value[num] = OCRX(Dimmer_percent[num]);
//     // config.Dimmer_old_value[num] = Dimmer_percent[num];
//     return Dimmer_percent[num];

//     // if (Dimmer_candle[num])
//     //     DtCan_Board_send(config.address, num + 9, CAN_BOARD_DIMMER_CANDLE, Dimmer_percent[num]);
//     // else
//     //     DtCan_Board_send(config.address, num + 9, CAN_BOARD_DIMMER, Dimmer_percent[num]);
// }

// uint8_t dimmer_down(uint8_t num)
// {
//     Dimmer_percent[num] = Dimmer_percent[num] = 0 ? 0 : Dimmer_percent[num] - 1;
//     Dimmer_value[num] = Dimmer_new_value[num] = OCRX(Dimmer_percent[num]);
//     // config.Dimmer_old_value[num] = Dimmer_percent[num];
//     return Dimmer_percent[num];
//     // if (Dimmer_candle[num])
//     //     DtCan_Board_send(config.address, num + 9, CAN_BOARD_DIMMER_CANDLE, temp_percent);
//     // else
//     //     DtCan_Board_send(config.address, num + 9, CAN_BOARD_DIMMER, temp_percent);
// }

uint8_t get_dimmer(uint8_t num)
{
    return Dimmer_percent[num];
}

bool get_dimmer_candle(uint8_t num)
{
    return Dimmer_candle[num];
};

void dimmer_loop()
{
    static unsigned long dimmed_speed_time[4] = {0, 0, 0, 0};
    static unsigned long dimmer_candle_old_time = 0;
    static unsigned long candle_interval = random(MIN_CANDLE_TIME, MAX_CANDLE_TIME);

    for (uint8_t num = 0; num < 4; num++)
    {
        /// Dimmed start / stop
        if (micros() - dimmed_speed_time[num] >= Dimmer_time_interval[num])
        {
            dimmed_speed_time[num] = micros();
            if (Dimmer_value[num] > Dimmer_new_value[num])
            {
                Dimmer_percent[num] = Dimmer_percent[num] >= 99 ? 100 : Dimmer_percent[num] + 1;
                Dimmer_value[num] = to_ocrx(num, Dimmer_percent[num]);
            }
            else if (Dimmer_value[num] < Dimmer_new_value[num])
            {

                Dimmer_percent[num] = Dimmer_percent[num] <= 1 ? 0 : Dimmer_percent[num] - 1;
                Dimmer_value[num] = to_ocrx(num, Dimmer_percent[num]);

                // if (num == 0)
                //     Serial.println(Dimmer_value[num]);
            }
        }

        /// Candle
        if (millis() - dimmer_candle_old_time >= candle_interval)
        {
            if (Dimmer_candle[num] == true) // FIXME: candle: need test and update
            {
                dimmer_candle_old_time = millis();
                uint8_t candle_percent = random(CANDLE_PERCENTE_MIN, CANDLE_PERCENTE_MAX + 1);
                // dimmer_set(num, config.Dimmer_old_value[num] - (uint8_t)((double)config.Dimmer_old_value[num] * (double)candle_percent / (double)100), CANDLE_SPEED);
                Dimmer_time_interval[num] = CANDLE_SPEED * 10; // convert to microsecond
                // unsigned int tmp = Dimmer_percent[num] - (uint8_t)((double)Dimmer_percent[num] * (double)candle_percent / (double)100);
                unsigned int tmp = Dimmer_percent[num] - candle_percent;
                // scale_percent = SCALE(tmp, 0, 100, DIMMER_SCALE_MIN, DIMMER_SCALE_MAX);
                Dimmer_new_value[num] = to_ocrx(num, tmp);
                Dimmer_time_interval[num] = 100;
                candle_interval = random(MIN_CANDLE_TIME, MAX_CANDLE_TIME);
            }
        }
    }
}
