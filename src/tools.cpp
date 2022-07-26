#include <tools.h>

int i2c_channel_to_multiplexer(int channel)
{
    if (channel == 1)
            return 0b10;
    else if (channel == 2)
            return 0b1;
    else 
            return 1 << (channel - 1);
}