#include <DT_3voies.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <CircularBuffer.h>

#include <DT_eeprom.h>

#include <config.h>

// T1 = Temp Ballon		T2 = Temp ECS			    T3 = Temp ECS2
// T5 = Temp Extérieur	T6 = Temp Vanne 3V PCBT	    T7 = Temp Vanne 3V MCBT	    T8 = Temp Vanne 3V Jacuzzi

#define NB_VANNE3_VOIES 2

#define D2MS 86400000 // 1 jours en miliseconde

#define temp_long_1 25 // en °C
#define temp_long_2 35 // en °C

#define NUM_LISSAGE 5 // en °C

#ifdef VANNES

float Input_PCBT, Output_PCBT;
float Input_MCBT, Output_MCBT;

float lissage_pcbt[NUM_LISSAGE];
float lissage_mcbt[NUM_LISSAGE];
// Specify the links and initial tuning parameters

QuickPID pid_pcbt = QuickPID(&Input_PCBT, &Output_PCBT, &mem_config.C2);
QuickPID pid_mcbt = QuickPID(&Input_MCBT, &Output_MCBT, &mem_config.C3);

uint32_t temp_etape_pcbt = 0;
uint32_t temp_etape_mcbt = 0;
uint32_t temp_start_pcbt = 0;
uint32_t temp_start_mcbt = 0;
uint8_t temperature_etape_pcbt = 0;
uint8_t temperature_etape_mcbt = 0;
uint32_t temp_lecture_temp_pcbt = 0;
uint32_t temp_lecture_temp_mcbt = 0;

void (*_callback_3_voies)(const float C2, const float C3);

void (*_callback_pcbt_pid)(const float P, const float I, const float D, const float Out);
bool async_call_pcbt_pid;
void (*_callback_mcbt_pid)(const float P, const float I, const float D, const float Out);
bool async_call_mcbt_pid;

void (*_callback_avg_temp)(const float temp);

CircularBuffer<float, 24> temp_buffer;

float DT_3voies_get_temp_moyen()
{
    float ret = 0;
    // for (uint8_t x = 0; x < 24; ++x)
    // {
    //     if (temp_buffer[x] != TEMP_DEFAULT_PT100)
    //     {
    //         ret += temp_buffer[x];
    //         diviseur++;
    //     }
    // }
    
    for (uint8_t x = 0; x < temp_buffer.size(); ++x)
    {
            ret += temp_buffer[x];
    }

    if (temp_buffer.size() != 0 )
        return ret / (float) temp_buffer.size();
    else 
        return TEMP_DEFAULT_PT100;

};

// fournie la temperature exterieur moyenné en fonction du decalage choisie
float get_temp_ext()
{
    if (DT_pt100_get(PT100_EXT) > DT_3voies_get_temp_moyen() + eeprom_config.in_offset_avg_temp_sup)
    {
        return DT_3voies_get_temp_moyen() + eeprom_config.in_offset_avg_temp_sup;
    }
    else if (DT_pt100_get(PT100_EXT) < DT_3voies_get_temp_moyen() - eeprom_config.in_offset_avg_temp_inf)
    {
        return DT_3voies_get_temp_moyen() - eeprom_config.in_offset_avg_temp_inf;
    }
    else
    {
        return DT_pt100_get(PT100_EXT);
    }
};

void calc_temp_moyen()
{
    uint32_t now = millis();
    static uint32_t time = 0;
    static bool init = false;
    if (DT_pt100_get(PT100_EXT) != TEMP_DEFAULT_PT100 && (init == false || now - time > 3600000))
    {
        init = true;
        time = now;
        temp_buffer.push(DT_pt100_get(PT100_EXT));
        if (_callback_avg_temp != nullptr)
        {
            _callback_avg_temp(DT_3voies_get_temp_moyen());
        }
    }
};

// mise a l'échelle
float scale(float in, float in_min, float in_max, float out_min, float out_max)
{
    return ((((in - in_min) / (in_max - in_min)) * (out_max - out_min)) + out_min);
}

// initialisation des vanne 3 voies
void DT_3voies_init()
{

    _callback_3_voies = nullptr;
    _callback_pcbt_pid = nullptr;
    _callback_mcbt_pid = nullptr;
    _callback_avg_temp = nullptr;

    mem_config.C2 = 0;
    mem_config.C3 = 0;
    // for (uint8_t x = 0; x < 24; ++x)
    // {
    //      temp_buffer.push(TEMP_DEFAULT_PT100);
    // }

    calc_temp_moyen();

    // calcule des consignes de temperature
    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_DEMMARAGE)
    {
        mem_config.C2 = DT_pt100_get(PT100_3_VOIES_PCBT);
        temp_start_pcbt = millis();
        temperature_etape_pcbt = mem_config.C2;
        if (mem_config.C2 == temp_long_1 || mem_config.C2 == temp_long_2)
            temp_etape_pcbt = D2MS * 3;
        else
            temp_etape_pcbt = D2MS;
    }
    else if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF)
    {
        mem_config.C2 = 0;
    }
    else
    {
        //mem_config.C2 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C8, eeprom_config.C9);
        mem_config.C2 = scale(get_temp_ext(), -10, 10, eeprom_config.C8, eeprom_config.C9);
    }

    if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_DEMMARAGE)
    {
        mem_config.C3 = DT_pt100_get(PT100_3_VOIES_MCBT);
        temp_start_mcbt = millis();
        temperature_etape_mcbt = mem_config.C3;
        if (mem_config.C3 == temp_long_1 || mem_config.C3 == temp_long_2)
            temp_etape_mcbt = D2MS * 3;
        else
            temp_etape_mcbt = D2MS;
    }
    else if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_OFF)
    {
        mem_config.C3 = 0;
    }
    else
    {
        // mem_config.C3 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C10, eeprom_config.C11);
        mem_config.C3 = scale(get_temp_ext(), -10, 10, eeprom_config.C10, eeprom_config.C11);
    }

    // KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);

    // min, max
    pid_pcbt.SetOutputLimits((float)((float)eeprom_config.pid_pcbt.KT * -1.0), eeprom_config.pid_pcbt.KT);
    pid_mcbt.SetOutputLimits((float)((float)eeprom_config.pid_mcbt.KT * -1.0), eeprom_config.pid_mcbt.KT);

    // loop time (KT)
    pid_pcbt.SetSampleTimeUs(eeprom_config.pid_pcbt.KT * 1000);
    pid_mcbt.SetSampleTimeUs(eeprom_config.pid_mcbt.KT * 1000);

    // Direction
    pid_pcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);
    pid_mcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);

    // pMode
    pid_pcbt.SetProportionalMode(eeprom_config.pid_pcbt.pmode);
    pid_mcbt.SetProportionalMode(eeprom_config.pid_mcbt.pmode);

    // dMode
    pid_pcbt.SetDerivativeMode(eeprom_config.pid_pcbt.dmode);
    pid_mcbt.SetDerivativeMode(eeprom_config.pid_mcbt.dmode);

    // Anti Windup
    pid_pcbt.SetAntiWindupMode(eeprom_config.pid_pcbt.iawmode);
    pid_mcbt.SetAntiWindupMode(eeprom_config.pid_mcbt.iawmode);

    // temperature de la vanne PCBT
    if (DT_pt100_get(PT100_3_VOIES_PCBT) > 0)
    {
        Input_PCBT = DT_pt100_get(PT100_3_VOIES_PCBT);
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_pcbt[num] = Input_PCBT;
        }
    }
    else
    {
        Input_PCBT = 85;
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_pcbt[num] = Input_PCBT;
        }
    }

    // temperature de la vanne MCBT
    if (DT_pt100_get(PT100_3_VOIES_MCBT) > 0)
    {
        Input_MCBT = DT_pt100_get(PT100_3_VOIES_MCBT);
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_mcbt[num] = Input_MCBT;
        }
    }
    else
    {
        Input_MCBT = 85;
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_mcbt[num] = Input_MCBT;
        }
    }

    // turn the PID on
    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF)
    {
        DT_relay(CIRCULATEUR_PCBT, false);
        pid_pcbt.SetMode(QuickPID::Control::manual);
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }
    else
    {
        DT_relay(CIRCULATEUR_PCBT, true);
        Output_PCBT = 0;
        pid_pcbt.SetMode(QuickPID::Control::automatic);
    }

    // turn the PID on
    if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_OFF)
    {
        DT_relay(CIRCULATEUR_MCBT, false);
        pid_mcbt.SetMode(QuickPID::Control::manual);
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
    else
    {
        DT_relay(CIRCULATEUR_MCBT, true);
        Output_MCBT = 0;
        pid_mcbt.SetMode(QuickPID::Control::automatic);
    }

    Output_PCBT = 0;
    Output_MCBT = 0;
}

// boucle principale des vanne 3 voie
void DT_3voies_loop()
{
    uint32_t now = millis();
    static uint32_t old_now = 0;
    static float old_C2 = 0;
    static float old_C3 = 0;

    calc_temp_moyen();
    // calcule des consignes de temperature
    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_DEMMARAGE)
    {
        if (now - temp_start_pcbt > temp_etape_pcbt)
        {
            temperature_etape_pcbt += 1;
            temp_start_pcbt = now;
            if (temperature_etape_pcbt > temp_long_2)
            {
                DT_3voies_PCBT_set_mode(DT_3VOIES_NORMAL);
            }
            else if (temperature_etape_pcbt == temp_long_1 || temperature_etape_pcbt == temp_long_2)
                temp_etape_pcbt = D2MS * 3;
            else
                temp_etape_pcbt = D2MS;
        }

        mem_config.C2 = temperature_etape_pcbt;
    }
    else if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF)
    {
        mem_config.C2 = 0;
    }
    else if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_NORMAL)
    {
        // mem_config.C2 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C8, eeprom_config.C9); // calcul de la consigne en fonction de la temperature exterieur
        mem_config.C2 = scale(get_temp_ext(), -10, 10, eeprom_config.C8, eeprom_config.C9); // calcul de la consigne en fonction de la temperature exterieur
        if (mem_config.MQTT_online)                                                                  // si la carte est connecte au serveur MQTT
        {
            mem_config.C2 += eeprom_config.in_offset_PCBT; // ajout du decalage de la consigne (mode eco)
        }
    }

    if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_DEMMARAGE)
    {
        if (now - temp_start_mcbt > temp_etape_mcbt)
        {
            temperature_etape_mcbt += 1;
            temp_start_mcbt = now;
            if (temperature_etape_mcbt > temp_long_2)
            {
                DT_3voies_MCBT_set_mode(DT_3VOIES_NORMAL);
            }
            else if (temperature_etape_mcbt == temp_long_1 || temperature_etape_mcbt == temp_long_2)
                temp_etape_mcbt = D2MS * 3;
            else
                temp_etape_mcbt = D2MS;
        }
        mem_config.C3 = temperature_etape_mcbt;
    }
    else if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_OFF)
    {
        mem_config.C3 = 0;
    }
    else if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_NORMAL)
    {
        // mem_config.C3 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C10, eeprom_config.C11); // calcul de la consigne en fonction de la temperature exterieur
        mem_config.C3 = scale(get_temp_ext(), -10, 10, eeprom_config.C10, eeprom_config.C11); // calcul de la consigne en fonction de la temperature exterieur
        if (mem_config.MQTT_online)                                                                    // si la carte est connecte au serveur MQTT
        {
            mem_config.C3 += eeprom_config.in_offset_MCBT; // ajout du decalage de la consigne (mode eco)
        }
    }

    // protection sur temperature
    if (mem_config.C2 > eeprom_config.C_PCBT_MAX)
        mem_config.C2 = eeprom_config.C_PCBT_MAX;

    if (mem_config.C3 > eeprom_config.C_MCBT_MAX)
        mem_config.C3 = eeprom_config.C_MCBT_MAX;

    if (mem_config.C2 > TMP_EAU_PCBT_MAX)
        mem_config.C2 = TMP_EAU_PCBT_MAX;

    if (mem_config.C3 > TMP_EAU_MCBT_MAX)
        mem_config.C3 = TMP_EAU_MCBT_MAX;

    // temperature de l'eau
    static uint32_t temp_lissage = 0;
    static uint8_t num_pcbt = 0;
    static uint8_t num_mcbt = 0;
    float calc_lissage = 0;
    if (now - temp_lissage > 1000)
    {
        temp_lissage = now;

        if (DT_pt100_get(PT100_3_VOIES_PCBT) > 0)
        {
            lissage_pcbt[num_pcbt] = DT_pt100_get(PT100_3_VOIES_PCBT);
            num_pcbt++;
            if (num_pcbt == NUM_LISSAGE)
            {
                num_pcbt = 0;
            }
        }

        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            calc_lissage += lissage_pcbt[num];
        }
        Input_PCBT = calc_lissage / NUM_LISSAGE;

        if (DT_pt100_get(PT100_3_VOIES_MCBT) > 0)
        {
            lissage_mcbt[num_mcbt] = DT_pt100_get(PT100_3_VOIES_MCBT);
            num_mcbt++;
            if (num_mcbt == NUM_LISSAGE)
            {
                num_mcbt = 0;
            }
        }
        calc_lissage = 0;
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            calc_lissage += lissage_mcbt[num];
        }
        Input_MCBT = calc_lissage / NUM_LISSAGE;
    }

    // consigne minimum pour fonctionnement des circulateur
    if ((eeprom_config.mode_3voies_PCBT != DT_3VOIES_DEMMARAGE) && (DT_pt100_get(PT100_EXT) > (eeprom_config.C_PCBT_MIN + 0.5)))
    {
        DT_relay(CIRCULATEUR_PCBT, false);           // arret du circulateur
        pid_pcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }
    else if ((eeprom_config.mode_3voies_PCBT != DT_3VOIES_OFF) && (DT_pt100_get(PT100_EXT) < (eeprom_config.C_PCBT_MIN - 0.5)))
    {
        DT_relay(CIRCULATEUR_PCBT, true); // demmarage du circulateur
        //Output_PCBT = 0;
        pid_pcbt.SetMode(QuickPID::Control::automatic); // demmarage de la vanne 3
    }
    else if ((eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF))
    {
        DT_relay(CIRCULATEUR_PCBT, false);           // arret du circulateur
        pid_pcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }

    if ((eeprom_config.mode_3voies_MCBT != DT_3VOIES_DEMMARAGE) && (DT_pt100_get(PT100_EXT) > (eeprom_config.C_MCBT_MIN + 0.5)))
    {
        DT_relay(CIRCULATEUR_MCBT, false);           // arret du circulateur
        pid_mcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
    else if ((eeprom_config.mode_3voies_MCBT != DT_3VOIES_OFF) && (DT_pt100_get(PT100_EXT) < (eeprom_config.C_MCBT_MIN - 0.5)))
    {
        DT_relay(CIRCULATEUR_MCBT, true); // demmarage du circulateur
        //Output_MCBT = 0;
        pid_mcbt.SetMode(QuickPID::Control::automatic); // demmarage de la vanne 3 voie
    }
    else if ((eeprom_config.mode_3voies_MCBT == DT_3VOIES_OFF))
    {
        DT_relay(CIRCULATEUR_MCBT, false);           // arret du circulateur
        pid_mcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
/*
    // Plage Morte PCBT
    if (Input_PCBT >= mem_config.C2 && (Input_PCBT - mem_config.C2) < eeprom_config.V3)
    {
        pid_pcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }
    else if (mem_config.C2 > Input_PCBT && (mem_config.C2 - Input_PCBT) < eeprom_config.V3)
    {
        pid_pcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }
*/
/*
    // Plage Morte MCBT
    if (Input_MCBT >= mem_config.C3 && (Input_MCBT - mem_config.C3) < eeprom_config.V3)
    {
        pid_mcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
    else if (mem_config.C3 > Input_MCBT && (mem_config.C3 - Input_MCBT) < eeprom_config.V3)
    {
        pid_mcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
*/
    /*
        // test de la temperature du planche
        for (uint8_t num = 0; num < NUM_PLANCHE; ++num)
        {
            uint8_t num_pt100 = pgm_read_byte(PT100_PLANCHEE + num);
            if ((DT_pt100_get(num_pt100) != TEMP_DEFAULT_PT100) && (DT_pt100_get(num_pt100) > MAX_TMP_PLANCHE + 1))
            {
               //auto Serial.println("temp planché max");
                DT_relay(CIRCULATEUR_PCBT, false); // arret du circulateur
                return;                            // arret de la fonction
            }
        }
    */

    // calcule du PID
    if (pid_pcbt.Compute())
    {
        if (Output_PCBT > 0)
        {
            if (eeprom_config.ratio_PCBT > 1)
            {
                Output_PCBT /= eeprom_config.ratio_PCBT;
            }
            
            if (Output_PCBT < eeprom_config.out_inhib_PCBT)
            {
                Output_PCBT = 0;
            }
            // Output_PCBT += eeprom_config.out_offset_PCBT;
            DT_relay(VANNE_PCBT_HOT, (uint32_t)(Output_PCBT)); // activation de la vanne
        }
        else
        {
            if (eeprom_config.ratio_PCBT < 1)
            {
                Output_PCBT /= (eeprom_config.ratio_PCBT * -1);
            }
            
            if (Output_PCBT > (eeprom_config.out_inhib_PCBT * -1) )
            {
                Output_PCBT = 0;
            }

            // Output_PCBT -= eeprom_config.out_offset_PCBT;
            DT_relay(VANNE_PCBT_COLD, (uint32_t)(Output_PCBT * -1)); // activation de la vanne
        }
        async_call_pcbt_pid = true;
    }
    else if (pid_mcbt.Compute())
    {
        if (Output_MCBT > 0)
        {
            if (eeprom_config.ratio_MCBT > 0)
            {
                Output_MCBT /= eeprom_config.ratio_MCBT;
            }
            
            if (Output_MCBT < eeprom_config.out_inhib_MCBT)
            {
                Output_MCBT = 0;
            }

            // Output_MCBT += eeprom_config.out_offset_MCBT;
            DT_relay(VANNE_MCBT_HOT, (uint32_t)(Output_MCBT)); // activation de la vanne
        }
        else
        {
            if (eeprom_config.ratio_MCBT < 0)
            {
                Output_MCBT /= eeprom_config.ratio_MCBT * -1;
            }
            
            if (Output_MCBT > (eeprom_config.out_inhib_MCBT * -1 ))
            {
                Output_MCBT = 0;
            }

            // Output_MCBT -= eeprom_config.out_offset_MCBT;
            DT_relay(VANNE_MCBT_COLD, (uint32_t)(Output_MCBT * -1)); // activation de la vanne
        }
        async_call_mcbt_pid = true;
    }
    else if (now - old_now > 1000 / 3)
    {
        // 220502  debug(F(AT));
        old_now = now;

        enum DT_3voie
        {
            DT_3voie_first,
            callback_pcbt_pid,
            callback_mcbt_pid,
            callback_3_voie,
            DT_3voie_last

        };

        static DT_3voie sequance = callback_pcbt_pid;

        switch (sequance)
        {
        case callback_pcbt_pid:
            if (_callback_pcbt_pid != nullptr && async_call_pcbt_pid == true)
            {
                async_call_pcbt_pid = false;
                _callback_pcbt_pid(pid_pcbt.GetPterm(), pid_pcbt.GetIterm(), pid_pcbt.GetDterm(), Output_PCBT);
            }
            break;
        case callback_mcbt_pid:
            if (_callback_mcbt_pid != nullptr && async_call_mcbt_pid == true)
            {
                async_call_mcbt_pid = false;
                _callback_mcbt_pid(pid_mcbt.GetPterm(), pid_mcbt.GetIterm(), pid_mcbt.GetDterm(), Output_MCBT);
            }
            break;
        case callback_3_voie:
            if ((_callback_3_voies != nullptr) && ((old_C2 != mem_config.C2) || (old_C3 != mem_config.C3)))
            {
                _callback_3_voies(mem_config.C2, mem_config.C3);
                old_C2 = mem_config.C2;
                old_C3 = mem_config.C3;
            }
            break;
        case DT_3voie_last:
            sequance = DT_3voie_first;
            break;

        default:
            break;
        }

        sequance = DT_3voie(sequance + 1);
    }
}

// changement du Mode de fonctionnement de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_mode(DT_3voies_mode mode)
{
    eeprom_config.mode_3voies_PCBT = mode;
    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF)
    {
        DT_relay(CIRCULATEUR_PCBT, false);
        pid_pcbt.SetMode(QuickPID::Control::manual);
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }
    else
    {
        DT_relay(CIRCULATEUR_PCBT, true);
        Output_PCBT = 0;
        pid_pcbt.SetMode(QuickPID::Control::automatic);
    }

    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_DEMMARAGE)
    {
        mem_config.C2 = DT_pt100_get(PT100_3_VOIES_PCBT);
        temp_start_pcbt = millis();
        temperature_etape_pcbt = mem_config.C2;
        if (mem_config.C2 == temp_long_1 || mem_config.C2 == temp_long_2)
            temp_etape_pcbt = D2MS * 3;
        else
            temp_etape_pcbt = D2MS;
    }
    else
    {
        // mem_config.C2 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C8, eeprom_config.C9);
        mem_config.C2 = scale(get_temp_ext(), -10, 10, eeprom_config.C8, eeprom_config.C9);
    }
    sauvegardeEEPROM();
}

// changement du Mode de fonctionnement de la vanne 3 voie du mure chauffant
void DT_3voies_MCBT_set_mode(DT_3voies_mode mode)
{
    eeprom_config.mode_3voies_MCBT = mode;
    // sauvegardeEEPROM();
    if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_OFF)
    {
        DT_relay(CIRCULATEUR_MCBT, false);
        pid_mcbt.SetMode(QuickPID::Control::manual);
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
    else
    {
        DT_relay(CIRCULATEUR_MCBT, true);
        Output_MCBT = 0;
        pid_mcbt.SetMode(QuickPID::Control::automatic);
    }

    if (eeprom_config.mode_3voies_MCBT == DT_3VOIES_DEMMARAGE)
    {
        mem_config.C3 = DT_pt100_get(PT100_3_VOIES_MCBT);
        temp_start_mcbt = millis();
        temperature_etape_mcbt = mem_config.C3;
        if (mem_config.C3 == temp_long_1 || mem_config.C3 == temp_long_2)
            temp_etape_mcbt = D2MS * 3;
        else
            temp_etape_mcbt = D2MS;
    }
    else
    {
        // mem_config.C3 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C10, eeprom_config.C11);
        mem_config.C3 = scale(get_temp_ext(), -10, 10, eeprom_config.C10, eeprom_config.C11);
    }
    sauvegardeEEPROM();
}

// recuperation du Mode de fonctionnement de la vanne 3 voie du planché chaffant
DT_3voies_mode DT_3voies_PCBT_get_mode(void)
{
    return eeprom_config.mode_3voies_PCBT;
}

// recuperation du Mode de fonctionnement de la vanne 3 voie du mur chaffant
DT_3voies_mode DT_3voies_MCBT_get_mode(void)
{
    return eeprom_config.mode_3voies_MCBT;
}

// deffinition du coefician Proportionnel de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_KP(float kp)
{
    eeprom_config.pid_pcbt.KP = kp;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);
}

// deffinition du coefician Proportionnel de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_set_KP(float kp)
{
    eeprom_config.pid_mcbt.KP = kp;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);
}

// deffinition du coefician d'Integral de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_KI(float ki)
{
    eeprom_config.pid_pcbt.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);
}

// deffinition du coefician d'Integral de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_set_KI(float ki)
{
    eeprom_config.pid_mcbt.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);
}

// deffinition du coefician de Dérivation de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_KD(float kd)
{
    eeprom_config.pid_pcbt.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);
}

// deffinition du coefician de Dérivation de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_set_KD(float kd)
{
    eeprom_config.pid_mcbt.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_KT(uint32_t kt)
{
    eeprom_config.pid_pcbt.KT = kt;
    sauvegardeEEPROM();
    // set loop time (KT)
    pid_pcbt.SetSampleTimeUs(eeprom_config.pid_pcbt.KT * 1000);
    // min, max
    pid_pcbt.SetOutputLimits((float)((float)eeprom_config.pid_pcbt.KT * -1.0), (float)eeprom_config.pid_pcbt.KT);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_set_KT(uint32_t kt)
{
    eeprom_config.pid_mcbt.KT = kt;
    sauvegardeEEPROM();
    pid_mcbt.SetSampleTimeUs(eeprom_config.pid_mcbt.KT * 1000);
    pid_mcbt.SetOutputLimits((float)((float)eeprom_config.pid_mcbt.KT * -1.0), (float)eeprom_config.pid_mcbt.KT);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_action(QuickPID::Action action)
{
    eeprom_config.pid_pcbt.action = action;
    pid_pcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du mure chaffant
void DT_3voies_MCBT_set_action(QuickPID::Action action)
{
    eeprom_config.pid_mcbt.action = action;
    sauvegardeEEPROM();
    pid_mcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_pcbt.pmode = pMode;
    sauvegardeEEPROM();
    pid_pcbt.SetProportionalMode(eeprom_config.pid_pcbt.pmode);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_mcbt.pmode = pMode;
    sauvegardeEEPROM();
    pid_mcbt.SetProportionalMode(eeprom_config.pid_mcbt.pmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_pcbt.dmode = dMode;
    sauvegardeEEPROM();
    pid_pcbt.SetDerivativeMode(eeprom_config.pid_pcbt.dmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_mcbt.dmode = dMode;
    sauvegardeEEPROM();
    pid_mcbt.SetDerivativeMode(eeprom_config.pid_mcbt.dmode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du planché chaffant
void DT_3voies_PCBT_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_pcbt.iawmode = iAwMode;
    sauvegardeEEPROM();
    pid_pcbt.SetAntiWindupMode(eeprom_config.pid_pcbt.iawmode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_mcbt.iawmode = iAwMode;
    sauvegardeEEPROM();
    pid_mcbt.SetAntiWindupMode(eeprom_config.pid_mcbt.iawmode);
}

// void DT_3voies_PCBT_set_action(Action action)
// {
//     eeprom_config.pid_pcbt.action = action;
//     // sauvegardeEEPROM();
//     pid_pcbt.SetControllerDirection((QuickPID::Action)action);
// }

// void DT_3voies_MCBT_set_action(Action action)
// {
//     eeprom_config.pid_mcbt.action = action;
//     // sauvegardeEEPROM();
//     pid_mcbt.SetControllerDirection((QuickPID::Action)action);
// }

// set consigne temp PCBT
void DT_3voies_set_C2(float c2)
{
    mem_config.C2 = c2;
}

// set consigne temp MCBT
void DT_3voies_set_C3(float c3)
{
    mem_config.C3 = c3;
}

float DT_3voies_PCBT_get_KP()
{
    return eeprom_config.pid_pcbt.KP;
}

float DT_3voies_MCBT_get_KP()
{
    return eeprom_config.pid_mcbt.KP;
}

float DT_3voies_PCBT_get_KI()
{
    return eeprom_config.pid_pcbt.KI;
}

float DT_3voies_MCBT_get_KI()
{
    return eeprom_config.pid_mcbt.KI;
}

float DT_3voies_PCBT_get_KD()
{
    return eeprom_config.pid_pcbt.KD;
}

float DT_3voies_MCBT_get_KD()
{
    return eeprom_config.pid_mcbt.KD;
}

uint32_t DT_3voies_PCBT_get_KT()
{
    return eeprom_config.pid_pcbt.KT;
}

uint32_t DT_3voies_MCBT_get_KT()
{
    return eeprom_config.pid_mcbt.KT;
}

void DT_3voies_set_callback(void (*callback)(const float C2, const float C3))
{
    _callback_3_voies = callback;
}

void DT_3voies_mcbt_set_callback_pid(void (*callback_mcbt_pid)(const float P, const float I, const float D, const float Out))
{
    _callback_mcbt_pid = callback_mcbt_pid;
}
void DT_3voies_pcbt_set_callback_pid(void (*callback_pcbt_pid)(const float P, const float I, const float D, const float Out))
{
    _callback_pcbt_pid = callback_pcbt_pid;
}
void DT_3voies_set_callback_avg_temp(void (*callback_avg_temp)(const float temp))
{
    _callback_avg_temp = callback_avg_temp;
}

// get consigne temp PCBT
float DT_3voies_get_C2()
{
    return mem_config.C2;
}

// get consigne temp MCBT
float DT_3voies_get_C3()
{
    return mem_config.C3;
}

#endif // VANNES