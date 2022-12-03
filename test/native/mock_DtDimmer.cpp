#include <DtDimmer.h>

#include <config.h>
#include <pinout.h>

uint8_t value[MAX_OUT];

void Dimmer_init(void)
{

    printf("Dimmer_init\n");
    for (uint8_t i = 0; i < MAX_OUT; ++i)
    {
        value[i] = 0;
    }
}

void dimmer_set(uint8_t num, uint8_t percent, uint8_t speed, bool candle)
{
    value[num] = percent;
    // printf("dimmer_set(num = %i ,percent = %i ,speed= %i ,candle=%i)\n", num, percent, speed, candle);
}

void dimmer_slow_set(uint8_t num, uint8_t percent)
{
    value[num] = percent;
    printf("dimmer_slow_set(%i,%i)\n", num, percent);
}

uint8_t get_dimmer(uint8_t num)
{
    return value[num];
}