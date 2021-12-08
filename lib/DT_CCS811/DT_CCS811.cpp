#include <DT_CCS811.h>
#include <pinout.h>

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
        uint16_t address = ccs811_address[num];
        uint8_t i2c_number = pgm_read_byte(ccs811_I2C + num);

        // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); //change I2C channel
        // Wire.write(1 << (i2c_number - 1));
        // Wire.endTransmission();
        Serial.println(address, HEX);
        Serial.println(i2c_number);
        unsigned status = ccs811[num].begin(address);

        if (!status)
        {
            Serial.println("failed to init chip, please check if the chip connection is fine");
            ccs811_active[num] = false;
        }
        else
        {
            // ccs811[num].setSampling();
            ccs811[num].setDriveMode(CCS811_DRIVE_MODE_60SEC);
            ccs811_active[num] = true;
        }
    }
}

void DT_CCS811_loop()
{
    uint16_t now = millis();
    static uint16_t old = 0;
    if (now - old >= 1000)
    {
        old = now;
        for (uint8_t num = 0; num < CCS811_NUM; ++num)
        {
            if (ccs811_active[num])
            {
                uint8_t i2c_number = pgm_read_byte(ccs811_I2C + num);

                // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); //change I2C channel
                // Wire.write(1 << (i2c_number - 1));
                // Wire.endTransmission();

                if (ccs811[num].available())
                {
                    if (!ccs811[num].readData())
                    {

                        float value = ccs811[num].geteCO2();
                        if (value != ccs811_co2[num])
                        {
                            ccs811_co2[num] = value;
                            if (ccs811_callback_co2 != nullptr)
                                ccs811_callback_co2(num + 1, value);
                        }

                        value = ccs811[num].getTVOC();
                        if (value != ccs811_cov[num])
                        {
                            ccs811_cov[num] = value;
                            if (ccs811_callback_cov != nullptr)
                                ccs811_callback_cov(num + 1, value);
                        }
                    }
                    else
                    {
                        Serial.println("ccs811 ERROR!");
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
    return ccs811_co2[num - 1];
}

float DT_CCS811_get_cov(const uint8_t num)
{
    return ccs811_cov[num - 1];
}

void DT_CCS811_set_environmental_data(uint8_t num, float humidity, float temperature)
{
    if (ccs811_active[num - 1])
        ccs811[num - 1].setEnvironmentalData(humidity, temperature);
}