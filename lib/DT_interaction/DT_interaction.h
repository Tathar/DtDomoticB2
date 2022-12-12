#pragma once
#include <config.h>
#include <DT_input.h>
#include <CircularBuffer.h>

#define dt_interaction_void 255

class dt_interaction_t;

extern CircularBuffer<dt_interaction_t, 5> interaction_buffer;

class dt_interaction_base_t
{
public:
    uint8_t btn_num;
    uint8_t act_num;
    dt_interaction_t *super;
};

// bouton poussoire active dimmer
class dt_button_push_dim_t : public dt_interaction_base_t
{
public:
    dt_button_push_dim_t(const uint8_t dim)
    {
        act_num = dim;
    }

    void call(const uint8_t num, const Bt_Action action);

    bool down = false;
};

// bouton poussoire active relais
class dt_button_push_rly_t : public dt_interaction_base_t
{
public:
    dt_button_push_rly_t(uint8_t rly)
    {
        act_num = rly;
    }

    void call(const uint8_t num, const Bt_Action action);
};

// interupteur active dimmer
class dt_switch_dim_t : public dt_interaction_base_t
{
public:
    dt_switch_dim_t(uint8_t dim)
    {
        act_num = dim;
    }

    void call(const uint8_t num, const Bt_Action action);
};

// interupteur active relais
class dt_switch_rly_t : public dt_interaction_base_t
{
public:
    dt_switch_rly_t(uint8_t rly)
    {
        act_num = rly;
    }

    void call(const uint8_t num, const Bt_Action action);
};

// 2 bouton pour 1 dimmer (plus - moins)
class dt_two_button_push_dim_t : public dt_interaction_base_t
{
public:
    enum action_t
    {
        up,
        down,
    };

    dt_two_button_push_dim_t(uint8_t dim, action_t act)
    {
        action = act;
        act_num = dim;
        interlock = false;
    }

    void call(const uint8_t num, const Bt_Action action);

private:
    action_t action;
    uint8_t speed;
    bool interlock;
};

class dt_interaction_t
{
    enum dt_interaction_type_t
    {
        dt_no_action,
        dt_button_push_dim,
        dt_button_push_rly,
        dt_switch_dim,
        dt_switch_rly,
        dt_two_button_push_dim,
    };

public:
    inline dt_interaction_t()
    {
        type = dt_no_action;
    };

    inline dt_interaction_t(dt_button_push_dim_t data)
    {
        type = dt_button_push_dim;
        button_push_dim = data;
    };

    inline dt_interaction_t(dt_button_push_rly_t data)
    {
        type = dt_button_push_rly;
        button_push_rly = data;
    };

    inline dt_interaction_t(dt_two_button_push_dim_t data)
    {
        type = dt_two_button_push_dim;
        two_button_push_dim = data;
    };

    inline bool is_linked(dt_interaction_t &remote)
    {
        if (type == remote.type && no_action.act_num == remote.no_action.act_num && no_action.btn_num == remote.no_action.btn_num)
            return true;
        else
            return false;
    }

    inline bool operator==(dt_interaction_t &remote)
    {
        if (type == remote.type && no_action.act_num == remote.no_action.act_num && no_action.btn_num == remote.no_action.btn_num)
            return true;
        else
            return false;
    }

    inline void call(const uint8_t num, const Bt_Action action)
    {
        no_action.btn_num = num;
        no_action.super = this;

        switch (type)
        {
        case dt_no_action:
            break;
        case dt_button_push_dim:
            button_push_dim.call(num, action);
            break;

        case dt_button_push_rly:
            button_push_rly.call(num, action);
            break;

        case dt_switch_dim:
            switch_dim.call(num, action);
            break;

        case dt_switch_rly:
            switch_rly.call(num, action);
            break;

        case dt_two_button_push_dim:
            two_button_push_dim.call(num, action);
            break;
        }
    }

    inline void store()
    {

        dt_interaction_t tmp = *this;
        tmp.no_action.super = nullptr;

        for (uint8_t i = 0; i < interaction_buffer.size(); ++i)
        {
            if (interaction_buffer[i] == *this)
            {
                interaction_buffer[i] = tmp;
                return;
            }
        }

        interaction_buffer.push(tmp);
    };

    inline void get()
    {
        for (int i = 0; i < interaction_buffer.size(); ++i)
        {
            if (interaction_buffer[i] == *this)
            {
                *this = interaction_buffer[i];
                no_action.super = this;
                return;
            }
        }
    };

    dt_interaction_type_t type;
    union
    {
        dt_interaction_base_t no_action;
        dt_button_push_dim_t button_push_dim;
        dt_button_push_rly_t button_push_rly;
        dt_switch_dim_t switch_dim;
        dt_switch_rly_t switch_rly;
        dt_two_button_push_dim_t two_button_push_dim;
    };
};

const dt_interaction_t interaction_input_1_push_config[4] PROGMEM = {
    dt_button_push_dim_t(0),
    dt_two_button_push_dim_t(1, dt_two_button_push_dim_t::action_t::up),
    dt_two_button_push_dim_t(1, dt_two_button_push_dim_t::action_t::down),
    dt_button_push_rly_t(0)};

const dt_interaction_t interaction_input_2_push_config[3] PROGMEM = {
    dt_button_push_dim_t(0),
    dt_two_button_push_dim_t(1, dt_two_button_push_dim_t::action_t::up),
    dt_button_push_rly_t(0)};

void DT_interation_init();
void DT_interation_loop();

void DT_interation_input_action(const uint8_t num, const Bt_Action action);
