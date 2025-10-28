
#ifndef _DT_3VOIES
#define _DT_3VOIES

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <QuickPID.h>

#define NUM_LISSAGE 10 // nombre de valeur pour le lissage de la temperature

// mise a l'échelle
float scale(float in, float in_min, float in_max, float out_min, float out_max);

class DT3voies
{

public:
    enum __attribute__((__packed__)) mode
    {
        ON,
        OFF,
    };

    enum __attribute__((__packed__)) cycle_antigrippage
    {
        end_cycle,
        Cold,
        circulateur,
        hot,
    };

    DT3voies();

    void init(float KP, float KI, float KD, uint32_t KT, QuickPID::Action action, QuickPID::pMode pMode, QuickPID::dMode dMode, QuickPID::iAwMode iAwMode, uint8_t relay_hot, uint8_t relay_cold, uint8_t relay_circulateur, float inhibit_time, mode mode);
    bool loop(float input, float setpoint);

    void set_mode(mode mode);
    DT3voies::mode get_mode();

    void set_KP(float kp);
    void set_KI(float ki);
    void set_KD(float kd);
    void set_KT(uint32_t kt);
    void set_inhibit_time(float inhibit_time);

    void set_action(QuickPID::Action action);
    void set_pmode(QuickPID::pMode pMode);
    void set_dmode(QuickPID::dMode dMode);
    void set_iawmode(QuickPID::iAwMode iAwMode);

    float get_ouput();
    float get_setpoint();

    QuickPID pid;

private:
    float Input, Output, SetPoint;
    uint32_t loop_old_now;
    float old_SetPoint;
    float lissage_temp[NUM_LISSAGE];
    mode Mode;
    uint8_t position_lissage;
    uint32_t Inhibit_time;
    uint8_t Relay_hot;
    uint8_t Relay_cold;
    uint8_t Relay_circulateur;
    cycle_antigrippage antigrippage;
    uint32_t tempo_antigrippage;
};
#endif