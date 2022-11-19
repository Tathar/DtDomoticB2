#include "Wire.h"
#include "HDC1080JS.h"

HDC1080JS::HDC1080JS()
{
}

void HDC1080JS::config()
{
	// config the temp sensor to read temp then humidity in one transaction
	// config the resolution to 14 bits for temp & humidity
	time_temp = time_humidity = 0;
	old_temp = TEMP_DEFAULT_HDC1080;
	old_humidity = 0;
	writeRegister(0x02, 0x10);
}

void HDC1080JS::writeRegister(uint8_t address, uint16_t value)
{
	Wire.beginTransmission(ADDR);
	Wire.write(address);
	Wire.write(value);
	Wire.endTransmission();
}

// returns temp in celcius
float HDC1080JS::getTemp_async()
{
	if (time_temp == 0)
	{
		Wire.beginTransmission(ADDR);
		Wire.write(0x00);
		Wire.endTransmission();
		time_temp = millis();
	}
	else if (millis() - time_temp > 15) // delay(15);
	{
		uint16_t temperatureRaw = temperatureRaw << 8 | Wire.read();
		temperatureRaw = temperatureRaw << 8 | Wire.read();
		// (rawTemp/2^16)*165 - 40
		old_temp = ((float)temperatureRaw) * 165 / 65536 - 40;
		time_temp = 0;
	}

	return old_temp;
}

float HDC1080JS::getRelativeHumidity_async()
{

	if (time_humidity == 0)
	{
		Wire.beginTransmission(ADDR);
		Wire.write(0x01);
		Wire.endTransmission();
		time_humidity = millis();
	}
	else if (millis() - time_humidity > 15) // delay(15);
	{
		Wire.requestFrom(ADDR, 2);
		uint16_t humidityRaw = humidityRaw << 8 | Wire.read();
		humidityRaw = humidityRaw << 8 | Wire.read();
		//(rawHumidity/2^16)*100
		old_humidity = ((float)humidityRaw) * 100 / 65536;
		time_humidity = 0;
	}
	return old_humidity;
}
