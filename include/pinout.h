#ifndef PINOUT
#define PINOUT

#include <avr/pgmspace.h>

// sortie relay
#define RELAY_MIN_TIME 10
#define RELAY_NUM 32
const uint8_t RELAY_ARRAY[RELAY_NUM] PROGMEM = {13, 12, 11, 8, 7, 6, 5, 3, 2, 44, 45, 46, 10, 9, 4, 47, 100, 101, 102, 103, 104, 105, 106, 107, 200, 201, 202, 203, 204, 205, 206, 207};
const bool RELAY_REVERT[RELAY_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

#define MCP_NUM 2
const uint8_t MCP_ADDRESS[MCP_NUM] PROGMEM = {0x40, 0x42};

// input 24V 230V
#define INPUT_NUM 20
const uint8_t INPUT_ARRAY[INPUT_NUM] PROGMEM = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 14, 15, 16, 17};
const bool INPUT_REVERT[INPUT_NUM] PROGMEM = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};

// sonde de temperature pt100
#define TEMP_RREF 430.0
#define TEMP_NUM 20
const uint8_t TEMP_ARRAY[TEMP_NUM] PROGMEM = {23, 22, 25, 24, 27, 26, 29, 28, 31, 30, 33, 30, 35, 34, 37, 36, 39, 38, 41, 40};

// sonde de temperature bme280
#define BME280_NUM 2
const uint8_t BME280_ADDRESS[BME280_NUM] PROGMEM = {0x76, 0x77};
const uint8_t BME280_I2C[BME280_NUM] PROGMEM = {1, 1};

// sonde cov CCS811
#define CCS811_NUM 2
const uint8_t CCS811_ADDRESS[CCS811_NUM] PROGMEM = {0x5A, 0x5B};
const uint8_t CCS811_I2C[CCS811_NUM] PROGMEM = {1, 1};

// reseau
#define NETWORK_CS 53
#define NETWORK_RESET 49

// I2C Multiplexer
#define I2C_MULTIPLEXER_ADDRESS 0x70

// Fake NTC (Digital pot)

#define FAKE_NTC_CS 43
#define FAKE_NTC_R1_ADDRESS 0x11
#define FAKE_NTC_R2_ADDRESS 0x12

//RS485
#define RS485_IO 42
#define RS485_TX 18
#define RS485_RX 19
#define RS485 Serial1

#endif