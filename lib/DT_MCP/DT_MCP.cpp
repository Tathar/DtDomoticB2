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
                Serial.print(mcp_num);
                Serial.println(F(" Init Error."));
            }
        }
    }
}

void DT_mcp_pinMode(uint8_t num, uint8_t pin, uint8_t mode, bool fast)
{
    if (fast == false)
    {
        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
        Wire.write(MCP_CHANNEL);
        Wire.endTransmission();
    }
    mcp[num].pinMode(pin, mode);
}

uint8_t DT_mcp_digitalRead(uint8_t num, uint8_t pin, bool fast)
{
    if (fast == false)
    {
        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
        Wire.write(MCP_CHANNEL);
        Wire.endTransmission();
    }
    return mcp[num].digitalRead(pin);
}

bool DT_mcp_digitalReads(uint8_t num, uint8_t (*data)[8]) // data is unit_t[8]
{
    Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
    Wire.write(MCP_CHANNEL);
    Wire.endTransmission();
    uint8_t gpio = mcp[num].readGPIO();

    (*data)[0] = (gpio & _BV(0)) != LOW;
    (*data)[1] = (gpio & _BV(1)) != LOW;
    (*data)[2] = (gpio & _BV(2)) != LOW;
    (*data)[3] = (gpio & _BV(3)) != LOW;
    (*data)[4] = (gpio & _BV(4)) != LOW;
    (*data)[6] = (gpio & _BV(5)) != LOW;
    (*data)[6] = (gpio & _BV(6)) != LOW;
    (*data)[7] = (gpio & _BV(7)) != LOW;

    return true;
}

void DT_mcp_digitalWrite(uint8_t num, uint8_t pin, uint8_t value, bool fast)
{
    if (fast == false)
    {
        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
        Wire.write(MCP_CHANNEL);
        Wire.endTransmission();
    }

    mcp[num].digitalWrite(pin, value);
}

void DT_mcp_setupInterrupts(uint8_t num, bool mirroring, bool openDrain, uint8_t polarity, bool fast)
{
    if (fast == false)
    {
        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
        Wire.write(MCP_CHANNEL);
        Wire.endTransmission();
    }

    mcp[num].setupInterrupts(mirroring, openDrain, polarity);
}

void DT_mcp_setupInterruptPin(uint8_t num, uint8_t pin, uint8_t mode, bool fast)
{
    if (fast == false)
    {
        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
        Wire.write(MCP_CHANNEL);
        Wire.endTransmission();
    }

    mcp[num].setupInterruptPin(pin, mode);
}