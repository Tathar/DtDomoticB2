#include <DT_mcp.h>

Adafruit_MCP23X08 mcp[MCP_NUM];

void DT_mcp_init()
{
    uint8_t i2c = 0;

    for (uint8_t mcp_num = 0; mcp_num < MCP_NUM; ++mcp_num)
    {
        i2c = pgm_read_byte(MCP_ADDRESS + mcp_num);
        if (!mcp[mcp_num].begin_I2C(i2c))
        {
            {
                Serial.println("MCP23008 Init Error.");
            }
        }
    }
}