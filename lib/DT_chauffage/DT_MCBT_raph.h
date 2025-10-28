
#ifdef DT_3VOIES_MCBT_RAPH

#ifndef _DT_3VOIES_MCBT_RAPH
#define _DT_3VOIES_MCBT_RAPH

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <QuickPID.h>

enum __attribute__((__packed__)) DT_3voies_MCBT_raph_mode
{
    DT_3voies_MCBT_raph_OFF,
    DT_3voies_MCBT_raph_NORMAL,
    DT_3voies_MCBT_raph_MANUAL,
};

void DT_3voies_MCBT_raph_init();
void DT_3voies_MCBT_raph_loop();
void DT_3voies_MCBT_raph_set_mode(DT_3voies_MCBT_raph_mode mode);
DT_3voies_MCBT_raph_mode DT_3voies_MCBT_raph_get_mode(void);

void DT_3voies_MCBT_raph_set_KP(float kp);
void DT_3voies_MCBT_raph_set_KI(float ki);
void DT_3voies_MCBT_raph_set_KD(float kd);
void DT_3voies_MCBT_raph_set_KT(uint32_t kt);
void DT_3voies_MCBT_raph_set_manual_setpoint(float setpoint); // set consigne temp MCBT
void DT_3voies_MCBT_raph_set_inhib_out(float inhib_out);

float DT_3voies_MCBT_raph_get_KP();
float DT_3voies_MCBT_raph_get_KI();
float DT_3voies_MCBT_raph_get_KD();
uint32_t DT_3voies_MCBT_raph_get_KT();
float DT_3voies_MCBT_raph_get_manual_setpoint(); // get manual consigne temp MCBT
float DT_3voies_MCBT_raph_get_setpoint(); // get consigne temp MCBT
float DT_3voies_MCBT_raph_get_inhib_out();


// float DT_get_temp_ext(); // fournie la temperature exterieur moyenné en fonction du decalage choisie

void DT_3voies_MCBT_raph_set_action(QuickPID::Action action);
void DT_3voies_MCBT_raph_set_pmode(QuickPID::pMode pMode);
void DT_3voies_MCBT_raph_set_dmode(QuickPID::dMode dMode);
void DT_3voies_MCBT_raph_set_iawmode(QuickPID::iAwMode iAwMode);

void DT_3voies_MCBT_raph_set_callback_pid(void (*callback_mcbt_pid)(const float setpoint, const float P, const float I, const float D, const float Out));
#endif //_DT_3VOIES_MCBT_RAPH

#endif //DT_3voies_MCBT_raph