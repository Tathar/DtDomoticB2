
#ifndef _DT_3VOIES_1_NATH
#define _DT_3VOIES_1_NATH

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <QuickPID.h>

enum __attribute__((__packed__)) DT_3voies_1_nath_mode
{
    DT_3voies_1_nath_OFF,
    DT_3voies_1_nath_NORMAL,
    DT_3voies_1_nath_MANUAL,
    DT_3voies_1_nath_STANDBY,
};

void DT_3voies_1_nath_init();
void DT_3voies_1_nath_loop();
void DT_3voies_1_nath_set_mode(DT_3voies_1_nath_mode mode);
DT_3voies_1_nath_mode DT_3voies_1_nath_get_mode(void);

void DT_3voies_1_nath_set_KP(float kp);
void DT_3voies_1_nath_set_KI(float ki);
void DT_3voies_1_nath_set_KD(float kd);
void DT_3voies_1_nath_set_KT(uint32_t kt);
void DT_3voies_1_nath_set_setpoint(float c3); // set consigne temp MCBT

float DT_3voies_1_nath_get_KP();
float DT_3voies_1_nath_get_KI();
float DT_3voies_1_nath_get_KD();
uint32_t DT_3voies_1_nath_get_KT();
float DT_3voies_1_nath_get_setpoint(); // get consigne temp MCBT
float DT_3voies_1_nath_get_temp_moyen();


// float get_temp_ext(); // fournie la temperature exterieur moyenné en fonction du decalage choisie

void DT_3voies_1_nath_set_action(QuickPID::Action action);
void DT_3voies_1_nath_set_pmode(QuickPID::pMode pMode);
void DT_3voies_1_nath_set_dmode(QuickPID::dMode dMode);
void DT_3voies_1_nath_set_iawmode(QuickPID::iAwMode iAwMode);

void DT_3voies_1_nath_set_callback(void (*callback)(const float Consigne));

void DT_3voies_1_nath_set_callback_pid(void (*callback_mcbt_pid)(const float P, const float I, const float D, const float Out));
#endif