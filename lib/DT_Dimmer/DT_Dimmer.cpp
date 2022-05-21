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

bool default_0 = false;
volatile unsigned int default_0_count = 0;

uint16_t Dimmer_time[DIMMER_NUM];         // in millisecond
uint32_t Dimmer_time_start[DIMMER_NUM];   // in millisecond
uint8_t Dimmer_percent[DIMMER_NUM];       // in Percent
uint8_t Dimmer_go_percent[DIMMER_NUM];    // in Percent
uint8_t Dimmer_old_percent[DIMMER_NUM];   // in Percent
uint8_t Dimmer_start_percent[DIMMER_NUM]; // in Percent
// volatile uint16_t Dimmer_value[DIMMER_NUM];      // in Slice
// volatile uint16_t Dimmer_new_value[DIMMER_NUM];  // in Slice
volatile bool Dimmer_candle[DIMMER_NUM];
// static bool update[DIMMER_NUM];

void set_ocrx(uint8_t dimmer_num, double percent)
{
    if (percent >= 100)
    {
        desativation_ocrx(dimmer_num);
        uint8_t pin = pgm_read_byte(OPT_ARRAY + dimmer_num + 1);
        digitalWrite(pin, HIGH);
        Dimmer_percent[dimmer_num] = 100;
        // return 0;
    }
    else if (percent == 0)
    {
        desativation_ocrx(dimmer_num);
        uint8_t pin = pgm_read_byte(OPT_ARRAY + dimmer_num + 1);
        digitalWrite(pin, LOW);
        Dimmer_percent[dimmer_num] = 0;
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
        Dimmer_percent[dimmer_num] = percent;
        // return SCALE(percent, 1, 99, eeprom_config.Dimmer_scale_min[dimmer_num], eeprom_config.Dimmer_scale_max[dimmer_num]);
    }

    Serial.print(F("Dimmer_percent["));
    Serial.print(dimmer_num);
    Serial.print(F("] = "));
    Serial.println(Dimmer_percent[dimmer_num]);

    Serial.print(F("ICR5 = "));
    Serial.println(ICR5);

    Serial.print(F("TCCR5A = "));
    Serial.println(TCCR5A, BIN);

    Serial.print(F("TCCR5B = "));
    Serial.println(TCCR5B, BIN);
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

    ++default_0_count;
}

// initialisation des dimmer
void Dimmer_init(void)
{

    _callback_dimmer = nullptr;

    for (uint8_t num = 1; num < DIMMER_NUM; ++num)
    {
        uint8_t pin = pgm_read_byte(OPT_ARRAY + num);
        pinMode(pin, OUTPUT);
        desativation_ocrx(num - 1);
        digitalWrite(pin, LOW); // extinction du dimmer
    }

    for (uint8_t i = 0; i < DIMMER_NUM; i++) // init variables
    {
        Dimmer_percent[i] = 0;
        Dimmer_old_percent[i] = 100;
        // Dimmer_value[i] = 65535;
        // Dimmer_new_value[i] = 65535;
        // Dimmer_time_interval[i] = 0;
        Dimmer_candle[i] = false;
    }

    noInterrupts(); // disable all interrupts

    // interuption PCINT1  (point zero)
#if DIMMER_NUM >= 1
    // EIMSK = 0b00000100;  // active INT2 interupt
    // EICRA = 0b00100000;
    // EIMSK = 1<<INT2;
    // EICRA = 1<<ISC21;
    pinMode(OPT_1, INPUT); // Point zero
    // attachInterrupt(digitalPinToInterrupt(19), point_zero, FALLING);
    attachInterrupt(digitalPinToInterrupt(19), point_zero, CHANGE);
    // PCMSK0 = 0b00000001; // active pcint at pin 8

    TCCR5A = 0b00000010; // PWM mode Fast PWM
    TCCR5B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK5 = 0b0000000;  // No interuption
    // ICR5 = 19840;        // Timer5 TOP
    ICR5 = 20000; // Timer5 TOP
#endif
#if DIMMER_NUM >= 2
    TCCR3A = 0b00000010; // PWM mode Fast PWM
    TCCR3B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK3 = 0b0000000;  // No interuption
    // ICR3 = 19840;        // Timer3 TOP
    ICR3 = 20000; // Timer3 TOP
#endif
#if DIMMER_NUM >= 3
    TCCR4A = 0b00000010; // PWM mode Fast PWM
    TCCR4B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK4 = 0b0000000;  // No interuption
    // ICR4 = 19840;        // Timer4 TOP
    ICR4 = 20000; // Timer4 TOP
#endif
#if DIMMER_NUM >= 5
    TCCR1A = 0b00000010; // PWM mode Fast PWM
    TCCR1B = 0b00011010; // PWM mode Fast PWM + prescale 8
    TIMSK1 = 0b0000000;  // No interuption
    // ICR1 = 19840;        // Timer1 TOP
    ICR1 = 20000; // Timer1 TOP
#endif
#if DIMMER_NUM >= 5
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
    Dimmer_candle[num] = candle;
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
    // if (percent > Dimmer_percent[num])
    // {
    //     interval = time / (percent - Dimmer_percent[num]);
    // }
    // else
    // {
    //     interval = time / (Dimmer_percent[num] - percent);
    // }

    if (percent != 0 && default_0)
    {
        // Dimmer_value[num] = Dimmer_new_value[num] = to_ocrx(num, percent);
        // Dimmer_percent[num] = 100;
        set_ocrx(num, 100);
        Dimmer_go_percent[num] = percent;
        Dimmer_time[num] = 1;
    }
    else if (percent == 0 && default_0)
    {
        // Dimmer_value[num] = Dimmer_new_value[num];
        // Dimmer_percent[num] = percent;
        set_ocrx(num, 0);
        Dimmer_go_percent[num] = percent;
        Dimmer_time[num] = 1;
    }
    else if (time == 0)
    {
        // Dimmer_value[num] = Dimmer_new_value[num];
        // Dimmer_percent[num] = percent;
        set_ocrx(num, percent);
        Dimmer_go_percent[num] = percent;
        Dimmer_time[num] = 1;
    }
    else
    {
        Dimmer_time[num] = time;           // in millisecond
        Dimmer_time_start[num] = millis(); // in millisecond
        Dimmer_go_percent[num] = percent;
        Dimmer_start_percent[num] = Dimmer_percent[TEMP_NUM];
    }

    if (percent != 0)
    {
        Dimmer_old_percent[num] = percent;
    }

    // update[num] = true;

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

    // Serial.print(F(" calc time  = "));
    // Serial.println(Dimmer_time_interval[num]);
}

void dimmer_set(uint8_t num, bool start, uint16_t time, bool candle)
{
    if (start)
        dimmer_set(num, Dimmer_old_percent[num], time, candle);
    else
        dimmer_set(num, (uint8_t)0, time, candle);
}



void dimmer_heat(uint8_t num, heat_mode mode)
{
    
    
}

// valeur en pourcentage de fonctionnement du dimmer
//  num: numero du dimmer
uint8_t get_dimmer(uint8_t num)
{
    // return Dimmer_percent[num];
    // if (Dimmer_new_value[num] == 65535)
    // {
    //     return 0;
    // }
    // else
    // {
    //     return Dimmer_old_percent[num];
    // }
    return Dimmer_go_percent[num];
}

// true si le dimmer est en mode bougie
bool get_dimmer_candle(uint8_t num)
{
    return Dimmer_candle[num];
};

// boucle d'execcution du dimmer
void dimmer_loop()
{
    // static unsigned long dimmed_speed_time[DIMMER_NUM];
    // static old_time = 0
    static unsigned long dimmer_candle_old_time = 0;
    static unsigned long candle_interval = random(MIN_CANDLE_TIME, MAX_CANDLE_TIME);
    // uint16_t Dimmer_time_interval[DIMMER_NUM];
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

    for (uint8_t num = 0; num < DIMMER_NUM; num++)
    {
        uint32_t time_go = Dimmer_time_start[num] + Dimmer_time[num];
        /// Dimmed start / stop
        if (millis() <= time_go)
        {
            // uint8_t percent = SCALE(millis(), Dimmer_time_start[num], Dimmer_time_go[num], Dimmer_start_percent[num], Dimmer_go_percent[num]);
            uint8_t percent = ((millis() - Dimmer_time_start[num]) * (Dimmer_go_percent[num] - Dimmer_start_percent[num]) / (time_go - Dimmer_time_start[num])) + Dimmer_start_percent[num];
            // Dimmer_percent[num] = Dimmer_percent[num] >= 99 ? 100 : Dimmer_percent[num] + 1;
            // Dimmer_value[num] = to_ocrx(num, Dimmer_percent[num]);

            Serial.print(F(" set_ocrx  = "));
            Serial.println(percent);
            set_ocrx(num, percent);
        }
        else if (Dimmer_time[num] > 1)
        {
            set_ocrx(num, Dimmer_go_percent[num]);
            Dimmer_time[num] = 1;
        }
        // else
        // {
        //     set_ocrx(num, Dimmer_go_percent[num]);
        // }

        /// Candle
        if (millis() - dimmer_candle_old_time >= candle_interval)
        {
            if (Dimmer_candle[num] == true) // FIXME: candle: need test and update
            {
                dimmer_candle_old_time = millis();
                uint8_t candle_percent = random(CANDLE_OFSSET_PERCENTE_MIN, CANDLE_OFSSET_PERCENTE_MAX + 1);
                // dimmer_set(num, config.Dimmer_old_value[num] - (uint8_t)((double)config.Dimmer_old_value[num] * (double)candle_percent / (double)100), CANDLE_SPEED);
                // unsigned int tmp = Dimmer_percent[num] - (uint8_t)((double)Dimmer_percent[num] * (double)candle_percent / (double)100);
                uint8_t percent_tmp = Dimmer_percent[num] - candle_percent;
                uint16_t time_tmp = random(CANDLE_SPEED_MIN, CANDLE_SPEED_MAX + 1);
                // scale_percent = SCALE(tmp, 0, 100, DIMMER_SCALE_MIN, DIMMER_SCALE_MAX);
                // Dimmer_new_value[num] = to_ocrx(num, tmp);
                // set_ocrx(num, tmp);
                Dimmer_time[num] = time_tmp;           // in millisecond
                Dimmer_time_start[num] = millis(); // in millisecond
                Dimmer_go_percent[num] = percent_tmp;
                Dimmer_start_percent[num] = Dimmer_percent[TEMP_NUM];

                // Dimmer_time_interval[num] = random(CANDLE_SPEED_MIN, CANDLE_SPEED_MAX + 1); // convert to 0.1ms
                candle_interval = random(MIN_CANDLE_TIME, MAX_CANDLE_TIME);
            }
        }
    }

    static uint8_t async_num = 0;

    if (_callback_dimmer != nullptr)
    {
        if (Dimmer_time[async_num] == 1)
        {
            _callback_dimmer(async_num, get_dimmer(async_num), Dimmer_candle[async_num]);
            Dimmer_time[async_num] = 0;
        }

        if (++async_num == DIMMER_NUM)
        {
            async_num = 0;
        }
    }
}

void set_dimmer_callback(void (*callback)(const uint8_t num, const uint8_t percent, const bool candle))
{
    _callback_dimmer = callback;
}