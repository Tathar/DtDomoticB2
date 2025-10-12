#include <DT_MCBT.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <./DT_temp_ext.h>

#include <DT_eeprom.h>

#include <config.h>

// T1 = Temp Ballon		T2 = Temp ECS			    T3 = Temp ECS2
// T5 = Temp Extérieur	T6 = Temp Vanne 3V PCBT	    T7 = Temp Vanne 3V MCBT	    T8 = Temp Vanne 3V Jacuzzi

#define D2MS 86400000 // 1 jours en miliseconde

#define temp_long_1 25 // en °C
#define temp_long_2 35 // en °C

#define NUM_LISSAGE 5 // en °C

#ifdef MCBT

float Input_MCBT, Output_MCBT;

float lissage_mcbt[NUM_LISSAGE];
// Specify the links and initial tuning parameters

QuickPID pid_mcbt = QuickPID(&Input_MCBT, &Output_MCBT, &mem_config.C3);

uint32_t temp_etape_mcbt = 0;
uint32_t temp_start_mcbt = 0;
uint8_t temperature_etape_mcbt = 0;
uint32_t temp_lecture_temp_mcbt = 0;

void (*_callback_3_voies)(const float consigne);
void (*_callback_mcbt_pid)(const float P, const float I, const float D, const float Out);
bool async_call_mcbt_pid;

// mise a l'échelle
float scale(float in, float in_min, float in_max, float out_min, float out_max)
{
    return ((((in - in_min) / (in_max - in_min)) * (out_max - out_min)) + out_min);
}

// initialisation des vanne 3 voies
void DT_MCBT_init()
{

    _callback_3_voies = nullptr;
    _callback_mcbt_pid = nullptr;

    mem_config.C3 = 0;
    // for (uint8_t x = 0; x < 24; ++x)
    // {
    //      temp_buffer.push(TEMP_DEFAULT_PT100);
    // }
    // calcule des consignes de temperature

    if (eeprom_config.mode_3voies_MCBT == DT_MCBT_DEMMARAGE)
    {
        mem_config.C3 = DT_pt100_get(PT100_3_VOIES_MCBT);
        temp_start_mcbt = millis();
        temperature_etape_mcbt = mem_config.C3;
        if (mem_config.C3 == temp_long_1 || mem_config.C3 == temp_long_2)
            temp_etape_mcbt = D2MS * 3;
        else
            temp_etape_mcbt = D2MS;
    }
    else if (eeprom_config.mode_3voies_MCBT == DT_MCBT_OFF || eeprom_config.mode_3voies_MCBT == DT_MCBT_STANDBY)
    {
        mem_config.C3 = 0;
    }
    else
    {
        // mem_config.C3 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C10, eeprom_config.C11);
        mem_config.C3 = scale(get_temp_ext(), -10, 10, eeprom_config.C10, eeprom_config.C11);
    }

    // KP, KI, KD
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);

    // min, max
    pid_mcbt.SetOutputLimits((float)((float)eeprom_config.pid_mcbt.KT * -1.0), eeprom_config.pid_mcbt.KT);

    // loop time (KT)
    pid_mcbt.SetSampleTimeUs(eeprom_config.pid_mcbt.KT * 1000);

    // Direction
    pid_mcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);

    // pMode
    pid_mcbt.SetProportionalMode(eeprom_config.pid_mcbt.pmode);

    // dMode
    pid_mcbt.SetDerivativeMode(eeprom_config.pid_mcbt.dmode);

    // Anti Windup
    pid_mcbt.SetAntiWindupMode(eeprom_config.pid_mcbt.iawmode);

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
        Input_MCBT = mem_config.C3;
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_mcbt[num] = Input_MCBT;
        }
    }

    // turn the PID on
    if (eeprom_config.mode_3voies_MCBT == DT_MCBT_OFF || eeprom_config.mode_3voies_MCBT == DT_MCBT_STANDBY)
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
        pid_mcbt.Initialize();
    }
    Output_MCBT = 0;
}

// boucle principale des vanne 3 voie
void DT_MCBT_loop()
{
    uint32_t now = millis();
    // static uint8_t first_run = 0;
    static uint32_t old_now = 0;
    static float old_C3 = 0;
    // calcule des consignes de temperature

    if (eeprom_config.mode_3voies_MCBT == DT_MCBT_DEMMARAGE)
    {
        if (now - temp_start_mcbt > temp_etape_mcbt)
        {
            temperature_etape_mcbt += 1;
            temp_start_mcbt = now;
            if (temperature_etape_mcbt > temp_long_2)
            {
                DT_MCBT_set_mode(DT_MCBT_NORMAL);
            }
            else if (temperature_etape_mcbt == temp_long_1 || temperature_etape_mcbt == temp_long_2)
                temp_etape_mcbt = D2MS * 3;
            else
                temp_etape_mcbt = D2MS;
        }
        mem_config.C3 = temperature_etape_mcbt;
    }
    else if (eeprom_config.mode_3voies_MCBT == DT_MCBT_OFF || eeprom_config.mode_3voies_MCBT == DT_MCBT_STANDBY)
    {
        mem_config.C3 = 0;
    }
    else if (eeprom_config.mode_3voies_MCBT == DT_MCBT_NORMAL)
    {
        // mem_config.C3 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C10, eeprom_config.C11); // calcul de la consigne en fonction de la temperature exterieur
        mem_config.C3 = scale(get_temp_ext(), -10, 10, eeprom_config.C10, eeprom_config.C11); // calcul de la consigne en fonction de la temperature exterieur
        if (mem_config.MQTT_online)                                                           // si la carte est connecte au serveur MQTT
        {
            mem_config.C3 += eeprom_config.in_offset_MCBT; // ajout du decalage de la consigne (mode eco)
        }
    }

    // protection sur temperature

    if (mem_config.C3 > eeprom_config.C_MCBT_MAX)
        mem_config.C3 = eeprom_config.C_MCBT_MAX;

    if (mem_config.C3 > TMP_EAU_MCBT_MAX)
        mem_config.C3 = TMP_EAU_MCBT_MAX;

    // temperature de l'eau
    static uint32_t temp_lissage = 0;
    static uint8_t num_mcbt = 0;
    float calc_lissage = 0;
    if (now - temp_lissage > 1000)
    {
        temp_lissage = now;

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

    if ((eeprom_config.mode_3voies_MCBT != DT_MCBT_DEMMARAGE) && (DT_pt100_get(PT100_EXT) > (eeprom_config.C_MCBT_MIN + 0.5)))
    {
        DT_relay(CIRCULATEUR_MCBT, false);           // arret du circulateur
        pid_mcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
    else if ((eeprom_config.mode_3voies_MCBT != DT_MCBT_OFF && eeprom_config.mode_3voies_MCBT != DT_MCBT_STANDBY) && (DT_pt100_get(PT100_EXT) < (eeprom_config.C_MCBT_MIN - 0.5)))
    {
        DT_relay(CIRCULATEUR_MCBT, true); // demmarage du circulateur
        // Output_MCBT = 0;
        pid_mcbt.SetMode(QuickPID::Control::automatic); // demmarage de la vanne 3 voie
    }
    else if ((eeprom_config.mode_3voies_MCBT == DT_MCBT_OFF || eeprom_config.mode_3voies_MCBT == DT_MCBT_STANDBY))
    {
        DT_relay(CIRCULATEUR_MCBT, false);           // arret du circulateur
        pid_mcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_MCBT = 0;
        async_call_mcbt_pid = true;
    }
   
   
    if (pid_mcbt.Compute())
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

            if (Output_MCBT > (eeprom_config.out_inhib_MCBT * -1))
            {
                Output_MCBT = 0;
            }

            // Output_MCBT -= eeprom_config.out_offset_MCBT;
            DT_relay(VANNE_MCBT_COLD, (uint32_t)(Output_MCBT * -1)); // activation de la vanne
        }
        async_call_mcbt_pid = true;
    }
    else if (now - old_now > 1000)
    {
        // 220502  debug(F(AT));
        old_now = now;

        if ((_callback_3_voies != nullptr) && (old_C3 != mem_config.C3))
        {
            _callback_3_voies(mem_config.C3);
            old_C3 = mem_config.C3;
        } else {
            _callback_mcbt_pid(pid_mcbt.GetPterm(), pid_mcbt.GetIterm(), pid_mcbt.GetDterm(), Output_MCBT);
        }

    }
}

// changement du Mode de fonctionnement de la vanne 3 voie du mure chauffant
void DT_MCBT_set_mode(DT_MCBT_mode mode)
{
    eeprom_config.mode_3voies_MCBT = mode;
    // sauvegardeEEPROM();
    if (eeprom_config.mode_3voies_MCBT == DT_MCBT_OFF || eeprom_config.mode_3voies_MCBT == DT_MCBT_STANDBY)
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

    if (eeprom_config.mode_3voies_MCBT == DT_MCBT_DEMMARAGE)
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

// recuperation du Mode de fonctionnement de la vanne 3 voie du mur chaffant
DT_MCBT_mode DT_MCBT_get_mode(void)
{
    return eeprom_config.mode_3voies_MCBT;
}

// deffinition du coefician Proportionnel de la vanne 3 voie du mur chaffant
void DT_MCBT_set_KP(float kp)
{
    eeprom_config.pid_mcbt.KP = kp;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);
}

// deffinition du coefician d'Integral de la vanne 3 voie du mur chaffant
void DT_MCBT_set_KI(float ki)
{
    eeprom_config.pid_mcbt.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);
}

// deffinition du coefician de Dérivation de la vanne 3 voie du mur chaffant
void DT_MCBT_set_KD(float kd)
{
    eeprom_config.pid_mcbt.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_mcbt.SetTunings(eeprom_config.pid_mcbt.KP, eeprom_config.pid_mcbt.KI, eeprom_config.pid_mcbt.KD);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du mur chaffant
void DT_MCBT_set_KT(uint32_t kt)
{
    eeprom_config.pid_mcbt.KT = kt;
    sauvegardeEEPROM();
    pid_mcbt.SetSampleTimeUs(eeprom_config.pid_mcbt.KT * 1000);
    pid_mcbt.SetOutputLimits((float)((float)eeprom_config.pid_mcbt.KT * -1.0), (float)eeprom_config.pid_mcbt.KT);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du mure chaffant
void DT_MCBT_set_action(QuickPID::Action action)
{
    eeprom_config.pid_mcbt.action = action;
    sauvegardeEEPROM();
    pid_mcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du mur chaffant
void DT_MCBT_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_mcbt.pmode = pMode;
    sauvegardeEEPROM();
    pid_mcbt.SetProportionalMode(eeprom_config.pid_mcbt.pmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du mur chaffant
void DT_MCBT_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_mcbt.dmode = dMode;
    sauvegardeEEPROM();
    pid_mcbt.SetDerivativeMode(eeprom_config.pid_mcbt.dmode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT_MCBT_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_mcbt.iawmode = iAwMode;
    sauvegardeEEPROM();
    pid_mcbt.SetAntiWindupMode(eeprom_config.pid_mcbt.iawmode);
}

// set consigne temp MCBT
void DT_MCBT_set_C3(float c3)
{
    mem_config.C3 = c3;
}

float DT_MCBT_get_KP()
{
    return eeprom_config.pid_mcbt.KP;
}

float DT_MCBT_get_KI()
{
    return eeprom_config.pid_mcbt.KI;
}

float DT_MCBT_get_KD()
{
    return eeprom_config.pid_mcbt.KD;
}
uint32_t DT_MCBT_get_KT()
{
    return eeprom_config.pid_mcbt.KT;
}

void DT_MCBT_set_callback(void (*callback)(const float Consigne))
{
    _callback_3_voies = callback;
}

void DT_MCBT_set_callback_pid(void (*callback_mcbt_pid)(const float P, const float I, const float D, const float Out))
{
    _callback_mcbt_pid = callback_mcbt_pid;
}

// get consigne temp MCBT
float DT_MCBT_get_C3()
{
    return mem_config.C3;
}

#endif // MCBT