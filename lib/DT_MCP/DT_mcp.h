#pragma once
#include <pinout.h>
#include <Adafruit_MCP23X08.h>

// extern Adafruit_MCP23X08 mcp[MCP_NUM];

void DT_mcp_init();
void DT_mcp_pinMode(uint8_t num, uint8_t pin, uint8_t mode, bool fast = false);
uint8_t DT_mcp_digitalRead(uint8_t num, uint8_t pin, bool fast = false);
void DT_mcp_digitalWrite(uint8_t num, uint8_t pin, uint8_t value, bool fast = false);

bool DT_mcp_digitalReads(uint8_t num, uint8_t (*data)[8]); // data is unit_t[8]

void DT_mcp_setupInterrupts(uint8_t num, bool mirroring, bool openDrain, uint8_t polarity, bool fast = false);
void DT_mcp_setupInterruptPin(uint8_t num, uint8_t pin, uint8_t mode, bool fast = false);