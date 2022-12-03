#ifndef HDC1080JS_h
#define HDC1080JS_h

#include "Arduino.h"

#define ADDR 0x40

// Can use up to 400khz Clockspeed

#define TEMP_DEFAULT_HDC1080 -273.15 // temperature renvoyer en cas de default (zero absolut)

class HDC1080JS
{
public:
	HDC1080JS();
	void config();

	void computeTempHumidityAsync();

	float getTemp_async();
	float getRelativeHumidity_async();

private:
	uint32_t time;
	float temp;
	float humidity;

	void writeRegister(uint8_t address, uint8_t value);
};
#endif
