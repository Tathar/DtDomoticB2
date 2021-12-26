#include <DT_BME280.h>
#include <pinout.h>

// #include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme280[BME280_NUM];
bool bme280_active[BME280_NUM];
float temperature[BME280_NUM];
float humidity[BME280_NUM];
float pressure[BME280_NUM];

void (*bme280_callback_temperature)(const uint8_t num, const float temperature);
void (*bme280_callback_humidity)(const uint8_t num, const float humidity);
void (*bme280_callback_pressure)(const uint8_t num, const float pressure);

void DT_BME280_init()
{
    bme280_callback_temperature = nullptr;
    bme280_callback_humidity = nullptr;
    bme280_callback_pressure = nullptr;
    for (uint8_t num = 0; num < BME280_NUM; ++num)
    {
        uint16_t address = pgm_read_byte(BME280_ADDRESS + num);
        uint8_t i2c_number = pgm_read_byte(BME280_ADDRESS + num);

        // Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); //change I2C channel
        // Wire.write(1 << (i2c_number - 1));
        // Wire.endTransmission();
       //auto Serial.println(address, HEX);
       //auto Serial.println(i2c_number);
        unsigned status = bme280[num].begin(address);

        if (!status)
        {
           //auto Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
           //auto Serial.print("SensorID was: 0x");
           //auto Serial.println(bme280[num].sensorID(), 16);
           //auto Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
           //auto Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
           //auto Serial.print("        ID of 0x60 represents a BME 280.\n");
           //auto Serial.print("        ID of 0x61 represents a BME 680.\n");
            bme280_active[num] = false;
        }
        else
        {
            bme280[num].setSampling();
            bme280_active[num] = true;
        }
    }
}

void DT_BME280_loop()
{
    uint16_t now = millis();
    static uint16_t old = 0;
    if (now - old >= 1000)
    {
        old = now;
        for (uint8_t num = 0; num < BME280_NUM; ++num)
        {
            if (bme280_active[num])
            {
                uint8_t i2c_number = pgm_read_byte(BME280_ADDRESS + num);

                Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); //change I2C channel
                Wire.write(1 << (i2c_number - 1));
                Wire.endTransmission();

                float value = bme280[num].readTemperature();
                if (value != temperature[num])
                {
                    temperature[num] = value;
                    if (bme280_callback_temperature != nullptr)
                        bme280_callback_temperature(num + 1, value);
                }

                value = bme280[num].readPressure();
                if (value != pressure[num])
                {
                    pressure[num] = value;
                    if (bme280_callback_pressure != nullptr)
                        bme280_callback_pressure(num + 1, value);
                }

                value = bme280[num].readHumidity();
                if (value != humidity[num])
                {
                    humidity[num] = value;
                    if (bme280_callback_humidity != nullptr)
                        bme280_callback_humidity(num + 1, value);
                }
            }
        }
    }
}

void DT_BME280_set_callback_temperature(void (*callback)(const uint8_t num, const float temperature))
{
    bme280_callback_temperature = callback;
}

void DT_BME280_set_callback_humidity(void (*callback)(const uint8_t num, const float humidity))
{
    bme280_callback_humidity = callback;
}

void DT_BME280_set_callback_pressure(void (*callback)(const uint8_t num, const float pressure))
{
    bme280_callback_pressure = callback;
}

float DT_BME280_get_temperature(const uint8_t num)
{
    return temperature[num - 1];
}

float DT_BME280_get_humidity(const uint8_t num)
{
    return humidity[num - 1];
}

float DT_BME280_get_pressure(const uint8_t num)
{
    return pressure[num - 1];
}