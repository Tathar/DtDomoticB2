#ifndef PINOUT
#define PINOUT

#include <avr/pgmspace.h>

//sortie relay
#define RELAY_MIN_TIME 10
#define RELAY_NUM 10
const uint8_t RELAY_ARRAY[RELAY_NUM] PROGMEM = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
const bool RELAY_REVERT[RELAY_NUM] PROGMEM = {true, true, false, false, false, false, false, false, false, false};

//sonde de temperature pt100
#define TEMP_RREF 430.0
#define TEMP_NUM 2
const uint8_t TEMP_ARRAY[TEMP_NUM] PROGMEM = {11, 12};

//sonde de temperature bme280
#define BME280_NUM 2
// const uint8_t BME280_ADDRESS[BME280_NUM] PROGMEM = {0x76, 0x77};
const uint8_t bme280_address[BME280_NUM] = {0x76, 0x77};
const uint8_t BME280_I2C[BME280_NUM] PROGMEM = {1, 1};

//reseau
#define NETWORK_CS 53
#define NETWORK_RESET 8

//I2C Multiplexer
#define I2C_MULTIPLEXER_ADDRESS 0x70

#endif