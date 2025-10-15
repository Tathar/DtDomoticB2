#include <DT_3voies.h>
#include <DT_input.h>
#include <DT_PT100.h>
#include <DT_relay.h>
#include <./DT_temp_ext.h>

#include <DT_eeprom.h>

#include <config.h>

// T1 = Temp Ballon		T2 = Temp ECS			    T3 = Temp ECS2
// T5 = Temp Extérieur	T6 = Temp Vanne 3V PCBT	    T7 = Temp Vanne 3V MCBT	    T8 = Temp Vanne 3V Jacuzzi

// #define NB_VANNE3_VOIES 2

#define D2MS 86400000 // 1 jours en miliseconde

#define temp_long_1 25 // en °C
#define temp_long_2 35 // en °C

#define NUM_LISSAGE 5 // en °C

#ifdef PCBT

float Input_PCBT, Output_PCBT;

float lissage_pcbt[NUM_LISSAGE];
// Specify the links and initial tuning parameters

QuickPID pid_pcbt = QuickPID(&Input_PCBT, &Output_PCBT, &mem_config.C2);

uint32_t temp_etape_pcbt = 0;
uint32_t temp_start_pcbt = 0;
uint8_t temperature_etape_pcbt = 0;
uint32_t temp_lecture_temp_pcbt = 0;

void (*_callback_3_voies)(const float C2, const float C3);

void (*_callback_pcbt_pid)(const float P, const float I, const float D, const float Out);
bool async_call_pcbt_pid;


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

    mem_config.C2 = 0;
    // for (uint8_t x = 0; x < 24; ++x)
    // {
    //      temp_buffer.push(TEMP_DEFAULT_PT100);
    // }

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
    else if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF || eeprom_config.mode_3voies_PCBT == DT_3VOIES_STANDBY)
    {
        mem_config.C2 = 0;
    }
    else
    {
        // mem_config.C2 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C8, eeprom_config.C9);
        mem_config.C2 = scale(get_temp_ext(), -10, 10, eeprom_config.C8, eeprom_config.C9);
    }

    // KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);

    // min, max
    pid_pcbt.SetOutputLimits((float)((float)eeprom_config.pid_pcbt.KT * -1.0), eeprom_config.pid_pcbt.KT);

    // loop time (KT)
    pid_pcbt.SetSampleTimeUs(eeprom_config.pid_pcbt.KT * 1000);

    // Direction
    pid_pcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);

    // pMode
    pid_pcbt.SetProportionalMode(eeprom_config.pid_pcbt.pmode);

    // dMode
    pid_pcbt.SetDerivativeMode(eeprom_config.pid_pcbt.dmode);

    // Anti Windup
    pid_pcbt.SetAntiWindupMode(eeprom_config.pid_pcbt.iawmode);

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
        Input_PCBT = mem_config.C2;
        for (uint8_t num = 0; num < NUM_LISSAGE; ++num)
        {
            lissage_pcbt[num] = Input_PCBT;
        }
    }

    // turn the PID on
    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF || eeprom_config.mode_3voies_PCBT == DT_3VOIES_STANDBY)
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
        pid_pcbt.Initialize();
    }

    Output_PCBT = 0;
}

// boucle principale des vanne 3 voie
void DT_3voies_loop()
{
    uint32_t now = millis();
    // static uint8_t first_run = 0;
    static uint32_t old_now = 0;
    static float old_C2 = 0;

    // calcule des consignes de temperature
    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_DEMMARAGE)
    {
        if (now - temp_start_pcbt > temp_etape_pcbt)
        {
            temperature_etape_pcbt += 1;
            temp_start_pcbt = now;
            if (temperature_etape_pcbt > temp_long_2)
            {
                DT_PCBT_set_mode(DT_3VOIES_NORMAL);
            }
            else if (temperature_etape_pcbt == temp_long_1 || temperature_etape_pcbt == temp_long_2)
                temp_etape_pcbt = D2MS * 3;
            else
                temp_etape_pcbt = D2MS;
        }

        mem_config.C2 = temperature_etape_pcbt;
    }
    else if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF || eeprom_config.mode_3voies_PCBT == DT_3VOIES_STANDBY)
    {
        mem_config.C2 = 0;
    }
    else if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_NORMAL)
    {
        // mem_config.C2 = scale(DT_pt100_get(PT100_EXT), -10, 10, eeprom_config.C8, eeprom_config.C9); // calcul de la consigne en fonction de la temperature exterieur
        mem_config.C2 = scale(get_temp_ext(), -10, 10, eeprom_config.C8, eeprom_config.C9); // calcul de la consigne en fonction de la temperature exterieur
        if (mem_config.MQTT_online)                                                         // si la carte est connecte au serveur MQTT
        {
            mem_config.C2 += eeprom_config.in_offset_PCBT; // ajout du decalage de la consigne (mode eco)
        }
    }


    // protection sur temperature
    if (mem_config.C2 > eeprom_config.C_PCBT_MAX)
        mem_config.C2 = eeprom_config.C_PCBT_MAX;

    if (mem_config.C2 > TMP_EAU_PCBT_MAX)
        mem_config.C2 = TMP_EAU_PCBT_MAX;

    // temperature de l'eau
    static uint32_t temp_lissage = 0;
    static uint8_t num_pcbt = 0;
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
    }

    // consigne minimum pour fonctionnement des circulateur
    if ((eeprom_config.mode_3voies_PCBT != DT_3VOIES_DEMMARAGE) && (DT_pt100_get(PT100_EXT) > (eeprom_config.C_PCBT_MIN + 0.5)))
    {
        DT_relay(CIRCULATEUR_PCBT, false);           // arret du circulateur
        pid_pcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }
    else if ((eeprom_config.mode_3voies_PCBT != DT_3VOIES_OFF && eeprom_config.mode_3voies_PCBT != DT_3VOIES_STANDBY) && (DT_pt100_get(PT100_EXT) < (eeprom_config.C_PCBT_MIN - 0.5)))
    {
        DT_relay(CIRCULATEUR_PCBT, true); // demmarage du circulateur
        // Output_PCBT = 0;
        pid_pcbt.SetMode(QuickPID::Control::automatic); // demmarage de la vanne 3
    }
    else if ((eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF || eeprom_config.mode_3voies_PCBT == DT_3VOIES_STANDBY))
    {
        DT_relay(CIRCULATEUR_PCBT, false);           // arret du circulateur
        pid_pcbt.SetMode(QuickPID::Control::manual); // arret de la vanne 3 voie
        Output_PCBT = 0;
        async_call_pcbt_pid = true;
    }
   
    if (pid_pcbt.Compute()) // calcule du PID
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

            if (Output_PCBT > (eeprom_config.out_inhib_PCBT * -1))
            {
                Output_PCBT = 0;
            }

            // Output_PCBT -= eeprom_config.out_offset_PCBT;
            DT_relay(VANNE_PCBT_COLD, (uint32_t)(Output_PCBT * -1)); // activation de la vanne
        }
        async_call_pcbt_pid = true;
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
void DT_PCBT_set_mode(DT_3voies_mode mode)
{
    eeprom_config.mode_3voies_PCBT = mode;
    if (eeprom_config.mode_3voies_PCBT == DT_3VOIES_OFF || eeprom_config.mode_3voies_PCBT == DT_3VOIES_STANDBY)
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


// recuperation du Mode de fonctionnement de la vanne 3 voie du planché chaffant
DT_3voies_mode DT_PCBT_get_mode(void)
{
    return eeprom_config.mode_3voies_PCBT;
}

// deffinition du coefician Proportionnel de la vanne 3 voie du planché chaffant
void DT_PCBT_set_KP(float kp)
{
    eeprom_config.pid_pcbt.KP = kp;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);
}

// deffinition du coefician d'Integral de la vanne 3 voie du planché chaffant
void DT_PCBT_set_KI(float ki)
{
    eeprom_config.pid_pcbt.KI = ki;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);
}


// deffinition du coefician de Dérivation de la vanne 3 voie du planché chaffant
void DT_PCBT_set_KD(float kd)
{
    eeprom_config.pid_pcbt.KD = kd;
    sauvegardeEEPROM();
    // set KP, KI, KD
    pid_pcbt.SetTunings(eeprom_config.pid_pcbt.KP, eeprom_config.pid_pcbt.KI, eeprom_config.pid_pcbt.KD);
}

// deffinition du temps cyclique du PID de la vanne 3 voie du planché chaffant
void DT_PCBT_set_KT(uint32_t kt)
{
    eeprom_config.pid_pcbt.KT = kt;
    sauvegardeEEPROM();
    // set loop time (KT)
    pid_pcbt.SetSampleTimeUs(eeprom_config.pid_pcbt.KT * 1000);
    // min, max
    pid_pcbt.SetOutputLimits((float)((float)eeprom_config.pid_pcbt.KT * -1.0), (float)eeprom_config.pid_pcbt.KT);
}

// deffinition du sens de fonctionnement du PID de la vanne 3 voie du planché chaffant
void DT_PCBT_set_action(QuickPID::Action action)
{
    eeprom_config.pid_pcbt.action = action;
    pid_pcbt.SetControllerDirection(eeprom_config.pid_pcbt.action);
}


// deffinition du mode fonctionnement du coefician KP de la vanne 3 voie du planché chaffant
void DT_PCBT_set_pmode(QuickPID::pMode pMode)
{
    eeprom_config.pid_pcbt.pmode = pMode;
    sauvegardeEEPROM();
    pid_pcbt.SetProportionalMode(eeprom_config.pid_pcbt.pmode);
}

// deffinition du mode fonctionnement du coefician KD de la vanne 3 voie du planché chaffant
void DT_PCBT_set_dmode(QuickPID::dMode dMode)
{
    eeprom_config.pid_pcbt.dmode = dMode;
    sauvegardeEEPROM();
    pid_pcbt.SetDerivativeMode(eeprom_config.pid_pcbt.dmode);
}


// deffinition du mode de reinitialisation de l'acumulateur KI de la vanne 3 voie du planché chaffant
void DT_PCBT_set_iawmode(QuickPID::iAwMode iAwMode)
{
    eeprom_config.pid_pcbt.iawmode = iAwMode;
    sauvegardeEEPROM();
    pid_pcbt.SetAntiWindupMode(eeprom_config.pid_pcbt.iawmode);
}


// set consigne temp PCBT
void DT_3voies_set_C2(float c2)
{
    mem_config.C2 = c2;
}

float DT_PCBT_get_KP()
{
    return eeprom_config.pid_pcbt.KP;
}

float DT_PCBT_get_KI()
{
    return eeprom_config.pid_pcbt.KI;
}

float DT_PCBT_get_KD()
{
    return eeprom_config.pid_pcbt.KD;
}

uint32_t DT_PCBT_get_KT()
{
    return eeprom_config.pid_pcbt.KT;
}

void DT_3voies_set_callback(void (*callback)(const float C2, const float C3))
{
    _callback_3_voies = callback;
}

void DT_PCBT_set_callback_pid(void (*callback_pcbt_pid)(const float P, const float I, const float D, const float Out))
{
    _callback_pcbt_pid = callback_pcbt_pid;
}

// get consigne temp PCBT
float DT_3voies_get_C2()
{
    return mem_config.C2;
}

#endif // PCBT
