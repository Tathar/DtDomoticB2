#ifndef PINOUT
#define PINOUT

#include <avr/pgmspace.h>

// sortie relay
#define RELAY_MIN_TIME 10
#define RELAY_NUM 32
const uint8_t RELAY_ARRAY[RELAY_NUM] PROGMEM = {23, 25, 27, 29, 31, 33, 35, 37, 22, 24, 26, 28, 30, 32, 34, 36, 100, 101, 102, 103, 104, 105, 106, 107, 200, 201, 202, 203, 204, 205, 206, 207};
const bool RELAY_REVERT[RELAY_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, true, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

#define MCP_NUM 4
const uint8_t MCP_ADDRESS[MCP_NUM] PROGMEM = {0x40, 0x42, 0x44, 0x46};

// input 12V 230V
#define INPUT_NUM 32
const uint16_t INPUT_ARRAY[INPUT_NUM] PROGMEM = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 200, 201, 202, 203, 204, 205, 206, 207, 300, 301, 302, 303, 304, 305, 306, 307};
const bool INPUT_REVERT[INPUT_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

// sonde de temperature pt100
#define TEMP_RREF 430.0
#define TEMP_NUM 10 // 12 ok 13 ko
const uint8_t TEMP_ARRAY[18] PROGMEM = {19, 46, 3, 6, 8, 12, 2, 45, 5, 7, 11, 13, 9, 10, 18, 38, 15};

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