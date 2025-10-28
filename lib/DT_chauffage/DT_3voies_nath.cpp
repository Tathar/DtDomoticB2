
#include "../lib/DT_chauffage/DT_3voies_nath.h"
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <./DT_temp_ext.h>
#include <./DT_3voies.h>

#include <DT_eeprom.h>

#include <config.h>

// T1 = Temp Ballon		T2 = Temp ECS			    T3 = Temp ECS2
// T5 = Temp Extérieur	T6 = Temp Vanne 3V PCBT	    T7 = Temp Vanne 3V MCBT	    T8 = Temp Vanne 3V Jacuzzi

#ifdef DT_3VOIES_1_NATH

DT3voies vanne_1_nath;

void (*_callback_1_nath_pid)(const float setpoint, const float P, const float I, const float D, const float Out);

// initialisation des vanne 3 voies
void DT_3voies_1_nath_init()
{
    _callback_1_nath_pid = nullptr;

    DT3voies::mode mode = DT3voies::mode::OFF;

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF)
    {
        // SetPoint_3voies_1 = 0;
        mode = DT3voies::mode::OFF;
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        // SetPoint_3voies_1 = scale(DT_get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        mode = DT3voies::mode::ON;
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        // SetPoint_3voies_1 = eeprom_config.SetPoint_manual_3voies_1_nath;
        mode = DT3voies::mode::ON;
    }

    vanne_1_nath.init(eeprom_config.pid_3voies_1_nath.KP, eeprom_config.pid_3voies_1_nath.KI, eeprom_config.pid_3voies_1_nath.KD, eeprom_config.pid_3voies_1_nath.KT, eeprom_config.pid_3voies_1_nath.action, eeprom_config.pid_3voies_1_nath.pmode, eeprom_config.pid_3voies_1_nath.dmode, eeprom_config.pid_3voies_1_nath.iawmode, DT_3VOIES_1_NATH_RELAY_HOT, DT_3VOIES_1_NATH_RELAY_COLD, DT_3VOIES_1_NATH_CIRCULATEUR,eeprom_config.out_inhib_3voies_1_nath, mode);

    // turn the PID on
    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF)
    {
        vanne_1_nath.set_mode(DT3voies::mode::OFF);
    }
    else
    {
        vanne_1_nath.set_mode(DT3voies::mode::ON);
    }
}

// boucle principale des vanne 3 voie
void DT_3voies_1_nath_loop()
{
    uint32_t now = millis();
    float setpoint = 0;

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        setpoint = scale(DT_get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath); // calcul de la consigne en fonction de la temperature exterieur
        if (mem_config.MQTT_online)                                                                                                          // si la carte est connecte au serveur MQTT
        {
            setpoint += eeprom_config.in_offset_3voies_1_nath; // ajout du decalage de la consigne (mode eco)
        }
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        setpoint = eeprom_config.SetPoint_manual_3voies_1_nath; // consigne manuel
    }

    // protection sur temperature

    if (setpoint > eeprom_config.SetPoint_3voies_max_1_nath)
        setpoint = eeprom_config.SetPoint_3voies_max_1_nath;

    if (setpoint > DT_3VOIES_1_NATH_MAX_TMP_EAU)
        setpoint = DT_3VOIES_1_NATH_MAX_TMP_EAU;

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        if (vanne_1_nath.get_mode() == DT3voies::mode::ON && (DT_pt100_get(DT_PT100_EXT) < eeprom_config.SetPoint_3voies_min_1_nath))
        {
            vanne_1_nath.set_mode(DT3voies::mode::OFF);
        }
        else if (vanne_1_nath.get_mode() == DT3voies::mode::OFF && (DT_pt100_get(DT_PT100_EXT) >= eeprom_config.SetPoint_3voies_min_1_nath))
        {
            vanne_1_nath.set_mode(DT3voies::mode::ON);
        }
    }

    static uint32_t old_now = 0;
    if (vanne_1_nath.loop(DT_pt100_get(DT_3VOIES_1_NATH_PT100), setpoint) && now - old_now > 1000)
    {
        old_now = now;
        _callback_1_nath_pid(setpoint, vanne_1_nath.pid.GetPterm(), vanne_1_nath.pid.GetIterm(), vanne_1_nath.pid.GetDterm(), vanne_1_nath.get_ouput());
    }
}

// changement du Mode de fonctionnement de la vanne 3 voie du mure chauffant
void DT_3voies_1_nath_set_mode(DT_3voies_1_nath_mode mode)
{
    eeprom_config.mode_3voies_1_nath = mode;
    // sauvegardeEEPROM();
    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF)
    {
        vanne_1_nath.set_mode(DT3voies::mode::OFF);
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        vanne_1_nath.set_mode(DT3voies::mode::ON);
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        vanne_1_nath.set_mode(DT3voies::mode::ON);
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
    vanne_1_nath.set_KP(eeprom_config.pid_3voies_1_nath.KP);
}

// deffinition du coefician d'Integral de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KI(float ki)
{
    eeprom_config.pid_3voies_1_nath.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    vanne_1_nath.set_KI(eeprom_config.pid_3voies_1_nath.KI);
}

// deffinition du coefician de Dérivation de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KD(float kd)
{
    eeprom_config.pid_3voies_1_nath.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    vanne_1_nath.set_KD(eeprom_config.pid_3voies_1_nath.KD);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KT(uint32_t kt)
{
    eeprom_config.pid_3voies_1_nath.KT = kt;
    sauvegardeEEPROM();
    vanne_1_nath.set_KT(eeprom_config.pid_3voies_1_nath.KT);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du mure chaffant
void DT_3voies_1_nath_set_action(QuickPID::Action action)
{
    eeprom_config.pid_3voies_1_nath.action = action;
    sauvegardeEEPROM();
    vanne_1_nath.set_action(eeprom_config.pid_3voies_1_nath.action);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_3voies_1_nath.pmode = pMode;
    sauvegardeEEPROM();
    vanne_1_nath.set_pmode(eeprom_config.pid_3voies_1_nath.pmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_3voies_1_nath.dmode = dMode;
    sauvegardeEEPROM();
    vanne_1_nath.set_dmode(eeprom_config.pid_3voies_1_nath.dmode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_3voies_1_nath.iawmode = iAwMode;
    sauvegardeEEPROM();
    vanne_1_nath.set_iawmode(eeprom_config.pid_3voies_1_nath.iawmode);
}

// set consigne temp MCBT
void DT_3voies_1_nath_set_manual_setpoint(float setpoint)
{
    eeprom_config.SetPoint_manual_3voies_1_nath = setpoint;
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

void DT_3voies_1_nath_set_callback_pid(void (*callback_mcbt_pid)(const float setpoint, const float P, const float I, const float D, const float Out))
{
    _callback_1_nath_pid = callback_mcbt_pid;
}

// get consigne temp MCBT
float DT_3voies_1_nath_get_manual_setpoint()
{
    return eeprom_config.SetPoint_manual_3voies_1_nath;
}

void DT_3voies_1_nath_set_inhib_out(float inhib_out)
{
    eeprom_config.out_inhib_3voies_1_nath = inhib_out;
    sauvegardeEEPROM();
    vanne_1_nath.set_inhibit_time(eeprom_config.out_inhib_3voies_1_nath);
}

float DT_3voies_1_nath_get_inhib_out()
{
    return eeprom_config.out_inhib_3voies_1_nath;
}


#endif // DT_3voies_1_nath