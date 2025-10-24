
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

DT3voies vannes;

void (*_callback_3_voies)(const float consigne);
void (*_callback_mcbt_pid)(const float setpoint, const float P, const float I, const float D, const float Out);
bool async_call_mcbt_pid;

// initialisation des vanne 3 voies
void DT_3voies_1_nath_init()
{

    _callback_3_voies = nullptr;
    _callback_mcbt_pid = nullptr;

    DT3voies::mode mode = DT3voies::mode::OFF;

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF || eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_STANDBY)
    {
        // SetPoint_3voies_1 = 0;
        mode = DT3voies::mode::OFF;
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        // SetPoint_3voies_1 = scale(get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        mode = DT3voies::mode::ON;
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath);
        // SetPoint_3voies_1 = eeprom_config.SetPoint_manual_3voies_1_nath;
        mode = DT3voies::mode::ON;
    }

    vannes.init(eeprom_config.pid_3voies_1_nath.KP, eeprom_config.pid_3voies_1_nath.KI, eeprom_config.pid_3voies_1_nath.KD, eeprom_config.pid_3voies_1_nath.KT, eeprom_config.pid_3voies_1_nath.action, eeprom_config.pid_3voies_1_nath.pmode, eeprom_config.pid_3voies_1_nath.dmode, eeprom_config.pid_3voies_1_nath.iawmode, DT_3VOIES_1_NATH_HOT, DT_3VOIES_1_NATH_COLD, DT_3VOIES_1_NATH_CIRCULATEUR, mode);

    // turn the PID on
    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF || eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_STANDBY)
    {
        vannes.set_mode(DT3voies::mode::OFF);
    }
    else
    {
        vannes.set_mode(DT3voies::mode::ON);
    }
}

// boucle principale des vanne 3 voie
void DT_3voies_1_nath_loop()
{
    uint32_t now = millis();
    float setpoint = 0;

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        setpoint = scale(get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_1_nath, eeprom_config.SetPoint_auto_2_3voies_1_nath); // calcul de la consigne en fonction de la temperature exterieur
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

    if (setpoint > eeprom_config.SetPoint_3voies_1_nath_max)
        setpoint = eeprom_config.SetPoint_3voies_1_nath_max;

    if (setpoint > DT_3VOIES_1_NATH_MAX)
        setpoint = DT_3VOIES_1_NATH_MAX;

    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        if (vannes.get_mode() == DT3voies::mode::ON && (DT_pt100_get(DT_PT100_EXT) < eeprom_config.SetPoint_3voies_1_nath_min))
        {
            vannes.set_mode(DT3voies::mode::OFF);
        }
        else if (vannes.get_mode() == DT3voies::mode::OFF && (DT_pt100_get(DT_PT100_EXT) >= eeprom_config.SetPoint_3voies_1_nath_min))
        {
            vannes.set_mode(DT3voies::mode::ON);
        }
    }

    static uint32_t old_now = 0;
    if (vannes.loop(DT_pt100_get(DT_3VOIES_1_NATH_CHAUFFAGE_PT100), setpoint) && now - old_now > 1000)
    {
        old_now = now;
        _callback_mcbt_pid(setpoint, vannes.pid.GetPterm(), vannes.pid.GetIterm(), vannes.pid.GetDterm(), vannes.get_ouput());
    }
}

// changement du Mode de fonctionnement de la vanne 3 voie du mure chauffant
void DT_3voies_1_nath_set_mode(DT_3voies_1_nath_mode mode)
{
    eeprom_config.mode_3voies_1_nath = mode;
    // sauvegardeEEPROM();
    if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_OFF)
    {
        vannes.set_mode(DT3voies::mode::OFF);
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_NORMAL)
    {
        vannes.set_mode(DT3voies::mode::ON);
    }
    else if (eeprom_config.mode_3voies_1_nath == DT_3voies_1_nath_MANUAL)
    {
        vannes.set_mode(DT3voies::mode::ON);
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
    vannes.set_KP(eeprom_config.pid_3voies_1_nath.KP);
}

// deffinition du coefician d'Integral de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KI(float ki)
{
    eeprom_config.pid_3voies_1_nath.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    vannes.set_KI(eeprom_config.pid_3voies_1_nath.KI);
}

// deffinition du coefician de Dérivation de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KD(float kd)
{
    eeprom_config.pid_3voies_1_nath.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    vannes.set_KD(eeprom_config.pid_3voies_1_nath.KD);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_KT(uint32_t kt)
{
    eeprom_config.pid_3voies_1_nath.KT = kt;
    sauvegardeEEPROM();
    vannes.set_KT(eeprom_config.pid_3voies_1_nath.KT);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du mure chaffant
void DT_3voies_1_nath_set_action(QuickPID::Action action)
{
    eeprom_config.pid_3voies_1_nath.action = action;
    sauvegardeEEPROM();
    vannes.set_action(eeprom_config.pid_3voies_1_nath.action);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_3voies_1_nath.pmode = pMode;
    sauvegardeEEPROM();
    vannes.set_pmode(eeprom_config.pid_3voies_1_nath.pmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_3voies_1_nath.dmode = dMode;
    sauvegardeEEPROM();
    vannes.set_dmode(eeprom_config.pid_3voies_1_nath.dmode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT_3voies_1_nath_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_3voies_1_nath.iawmode = iAwMode;
    sauvegardeEEPROM();
    vannes.set_iawmode(eeprom_config.pid_3voies_1_nath.iawmode);
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