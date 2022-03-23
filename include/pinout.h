#ifndef PINOUT
#define PINOUT

#include <avr/pgmspace.h>

#define OPT_1 19
#define OPT_2 46
#define OPT_3 3
#define OPT_4 6
#define OPT_5 8
#define OPT_6 12
#define OPT_7 2
#define OPT_8 45
#define OPT_9 5
#define OPT_10 7
#define OPT_11 11
#define OPT_12 13
#define OPT_13 9
#define OPT_14 10
#define OPT_15 18
#define OPT_16 38
#define OPT_17 15
#define OPT_18 14

// sortie relay
#define RELAY_MIN_TIME 10
#define RELAY_NUM 32
const uint8_t RELAY_ARRAY[RELAY_NUM] PROGMEM = {23, 25, 27, 29, 31, 33, 35, 37, 22, 24, 26, 28, 30, 32, 34, 36, 100, 101, 102, 103, 104, 105, 106, 107, 200, 201, 202, 203, 204, 205, 206, 207};
const bool RELAY_REVERT[RELAY_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, true, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

#define MCP_NUM 4
const uint8_t MCP_ADDRESS[MCP_NUM] PROGMEM = {0x20, 0x21, 0x22, 0x23};
#define MCP_CHANNEL 0b00000010
#define MCP_PIN_INTERUPT 48

// input 12V 230V
#define INPUT_NUM 32
const uint16_t INPUT_ARRAY[INPUT_NUM] PROGMEM = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 300, 301, 302, 303, 304, 305, 306, 307, 400, 401, 402, 403, 404, 405, 406, 407};
const bool INPUT_REVERT[INPUT_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

// sonde de temperature pt100
#define TEMP_RREF 430.0
#define TEMP_NUM 10 // 12 ok 13 ko
const uint8_t OPT_ARRAY[18] PROGMEM = {OPT_1, OPT_2, OPT_3, OPT_4, OPT_5, OPT_6, OPT_7, OPT_8, OPT_9, OPT_10, OPT_11, OPT_12, OPT_13, OPT_14, OPT_15, OPT_16, OPT_17, OPT_18};

// Dimmer
#define DIMMER_NUM 14
const uint8_t DIMMER_ARRAY[14] PROGMEM = {OPT_2, OPT_3, OPT_4, OPT_5, OPT_6, OPT_7, OPT_8, OPT_9, OPT_10, OPT_11, OPT_12, OPT_13, OPT_14, OPT_15};

// sonde de temperature bme280
#define BME280_NUM 2
const uint8_t BME280_ADDRESS[BME280_NUM] PROGMEM = {0x76, 0x77};
const uint8_t BME280_I2C[BME280_NUM] PROGMEM = {2, 2};

// sonde cov CCS811
#define CCS811_NUM 2
const uint8_t CCS811_ADDRESS[CCS811_NUM] PROGMEM = {0x5A, 0x5B};
const uint8_t CCS811_I2C[CCS811_NUM] PROGMEM = {2, 2};

// reseau
#define NETWORK_CS 40
#define NETWORK_RESET 49

// I2C Multiplexer
#define I2C_MULTIPLEXER_ADDRESS 0x70

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