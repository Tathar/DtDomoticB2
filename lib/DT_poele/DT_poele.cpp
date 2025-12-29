#include <DT_poele.h>
// #include <DT_input.h>
#include <DT_PT100.h>
#include <DT_HDC1080.h>
#include <DT_SCD4X.h>
#include <DT_clock.h>
#include <DT_relay.h>
#include <DT_mqtt_send.h>
#include <DT_cpt_pulse_input.h>

#include <QuickPID.h>
#include <DT_eeprom.h>

#include <config.h>
#ifdef POELE
// void (*poele_C1_callback)(const uint8_t C1);
//  void (*poele_T4_callback)(const float t4);

// float T4; // Temperature envoyé au poêle
// float C1; // consigne temp Ballon
// T1 = Temp Ballon					T2 = Temp ECS							T3 = Temp ECS2
// T5 = Temp Extérieur					T6 = Temp Vanne 3V PCBT					T7 = Temp Vanne 3V MCBT				T8 = Temp Vanne 3V Jacuzzi

uint32_t temp_default_pt100_B = 0;
uint32_t temp_default_pt100_ECS = 0;
uint32_t millis_default_poele = 0;

void DT_Poele_init()
{
    // DT_relay(RELAY_EV1, false);
}

// decision de mise en service du poele en fonction des temperature
bool start_poele_tank_winter(bool actuel, uint32_t now)
{
    static uint32_t old_start_time = 0;
    bool old_state = actuel;

#ifdef DT_3VOIES_MCBT_RAPH
    // mise en marche du poele en fonction de la consigne MCBT
    if (DT_pt100_get(PT100_H_BALON) > 0)
    {
        if (DT_pt100_get(PT100_H_BALON) < (DT_3voies_MCBT_raph_get_setpoint() + eeprom_config.V2)) // temp haut balon  < consigne MCBT + reserve temp
        {
            actuel = true;
        }
    }
#endif // DT_3VOIES_MCBT_RAPH

#ifdef DT_3VOIES_PCBT_RAPH
    // mise en marche du poele en fonction de la consigne PCBT
    if (DT_pt100_get(PT100_M_BALON) > 0)
    {
        if (DT_pt100_get(PT100_M_BALON) < (DT_3voies_PCBT_raph_get_setpoint() + eeprom_config.V2)) // temp milieu balon  < consigne PCBT + reserve temp
        {
            actuel = true;
        }
    }
#endif // DT_3VOIES_PCBT_RAPH

#ifdef ECS_RAPH
    // mise en marche du poele en fonction de la temperature ECS
    if (DT_pt100_get(PT100_ECS) > 0)
    {
        if (DT_pt100_get(PT100_ECS) < ECS_START_POELE)
        {
            actuel = true;
        }
    }
#endif // ECS_RAPH

    if (actuel && old_state != actuel)
    {
        if ((now - old_start_time) > DT_Chauffage_get_temps_inter_demmarage())
        {
            if (((now - old_start_time) - DT_Chauffage_get_temps_inter_demmarage()) > 60 * 60 * 1000) // 60 min de moins que la consigne de temps inter demmarage
            {
                DT_Chauffage_set_temperature_arret_poele_hiver(DT_Chauffage_get_temperature_arret_poele_hiver() + 2); // on augmente la consigne d'arret poele hiver
            }
            else if (((now - old_start_time) - DT_Chauffage_get_temps_inter_demmarage()) > 30 * 60 * 1000) // 30 min de moins que la consigne de temps inter demmarage
            {
                DT_Chauffage_set_temperature_arret_poele_hiver(DT_Chauffage_get_temperature_arret_poele_hiver() + 1); // on augmente la consigne d'arret poele hiver
            }
        }
        else
        {
            if ((DT_Chauffage_get_temps_inter_demmarage() - (now - old_start_time)) > 60 * 60 * 1000) // 60 min de plus que la consigne de temps inter demmarage
            {
                DT_Chauffage_set_temperature_arret_poele_hiver(DT_Chauffage_get_temperature_arret_poele_hiver() - 2); // on diminue la consigne d'arret poele hiver
            }
            else if ((DT_Chauffage_get_temps_inter_demmarage() - (now - old_start_time)) > 30 * 60 * 1000) // 30 min de plus que la consigne de temps inter demmarage
            {
                DT_Chauffage_set_temperature_arret_poele_hiver(DT_Chauffage_get_temperature_arret_poele_hiver() - 1); // on augmente la consigne d'arret poele hiver
            }
        }
        old_start_time = millis();
    }

    // arret du poele en fonction de la temperature bas balon
    if (DT_pt100_get(PT100_B_BALON) > 0)
    {
        if (DT_pt100_get(PT100_B_BALON) > DT_Chauffage_get_temperature_arret_poele_hiver()) // temp bas balon > DT_Chauffage_get_temperature_arret_poele_hiver
        {
            actuel = false;
        }
        temp_default_pt100_B = now;
    }
    else if (now - temp_default_pt100_B > TEMPS_DEFAULT_PT100_POELE)
    {
        actuel = false;
    }

    return actuel;
}

// decision de mise en service du poele en fonction des temperature en mode intersaison
bool start_poele_tank_between(bool actuel, uint32_t now)
{
    static uint32_t old_start_time = 0;
    static bool hour_rise = false;
    bool old_state = actuel;

    // mise en marche du poele en fonction de la temperature ECS
    // a leur debut de cycle, si SDB parantal ou sdb principal ou salon < consigne alors demarage poele
    if (!hour_rise && rtcNtp.ToD() > (DT_Chauffage_get_debut_cycle()))
    {
        hour_rise = true;
        if (DT_HDC1080_get_temperature(SDB_PARENTAL) > 0)
        {
            if (DT_HDC1080_get_temperature(SDB_PARENTAL) < DT_Chauffage_get_temperature_between_start())
            {
                actuel = true;
            }
        }

        if (DT_HDC1080_get_temperature(SDB_PRINCIPALE) > 0)
        {
            if (DT_HDC1080_get_temperature(SDB_PRINCIPALE) < DT_Chauffage_get_temperature_between_start())
            {
                actuel = true;
            }
        }

        if (DT_SCD4X_get_temperature(SALON) > 0)
        {
            if (DT_SCD4X_get_temperature(SALON) < DT_Chauffage_get_temperature_between_start())
            {
                actuel = true;
            }
        }
    }
    else if (hour_rise && rtcNtp.ToD() < DT_Chauffage_get_debut_cycle())
    {
        hour_rise = false;
    }

    // arret du poele en fonction de la temperature bas balon
    if (DT_pt100_get(PT100_B_BALON) > 0)
    {
        if (DT_pt100_get(PT100_B_BALON) > DT_Chauffage_get_temperature_arret_poele_intersaison()) // temp bas balon > DT_Chauffage_get_temperature_arret_poele_hiver
        {
            actuel = false;
        }
        temp_default_pt100_B = now;
    }
    else if (now - temp_default_pt100_B > TEMPS_DEFAULT_PT100_POELE)
    {
        actuel = false;
    }

    return actuel;
}

void DT_Poele_loop()
{
    uint32_t now = millis();
    static uint32_t old = 0;
    bool poele = DT_relay_get(MARCHE_POELE);
    static uint32_t cpt_default_poele = DT_cpt_pulse_input_get(CPT_VIS_POELE);
    static bool raise_poele = false;

    if (now - old >= 1000)
    {
        // 220502  debug(F(AT));
        old = now;
        if (eeprom_config.poele_mode == DT_POELE_WINTER)
        {
            // mode ECS + Chauffage
            // marche poele
            poele = start_poele_tank_winter(poele, now);
        }
        else if (eeprom_config.poele_mode == DT_POELE_OFF)
        {
            // mode arret
            poele = false;
        }
        else if (eeprom_config.poele_mode == DT_POELE_MAINTENANCE)
        {
            // mode maintenace (disponible HA)
        }
        else if (eeprom_config.poele_mode == DTP_POELE_BETWEEN)
        {
            // mode intersaison
            
            poele = start_poele_tank_between(poele, now);
        }

        // securité
        if (DT_pt100_get(PT100_H_BALON) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }
        else if (DT_pt100_get(PT100_M_BALON) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }
        else if (DT_pt100_get(PT100_B_BALON) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }
        else if (DT_pt100_get(PT100_ECS) > POELE_MAX_TEMPERATURE)
        {
            // arret du poele
            poele = false;
        }

        if (DT_cpt_pulse_input_get(CPT_VIS_POELE) > cpt_default_poele ) 
        {
            millis_default_poele = now;
        }else if (raise_poele != poele) {
            raise_poele = poele;
            millis_default_poele = now;
            DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/default/state"), F("off"));
        }

        DT_relay(MARCHE_POELE, poele);


        // 220502  debug(F(AT));
    }

    static uint32_t old2 = millis();
    if (now - old2 > 600000)
    {
        old2 = now;
        if (poele && millis() - millis_default_poele > 600000) {
            DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/default/state"), F("on"));
        }
    }


}

bool DT_Poele_started(void)
{
    if (DT_relay_get(MARCHE_POELE) && millis() - millis_default_poele < 600000)
    {
        return true;
    }else{
        return false;
    }
}

void DT_Poele_set_mode(DT_Poele_mode mode)
{
    if (mode != eeprom_config.poele_mode)
    {
        eeprom_config.poele_mode = mode;
        sauvegardeEEPROM();
        DT_Poele_mode_callback(eeprom_config.poele_mode);
    }
}

DT_Poele_mode DT_Poele_get_mode(void)
{
    return eeprom_config.poele_mode;
}

void DT_Poele_mode_callback(const DT_Poele_mode mode)
{
    debug(F(AT));
    memory(false);
    const __FlashStringHelper *payload;
    // mode poele
    if (DT_mqtt_can_send())
    {
        // strlcpy_P(topic, F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), 56);
        switch (mode)
        {
        case DT_POELE_OFF:
            payload = F("Arret");
            break;
        case DT_POELE_WINTER:
            payload = F("Hiver");
            break;
        case DT_POELE_MAINTENANCE:
            payload = F("Maintenance");
            break;
        case DTP_POELE_BETWEEN:
            payload = F("Inter-saison");
            break;
        }
        DT_mqtt_send(F(MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/poele/mode/state"), payload);
    }
    memory(false);
}
#endif // POELE