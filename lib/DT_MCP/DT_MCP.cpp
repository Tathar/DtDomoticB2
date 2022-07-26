#include <DT_mcp.h>

Adafruit_MCP23X08 mcp[MCP_NUM];

void DT_mcp_init()
{
    uint8_t i2c = 0;

    Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
    Wire.write(MCP_CHANNEL);
    Wire.endTransmission();

    for (uint8_t mcp_num = 0; mcp_num < MCP_NUM; ++mcp_num)
    {
        i2c = pgm_read_byte(MCP_ADDRESS + mcp_num);
        // Serial.println(i2c,HEX);
        if (!mcp[mcp_num].begin_I2C(i2c))
        {
            {
                Serial.print(F("MCP23008 "));
                Serial.print( mcp_num );
                Serial.println(F(" Init Error."));
            }
        }
    }
}