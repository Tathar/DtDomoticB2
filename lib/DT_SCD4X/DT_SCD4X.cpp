#include <DT_SCD4X.h>
#include <config.h>
#if SCD4X_NUM > 0
#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>

SensirionI2CScd4x scd4x[SCD4X_NUM];
bool scd4x_active[SCD4X_NUM];
float _temperature[SCD4X_NUM];
float _humidity[SCD4X_NUM];
float _co2[SCD4X_NUM];

void (*scd4x_callback_temperature)(const uint8_t num, const float temperature);
void (*scd4x_callback_humidity)(const uint8_t num, const float humidity);
void (*scd4x_callback_co2)(const uint8_t num, const float co2);

void DT_SCD4X_init()
{
    scd4x_callback_temperature = nullptr;
    scd4x_callback_humidity = nullptr;
    scd4x_callback_co2 = nullptr;
    for (uint8_t num = 0; num < SCD4X_NUM; ++num)
    {
        uint8_t i2c_channel = pgm_read_byte(SCD4X_CHANNEL_ARRAY + num);

        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
        Wire.write(i2c_channel_to_multiplexer(i2c_channel));
        Wire.endTransmission();

        // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS);
        // Wire.write(MCP_CHANNEL);
        // Wire.endTransmission();

        // auto Serial.println(address, HEX);
        // auto Serial.println(i2c_number);
        scd4x[num].begin(Wire);

        // stop potentially previously started measurement
        // uint16_t error = scd4x.stopPeriodicMeasurement();
        // if (error)
        // {
        //     Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        //     errorToString(error, errorMessage, 256);
        //     Serial.println(errorMessage);
        // }

        scd4x[num].stopPeriodicMeasurement();
        // Start Measurement
        uint16_t error = scd4x[num].startPeriodicMeasurement();
        if (error)
        {
            Serial.print(F("Error trying to execute startPeriodicMeasurement(): "));
            Serial.println(error);
            // char errorMessage[256];
            // errorToString(error, errorMessage, 256);
            // Serial.println(errorMessage);
        }
        else
        {
            scd4x_active[num] = true;
        }
    }
}

void DT_SCD4X_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;
    if (now - old >= 10000)
    {
        old = now;
        for (uint8_t num = 0; num < SCD4X_NUM; ++num)
        {
            if (scd4x_active[num])
            {
                uint8_t i2c_channel = pgm_read_byte(SCD4X_CHANNEL_ARRAY + num);

                Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
                Wire.write(i2c_channel_to_multiplexer(i2c_channel));
                Wire.endTransmission();

                uint16_t co2;
                float temperature;
                float humidity;
                uint16_t error = scd4x[num].readMeasurement(co2, temperature, humidity);
                if (error)
                {
                    Serial.print(F("Error trying to execute readMeasurement(): "));
                    // char errorMessage[256];
                    // errorToString(error, errorMessage, 256);
                    Serial.println(error);
                }
                else
                {
                    if (temperature != _temperature[num])
                    {
                        _temperature[num] = temperature;
                        Serial.print(F("SCD4X temperature = "));
                        Serial.println(temperature);
                        if (scd4x_callback_temperature != nullptr)
                            scd4x_callback_temperature(num, temperature);
                    }

                    if (co2 != _co2[num])
                    {
                        _co2[num] = co2;
                        if (scd4x_callback_co2 != nullptr)
                            scd4x_callback_co2(num, co2);
                    }

                    if (humidity != _humidity[num])
                    {
                        _humidity[num] = humidity;
                        if (scd4x_callback_humidity != nullptr)
                            scd4x_callback_humidity(num, humidity);
                    }
                }
            }
        }
    }
}

void DT_SCD4X_set_callback_temperature(void (*callback)(const uint8_t num, const float temperature))
{
    scd4x_callback_temperature = callback;
}

void DT_SCD4X_set_callback_humidity(void (*callback)(const uint8_t num, const float humidity))
{
    scd4x_callback_humidity = callback;
}

void DT_SCD4X_set_callback_co2(void (*callback)(const uint8_t num, const float pressure))
{
    scd4x_callback_co2 = callback;
}

float DT_SCD4X_get_temperature(const uint8_t num)
{
    return _temperature[num];
}

float DT_SCD4X_get_humidity(const uint8_t num)
{
    return _humidity[num];
}

float DT_SCD4X_get_co2(const uint8_t num)
{
    return _co2[num];
}
#endif // BME280_NUM