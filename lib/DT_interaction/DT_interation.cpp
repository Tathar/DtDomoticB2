#include <DT_interaction.h>
#include <DT_Dimmer.h>
#include <DT_relay.h>

CircularBuffer<dt_interaction_t, 5> interaction_buffer;

void DT_interation_init(void)
{
}

void DT_interation_loop(void)
{
}

void DT_interation_input_action(const uint8_t num, const Bt_Action action)
{
    if (num < INPUT_NUM)
    {
        dt_interaction_t interaction;
        memcpy_P(&interaction, interaction_input_1_push_config + num, sizeof(action));
        interaction.call(num, action);
    }
}

void dt_switch_rly_t::call(const uint8_t num, const Bt_Action action)
{
    switch (action)
    {
    case Bt_Action::IN_PUSHED:
    case Bt_Action::IN_RELEASE:
        if (DT_relay_get(num) == true)
        {
            DT_relay(num, false);
        }
        else
        {
            DT_relay(num, true);
        }
        break;
    default:
        break;
    }
}

void dt_switch_dim_t::call(const uint8_t num, const Bt_Action action)
{
    switch (action)
    {
    case Bt_Action::IN_PUSHED:
    case Bt_Action::IN_RELEASE:
        if (get_dimmer(num) != 0)
        {
            dimmer_set(num, false, DIMMER_ON_OFF_SPEED);
        }
        else
        {
            dimmer_set(num, true, DIMMER_ON_OFF_SPEED);
        }
        break;
    default:
        break;
    }
}

void dt_button_push_rly_t::call(const uint8_t num, const Bt_Action action)
{

    switch (action)
    {
    case Bt_Action::IN_PUSH:
        if (DT_relay_get(num) == true)
        {
            DT_relay(num, false);
        }
        else
        {
            DT_relay(num, true);
        }
        break;
    default:
        break;
    }
}

void dt_button_push_dim_t::call(const uint8_t num, const Bt_Action action)
{
    switch (action)
    {
    case Bt_Action::IN_PUSH:
        if (get_dimmer(num) != 0)
        {
            dimmer_set(num, false, DIMMER_ON_OFF_SPEED);
        }
        else
        {
            dimmer_set(num, true, DIMMER_ON_OFF_SPEED);
        }
        break;

    case Bt_Action::IN_LPUSH:
        super->get();
        uint8_t actual_power = get_dimmer(num);
        if (actual_power == 255)
        {
            dimmer_set(num, (uint8_t)0, DIMMER_SETTING_SPEED);
            down = false;
        }
        else if (actual_power == 0)
        {
            dimmer_set(num, (uint8_t)255, DIMMER_SETTING_SPEED);
            down = true;
        }
        else if (down)
        {
            uint16_t time = SCALE(actual_power, 0, 255, 0, DIMMER_SETTING_SPEED);
            dimmer_set(num, (uint8_t)0, time);
            down = false;
        }
        else
        {
            uint16_t time = SCALE((255 - actual_power), 0, 255, 0, DIMMER_SETTING_SPEED);
            dimmer_set(num, (uint8_t)255, time);
            down = true;
        }
        super->store();
        break;

    case Bt_Action::IN_RELEASE:
        dimmer_stop(num);
        break;

    default:
        break;
    }
}