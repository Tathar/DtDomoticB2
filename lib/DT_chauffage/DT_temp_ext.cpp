#include <./DT_temp_ext.h>

#include <DT_PT100.h>
#include <CircularBuffer.h>
#include <DT_eeprom.h>
#include <config.h>

#ifdef DT_PT100_EXT

CircularBuffer<float, 24> temp_buffer;

void (*_callback_avg_temp)();

// initialisation temp_ext
void DT_get_avg_temp_init()
{
    _callback_avg_temp = nullptr;
}

float DT_get_temp_moyen()
{
    float ret = 0;

    for (uint8_t x = 0; x < temp_buffer.size(); ++x)
    {
        ret += temp_buffer[x];
    }

    if (temp_buffer.size() != 0)
        return ret / (float)temp_buffer.size();
    else
        return TEMP_DEFAULT_PT100;
};

// fournie la temperature exterieur moyenné en fonction du decalage choisie
float get_temp_ext()
{
    if (DT_pt100_get(DT_PT100_EXT) > DT_get_temp_moyen() + eeprom_config.in_offset_avg_temp_sup)
    {
        return DT_get_temp_moyen() + eeprom_config.in_offset_avg_temp_sup;
    }
    else if (DT_pt100_get(DT_PT100_EXT) < DT_get_temp_moyen() - eeprom_config.in_offset_avg_temp_inf)
    {
        return DT_get_temp_moyen() - eeprom_config.in_offset_avg_temp_inf;
    }
    else
    {
        return DT_pt100_get(DT_PT100_EXT);
    }
};

void DT_get_avg_temp_loop()
{
    uint32_t now = millis();
    static uint32_t time = 0;
    static bool init = false;
    if (DT_pt100_get(DT_PT100_EXT) != TEMP_DEFAULT_PT100 && (init == false || now - time > 3600000))
    {
        init = true;
        time = now;
        temp_buffer.push(DT_pt100_get(DT_PT100_EXT));
        if (_callback_avg_temp != nullptr)
        {
            _callback_avg_temp();
        }
    }
};

void DT_3voies_set_callback_avg_temp(void (*callback_avg_temp)())
{
    _callback_avg_temp = callback_avg_temp;
}


#endif //DT_PT100_EXT
