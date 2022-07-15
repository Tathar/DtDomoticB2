#include "DtFuncGroup.h"
#include "config.h"
#include "DT_Dimmer.h"
#include "DT_eeprom.h"

enum __attribute__((__packed__)) Dim_action // l'utilisation de "__attribute__((__packed__))" indique au compilateur de codé l'enum sur 1 octet
{
    DIM_NULL,
    DIM_NULL_OLD_INC,
    DIM_NULL_OLD_DEC,
    DIM_INC,
    DIM_DEC,
    DIM_CANDLE,
    DIM_CANDLE_INC,
    DIM_CANDLE_DEC,
    DIM_STORE_UP,
    DIM_STORE_DOWN,
    DIM_STORE_STOP,
    DIM_STORE_MEMORY_POSITION_UP,
    DIM_STORE_MEMORY_POSITION_DOWN,
    DIM_STORE_VALIDE_SAVE_UP,
    DIM_STORE_VALIDE_SAVE_DOWN,
    DIM_STORE_TIME_CONFIG_UP,
    DIM_STORE_TIME_CONFIG_DOWN,
    DIM_STORE_GO_POSITION_UP,
    DIM_STORE_GO_POSITION_DOWN,

};

struct DIM_2_PUSH_BTN
{
    uint8_t interlock; //adresse
    uint32_t interlock_time;
};

struct STORE
{
    uint8_t interlock; //adresse
    uint8_t old_value; //in percente
    uint8_t go_pos;    //in percent
    Bt_Action push2;   //2 button pushed a same time
    // uint32_t interlock_time;
    uint32_t mouve_time;  //strat move time for percent calcule (in ms)
    uint32_t wait;        //wait before next move (in ms)
    uint32_t store_ratio; // ms per percent
};

struct Data
{
    uint8_t Stats;
    Dim_action Action;
    union
    {
        DIM_2_PUSH_BTN D2PB;
        STORE Store;
    };
};
// union
// {
//     Base base;
//     DIM_2_PUSH_BTN D2PB;
// };

volatile Dim_action dim_func[4] = {DIM_NULL, DIM_NULL, DIM_NULL, DIM_NULL};
uint16_t Dim_action_fg[4] = {0, 0, 0, 0};
Dim_action old_dim_up_down[4] = {DIM_INC, DIM_INC, DIM_INC, DIM_INC};
Data memory[MAX_FG];

void apply_FG_out(uint8_t num_fg, uint8_t value)
{
    for (uint8_t out = 0; out < MAX_OUT; out++)
    {
        if (config.FG[num_fg].out[out].device_address != 0)
        {
            dimmer_set(config.FG[num_fg].out[out].device_address - 9, value);
        }
        else
        {
            break;
        }
    }
}

void slow_apply_FG_out(uint8_t num_fg, uint8_t value)
{
    for (uint8_t out = 0; out < MAX_OUT; out++)
    {
        if (config.FG[num_fg].out[out].device_address != 0)
        {
            dimmer_set(config.FG[num_fg].out[out].device_address - 9, value, config.Dimmer_on_off_time);
        }
        else
        {
            break;
        }
    }
}

void FG_Init()
{
    randomSeed(analogRead(0));

    for (int num_fg = 0; num_fg < MAX_FG; ++num_fg)
    {
        memory[num_fg].Stats = 0;
        memory[num_fg].Action = DIM_NULL;
        if (config.FG[num_fg].type == FG_DIM_2_PUSH_BTN)
        {
            memory[num_fg].D2PB.interlock_time = 0;
            memory[num_fg].D2PB.interlock = 0;
        }
        if (config.FG[num_fg].type == FG_STORE_2_PUSH_BTN)
        {
            memory[num_fg].Store.interlock = 0;
            memory[num_fg].Store.mouve_time = 0;
            memory[num_fg].Store.wait = 0;
            memory[num_fg].Store.push2 = IN_NULL;
            memory[num_fg].Store.store_ratio = config.FG[num_fg].store_time / 100;
        }
    }
}

void FG_loop()
{
    static uint32_t dimmer_refresh = 0;
    CanData data;
    data.u16[0] = 0;
    data.u16[1] = 0;
    data.u16[2] = 0;
    uint32_t now = millis();
    if (now - dimmer_refresh >= config.Dimmer_Speed)
    {
        dimmer_refresh = now;
        //dimer
        for (uint8_t num_fg = 0; num_fg < MAX_FG; ++num_fg)
        {

            if (memory[num_fg].Action == DIM_INC)
            {
                if (memory[num_fg].Stats < 100)
                {
                    memory[num_fg].Stats += 1;
                    // config.FG[num_fg].old_value = memory[num_fg].Stats;
                    data.u8[0] = memory[num_fg].Stats;
                    DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_INC, data); //TODO: Can OK
                    apply_FG_out(num_fg, memory[num_fg].Stats);
                }
            }
            else if (memory[num_fg].Action == DIM_CANDLE_INC)
            {
                if (memory[num_fg].Stats < 100)
                {
                    memory[num_fg].Stats += 1;
                    // config.FG[num_fg].old_value = memory[num_fg].Stats;
                    data.u8[0] = memory[num_fg].Stats;
                    DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_INC, data); //TODO: Can OK
                    apply_FG_out(num_fg, memory[num_fg].Stats);
                }
            }
            else if (memory[num_fg].Action == DIM_DEC)
            {

                if (memory[num_fg].Stats > DIMMER_MIN)
                {

                    memory[num_fg].Stats -= 1;
                    // config.FG[num_fg].old_value = memory[num_fg].Stats;
                    data.u8[0] = memory[num_fg].Stats;
                    DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_DEC, data); //TODO: Can OK
                    apply_FG_out(num_fg, memory[num_fg].Stats);
                }
            }

            else if (memory[num_fg].Action == DIM_CANDLE_DEC)
            {

                if (memory[num_fg].Stats > DIMMER_MIN)
                {
                    if (memory[num_fg].Stats > DIMMER_MIN)
                    {
                        memory[num_fg].Stats -= 1;
                        // config.FG[num_fg].old_value = memory[num_fg].Stats;
                        data.u8[0] = memory[num_fg].Stats;
                        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_DEC, data); //TODO: Can OK
                        apply_FG_out(num_fg, memory[num_fg].Stats);
                    }
                }
            }
        }
    }

    //store
    static uint32_t store_refresh = 0;
    static uint32_t can_store_refresh = 0;
    // if (now - store_refresh >= 50)
    {
        store_refresh = now;
        for (uint8_t num_fg = 0; num_fg < MAX_FG; ++num_fg)
        {
            if (now - memory[num_fg].Store.wait <= FG_STORE_WAIT) //wait before next move
            {
                continue;
            }
            else if (memory[num_fg].Action == DIM_STORE_UP ||
                     memory[num_fg].Action == DIM_STORE_MEMORY_POSITION_UP ||
                     memory[num_fg].Action == DIM_STORE_VALIDE_SAVE_UP ||
                     memory[num_fg].Action == DIM_STORE_TIME_CONFIG_UP)
            {
                if (get_dimmer(config.FG[num_fg].out[0].device_address + 1 - 9) != 0) //if store actualy mouve down
                {
                    for (uint8_t out = 0; out < MAX_OUT; out += 2)
                    {
                        if (config.FG[num_fg].out[out].device_address != 0)
                        {
                            dimmer_set(config.FG[num_fg].out[out].device_address + 1 - 9, 0); //stop store
                        }
                        else
                            break;
                    }
                    memory[num_fg].Store.wait = now; //wait FG_STORE_WAIT time
                }
                // printf("loop DIM_STORE_UP fg = %d\n", num_fg);
                else if (memory[num_fg].Store.mouve_time == 0)
                {
                    // printf("loop start DIM_STORE_UP fg = %d\n", num_fg);

                    //   printf("start ++Stats = %d\n", memory[num_fg].Stats);
                    memory[num_fg].Store.mouve_time = now;
                    memory[num_fg].Store.old_value = memory[num_fg].Stats;
                    for (uint8_t out = 0; out < MAX_OUT; out += 2)
                    {
                        if (config.FG[num_fg].out[out].device_address != 0)
                        {
                            dimmer_set(config.FG[num_fg].out[out].device_address + 1 - 9, 0);
                            dimmer_set(config.FG[num_fg].out[out].device_address - 9, 100);
                        }
                        else
                            break;
                    }
                }
                else if (memory[num_fg].Action == DIM_STORE_UP && now - memory[num_fg].Store.mouve_time >= config.FG[num_fg].store_time + FG_STORE_ADD_TIME) //temps necessaire a la remonté du store + 10 seconds
                {
                    // printf("DIM_STORE_UP time = %i\n", millis() - memory[num_fg].Store.mouve_time);
                    memory[num_fg].Action = DIM_STORE_STOP;
                    memory[num_fg].Store.mouve_time = 0;
                    memory[num_fg].Store.old_value = memory[num_fg].Stats = 100;
                    // printf("++Stats = %d\n", memory[num_fg].Stats);
                    // printf("loop Stop DIM_STORE_UP fg = %d\n", num_fg);
                    for (uint8_t out = 0; out < MAX_OUT; ++out)
                    {
                        if (config.FG[num_fg].out[out].device_address != 0)
                        {
                            dimmer_set(config.FG[num_fg].out[out].device_address - 9, 0);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else // update store position end stop if as memory position
                {
                    if (memory[num_fg].Stats < 100)
                    {
                        memory[num_fg].Stats = memory[num_fg].Store.old_value + ((now - memory[num_fg].Store.mouve_time) / memory[num_fg].Store.store_ratio);
                        memory[num_fg].Stats = memory[num_fg].Stats > 100 ? 100 : memory[num_fg].Stats;
                        data.u8[0] = memory[num_fg].Stats;
                        if (now - can_store_refresh >= 1000)
                        {
                            can_store_refresh = now;
                            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_STORE_POS, data); //TODO: Can ok}
                            Serial.print("store mouve up = ");
                            Serial.println(data.u8[0]);
                        }
                    }

                    if (memory[num_fg].Action == DIM_STORE_MEMORY_POSITION_UP && memory[num_fg].Stats >= config.FG[num_fg].old_value)
                    {
                        memory[num_fg].Action = DIM_STORE_STOP;
                    }
                    else if (memory[num_fg].Action == DIM_STORE_VALIDE_SAVE_UP && memory[num_fg].Stats >= config.FG[num_fg].old_value + 2)
                    {
                        memory[num_fg].Action = DIM_STORE_MEMORY_POSITION_DOWN;
                        memory[num_fg].Store.mouve_time = 0;
                    }
                    // printf("++Stats = %d\n", memory[num_fg].Stats);
                }
            }
            else if (memory[num_fg].Action == DIM_STORE_DOWN ||
                     memory[num_fg].Action == DIM_STORE_MEMORY_POSITION_DOWN ||
                     memory[num_fg].Action == DIM_STORE_VALIDE_SAVE_DOWN ||
                     memory[num_fg].Action == DIM_STORE_TIME_CONFIG_DOWN)
            {
                if (get_dimmer(config.FG[num_fg].out[0].device_address - 9) != 0) //if store actualy mouve up
                {
                    for (uint8_t out = 0; out < MAX_OUT; out += 2)
                    {
                        if (config.FG[num_fg].out[out].device_address != 0)
                        {
                            dimmer_set(config.FG[num_fg].out[out].device_address - 9, 0); //stop store
                        }
                        else
                            break;
                    }
                    memory[num_fg].Store.wait = now; //wait FG_STORE_WAIT time
                }
                // printf("loop DIM_STORE_DOWN fg = %d\n", num_fg);
                else if (memory[num_fg].Store.mouve_time == 0)
                {
                    // printf("loop start DIM_STORE_DOWN fg = %d\n", num_fg);

                    //   printf("start --Stats = %d\n", memory[num_fg].Stats);
                    memory[num_fg].Store.mouve_time = now;
                    memory[num_fg].Store.old_value = memory[num_fg].Stats;
                    for (uint8_t out = 0; out < MAX_OUT; out += 2)
                    {
                        if (config.FG[num_fg].out[out].device_address != 0)
                        {
                            dimmer_set(config.FG[num_fg].out[out].device_address - 9, 0);
                            dimmer_set(config.FG[num_fg].out[out].device_address + 1 - 9, 100);
                        }
                        else
                            break;
                    }
                }
                else if (memory[num_fg].Action == DIM_STORE_DOWN && now - memory[num_fg].Store.mouve_time >= config.FG[num_fg].store_time + FG_STORE_ADD_TIME) //temps necessaire a la remonté du store + 10 seconds
                {
                    // printf("DIM_STORE_DOWN time = %i\n", millis() - memory[num_fg].Store.mouve_time);
                    memory[num_fg].Action = DIM_STORE_STOP;
                    memory[num_fg].Store.mouve_time = 0;
                    memory[num_fg].Store.old_value = memory[num_fg].Stats = 0;
                    // printf("--Stats = %d\n", memory[num_fg].Stats);
                    // printf("loop stop DIM_STORE_DOWN fg = %d\n", num_fg);
                    for (uint8_t out = 0; out < MAX_OUT; ++out)
                    {
                        if (config.FG[num_fg].out[out].device_address != 0)
                        {
                            dimmer_set(config.FG[num_fg].out[out].device_address - 9, 0);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                else // update store position
                {
                    if (memory[num_fg].Stats > 0)
                    {
                        memory[num_fg].Stats = memory[num_fg].Store.old_value - ((now - memory[num_fg].Store.mouve_time) / memory[num_fg].Store.store_ratio);
                        memory[num_fg].Stats = memory[num_fg].Stats > 100 ? 0 : memory[num_fg].Stats;
                        data.u8[0] = memory[num_fg].Stats;
                        if (now - can_store_refresh >= 1000)
                        {
                            can_store_refresh = now;
                            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_STORE_POS, data); //TODO: Can ok}
                            Serial.print("store mouve up = ");
                            Serial.println(memory[num_fg].Stats);
                        }
                    }

                    if (memory[num_fg].Action == DIM_STORE_MEMORY_POSITION_DOWN && memory[num_fg].Stats <= config.FG[num_fg].old_value)
                    {
                        memory[num_fg].Action = DIM_STORE_STOP;
                    }
                    else if (memory[num_fg].Action == DIM_STORE_VALIDE_SAVE_DOWN && memory[num_fg].Stats <= config.FG[num_fg].old_value - 2) //
                    {
                        memory[num_fg].Action = DIM_STORE_MEMORY_POSITION_UP;
                        memory[num_fg].Store.mouve_time = 0;
                    }
                    // printf("--Stats = %d\n", memory[num_fg].Stats);
                }
            }
            else if (memory[num_fg].Action == DIM_STORE_STOP)
            {

                //   printf("DIM_STORE_STOP -+Stats = %d\n", memory[num_fg].Stats);
                // printf("loop DIM_STORE_STOP\n");
                memory[num_fg].Action = DIM_NULL;
                memory[num_fg].Store.mouve_time = 0;
                memory[num_fg].Store.old_value = 0;
                Serial.print("store stop = ");
                Serial.println(memory[num_fg].Stats);
                // printf("DIM_STORE_STOP\n");
                for (uint8_t out = 0; out < MAX_OUT; ++out)
                {
                    if (config.FG[num_fg].out[out].device_address != 0)
                    {
                        dimmer_set(config.FG[num_fg].out[out].device_address - 9, 0);
                    }
                }
            }
        }
    }
}

uint8_t FG_Meta_Switch_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg)
{
    Serial.print("FG_Meta_Switch_BTN");
    for (uint8_t i = 0; i < MAX_IN; ++i)
    {
        if (config.FG[num_fg].in[i].device_address == device_address)
        {
            if (bt_action == IN_PUSHED) // short push
            {
                if (memory[num_fg].Stats == 0)
                {
                    Serial.println(config.FG[num_fg].old_value);
                    return config.FG[num_fg].old_value;
                }
                else
                {
                    Serial.println(0);
                    return 0;
                }
            }
            else if (bt_action == IN_RELEASE) // short push
            {
                if (memory[num_fg].Stats == 0)
                {
                    Serial.println(config.FG[num_fg].old_value);
                    return config.FG[num_fg].old_value;
                }
                else
                {
                    Serial.println(0);
                    return 0;
                }
            }
        }
    }
    return 255;
}

void FG_Func_SWITCH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg) //TODO: FG_Func_SWITCH_BTN: Test me
{
    uint8_t ret = FG_Meta_Switch_BTN(device_address, bt_action, num_fg);
    CanData data;
    data.u8[0] = memory[num_fg].Stats;
    if (ret == 0)
    {
        memory[num_fg].Stats = ret;
        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_OFF, data); //TODO: Can
        apply_FG_out(num_fg, memory[num_fg].Stats);
    }
    else if (ret <= 100)
    {
        memory[num_fg].Stats = ret;
        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_ON, data); //TODO: Can
        apply_FG_out(num_fg, memory[num_fg].Stats);
    }
}

void FG_Func_DIM_SWITCH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg) //TODO: FG_Func_DIM_SWITCH_BTN: Test me
{
    uint8_t ret = FG_Meta_Switch_BTN(device_address, bt_action, num_fg);
    CanData data;
    data.u8[0] = memory[num_fg].Stats;
    if (ret == 0)
    {
        memory[num_fg].Stats = ret;
        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_OFF, data); //TODO: Can
        slow_apply_FG_out(num_fg, memory[num_fg].Stats);
    }
    else if (ret <= 100)
    {
        memory[num_fg].Stats = ret;
        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_ON, data); //TODO: Can
        slow_apply_FG_out(num_fg, memory[num_fg].Stats);
    }
}

void FG_Func_PUSH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg, uint8_t num_in) //TODO: FG_Func_PUSH_BTN: Test me
{
    CanData data;
    // printf("FG_Func_PUSH_BTN(%i,%i,%i,%i)\n", device_address, bt_action, num_fg, num_in);
    if (config.FG[num_fg].in[num_in].bt_action == bt_action) // short push
    {
        // printf("bt_action %i", bt_action);
        // if (config.FG[num_fg].in[num_in].type == FG_BTN_UP)
        //     Serial.println("FG_BTN_UP");
        // else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN)
        //     Serial.println("FG_BTN_DOWN");

        if (memory[num_fg].Stats == 0) //si dimmer off
        {
            memory[num_fg].Stats = config.FG[num_fg].old_value < DIMMER_MIN ? DIMMER_MIN : config.FG[num_fg].old_value;
            data.u8[0] = memory[num_fg].Stats;
            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_ON, data); //TODO: Can
            apply_FG_out(num_fg, memory[num_fg].Stats);
            return;
        }
        else //si dimmer on
        {
            memory[num_fg].Stats = 0;
            data.u8[0] = memory[num_fg].Stats;
            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_OFF, data); //TODO: Can
            apply_FG_out(num_fg, memory[num_fg].Stats);
            return;
        }
    }
}

void FG_Func_DIM_PUSH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg, uint8_t num_in) //TODO: FG_Func_DIM_PUSH_BTN: Test me
{

    CanData data;
    // printf("FG_Func_DIM_PUSH_BTN(%i,%i,%i,%i)\n", device_address, bt_action, num_fg, num_in);
    if (config.FG[num_fg].in[num_in].bt_action == bt_action) // short push
    {
        // printf("short push\n");

        if (memory[num_fg].Stats == 0) //si dimmer off
        {
            memory[num_fg].Stats = config.FG[num_fg].old_value < DIMMER_MIN ? DIMMER_MIN : config.FG[num_fg].old_value;

            data.u8[0] = memory[num_fg].Stats;
            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_ON, data); //TODO: Can
            slow_apply_FG_out(num_fg, memory[num_fg].Stats);                          //Turn ON
            // return;
        }
        else //si dimmer on
        {

            config.FG[num_fg].old_value = memory[num_fg].Stats < DIMMER_MIN ? DIMMER_MIN : memory[num_fg].Stats; //copy value in EEPROM struct
            memory[num_fg].Stats = 0;
            data.u8[0] = memory[num_fg].Stats;
            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_OFF, data); //TODO: Can
            slow_apply_FG_out(num_fg, memory[num_fg].Stats);                           //Turn OFF
            // return;
        }
    }
    else if (config.FG[num_fg].in[num_in].bt_action + 1 == bt_action) // long push
    {
        // printf("long push ");
        // Serial.println("FG_BTN_UP (long)");
        if (memory[num_fg].Stats <= DIMMER_MIN) //si dimmer inferieur a DIMMER_MIN
        {
            memory[num_fg].Action = DIM_INC; //incrementé lumiere;

            // printf("0 > DIM_INC \n");
            // return;
        }
        else if (memory[num_fg].Stats == 100) //si dimmer deja a 100%
        {
            memory[num_fg].Action = DIM_DEC; //décremente lumiere;
            // printf("100 >DIM_DEC \n");
            // return;
        }
        else if (memory[num_fg].Action == DIM_NULL_OLD_DEC) //si l'ancienne action etais une décrémentation alors
        {
            memory[num_fg].Action = DIM_INC; //incrementé lumiere;
            // printf("DIM_NULL_OLD_DEC > DIM_INC \n");
            // return;
        }
        else if (memory[num_fg].Action == DIM_NULL_OLD_INC) //si l'ancienne action etais une incrémentation alors
        {
            memory[num_fg].Action = DIM_DEC; //incrementé lumiere;
            // printf("DIM_NULL_OLD_INC > DIM_DEC \n");
            // return;
        }
        else // ne deverait jamais arriver (Action est initialisé a DIM_NULL mais Stats est initialisé a 0)
        {
            memory[num_fg].Action = DIM_INC; //incrementé lumiere;
            // printf("else > DIM_INC \n");
        }
    }
    else if (bt_action == IN_RELEASE && memory[num_fg].Action == DIM_INC)
    {
        // printf("Release\n");
        // Serial.print("RELEASE num_fg = ");
        // Serial.println(num_fg);

        config.FG[num_fg].old_value = memory[num_fg].Stats < DIMMER_MIN ? DIMMER_MIN : memory[num_fg].Stats; //copy value in EEPROM struct
        memory[num_fg].Action = DIM_NULL_OLD_INC;
        data.u8[0] = memory[num_fg].Stats;
        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_VALUE, data); //TODO: Can
        apply_FG_out(num_fg, memory[num_fg].Stats);
        // slow_apply_FG_out(num_fg);
        // return;
    }
    else if (bt_action == IN_RELEASE && memory[num_fg].Action == DIM_DEC)
    {
        // printf("Release > DIM_NULL_OLD_DEC\n");
        // Serial.print("RELEASE num_fg = ");
        // Serial.println(num_fg);

        config.FG[num_fg].old_value = memory[num_fg].Stats < DIMMER_MIN ? DIMMER_MIN : memory[num_fg].Stats; //copy value in EEPROM struct
        memory[num_fg].Action = DIM_NULL_OLD_DEC;
        data.u8[0] = memory[num_fg].Stats;
        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_VALUE, data); //TODO: Can
        apply_FG_out(num_fg, memory[num_fg].Stats);
        // slow_apply_FG_out(num_fg);
        // return;
    }
}

void FG_Func_DIM_CANDLE_PUSH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg) //TODO: FG_Func_DIM_CANDLE_PUSH_BTN: Write me
{
}

void FG_Func_DIM_2_PUSH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg, uint8_t num_in)
{
    CanData data;
    if (bt_action != IN_RELEASE && millis() - memory[num_fg].D2PB.interlock_time < FG_DEBOUNCE_TIME) //debounce (antirebond)
    {
        // Serial.println(F("FG_DEBOUNCE_TIME"));
        return;
    }

    if (bt_action == IN_RELEASE && memory[num_fg].D2PB.interlock == device_address)
    {
        // Serial.print("RELEASE num_fg = ");
        // Serial.println(num_fg);
        memory[num_fg].Action = DIM_NULL;

        //interlock
        memory[num_fg].D2PB.interlock = 0;
        memory[num_fg].D2PB.interlock_time = millis();
        data.u8[0] = memory[num_fg].Stats;
        config.FG[num_fg].old_value = memory[num_fg].Stats < DIMMER_MIN ? DIMMER_MIN : memory[num_fg].Stats; //copy value in EEPROM struct
        DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_VALUE, data);                         //TODO: Can
        apply_FG_out(num_fg, memory[num_fg].Stats);
        // slow_apply_FG_out(num_fg);
        return;
    }

    if (memory[num_fg].D2PB.interlock != 0 && millis() - memory[num_fg].D2PB.interlock_time > FG_INTERLOCK_MAX_TIME) //si l'interlock est actif plus de X ms on le deverrouille
    {
        //memory[num_fg].D2PB.interlock_time = millis();
        // Serial.println(F("FG_INTERLOCK_MAX_TIME"));
        memory[num_fg].D2PB.interlock = 0;
    }

    if (memory[num_fg].D2PB.interlock != 0 && memory[num_fg].D2PB.interlock != device_address) //si l'interlock est actif traite uniquement les info du bouton verouille
    {
        Serial.println("Interlock");
        return;
    }

    if (config.FG[num_fg].in[num_in].bt_action == bt_action) // short push
    {
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP || config.FG[num_fg].in[num_in].type == FG_BTN_DOWN) //short push on button up or down
        {
            //interlock
            memory[num_fg].D2PB.interlock_time = millis();

            // if (config.FG[num_fg].in[num_in].type == FG_BTN_UP)
            //     Serial.println("FG_BTN_UP");
            // else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN)
            //     Serial.println("FG_BTN_DOWN");

            if (memory[num_fg].Stats == 0) //si dimmer off
            {
                memory[num_fg].Stats = config.FG[num_fg].old_value < DIMMER_MIN ? DIMMER_MIN : config.FG[num_fg].old_value;
                data.u8[0] = memory[num_fg].Stats;
                DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_ON, data); //TODO: Can
                slow_apply_FG_out(num_fg, memory[num_fg].Stats);                          //Turn ON
                return;
            }
            else //si dimmer on
            {

                config.FG[num_fg].old_value = memory[num_fg].Stats < DIMMER_MIN ? DIMMER_MIN : memory[num_fg].Stats; //copy value in EEPROM struct
                memory[num_fg].Stats = 0;
                data.u8[0] = memory[num_fg].Stats;
                DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_DIM_OFF, data); //TODO: Can
                slow_apply_FG_out(num_fg, memory[num_fg].Stats);                           //Turn OFF
                return;
            }
        }
    }
    else if (config.FG[num_fg].in[num_in].bt_action + 1 == bt_action) // long push
    {

        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP) //long push on button up
        {
            //interlock
            memory[num_fg].D2PB.interlock = device_address;
            memory[num_fg].D2PB.interlock_time = millis();

            // Serial.println("FG_BTN_UP (long)");
            if (memory[num_fg].Stats < 100) //si dimmer inferieur a 100%
            {
                memory[num_fg].Action = DIM_INC; //incrementé lumiere;
                return;
            }
            else //si dimmer deja a 100%
            {
                return;
            }
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN) //long push on button down
        {
            //interlock
            memory[num_fg].D2PB.interlock = device_address;
            memory[num_fg].D2PB.interlock_time = millis();

            // Serial.println("FG_BTN_DOWN (long)");
            if (memory[num_fg].Stats > DIMMER_MIN) //si dimmer superieur a 0%
            {
                memory[num_fg].Action = DIM_DEC; //décrémenté la lumiere;
                return;
            }
            else //si dimmer deja a 0%
            {
                return;
            }
        }
    }
}

void FG_Func_DIM_CANDLE_2_PUSH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg) //TODO: FG_Func_DIM_CANDLE_2_PUSH_BTN: Write me
{
}

void FG_Func_STORE_2_PUSH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg, uint8_t num_in) //TODO: FG_Func_STORE_2_PUSH_BTN: not fully implemented
{
    // printf("FG_Func_STORE num_fg=%d\n", num_fg);
    // printf("FG_Func_STORE\n");
    if (bt_action == IN_PUSHED && memory[num_fg].Store.interlock == 0) //set interlock
    {
        //   printf("set interlock\n");
        memory[num_fg].Store.interlock = device_address;
        // memory[num_fg].Store.interlock_time = millis();
    }
    else if (bt_action == IN_RELEASE &&
             (memory[num_fg].Action == DIM_STORE_TIME_CONFIG_UP || memory[num_fg].Action == DIM_STORE_TIME_CONFIG_DOWN) &&
             memory[num_fg].Store.interlock == device_address) //save up ou down time and reset interlock
    {
        // printf("DIM_STORE_TIME_CONFIG_UP ou DIM_STORE_TIME_CONFIG_DOWN\n");
        memory[num_fg].Store.interlock = 0;     //on relache l interlock
        memory[num_fg].Action = DIM_STORE_STOP; //on demande l'arret du store
        config.FG[num_fg].store_time = millis() - memory[num_fg].Store.mouve_time;
        memory[num_fg].Store.store_ratio = config.FG[num_fg].store_time / 100;
        sauvegardeEEPROM(); //on sauvegarde la configuration dans l'EEPROM
    }
    else if (bt_action == IN_RELEASE && memory[num_fg].Store.interlock == device_address) //reset interlock
    {
        //   printf("reset interlock\n");
        memory[num_fg].Store.interlock = 0;
    }
    else if (bt_action == IN_PUSHED && memory[num_fg].Store.interlock != 0 && memory[num_fg].Store.interlock != device_address) //push 2 button a same time
    {
        //   printf("push 2 button a same time\n");
        for (uint8_t i = 0; i < MAX_IN; ++i)
        {
            if (config.FG[num_fg].in[i].device_address == memory[num_fg].Store.interlock)
            {
                // printf("push 2 button a same time i device == interlock\n");
                if ((config.FG[num_fg].in[i].type == FG_BTN_UP && config.FG[num_fg].in[num_in].type == FG_BTN_DOWN) || (config.FG[num_fg].in[i].type == FG_BTN_DOWN && config.FG[num_fg].in[num_in].type == FG_BTN_UP))
                {
                    // printf("push 2 button a same time = true\n");
                    memory[num_fg].Store.push2 = IN_PUSHED;
                    return;
                }
            }
        }
    }
    else if (memory[num_fg].Store.push2 == bt_action) //reset push2
    {
        //   printf("reset push2 button a same time stage 2\n");
        memory[num_fg].Store.push2 = IN_NULL;
    }
    else if (memory[num_fg].Store.interlock != 0 && memory[num_fg].Store.interlock != device_address) // if not a locked device address
    {
        //   printf("interlock");
        return;
    }

    else if (config.FG[num_fg].in[num_in].bt_action == bt_action) // short push
    {
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == IN_NULL) //short push on button up
        {
            // printf("FG_Func_STORE FG_BTN_UP\n");
            //   printf("btn push UP\n");
            memory[num_fg].Action = DIM_STORE_UP;
            memory[num_fg].Store.mouve_time = 0;
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == IN_NULL) //short push on button down
        {
            // printf("FG_Func_STORE FG_BTN_DOWN\n");
            //   printf("btn push down\n");
            memory[num_fg].Action = DIM_STORE_DOWN;
            memory[num_fg].Store.mouve_time = 0;
        }
        else if (memory[num_fg].Store.push2 == IN_PUSHED) //short push on 2 buttons (STOP or memory position)
        {
            // printf("FG_Func_STORE push 2 btn\n");
            //   printf("btn 2 push btn\n");
            if (memory[num_fg].Action == DIM_STORE_DOWN || memory[num_fg].Action == DIM_STORE_UP) //if store mouve up or down
            {
                // printf("btn 2 push btn DIM_STORE_STOP\n");
                memory[num_fg].Action = DIM_STORE_STOP;
            }
            else if (memory[num_fg].Action == DIM_NULL) //if store is stoped
            {
                // printf("btn 2 push btn DIM_STORE_MEMORY_POSITION memory = %d old_value= %d\n", memory[num_fg].Stats, config.FG[num_fg].old_value);
                if (memory[num_fg].Stats < config.FG[num_fg].old_value) //go memory up
                {
                    memory[num_fg].Action = DIM_STORE_MEMORY_POSITION_UP;
                    // printf("btn 2 push btn DIM_STORE_MEMORY_POSITION_UP\n");
                }
                else if (memory[num_fg].Stats > config.FG[num_fg].old_value) //Go memory down
                {
                    memory[num_fg].Action = DIM_STORE_MEMORY_POSITION_DOWN;
                    // printf("btn 2 push btn DIM_STORE_MEMORY_POSITION_DOWN\n");
                }
            }
            memory[num_fg].Store.push2 = bt_action;
        }
    }
    /*
    else if (config.FG[num_fg].in[num_in].bt_action +1 == bt_action ) // long push
    {
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == false) //long push on button up
        {
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == false) //long push on button down
        {
        }
        else if (memory[num_fg].Store.push2) //long push on 2 buttons (Memory)
        {
        }
    }*/
    else if (config.FG[num_fg].in[num_in].bt_action + 2 == bt_action) // long long push
    {
        //   printf("long_long_push\n");
        /*if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == false) //long push on button up
        {
            // printf("FG_Func_STORE long FG_BTN_UP\n");
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == false) //long push on button down
        {
            // printf("FG_Func_STORE long FG_BTN_DOWN\n");
        }
        else */
        if (memory[num_fg].Store.push2 != IN_NULL) //long push on 2 buttons (Memory position)
        {
            // printf("FG_Func_STORE long push 2 btn\n");
            if (memory[num_fg].Action == DIM_NULL)
            {
                config.FG[num_fg].old_value = memory[num_fg].Stats;
                if (memory[num_fg].Stats > 50)
                {
                    memory[num_fg].Action = DIM_STORE_VALIDE_SAVE_DOWN;
                }
                else
                {
                    memory[num_fg].Action = DIM_STORE_VALIDE_SAVE_UP;
                }
                //   printf("FG_Func_STORE save position %i in eeprom\n", config.FG[num_fg].old_value);
            }
            memory[num_fg].Store.push2 = bt_action;
        }
    }
    else if (config.FG[num_fg].in[num_in].bt_action + 3 == bt_action) // XL long push
    {
        //   printf("xl_long_push\n");
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == IN_NULL) //XL long push on button up
        {
            // printf("DIM_STORE_TIME_CONFIG_UP\n");
            //TODO: FG_Func_STORE_2_PUSH_BTN: enregistrement du temps de montée
            memory[num_fg].Store.mouve_time = 0;
            memory[num_fg].Action = DIM_STORE_TIME_CONFIG_UP;
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == IN_NULL) //XL long push on button down
        {
            // printf("DIM_STORE_TIME_CONFIG_DOWN\n");
            //TODO: FG_Func_STORE_2_PUSH_BTN: enregistrement du temps de déssente
            memory[num_fg].Store.mouve_time = 0;
            memory[num_fg].Action = DIM_STORE_TIME_CONFIG_DOWN;
        }
    }
}

/*
void FG_Func_STORE_2_PUSH_BTN(uint8_t device_address, Bt_Action bt_action, uint8_t num_fg, uint8_t num_in) //TODO: FG_Func_STORE_2_PUSH_BTN: not fully implemented
{
    CanData data;
    data.u16[0] = 0;
    data.u16[1] = 0;
    data.u16[2] = 0;
    // printf("FG_Func_STORE num_fg=%d\n", num_fg);
    // printf("FG_Func_STORE\n");
    if (bt_action == IN_PUSHED && memory[num_fg].Store.interlock == 0) //set interlock
    {
        //   printf("set interlock\n");
        memory[num_fg].Store.interlock = device_address;
        // memory[num_fg].Store.interlock_time = millis();
    }
    else if (bt_action == IN_RELEASE &&
             (memory[num_fg].Action == DIM_STORE_TIME_CONFIG_UP || memory[num_fg].Action == DIM_STORE_TIME_CONFIG_DOWN) &&
             memory[num_fg].Store.interlock == device_address) //save up ou down time and reset interlock
    {
        // printf("DIM_STORE_TIME_CONFIG_UP ou DIM_STORE_TIME_CONFIG_DOWN\n");
        memory[num_fg].Store.interlock = 0;     //on relache l interlock
        memory[num_fg].Action = DIM_STORE_STOP; //on demande l'arret du store
        config.FG[num_fg].store_time = millis() - memory[num_fg].Store.mouve_time;
        sauvegardeEEPROM(); //on sauvegarde la configuration dans l'EEPROM
    }
    else if (bt_action == IN_RELEASE && memory[num_fg].Store.interlock == device_address) //reset interlock
    {
        //   printf("reset interlock\n");
        memory[num_fg].Store.interlock = 0;
    }
    else if (bt_action == IN_PUSHED && memory[num_fg].Store.interlock != 0 && memory[num_fg].Store.interlock != device_address) //push 2 button a same time
    {
        //   printf("push 2 button a same time\n");
        for (uint8_t i = 0; i < MAX_IN; ++i)
        {
            if (config.FG[num_fg].in[i].device_address == memory[num_fg].Store.interlock)
            {
                // printf("push 2 button a same time i device == interlock\n");
                if ((config.FG[num_fg].in[i].type == FG_BTN_UP && config.FG[num_fg].in[num_in].type == FG_BTN_DOWN) || (config.FG[num_fg].in[i].type == FG_BTN_DOWN && config.FG[num_fg].in[num_in].type == FG_BTN_UP))
                {
                    // printf("push 2 button a same time = true\n");
                    memory[num_fg].Store.push2 = IN_PUSHED;
                    return;
                }
            }
        }
    }
    else if (memory[num_fg].Store.push2 == bt_action) //reset push2
    {
        //   printf("reset push2 button a same time stage 2\n");
        memory[num_fg].Store.push2 = IN_NULL;
    }
    else if (memory[num_fg].Store.interlock != 0 && memory[num_fg].Store.interlock != device_address) // if not a locked device address
    {
        //   printf("interlock");
        return;
    }

    else if (config.FG[num_fg].in[num_in].bt_action == bt_action) // short push
    {
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == IN_NULL) //short push on button up
        {
            // printf("FG_Func_STORE FG_BTN_UP\n");
            // printf("btn push UP\n");
            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_STORE_UP, data); //TODO: Can ok
            memory[num_fg].Action = DIM_STORE_UP;
            memory[num_fg].Store.mouve_time = 0;
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == IN_NULL) //short push on button down
        {
            // printf("FG_Func_STORE FG_BTN_DOWN\n");
            // printf("btn push down\n");
            DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_STORE_DOWN, data); //TODO: Can ok
            memory[num_fg].Action = DIM_STORE_DOWN;
            memory[num_fg].Store.mouve_time = 0;
        }
        else if (memory[num_fg].Store.push2 == IN_PUSHED) //short push on 2 buttons (STOP or memory position)
        {
            // printf("FG_Func_STORE push 2 btn\n");
            //   printf("btn 2 push btn\n");
            if (memory[num_fg].Action == DIM_STORE_DOWN || memory[num_fg].Action == DIM_STORE_UP) //if store mouve up or down
            {
                // printf("btn 2 push btn DIM_STORE_STOP\n");
                DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_STORE_STOP, data); //TODO: Can ok
                memory[num_fg].Action = DIM_STORE_STOP;
            }
            else if (memory[num_fg].Action == DIM_NULL) //if store is stoped
            {
                // printf("btn 2 push btn DIM_STORE_MEMORY_POSITION memory = %d old_value= %d\n", memory[num_fg].Stats, config.FG[num_fg].old_value);
                if (memory[num_fg].Stats < config.FG[num_fg].old_value) //go memory up
                {
                    DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_STORE_MEMORY, data); //TODO: Can ok
                    memory[num_fg].Action = DIM_STORE_MEMORY_POSITION_UP;
                    // printf("btn 2 push btn DIM_STORE_MEMORY_POSITION_UP\n");
                }
                else if (memory[num_fg].Stats > config.FG[num_fg].old_value) //Go memory down
                {
                    DtCan_FG_send(config.FG[num_fg].address, 0, CAN_FG, CAN_FG_STORE_MEMORY, data); //TODO: Can ok
                    memory[num_fg].Action = DIM_STORE_MEMORY_POSITION_DOWN;
                    // printf("btn 2 push btn DIM_STORE_MEMORY_POSITION_DOWN\n");
                }
            }
            memory[num_fg].Store.push2 = bt_action;
        }
    }
    else if (config.FG[num_fg].in[num_in].bt_action + 1 == bt_action) // long push
    {
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == false) //long push on button up
        {
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == false) //long push on button down
        {
        }
        else if (memory[num_fg].Store.push2) //long push on 2 buttons (Memory)
        {
        }
    }
    else if (config.FG[num_fg].in[num_in].bt_action + 2 == bt_action) // long long push
    {
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == false) //long push on button up
        {
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == false) //long push on button down
        {
        }
        else if (memory[num_fg].Store.push2) //long push on 2 buttons (Memory)
        {
        }
    }
    else if (config.FG[num_fg].in[num_in].bt_action + 3 == bt_action) // XL long push
    {
        if (config.FG[num_fg].in[num_in].type == FG_BTN_UP && memory[num_fg].Store.push2 == false) //long push on button up
        {
        }
        else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN && memory[num_fg].Store.push2 == false) //long push on button down
        {
        }
        else if (memory[num_fg].Store.push2) //long push on 2 buttons (Memory)
        {
        }
    }
}
*/
//recherche de l'acction affectée au bouton
//
//button: numero du bouton
//bt_code: code action du bouton
void FG_Button(uint8_t button, Bt_Action bt_action)
{

    for (uint8_t fg = 0; fg < MAX_FG; fg++) //boucle sur les groupes de fonction
    {
        if (config.FG[fg].address == 0)
        {
            break;
        }

        for (uint8_t in = 0; in < MAX_IN; in++) //boucle sur les boutons du groupe de fonction
        {
            // if (config.FG[fg].in[in].device_address == 0)
            // {
            //     break;
            // }

            if (sub_address == config.FG[fg].in[in].device_address) //si le bouton est associer au groupe de fonction
            {

                //on execute l'action liée au bouton sur toute les sortie
                if (config.FG[fg].type == FG_DIM_2_PUSH_BTN)
                    FG_Func_DIM_2_PUSH_BTN(sub_address, bt_action, fg, in);

                else if (config.FG[fg].type == FG_SWITCH_BTN)
                    FG_Func_SWITCH_BTN(sub_address, bt_action, fg);

                else if (config.FG[fg].type == FG_DIM_SWITCH_BTN)
                    FG_Func_DIM_SWITCH_BTN(sub_address, bt_action, fg);

                else if (config.FG[fg].type == FG_STORE_2_PUSH_BTN)
                    FG_Func_STORE_2_PUSH_BTN(sub_address, bt_action, fg, in);
                else if (config.FG[fg].type == FG_PUSH_BTN)
                    FG_Func_PUSH_BTN(sub_address, bt_action, fg, in);
                else if (config.FG[fg].type == FG_DIM_PUSH_BTN)
                    FG_Func_DIM_PUSH_BTN(sub_address, bt_action, fg, in);
            }
        }
    }
    Serial.print("Dt-Button sub_address = ");
    Serial.print(sub_address);
    Serial.print(" bt_action = ");
    Serial.println(bt_action);
}

void DtCan_FG_receve(uint8_t num_fg, uint16_t address, uint8_t sub_address, DTCanDataCode code, DTCanSubDataCode subcode, CanData data) //FIXME: FG_Can: set config.FG[num_fg].old_value
                                                                                                                                        //TODO: Implement Me
{
    CanData send_data;
    send_data.u16[0] = 0;
    send_data.u16[1] = 0;
    send_data.u16[2] = 0;
    switch (code)
    {
    case CAN_FG:
        switch (subcode)
        {
        case CAN_FG_DIM_VALUE:
            if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
            {
                memory[num_fg].Stats = data.u8[0];
                config.FG[num_fg].old_value = data.u8[0];
                apply_FG_out(num_fg, memory[num_fg].Stats);
            }
            break;
        case CAN_FG_DIM_MEMORY:
            if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
            {
                memory[num_fg].Stats = data.u8[0];
                config.FG[num_fg].old_value = data.u8[0];
                apply_FG_out(num_fg, memory[num_fg].Stats);
            }
            break;
        case CAN_FG_DIM_ON:
            if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
            {
                memory[num_fg].Stats = config.FG[num_fg].old_value;
                apply_FG_out(num_fg, memory[num_fg].Stats);
            }
            break;
        case CAN_FG_DIM_OFF:
            if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
            {
                memory[num_fg].Stats = 0;
                apply_FG_out(num_fg, memory[num_fg].Stats);
            }
            break;
        case CAN_FG_DIM_INC:
            if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
            {
                memory[num_fg].Stats += 1;
                apply_FG_out(num_fg, memory[num_fg].Stats);
            }
            break;
        case CAN_FG_DIM_DEC:
            if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
            {
                memory[num_fg].Stats -= 1;
                apply_FG_out(num_fg, memory[num_fg].Stats);
            }
            break;
        case CAN_FG_STORE_POS:
            if (config.FG[num_fg].type == FG_STORE_2_PUSH_BTN) //if store
            {
                memory[num_fg].Store.go_pos = data.u8[0];
                if (memory[num_fg].Store.go_pos > memory[num_fg].Stats)
                    memory[num_fg].Action = DIM_STORE_GO_POSITION_UP;
                else
                    memory[num_fg].Action = DIM_STORE_GO_POSITION_DOWN;
            }
            break;
        case CAN_FG_STORE_MEMORY:
            if (config.FG[num_fg].type == FG_STORE_2_PUSH_BTN) //if store
            {
                if (config.FG[num_fg].old_value > memory[num_fg].Stats)
                    memory[num_fg].Action = DIM_STORE_MEMORY_POSITION_UP;
                {
                    memory[num_fg].Action = DIM_STORE_UP;
                    memory[num_fg].Action = DIM_STORE_UP;
                }
                break;
            case CAN_FG_STORE_DOWN:
                if (config.FG[num_fg].type == FG_STORE_2_PUSH_BTN) //if store
                {
                    memory[num_fg].Action = DIM_STORE_DOWN;
                }
                break;
            default:
                break;
            }
            break;
        }
    case CAN_FG_REQ:
        switch (subcode)
        {
        case CAN_FG_DIM_VALUE:
        case CAN_FG_STORE_POS:
            send_data.u8[0] = memory[num_fg].Stats;
            DtCan_FG_send(address, sub_address, CAN_FG_RESP, subcode, send_data);
            break;

        case CAN_FG_DIM_MEMORY:
        case CAN_FG_STORE_MEMORY:
            send_data.u8[0] = config.FG[num_fg].old_value;
            DtCan_FG_send(address, sub_address, CAN_FG_RESP, subcode, send_data);
            break;

        case CAN_FG_DIM_ON:
        case CAN_FG_DIM_OFF:
            if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
            {
                if (memory[num_fg].Stats > 0)
                    DtCan_FG_send(address, sub_address, CAN_FG_RESP, CAN_FG_DIM_OFF, send_data);
                else
                    DtCan_FG_send(address, sub_address, CAN_FG_RESP, CAN_FG_DIM_ON, send_data);
            }
            break;

        case CAN_FG_STORE_STOP:
        case CAN_FG_STORE_UP:
        case CAN_FG_STORE_DOWN:
            if (config.FG[num_fg].type == FG_STORE_2_PUSH_BTN) //if light
            {
                if (memory[num_fg].Action == DIM_STORE_DOWN)
                    DtCan_FG_send(address, sub_address, CAN_FG_RESP, CAN_FG_STORE_DOWN, send_data);
                else if (memory[num_fg].Action == DIM_STORE_UP)
                    DtCan_FG_send(address, sub_address, CAN_FG_RESP, CAN_FG_STORE_UP, send_data);
                else
                    DtCan_FG_send(address, sub_address, CAN_FG_RESP, CAN_FG_STORE_STOP, send_data);
            }
            break;

        case CAN_FG_DIM_INC:
        case CAN_FG_DIM_DEC:
        default:
            break;
        }
        break;

    case CAN_FG_RESP:
        switch (subcode)
        {
        case CAN_FG_DIM_VALUE:
        case CAN_FG_DIM_ON:
        case CAN_FG_DIM_OFF:
        case CAN_FG_DIM_INC:
        case CAN_FG_DIM_DEC:
        case CAN_FG_STORE_POS:
        case CAN_FG_STORE_MEMORY:
        case CAN_FG_STORE_STOP:
        case CAN_FG_STORE_UP:
        case CAN_FG_STORE_DOWN:
            break;
        default:
            break;
        }
        break;

    case CAN_FG_SET: //TODO: Implement Me
        switch (subcode)
        {
            switch (subcode)
            {
            case CAN_FG_DIM_VALUE:
                break;
            case CAN_FG_DIM_MEMORY:
                if (config.FG[num_fg].type != FG_STORE_2_PUSH_BTN) //if light
                {
                    config.FG[num_fg].old_value = data.u8[0];
                }
                break;
            case CAN_FG_DIM_ON:
            case CAN_FG_DIM_OFF:
            case CAN_FG_DIM_INC:
            case CAN_FG_DIM_DEC:
            case CAN_FG_STORE_POS:
                break;

            case CAN_FG_STORE_MEMORY:
                if (config.FG[num_fg].type == FG_STORE_2_PUSH_BTN) //if store
                {
                    config.FG[num_fg].old_value = data.u8[0];
                }
                break;

            case CAN_FG_STORE_STOP:
            case CAN_FG_STORE_UP:
            case CAN_FG_STORE_DOWN:
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}
