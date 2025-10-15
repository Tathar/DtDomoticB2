
#include "../lib/DT_chauffage/DT_3voies_nath.h"
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

#ifdef DT_3VOIES_1_NATH

float Input_3voies_1_nath, Output_3voies_1_nath, SetPoint_3voies_1;

float lissage_mcbt[NUM_LISSAGE];
// Specify the links and initial tuning parameters

QuickPID pid_3voies_1_nath = QuickPID(&Input_3voies_1_nath, &Output_3voies_1_nath, &SetPoint_3voies_1);

uint32_t temp_etape_mcbt = 0;
uint32_t temp_start_mcbt = 0;
uint8_t temperature_etape_mcbt = 0;
uint32_t temp_lecture_temp_mcbt = 0;

void (*_callback_3_voies)(const float consigne);
void (*_callback_mcbt_pid)(const float setpoint, const float P, const float I, const float D, const float Out);
bool async_call_mcbt_pid;

// mise a l'échelle
float scale(float in, float in_min, float in_max, float out_min, float out_max)
{
    return ((((in - in_min) / (in_max - in_min)) * (out_max - out_min)) + out_min);
}

// initialisation des vanne 3 voies
void DT_3voies_1_nath_init()
{

    _callback_3_voies = nullptr;
    _callback_mcbt_pid = nullptr;

    SetPoint_3voies_1 = 0;
    // for (uint8_t x = 0; x < 24; ++x)
    // {
    //      temp_buffer.push(TEMP_DEFAULT_PT100);
    // }
    // calcule des consignes de temperature

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF || eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_STANDBY)
    {
        SetPoint_3voies_1 = 0;
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        SetPoint_3voies_1 = scale(get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        SetPoint_3voies_1 = eeprom_config.SetPoint_manual_3voies_1_nath;
    }

    // KP, KI, KD
    pid_3voies_1_nath.SetTunings(eeprom_config.pid_3voies_1_nath.KP, eeprom_config.pid_3voies_1_nath.KI, eeprom_config.pid_3voies_1_nath.KD);

    // min, max
    pid_3voies_1_nath.SetOutputLimits((float)((float)eeprom_config.pid_3voies_1_nath.KT * -1.0), eeprom_config.pid_3voies_1_nath.KT);

    // loop time (KT)
    pid_3voies_1_nath.SetSampleTimeUs(eeprom_config.pid_3voies_1_nath.KT * 1000);

    // Direction
    pid_3voies_1_nath.SetControllerDirection(eeprom_config.pid_3voies_1_nath.action);

    // pMode
    pid_3voies_1_nath.SetProportionalMode(eeprom_config.pid_3voies_1_nath.pmode);

    // dMode
    pid_3voies_1_nath.SetDerivativeMode(eeprom_config.pid_3voies_1_nath.dmode);

    // Anti Windup
    pid_3voies_1_nath.SetAntiWindupMode(eeprom_config.pid_3voies_1_nath.iawmode);

    // temperature de la vanne MCBT
    if (DT_pt100_get(DT_3VOIES_1_NATH_CHAUFFAGE_PT100) > 0)
    {
        Input_3voies_1_nath = DT_pt100_get(DT_3VOIES_1_NATH_CHAUFFAGE_PT100);
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_mcbt[num] = Input_3voies_1_nath;
        }
    }
    else
    {
        Input_3voies_1_nath = SetPoint_3voies_1;
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_mcbt[num] = Input_3voies_1_nath;
        }
    }

    // turn the PID on
    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF || eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_STANDBY)
    {
        DT_relay(DT_3VOIES_1_NATH_CIRCULATEUR, false);
        pid_3voies_1_nath.SetMode(QuickPID::Control::manual);
        Output_3voies_1_nath = 0;
        async_call_mcbt_pid = true;
    }
    else
    {
        DT_relay(DT_3VOIES_1_NATH_CIRCULATEUR, true);
        Output_3voies_1_nath = 0;
        pid_3voies_1_nath.SetMode(QuickPID::Control::automatic);
        pid_3voies_1_nath.Initialize();
    }
    Output_3voies_1_nath = 0;
}

// boucle principale des vanne 3 voie
void DT_3voies_1_nath_loop()
{
    uint32_t now = millis();
    // static uint8_t first_run = 0;
    static uint32_t old_now = 0;
    static float old_SetPoint_3voies_1 = 0;
    // calcule des consignes de temperature

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF || eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_STANDBY)
    {
        SetPoint_3voies_1 = 0;
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath); // calcul de la consigne en fonction de la temperature exterieur
        SetPoint_3voies_1 = scale(get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath); // calcul de la consigne en fonction de la temperature exterieur
        if (mem_config.MQTT_online)                                                               // si la carte est connecte au serveur MQTT
        {
            SetPoint_3voies_1 += eeprom_config.in_offset_3voies_1_nath; // ajout du decalage de la consigne (mode eco)
        }
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        SetPoint_3voies_1 = eeprom_config.SetPoint_manual_3voies_1_nath; // calcul de la consigne en fonction de la temperature exterieur
    }

    // protection sur temperature

    if (SetPoint_3voies_1 > eeprom_config.SetPoint_3voies_1_nath_max)
        SetPoint_3voies_1 = eeprom_config.SetPoint_3voies_1_nath_max;

    if (SetPoint_3voies_1 > DT_3VOIES_1_NATH_MAX)
        SetPoint_3voies_1 = DT_3VOIES_1_NATH_MAX;

    // temperature de l'eau
    static uint32_t temp_lissage = 0;
    static uint8_t num_mcbt = 0;
    float calc_lissage = 0;
    if (now - temp_lissage > 1000)
    {
        temp_lissage = now;

        if (DT_pt100_get(DT_3VOIES_1_NATH_CHAUFFAGE_PT100) > 0)
        {
            lissage_mcbt[num_mcbt] = DT_pt100_get(DT_3VOIES_1_NATH_CHAUFFAGE_PT100);
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
        Input_3voies_1_nath = calc_lissage / NUM_LISSAGE;
    }

    if ((eeprom_config.mode_3voies_1_nath != DT_3voies_1_nath_OFF && eeprom_config.mode_3voies_1_nath != DT_3voies_1_nath_STANDBY) && (DT_pt100_get(DT_PT100_EXT) < (eeprom_config.SetPoint_3voies_1_nath_min - 0.5)))
    {
        DT_relay(DT_3VOIES_1_NATH_CIRCULATEUR, true); // demmarage du circulateur
        // Output_3voies_1_nath = 0;
        pid_3voies_1_nath.SetMode(QuickPID::Control::automatic); // demmarage de la vanne 3 voie
    }
    else if ((eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF || eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_STANDBY))
    {
        DT_relay(DT_3VOIES_1_NATH_CIRCULATEUR, false);        // arret du circulateur
        pid_3voies_1_nath.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_3voies_1_nath = 0;
        async_call_mcbt_pid = true;
    }

    if (pid_3voies_1_nath.Compute())
    {
        if (Output_3voies_1_nath > 0)
        {
            if (eeprom_config.ratio_3voies_1_nath > 0)
            {
                Output_3voies_1_nath /= eeprom_config.ratio_3voies_1_nath;
            }

            if (Output_3voies_1_nath < eeprom_config.out_inhib_3voies_1_nath)
            {
                Output_3voies_1_nath = 0;
            }

            // Output_3voies_1_nath += eeprom_config.out_offset_MCBT;
            DT_relay(DT_3VOIES_1_NATH_HOT, (uint32_t)(Output_3voies_1_nath)); // activation de la vanne
        }
        else
        {
            if (eeprom_config.ratio_3voies_1_nath < 0)
            {
                Output_3voies_1_nath /= eeprom_config.ratio_3voies_1_nath * -1;
            }

            if (Output_3voies_1_nath > (eeprom_config.out_inhib_3voies_1_nath * -1))
            {
                Output_3voies_1_nath = 0;
            }

            // Output_3voies_1_nath -= eeprom_config.out_offset_MCBT;
            DT_relay(DT_3VOIES_1_NATH_COLD, (uint32_t)(Output_3voies_1_nath * -1)); // activation de la vanne
        }
        async_call_mcbt_pid = true;
    }
    else if (now - old_now > 1000)
    {
        // 220502  debug(F(AT));
        old_now = now;

        if ((_callback_3_voies != nullptr) && (old_SetPoint_3voies_1 != SetPoint_3voies_1))
        {
            _callback_3_voies(SetPoint_3voies_1);
            old_SetPoint_3voies_1 = SetPoint_3voies_1;
        }
        else
        {
            _callback_mcbt_pid(SetPoint_3voies_1, pid_3voies_1_nath.GetPterm(), pid_3voies_1_nath.GetIterm(), pid_3voies_1_nath.GetDterm(), Output_3voies_1_nath);
        }
    }
}

// changement du Mode de fonctionnement de la vanne 3 voie du mure chauffant
void DT_3voies_1_nath_set_mode(DT_3voies_1_nath_mode mode)
{
    eeprom_config.mode_3voies_1_nath = mode;
    // sauvegardeEEPROM();
    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF || eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_STANDBY)
    {
        DT_relay(DT_3VOIES_1_NATH_CIRCULATEUR, false);
        pid_3voies_1_nath.SetMode(QuickPID::Control::manual);
        Output_3voies_1_nath = 0;
        async_call_mcbt_pid = true;
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        DT_relay(DT_3VOIES_1_NATH_CIRCULATEUR, true);
        Output_3voies_1_nath = 0;
        pid_3voies_1_nath.SetMode(QuickPID::Control::automatic);
        SetPoint_3voies_1 = scale(get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        Output_3voies_1_nath = 0;
        pid_3voies_1_nath.SetMode(QuickPID::Control::automatic);
        SetPoint_3voies_1 = eeprom_config.SetPoint_manual_3voies_1_nath;
    }
    sauvegardeEEPROM();
}

// recuperation du Mode de fonctionnement de la vanne 3 voie du mur chaffant
DT_3voies_1_nath_mode DT_3voies_1_nath_get_mode(void)
{
    return eeprom_config.mode_3voies_1_nath;
}

// deffinition du coefician Proportionnel de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KP(float kp)
{
    eeprom_config.pid_3voies_1_nath.KP = kp;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_3voies_1_nath.SetTunings(eeprom_config.pid_3voies_1_nath.KP, eeprom_config.pid_3voies_1_nath.KI, eeprom_config.pid_3voies_1_nath.KD);
}

// deffinition du coefician d'Integral de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KI(float ki)
{
    eeprom_config.pid_3voies_1_nath.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_3voies_1_nath.SetTunings(eeprom_config.pid_3voies_1_nath.KP, eeprom_config.pid_3voies_1_nath.KI, eeprom_config.pid_3voies_1_nath.KD);
}

// deffinition du coefician de Dérivation de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KD(float kd)
{
    eeprom_config.pid_3voies_1_nath.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_3voies_1_nath.SetTunings(eeprom_config.pid_3voies_1_nath.KP, eeprom_config.pid_3voies_1_nath.KI, eeprom_config.pid_3voies_1_nath.KD);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KT(uint32_t kt)
{
    eeprom_config.pid_3voies_1_nath.KT = kt;
    sauvegardeEEPROM();
    pid_3voies_1_nath.SetSampleTimeUs(eeprom_config.pid_3voies_1_nath.KT * 1000);
    pid_3voies_1_nath.SetOutputLimits((float)((float)eeprom_config.pid_3voies_1_nath.KT * -1.0), (float)eeprom_config.pid_3voies_1_nath.KT);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du mure chaffant
void DT_3voies_1_nath_set_action(QuickPID::Action action)
{
    eeprom_config.pid_3voies_1_nath.action = action;
    sauvegardeEEPROM();
    pid_3voies_1_nath.SetControllerDirection(eeprom_config.pid_3voies_1_nath.action);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_3voies_1_nath.pmode = pMode;
    sauvegardeEEPROM();
    pid_3voies_1_nath.SetProportionalMode(eeprom_config.pid_3voies_1_nath.pmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_3voies_1_nath.dmode = dMode;
    sauvegardeEEPROM();
    pid_3voies_1_nath.SetDerivativeMode(eeprom_config.pid_3voies_1_nath.dmode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_3voies_1_nath.iawmode = iAwMode;
    sauvegardeEEPROM();
    pid_3voies_1_nath.SetAntiWindupMode(eeprom_config.pid_3voies_1_nath.iawmode);
}

// set consigne temp MCBT
void DT_3voies_1_nath_set_manual_setpoint(float setpoint)
{
    eeprom_config.SetPoint_manual_3voies_1_nath = setpoint;
    SetPoint_3voies_1 = setpoint;
    sauvegardeEEPROM();
}

float DT_3voies_1_nath_get_KP()
{
    return eeprom_config.pid_3voies_1_nath.KP;
}

float DT_3voies_1_nath_get_KI()
{
    return eeprom_config.pid_3voies_1_nath.KI;
}

float DT_3voies_1_nath_get_KD()
{
    return eeprom_config.pid_3voies_1_nath.KD;
}
uint32_t DT_3voies_1_nath_get_KT()
{
    return eeprom_config.pid_3voies_1_nath.KT;
}

void DT_3voies_1_nath_set_callback(void (*callback)(const float Consigne))
{
    _callback_3_voies = callback;
}

void DT_3voies_1_nath_set_callback_pid(void (*callback_mcbt_pid)(const float setpoint, const float P, const float I, const float D, const float Out))
{
    _callback_mcbt_pid = callback_mcbt_pid;
}

// get consigne temp MCBT
float DT_3voies_1_nath_get_manual_setpoint()
{
    return eeprom_config.SetPoint_manual_3voies_1_nath;
}

#endif // DT_3voies_1_nath