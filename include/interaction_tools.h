#pragma once

struct dt_interaction_eeprom_config
{
public:
    enum dt_interaction_type_t
    {
        dt_no_action,
        dt_button_push_dim,
        dt_button_push_rly,
        dt_button_push_opt_rly,
        dt_switch_dim,
        dt_switch_rly,
        dt_switch_opt_rly,
        dt_two_button_push_dim,
        dt_two_button_push_cover,
        dt_two_button_push_portal,
    };

    enum action_t
    {
        none,
        up,
        down,
    };

    dt_interaction_type_t type;
    uint8_t act_num;
    action_t act_type;

    inline void clean()
    {
        type = dt_interaction_type_t::dt_no_action;
        act_num = 0;
        act_type = action_t::none;
    }

    inline bool operator==(dt_interaction_eeprom_config &remote)
    {
        if (type == remote.type && act_num == remote.act_num && act_type == remote.act_type)
            return true;
        else
            return false;
    }
};

#define dt_no_action()                                                                                                     \
    {                                                                                                                      \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_no_action, 0, dt_interaction_eeprom_config::action_t::none \
    }

#define dt_button_push_rly(action_num)                                                                                                    \
    {                                                                                                                                     \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_button_push_rly, action_num, dt_interaction_eeprom_config::action_t::none \
    }

#define dt_button_push_opt_rly(action_num)                                                                                                    \
    {                                                                                                                                     \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_button_push_opt_rly, action_num, dt_interaction_eeprom_config::action_t::none \
    }

#define dt_button_push_dim(action_num)                                                                                                    \
    {                                                                                                                                     \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_button_push_dim, action_num, dt_interaction_eeprom_config::action_t::none \
    }

#define dt_switch_dim(action_num)                                                                                                    \
    {                                                                                                                                \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_switch_dim, action_num, dt_interaction_eeprom_config::action_t::none \
    }

#define dt_switch_rly(action_num)                                                                                                    \
    {                                                                                                                                \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_switch_rly, action_num, dt_interaction_eeprom_config::action_t::none \
    }

#define dt_switch_opt_rly(action_num)                                                                                                    \
    {                                                                                                                                \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_switch_opt_rly, action_num, dt_interaction_eeprom_config::action_t::none \
    }

#define dt_two_button_push_dim(action_num, action_type)                                                                                              \
    {                                                                                                                                                \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_two_button_push_dim, action_num, dt_interaction_eeprom_config::action_t::action_type \
    }

#define dt_two_button_push_cover(action_num, action_type)                                                                                              \
    {                                                                                                                                                  \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_two_button_push_cover, action_num, dt_interaction_eeprom_config::action_t::action_type \
    }

#define dt_two_button_push_portal(action_num, action_type)                                                                                              \
    {                                                                                                                                                  \
        dt_interaction_eeprom_config::dt_interaction_type_t::dt_two_button_push_portal, action_num, dt_interaction_eeprom_config::action_t::action_type \
    }
