#ifndef PINOUT
#define PINOUT

#include <avr/pgmspace.h>

#define OPT_1 19  // PD2 -> RXD1/INT2
#define OPT_2 46  // PL3 -> OC5A
#define OPT_3 3   // PE5 -> OC3C
#define OPT_4 6   // PH3 -> OC4A
#define OPT_5 8   // PH5 -> OC4C
#define OPT_6 12  // PB6 -> OC1B
#define OPT_7 2   // PE4 -> OC3B
#define OPT_8 45  // PL4 -> OC5B
#define OPT_9 5   // PE3 -> OC3A
#define OPT_10 7  // PH4 -> OC4B
#define OPT_11 11 // PB5 -> OC1A
#define OPT_12 13 // PB7 -> OC1C
#define OPT_13 44 // PL5 -> OC5C
#define OPT_14 9  // PH6 -> OC2B
#define OPT_15 10 // PB4 -> OC2A
#define OPT_16 18 // PD3 -> TXD1
#define OPT_17 38 // PD7 -> T0
#define OPT_18 15 // PJ0 -> RXD3

/*
OPT1 = 19 -> PD2 -> RXD1/INT2
OPT2 = 46 -> PL3 -> OC5A
OPT3 = 3 -> PE5 -> OC3C
OPT4 = 6 -> PH3 -> OC4A
OPT5 = 8 -> PH5 -> OC4C
OPT6 = 12 -> PB6 -> OC1B
OPT7 = 2 -> PE4 -> OC3B
OPT8 = 45 -> PL4 -> OC5B
OPT9 = 5 -> PE3 -> OC3A
OPT10 = 7 -> PH4 -> OC4B
OPT11 = 11 -> PB5 -> OC1A
OPT12 = 13 -> PB7 -> OC1C
OPT13 = 44 -> PL5 -> OC5C
OPT14 = 9 -> PH6 -> OC2B
OPT15 = 10 -> PB4 -> OC2A
OPT16 = 18 -> PD3 -> TXD1
OPT17 = 38 -> PD7 -> T0
OPT18 = 15 -> PJ0 -> RXD3
*/
// I2C Multiplexer
#define I2C_Multiplexeur
#define INTERNAL_INPUT_I2C
#define INTERNAL_OUTPUT_I2C
// I2C Multiplexer
#define I2C_MULTIPLEXER_ADDRESS 0x70
#define MCP_NUM 4
const uint8_t MCP_ADDRESS[MCP_NUM] PROGMEM = {0x20, 0x21, 0x22, 0x23};
#define MCP_CHANNEL 0b00000010
#define MCP_PIN_INTERUPT 48

// sortie relay
#define RELAY_MIN_TIME 10
#define RELAY_NUM 32
const uint8_t RELAY_ARRAY[RELAY_NUM] PROGMEM = {23, 25, 27, 29, 31, 33, 35, 37, 22, 24, 26, 28, 30, 32, 34, 36, 100, 101, 102, 103, 104, 105, 106, 107, 200, 201, 202, 203, 204, 205, 206, 207};
const bool RELAY_REVERT[RELAY_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, true, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};



// input 12V 230V
#define INPUT_NUM 32
const uint16_t INPUT_ARRAY[INPUT_NUM] PROGMEM = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 300, 301, 302, 303, 304, 305, 306, 307, 400, 401, 402, 403, 404, 405, 406, 407};
const bool INPUT_REVERT[INPUT_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

// sonde de temperature pt100
#define TEMP_RREF 430.0
const uint8_t OPT_ARRAY[18] PROGMEM = {OPT_1, OPT_2, OPT_3, OPT_4, OPT_5, OPT_6, OPT_7, OPT_8, OPT_9, OPT_10, OPT_11, OPT_12, OPT_13, OPT_14, OPT_15, OPT_16, OPT_17, OPT_18};

// Dimmer
// const uint8_t DIMMER_ARRAY[14] PROGMEM = {OPT_2, OPT_3, OPT_4, OPT_5, OPT_6, OPT_7, OPT_8, OPT_9, OPT_10, OPT_11, OPT_12, OPT_13, OPT_14, OPT_15};


// reseau
#define NETWORK_CS 40
#define NETWORK_RESET 49


// // Fake NTC (Digital pot)

// #define FAKE_NTC_CS 43
// #define FAKE_NTC_R1_ADDRESS 0x11
// #define FAKE_NTC_R2_ADDRESS 0x12

// RS485
//#define RS485_IO 42
#define RS485_TX 16
#define RS485_RX 17
#define RS485 Serial1

#endif