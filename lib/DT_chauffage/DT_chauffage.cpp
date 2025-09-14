// #include <DT_chauffage.h>
#include <DT_poele.h>
// #include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
// #include <DT_fake_ntc.h>

#include <QuickPID.h>
#include <DT_eeprom.h>

#include <config.h>
#ifdef CHAUFFAGE
void (*chauffage_mode_callback)(const DT_Chauffage_mode mode);
void (*temperature_arret_poele_hiver_callback)(const float data);
void (*arret_meteo_callback)(const bool data);

bool async_call_chauffage_mode;
bool async_call_temperature_arret_poele_hiver;
bool async_call_arret_meteo;

void DT_Chauffage_init()
{
    // DT_relay(RELAY_EV1, false);
    chauffage_mode_callback = nullptr;

    async_call_chauffage_mode = false;
    async_call_temperature_arret_poele_hiver = false;
    async_call_arret_meteo = false;
}

void DT_Chauffage_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;

    if (now - old >= 1000)
    {
        // 220502  debug(F(AT));
        old = now;
        if (eeprom_config.chauffage_mode == DT_CHAUFFAGE_ETE)
        {
        }
        else if (eeprom_config.chauffage_mode == DT_CHAUFFAGE_INTERSAISON)
        {
        }
        else if (eeprom_config.chauffage_mode == DT_CHAUFFAGE_HIVER)
        {
            DT_relay(9, false);
            DT_relay(12, false);
            

        }
    }

    if (chauffage_mode_callback != nullptr && async_call_chauffage_mode == true)
    {
        async_call_chauffage_mode = false;
        chauffage_mode_callback(eeprom_config.chauffage_mode);
    }

    if (temperature_arret_poele_hiver_callback != nullptr && async_call_chauffage_mode == true)
    {
        async_call_chauffage_mode = false;
        temperature_arret_poele_hiver_callback(eeprom_config.temperature_arret_poele_hiver);
    }

    if (arret_meteo_callback != nullptr && async_call_arret_meteo == true)
    {
        async_call_arret_meteo = false;
        arret_meteo_callback(mem_config.ha_arret_meteo);
    }
}

void DT_Chauffage_set_mode(DT_Chauffage_mode mode)
{
    if (mode != eeprom_config.chauffage_mode)
    {
        eeprom_config.chauffage_mode = mode;
        async_call_chauffage_mode = true;
    }
    sauvegardeEEPROM();
}

DT_Chauffage_mode DT_Chauffage_get_mode(void)
{
    return eeprom_config.chauffage_mode;
}

void DT_Chauffage_set_mode_callback(void (*callback)(const DT_Chauffage_mode mode))
{
    chauffage_mode_callback = callback;
}

void DT_Chauffage_set_temperature_arret_poele_hiver(float data)
{
    if (data != eeprom_config.temperature_arret_poele_hiver)
    {
        eeprom_config.temperature_arret_poele_hiver = data;
        async_call_temperature_arret_poele_hiver = true;
    }
    sauvegardeEEPROM();
}

float DT_Chauffage_get_temperature_arret_poele_hiver(void)
{
    return eeprom_config.temperature_arret_poele_hiver;
};

void DT_Chauffage_set_temperature_arret_poele_hiver_callback(void (*callback)(const float data))
{
    temperature_arret_poele_hiver_callback = callback;
}

void DT_Chauffage_set_arret_meteo(bool data)
{
    if (data != mem_config.ha_arret_meteo)
    {
        mem_config.ha_arret_meteo = data;
        async_call_arret_meteo = true;
    }
};

bool DT_Chauffage_get_arret_meteo(void){
    return mem_config.ha_arret_meteo;
};

void DT_Chauffage_set_arret_meteo_callback(void (*callback)(const bool data)){
    arret_meteo_callback = callback;
};

#endif // CHAUFFAGE