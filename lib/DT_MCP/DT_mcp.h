#pragma once
#include <pinout.h>
#include <Adafruit_MCP23X08.h>

// extern Adafruit_MCP23X08 mcp[MCP_NUM];

void DT_mcp_init();
void DT_mcp_pinMode(uint8_t num, uint8_t pin, uint8_t mode);
uint8_t DT_mcp_digitalRead(uint8_t num, uint8_t pin);
void DT_mcp_digitalWrite(uint8_t num, uint8_t pin, uint8_t value);

void DT_mcp_setupInterrupts(uint8_t num, bool mirroring, bool openDrain, uint8_t polarity);
void DT_mcp_setupInterruptPin(uint8_t num, uint8_t pin, uint8_t mode);