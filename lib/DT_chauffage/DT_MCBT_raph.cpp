
#include "../lib/DT_chauffage/DT_3voies_nath.h"
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <./DT_temp_ext.h>
#include <./DT_3voies.h>

#include <DT_eeprom.h>

#include <config.h>

// T1 = Temp Ballon		T2 = Temp ECS			    T3 = Temp ECS2
// T5 = Temp Extérieur	T6 = Temp Vanne 3V MCBT	    T7 = Temp Vanne 3V MCBT	    T8 = Temp Vanne 3V Jacuzzi

#ifdef DT_3VOIES_MCBT_RAPH

DT3voies vanne_MCBT_raph;

void (*_callback_mcbt_pid)(const float setpoint, const float P, const float I, const float D, const float Out);

// initialisation des vanne 3 voies
void DT_3voies_MCBT_raph_init()
{

    _callback_mcbt_pid = nullptr;

    DT3voies::mode mode = DT3voies::mode::OFF;

    if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_OFF )
    {
        // SetPoint_3voies_1 = 0;
        mode = DT3voies::mode::OFF;
    }
    else if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_NORMAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_MCBT_raph, eeprom_config.SetPoint_auto_2_3voies_MCBT_raph);
        // SetPoint_3voies_1 = scale(DT_get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_MCBT_raph, eeprom_config.SetPoint_auto_2_3voies_MCBT_raph);
        mode = DT3voies::mode::ON;
    }
    else if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_MANUAL)
    {
        // SetPoint_3voies_1 = scale(DT_pt100_get(DT_PT100_EXT), -10, 10, eeprom_config.SetPoint_auto_1_3voies_MCBT_raph, eeprom_config.SetPoint_auto_2_3voies_MCBT_raph);
        // SetPoint_3voies_1 = eeprom_config.SetPoint_manual_3voies_MCBT_raph;
        mode = DT3voies::mode::ON;
    }

    vanne_MCBT_raph.init(eeprom_config.pid_3voies_MCBT_raph.KP, eeprom_config.pid_3voies_MCBT_raph.KI, eeprom_config.pid_3voies_MCBT_raph.KD, eeprom_config.pid_3voies_MCBT_raph.KT, eeprom_config.pid_3voies_MCBT_raph.action, eeprom_config.pid_3voies_MCBT_raph.pmode, eeprom_config.pid_3voies_MCBT_raph.dmode, eeprom_config.pid_3voies_MCBT_raph.iawmode, VANNE_MCBT_HOT, VANNE_MCBT_COLD, CIRCULATEUR_MCBT, eeprom_config.out_inhib_3voies_MCBT_raph, mode);

    // turn the PID on
    if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_OFF)
    {
        vanne_MCBT_raph.set_mode(DT3voies::mode::OFF);
    }
    else
    {
        vanne_MCBT_raph.set_mode(DT3voies::mode::ON);
    }
}

// boucle principale des vanne 3 voie
void DT_3voies_MCBT_raph_loop()
{
    uint32_t now = millis();
    float setpoint = 0;

    if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_NORMAL)
    {
        setpoint = scale(DT_get_temp_ext(), -10, 10, eeprom_config.SetPoint_auto_1_3voies_MCBT_raph, eeprom_config.SetPoint_auto_2_3voies_MCBT_raph); // calcul de la consigne en fonction de la temperature exterieur
        if (mem_config.MQTT_online)                                                                                                                // si la carte est connecte au serveur MQTT
        {
            setpoint += eeprom_config.in_offset_3voies_MCBT_raph; // ajout du decalage de la consigne (mode eco)
        }
    }
    else if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_MANUAL)
    {
        setpoint = eeprom_config.SetPoint_manual_3voies_MCBT_raph; // consigne manuel
    }

    // protection sur temperature

    if (setpoint > eeprom_config.SetPoint_3voies_max_MCBT_raph)
        setpoint = eeprom_config.SetPoint_3voies_max_MCBT_raph;

    if (setpoint > TMP_EAU_MCBT_MAX)
        setpoint = TMP_EAU_MCBT_MAX;

    if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_NORMAL)
    {
        if (vanne_MCBT_raph.get_mode() == DT3voies::mode::ON && (DT_pt100_get(DT_PT100_EXT) < eeprom_config.SetPoint_3voies_min_MCBT_raph))
        {
            vanne_MCBT_raph.set_mode(DT3voies::mode::OFF);
        }
        else if (vanne_MCBT_raph.get_mode() == DT3voies::mode::OFF && (DT_pt100_get(DT_PT100_EXT) >= eeprom_config.SetPoint_3voies_min_MCBT_raph))
        {
            vanne_MCBT_raph.set_mode(DT3voies::mode::ON);
        }
    }

    static uint32_t old_now = 0;
    if (vanne_MCBT_raph.loop(DT_pt100_get(PT100_3_VOIES_MCBT), setpoint) && now - old_now > 1000)
    {
        old_now = now;
        _callback_mcbt_pid(setpoint, vanne_MCBT_raph.pid.GetPterm(), vanne_MCBT_raph.pid.GetIterm(), vanne_MCBT_raph.pid.GetDterm(), vanne_MCBT_raph.get_ouput());
    }
}

// changement du Mode de fonctionnement de la vanne 3 voie du mure chauffant
void DT_3voies_MCBT_raph_set_mode(DT_3voies_MCBT_raph_mode mode)
{
    eeprom_config.mode_3voies_MCBT_raph = mode;
    // sauvegardeEEPROM();
    if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_OFF)
    {
        vanne_MCBT_raph.set_mode(DT3voies::mode::OFF);
    }
    else if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_NORMAL)
    {
        vanne_MCBT_raph.set_mode(DT3voies::mode::ON);
    }
    else if (eeprom_config.mode_3voies_MCBT_raph == DT_3voies_MCBT_raph_MANUAL)
    {
        vanne_MCBT_raph.set_mode(DT3voies::mode::ON);
    }
    sauvegardeEEPROM();
}

// recuperation du Mode de fonctionnement de la vanne 3 voie du mur chaffant
DT_3voies_MCBT_raph_mode DT_3voies_MCBT_raph_get_mode(void)
{
    return eeprom_config.mode_3voies_MCBT_raph;
}

// deffinition du coefician Proportionnel de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_raph_set_KP(float kp)
{
    eeprom_config.pid_3voies_MCBT_raph.KP = kp;
    sauvegardeEEPROM();
    // set KP, KI, KD
    vanne_MCBT_raph.set_KP(eeprom_config.pid_3voies_MCBT_raph.KP);
}

// deffinition du coefician d'Integral de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_raph_set_KI(float ki)
{
    eeprom_config.pid_3voies_MCBT_raph.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    vanne_MCBT_raph.set_KI(eeprom_config.pid_3voies_MCBT_raph.KI);
}

// deffinition du coefician de Dérivation de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_raph_set_KD(float kd)
{
    eeprom_config.pid_3voies_MCBT_raph.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    vanne_MCBT_raph.set_KD(eeprom_config.pid_3voies_MCBT_raph.KD);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_raph_set_KT(uint32_t kt)
{
    eeprom_config.pid_3voies_MCBT_raph.KT = kt;
    sauvegardeEEPROM();
    vanne_MCBT_raph.set_KT(eeprom_config.pid_3voies_MCBT_raph.KT);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du mure chaffant
void DT_3voies_MCBT_raph_set_action(QuickPID::Action action)
{
    eeprom_config.pid_3voies_MCBT_raph.action = action;
    sauvegardeEEPROM();
    vanne_MCBT_raph.set_action(eeprom_config.pid_3voies_MCBT_raph.action);
}

// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_raph_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_3voies_MCBT_raph.pmode = pMode;
    sauvegardeEEPROM();
    vanne_MCBT_raph.set_pmode(eeprom_config.pid_3voies_MCBT_raph.pmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_raph_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_3voies_MCBT_raph.dmode = dMode;
    sauvegardeEEPROM();
    vanne_MCBT_raph.set_dmode(eeprom_config.pid_3voies_MCBT_raph.dmode);
}

// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du mur chaffant
void DT_3voies_MCBT_raph_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_3voies_MCBT_raph.iawmode = iAwMode;
    sauvegardeEEPROM();
    vanne_MCBT_raph.set_iawmode(eeprom_config.pid_3voies_MCBT_raph.iawmode);
}

// set consigne temp MCBT
void DT_3voies_MCBT_raph_set_manual_setpoint(float setpoint)
{
    eeprom_config.SetPoint_manual_3voies_MCBT_raph = setpoint;
    sauvegardeEEPROM();
}

float DT_3voies_MCBT_raph_get_KP()
{
    return eeprom_config.pid_3voies_MCBT_raph.KP;
}

float DT_3voies_MCBT_raph_get_KI()
{
    return eeprom_config.pid_3voies_MCBT_raph.KI;
}

float DT_3voies_MCBT_raph_get_KD()
{
    return eeprom_config.pid_3voies_MCBT_raph.KD;
}
uint32_t DT_3voies_MCBT_raph_get_KT()
{
    return eeprom_config.pid_3voies_MCBT_raph.KT;
}

void DT_3voies_MCBT_raph_set_callback_pid(void (*callback_mcbt_pid)(const float setpoint, const float P, const float I, const float D, const float Out))
{
    _callback_mcbt_pid = callback_mcbt_pid;
}

// get consigne temp MCBT
float DT_3voies_MCBT_raph_get_manual_setpoint()
{
    return eeprom_config.SetPoint_manual_3voies_MCBT_raph;
}

float DT_3voies_MCBT_raph_get_setpoint()
{
    return vanne_MCBT_raph.get_setpoint();
}

void DT_3voies_MCBT_raph_set_inhib_out(float inhib_out)
{
    eeprom_config.out_inhib_3voies_MCBT_raph = inhib_out;
    sauvegardeEEPROM();
    vanne_MCBT_raph.set_inhibit_time(eeprom_config.out_inhib_3voies_MCBT_raph);
}

float DT_3voies_MCBT_raph_get_inhib_out()
{
    return eeprom_config.out_inhib_3voies_MCBT_raph;
}

#endif // DT_3VOIES_MCBT_RAPH
