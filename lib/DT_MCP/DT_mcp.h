#pragma once
#include <pinout.h>
#include <Adafruit_MCP23X08.h>

extern Adafruit_MCP23X08 mcp[MCP_NUM];

void DT_mcp_init();