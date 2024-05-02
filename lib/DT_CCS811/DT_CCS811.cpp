#include <DT_CCS811.h>
#include <config.h>
#if CCS811_NUM > 0
// #include <Adafruit_Sensor.h>
#include <Adafruit_CCS811.h>

Adafruit_CCS811 ccs811[CCS811_NUM];
bool ccs811_active[CCS811_NUM];
float ccs811_co2[CCS811_NUM];
float ccs811_cov[CCS811_NUM];

void (*ccs811_callback_co2)(const uint8_t num, const float co2);
void (*ccs811_callback_cov)(const uint8_t num, const float cov);

void DT_CCS811_init()
{
    ccs811_callback_co2 = nullptr;
    ccs811_callback_cov = nullptr;
    for (uint8_t num = 0; num < CCS811_NUM; ++num)
    {
        uint16_t address = pgm_read_byte(CCS811_ADDRESS_ARRAY + num);
        uint8_t i2c_channel = pgm_read_byte(CCS811_CHANNEL_ARRAY + num);

        Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
        Wire.write(i2c_channel_to_multiplexer(i2c_channel));
        Wire.endTransmission();

        Serial.println(address, HEX);
        Serial.println(i2c_channel);
        unsigned status = ccs811[num].begin(address,&Wire);

        if (!status)
        {
            Serial.println(F("failed to init chip, please check if the chip connection is fine"));
            ccs811_active[num] = false;
        }
        else
        {
            // ccs811[num].setSampling();
            ccs811[num].setDriveMode(CCS811_DRIVE_MODE_10SEC);
            ccs811_active[num] = true;
            Serial.println(F("CCS811 initialised"));
        }
    }
}

void DT_CCS811_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;
    if (now - old >= 1000)
    {
        old = now;
        for (uint8_t num = 0; num < CCS811_NUM; ++num)
        {
            if (ccs811_active[num])
            {
                uint8_t i2c_channel = pgm_read_byte(CCS811_CHANNEL_ARRAY + num);

                Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
                Wire.write(i2c_channel_to_multiplexer(i2c_channel));
                Wire.endTransmission();

                if (ccs811[num].available())
                {
                    if (!ccs811[num].readData())
                    {

                        float value = ccs811[num].geteCO2();
                        // Serial.print(F("ccs811 co2 "));
                        // Serial.println(value);
                        if (value != ccs811_co2[num])
                        {
                            ccs811_co2[num] = value;
                            if (ccs811_callback_co2 != nullptr)
                                ccs811_callback_co2(num, value);
                        }

                        value = ccs811[num].getTVOC();
                        // Serial.print(F("ccs811 tov "));
                        // Serial.println(value);
                        if (value != ccs811_cov[num])
                        {
                            ccs811_cov[num] = value;
                            if (ccs811_callback_cov != nullptr)
                                ccs811_callback_cov(num, value);
                        }
                    }
                    else
                    {
                        Serial.println(F("ccs811 ERROR!"));
                    }
                }
            }
        }
    }
}

void DT_CCS811_set_callback_co2(void (*callback)(const uint8_t num, const float temperature))
{
    ccs811_callback_co2 = callback;
}

void DT_CCS811_set_callback_cov(void (*callback)(const uint8_t num, const float humidity))
{
    ccs811_callback_cov = callback;
}

float DT_CCS811_get_co2(const uint8_t num)
{
    return ccs811_co2[num];
}

float DT_CCS811_get_cov(const uint8_t num)
{
    return ccs811_cov[num];
}

void DT_CCS811_set_environmental_data(uint8_t num, float humidity, float temperature)
{
    if (ccs811_active[num])
        ccs811[num].setEnvironmentalData(humidity, temperature);
}
#endif