#ifndef DT_3VOIES
#define DT_3VOIES

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <QuickPID.h>

enum __attribute__((__packed__)) DT_3voies_mode
{
    DT_3VOIES_OFF,
    DT_3VOIES_DEMMARAGE,
    DT_3VOIES_NORMAL,
    DT_3VOIES_MANUAL,
    DT_3VOIES_STANDBY,
};

void DT_3voies_init();
void DT_3voies_loop();
void DT_PCBT_set_mode(DT_3voies_mode mode);
DT_3voies_mode DT_PCBT_get_mode(void);
void DT_3voies_MCBT_set_mode(DT_3voies_mode mode);
DT_3voies_mode DT_3voies_MCBT_get_mode(void);

void DT_PCBT_set_KP(float kp);
void DT_PCBT_set_KI(float ki);
void DT_PCBT_set_KD(float kd);
void DT_PCBT_set_KT(uint32_t kt);
void DT_3voies_set_C2(float c2); // set consigne temp PCBT

void DT_3voies_MCBT_set_KP(float kp);
void DT_3voies_MCBT_set_KI(float ki);
void DT_3voies_MCBT_set_KD(float kd);
void DT_3voies_MCBT_set_KT(uint32_t kt);
void DT_3voies_set_C3(float c3); // set consigne temp MCBT

float DT_PCBT_get_KP();
float DT_PCBT_get_KD();
float DT_PCBT_get_KI();
uint32_t DT_PCBT_get_KT();
float DT_3voies_get_C2(); // get consigne temp PCBT

float DT_3voies_MCBT_get_KP();
float DT_3voies_MCBT_get_KI();
float DT_3voies_MCBT_get_KD();
uint32_t DT_3voies_MCBT_get_KT();
float DT_3voies_get_C3(); // get consigne temp MCBT
// float DT_3voies_get_temp_moyen();
// float get_temp_ext(); // fournie la temperature exterieur moyenné en fonction du decalage choisie

void DT_PCBT_set_action(QuickPID::Action action);
void DT_3voies_MCBT_set_action(QuickPID::Action action);
void DT_PCBT_set_pmode(QuickPID::pMode pMode);
void DT_3voies_MCBT_set_pmode(QuickPID::pMode pMode);
void DT_PCBT_set_dmode(QuickPID::dMode dMode);
void DT_3voies_MCBT_set_dmode(QuickPID::dMode dMode);
void DT_PCBT_set_iawmode(QuickPID::iAwMode iAwMode);
void DT_3voies_MCBT_set_iawmode(QuickPID::iAwMode iAwMode);

void DT_3voies_set_callback(void (*callback)(const float C2, const float C3));

void DT_3voies_mcbt_set_callback_pid(void (*callback_pcbt_pid)(const float P, const float I, const float D, const float Out));
void DT_PCBT_set_callback_pid(void (*callback_mcbt_pid)(const float P, const float I, const float D, const float Out));
void DT_3voies_set_callback_avg_temp(void (*callback_avg_temp)(const float temp));

#endif