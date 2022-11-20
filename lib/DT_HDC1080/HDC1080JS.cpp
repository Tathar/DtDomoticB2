#include "Wire.h"
#include "HDC1080JS.h"

HDC1080JS::HDC1080JS()
{
}

void HDC1080JS::config()
{
	// config the temp sensor to read temp then humidity in one transaction
	// config the resolution to 14 bits for temp & humidity
	time = 0;
	temp = TEMP_DEFAULT_HDC1080;
	humidity = 0;
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
void HDC1080JS::computeTempHumidityAsync()
{
	if (time == 0)
	{
		Wire.beginTransmission(ADDR);
		Wire.write(0x00);
		Wire.endTransmission();
		time = millis();
	}
	else if (millis() - time > 15) // delay(15);
	{
		Wire.requestFrom(ADDR, 4);
		uint16_t temperatureRaw = temperatureRaw << 8 | Wire.read();
		temperatureRaw = temperatureRaw << 8 | Wire.read();
		uint16_t humidityRaw = humidityRaw << 8 | Wire.read();
		humidityRaw = humidityRaw << 8 | Wire.read();
		// (rawTemp/2^16)*165 - 40
		temp = ((float)temperatureRaw) * 165 / 65536 - 40;
		humidity = ((float)humidityRaw) * 100 / 65536;
		time = 0;
	}
}

float HDC1080JS::getTemp_async()
{
	return temp;
}

float HDC1080JS::getRelativeHumidity_async()
{
	return humidity;
}
