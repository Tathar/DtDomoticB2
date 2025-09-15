#include <DT_HDC1080.h>
#include <config.h>
#if HDC1080_NUM > 0
#include <HDC1080JS.h>
#include <Wire.h>

HDC1080JS HDC1080[HDC1080_NUM];
float hdc1080_temperature[HDC1080_NUM];
float hdc1080_humidity[HDC1080_NUM];

void (*HDC1080_callback_temperature)(const uint8_t num, const float temperature);
void (*HDC1080_callback_humidity)(const uint8_t num, const float humidity);

void DT_HDC1080_init()
{
    HDC1080_callback_temperature = nullptr;
    HDC1080_callback_humidity = nullptr;
    for (uint8_t num = 0; num < HDC1080_NUM; ++num)
    {
        uint8_t i2c_channel = pgm_read_byte(HDC1080_CHANNEL_ARRAY + num);
        // Serial.println(i2c_channel);
        // Serial.println(i2c_channel_to_multiplexer(i2c_channel), 2);

        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
        Wire.write(i2c_channel_to_multiplexer(i2c_channel));
        Wire.endTransmission();
        // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
        // Wire.write(MCP_CHANNEL);
        // Wire.endTransmission();

        // auto Serial.println(address, HEX);
        // auto Serial.println(i2c_number);
        HDC1080[num].config();
    }
}

void DT_HDC1080_loop()
{
    uint16_t now = millis();
    static uint16_t old = 0;
    static uint8_t num = 0;

    if (now - old >= 1000)
    {
        old = now;
        uint8_t i2c_channel = pgm_read_byte(HDC1080_CHANNEL_ARRAY + num);

        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
        Wire.write(i2c_channel_to_multiplexer(i2c_channel));
        Wire.endTransmission();

        HDC1080[num].computeTempHumidityAsync();

        float value = HDC1080[num].getTemp_async();
        if (value != hdc1080_temperature[num])
        {
            hdc1080_temperature[num] = value;
            if (HDC1080_callback_temperature != nullptr)
                HDC1080_callback_temperature(num, value);
        }

        value = HDC1080[num].getRelativeHumidity_async();
        if (value != hdc1080_humidity[num])
        {
            hdc1080_humidity[num] = value;
            if (HDC1080_callback_humidity != nullptr)
                HDC1080_callback_humidity(num, value);
        }

        if (++num == HDC1080_NUM)
        {
            num = 0;
        }
    }
}

void DT_HDC1080_set_callback_temperature(void (*callback)(const uint8_t num, const float hdc1080_temperature))
{
    HDC1080_callback_temperature = callback;
}

void DT_HDC1080_set_callback_humidity(void (*callback)(const uint8_t num, const float humidity))
{
    HDC1080_callback_humidity = callback;
}

float DT_HDC1080_get_temperature(const uint8_t num)
{
    return hdc1080_temperature[num];
}

float DT_HDC1080_get_humidity(const uint8_t num)
{
    return hdc1080_humidity[num];
}

#endif // HDC1080_NUM