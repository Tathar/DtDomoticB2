#ifndef DT_MCBT
#define DT_MCBT

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <QuickPID.h>

enum __attribute__((__packed__)) DT_MCBT_mode
{
    DT_MCBT_OFF,
    DT_MCBT_DEMMARAGE,
    DT_MCBT_NORMAL,
    DT_MCBT_MANUAL,
    DT_MCBT_STANDBY,
};

void DT_MCBT_init();
void DT_MCBT_loop();
void DT_MCBT_set_mode(DT_MCBT_mode mode);
DT_MCBT_mode DT_MCBT_get_mode(void);

void DT_MCBT_set_KP(float kp);
void DT_MCBT_set_KI(float ki);
void DT_MCBT_set_KD(float kd);
void DT_MCBT_set_KT(uint32_t kt);
void DT_MCBT_set_C3(float c3); // set consigne temp MCBT

float DT_MCBT_get_KP();
float DT_MCBT_get_KI();
float DT_MCBT_get_KD();
uint32_t DT_MCBT_get_KT();
float DT_MCBT_get_C3(); // get consigne temp MCBT
float DT_MCBT_get_temp_moyen();
float get_temp_ext(); // fournie la temperature exterieur moyenné en fonction du decalage choisie

void DT_MCBT_set_action(QuickPID::Action action);
void DT_MCBT_set_pmode(QuickPID::pMode pMode);
void DT_MCBT_set_dmode(QuickPID::dMode dMode);
void DT_MCBT_set_iawmode(QuickPID::iAwMode iAwMode);

void DT_MCBT_set_callback(void (*callback)(const float Consigne));

void DT_MCBT_set_callback_pid(void (*callback_mcbt_pid)(const float P, const float I, const float D, const float Out));


#endif