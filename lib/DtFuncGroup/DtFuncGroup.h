#ifndef DtFuncGroup
#define DtFuncGroup

#ifdef UNIT_TEST
#include "ArduinoFake.h"
#else
#include "Arduino.h"
#endif

#include <DT_input.h>


#define  MAX_IN 1

enum __attribute__((__packed__)) FG_Action_type
{
    FG_NULL,
    FG_SWITCH_BTN,
    FG_DIM_SWITCH_BTN,
    FG_PUSH_BTN,
    FG_DIM_PUSH_BTN,
    FG_DIM_CANDLE_PUSH_BTN,
    FG_DIM_2_PUSH_BTN,
    FG_DIM_CANDLE_2_PUSH_BTN,
    FG_STORE_2_PUSH_BTN,
};

struct Config_FG_In
{
    uint8_t num = 0;
    Bt_Action bt_action = IN_NULL;
    // FG_Action_type type = FG_NULL;
};

struct FG_Config_Out
{
    // uint16_t address = 0;
    uint8_t num = 0;
};

struct FG_Action
{
    FG_Action_type type;
    Config_FG_In in[MAX_IN];
    FG_Config_Out out;
};


void FG_Init(void);
void FG_Button(uint8_t button, Bt_Action bt_action);

void FG_loop(void);

#endif