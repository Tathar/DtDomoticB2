
#include "../lib/DT_chauffage/DT_3voies.h"
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <./DT_temp_ext.h>

#include <DT_eeprom.h>

#include <config.h>

// T1 = Temp Ballon		T2 = Temp ECS			    T3 = Temp ECS2
// T5 = Temp Extérieur	T6 = Temp Vanne 3V PCBT	    T7 = Temp Vanne 3V MCBT	    T8 = Temp Vanne 3V Jacuzzi

// Specify the links and initial tuning parameters

// mise a l'échelle
float scale(float in, float in_min, float in_max, float out_min, float out_max)
{
    return ((((in - in_min) / (in_max - in_min)) * (out_max - out_min)) + out_min);
}

DT3voies::DT3voies() : pid(&Input, &Output, &SetPoint) {};

// initialisation des vanne 3 voies
void DT3voies::init(float KP, float KI, float KD, uint32_t KT, QuickPID::Action action, QuickPID::pMode pMode, QuickPID::dMode dMode, QuickPID::iAwMode iAwMode, uint8_t relay_hot, uint8_t relay_cold, uint8_t relay_circulateur, mode mode)
{

    pid = QuickPID(&Input, &Output, &SetPoint);

    // KP, KI, KD
    pid.SetTunings(KP, KI, KD);

    // min, max
    pid.SetOutputLimits((float)((float)KT * -1.0), (float)KT);

    // loop time (KT)
    pid.SetSampleTimeUs(KT * 1000);

    // Direction
    pid.SetControllerDirection(action);

    // pMode
    pid.SetProportionalMode(pMode);

    // dMode
    pid.SetDerivativeMode(dMode);

    // Anti Windup
    pid.SetAntiWindupMode(iAwMode);

    Input = 0;
    Output = 0;
    loop_old_now = 0;
    SetPoint = 0;
    old_SetPoint = 0;
    Relay_hot = relay_hot;
    Relay_cold = relay_cold;
    Relay_circulateur = relay_circulateur;
}

// boucle principale des vanne 3 voie
bool DT3voies::loop(float input, float setpoint)
{

    uint32_t now = millis();

    // temperature de l'eau
    float calc_lissage = 0;

    if (now - loop_old_now > 1000)
    {
        loop_old_now = now;
        if (input > 0)
        {

            if (lissage_temp[NUM_LISSAGE - 1] > 0)
            {
                lissage_temp[position_lissage] = input;
                position_lissage++;
                if (position_lissage == NUM_LISSAGE)
                {
                    position_lissage = 0;
                }
            }
            else
            {
                for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
                {
                    lissage_temp[num] = input;
                }
            }
        }
    }

    if (Mode == ON)
    {

        // calcule des consignes de temperature
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            calc_lissage += lissage_temp[num];
        }
        Input = calc_lissage / NUM_LISSAGE;

        if (pid.Compute())
        {
            if (Output > 0)
            {
                if (Output > inhibit_time)
                {
                    DT_relay(Relay_hot, (uint32_t)(Output)); // activation de la vanne
                }
            }
            else
            {

                if (Output > (inhibit_time * -1))
                {
                    DT_relay(Relay_cold, (uint32_t)(Output * -1)); // activation de la vanne
                }
            }
            return true;
        }
        return false;
    }
    else if (Mode == OFF)
    {
        // movement de la vanne 3voies et des circulateur toute les semaines pour eviter le grippage
        if (now - tempo_antigrippage > 604800000)
        {
            tempo_antigrippage = now;
            DT_relay(Relay_cold, true); // activation de la vanne froide pendant 60s
            antigrippage = cycle_antigrippage::Cold;
        }
        else if (cycle_antigrippage::Cold == antigrippage && now - tempo_antigrippage > 60000)
        {
            tempo_antigrippage = now;
            DT_relay(Relay_cold, false);       // arret de la vanne froide
            DT_relay(Relay_circulateur, true); // demarrage du circulateur pendant 60s
            antigrippage = cycle_antigrippage::circulateur;
        }
        else if (cycle_antigrippage::circulateur == antigrippage && now - tempo_antigrippage > 60000)
        {
            tempo_antigrippage = now;
            DT_relay(Relay_circulateur, false); // arret du circulateur
            DT_relay(Relay_hot, true);          // activation de la vanne chaude pendant 60s
            antigrippage = cycle_antigrippage::hot;
        }
        else if (cycle_antigrippage::hot == antigrippage && now - tempo_antigrippage > 60000)
        {
            tempo_antigrippage = now;
            DT_relay(Relay_hot, false); // arret de la vanne chaude
            antigrippage = cycle_antigrippage::end_cycle;
        }
        return false;
    }

    return false;
}

// deffinition du coefician Proportionnel de la vanne 3 voie du mur chaffant
void DT3voies::set_KP(float kp)
{
    pid.SetTunings(kp, pid.GetKi(), pid.GetKd());
}

// deffinition du coefician d'Integral de la vanne 3 voie du mur chaffant
void DT3voies::set_KI(float ki)
{
    pid.SetTunings(pid.GetKi(), ki, pid.GetKd());
}

// deffinition du coefician de Dérivation de la vanne 3 voie du mur chaffant
void DT3voies::set_KD(float kd)
{
    pid.SetTunings(pid.GetKi(), pid.GetKi(), kd);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du mur chaffant
void DT3voies::set_KT(uint32_t kt)
{

    pid.SetSampleTimeUs(kt * 1000);
    pid.SetOutputLimits((float)((float)kt * -1.0), (float)kt);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du mure chaffant
void DT3voies::set_action(QuickPID::Action action)
{
    pid.SetControllerDirection(action);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du mur chaffant
void DT3voies::set_pmode(QuickPID::pMode pMode)
{
    pid.SetProportionalMode(pMode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du mur chaffant
void DT3voies::set_dmode(QuickPID::dMode dMode)
{
    pid.SetDerivativeMode(dMode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT3voies::set_iawmode(QuickPID::iAwMode iAwMode)
{
    pid.SetAntiWindupMode(iAwMode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT3voies::set_mode(DT3voies::mode mode)
{
    Mode = mode;
}

DT3voies::mode DT3voies::get_mode()
{
    return Mode;
}

float DT3voies::get_ouput()
{
    return Output;
}

float DT3voies::get_setpoint()
{
    return SetPoint;
}