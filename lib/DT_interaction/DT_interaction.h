#pragma once
#include <config.h>
#include <DT_input.h>
#include <CircularBuffer.h>
#include <interaction_tools.h>

#define dt_interaction_void 255

// class dt_interaction_t;
struct dt_interaction_config;

extern CircularBuffer<dt_interaction_config, 10> interaction_buffer;

struct dt_interaction_memory_config
{
public:
    uint8_t btn_num;
    union
    {
        bool interlock = false;
        bool down;
    };

    bool run = false;
};

struct dt_interaction_config
{
    dt_interaction_memory_config memory_config;
    dt_interaction_eeprom_config eeprom_config;
};

class dt_interaction_t
{

public:
    inline dt_interaction_t(dt_interaction_eeprom_config config)
    {
        eeprom_config = config;
        // get();
    };

    inline bool is_linked(dt_interaction_t &remote)
    {
        if (eeprom_config.type == remote.eeprom_config.type && eeprom_config.act_num == remote.eeprom_config.act_num && memory_config.btn_num != remote.memory_config.btn_num)
            return true;
        else
            return false;
    }

    inline bool operator==(dt_interaction_t &remote)
    {
        if (eeprom_config.type == remote.eeprom_config.type && eeprom_config.act_num == remote.eeprom_config.act_num && memory_config.btn_num == remote.memory_config.btn_num)
            return true;
        else
            return false;
    }

    inline bool is_locked()
    {
        for (int i = 0; i < interaction_buffer.size(); ++i)
        {
            if (eeprom_config.type == interaction_buffer[i].eeprom_config.type && eeprom_config.act_num == interaction_buffer[i].eeprom_config.act_num && memory_config.btn_num != interaction_buffer[i].memory_config.btn_num && interaction_buffer[i].memory_config.interlock == true)
            {
                return true;
            }
        }
        return false;
    }

    inline void call(const uint8_t num, const Bt_Action action)
    {
        memory_config.btn_num = num;
        // super = this;

        switch (eeprom_config.type)
        {
        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_no_action:
            break;
        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_button_push_dim:
            button_push_dim(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_button_push_rly:
            button_push_rly(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_button_push_opt_rly:
            button_push_opt_rly(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_switch_dim:
            switch_dim(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_switch_rly:
            switch_rly(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_switch_opt_rly:
            switch_opt_rly(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_two_button_push_dim:
            two_button_push_dim(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_two_button_push_cover:
            two_button_push_cover(num, action);
            break;

        case dt_interaction_eeprom_config::dt_interaction_type_t::dt_two_button_push_portal:
            two_button_push_portal(num, action);
            break;
        }
    }

    inline void store()
    {

        dt_interaction_config tmp;

        // uint8_t next = 254;
        for (uint8_t i = 0; i < interaction_buffer.size(); ++i)
        {

            tmp = interaction_buffer.shift();

            if (tmp.eeprom_config == eeprom_config)
            {
                tmp.memory_config = memory_config;
                interaction_buffer.push(tmp);

                return;
            }
            interaction_buffer.push(tmp);
        }

        tmp.eeprom_config = eeprom_config;
        tmp.memory_config = memory_config;
        interaction_buffer.push(tmp);
    };

    inline bool restore()
    {
        for (int i = 0; i < interaction_buffer.size(); ++i)
        {
            if (interaction_buffer[i].eeprom_config == eeprom_config)
            {
                memory_config = interaction_buffer[i].memory_config;
                return true;
            }
        }
        return false;
    };

    inline void remove()
    {

        dt_interaction_config tmp;
        uint8_t size = interaction_buffer.size();

        for (uint8_t i = 0; i < size; ++i)
        {
            tmp = interaction_buffer.shift();

            if (tmp.eeprom_config == eeprom_config)
                return;

            interaction_buffer.push(tmp);
        }
    };

    inline void remove_all()
    {

        dt_interaction_config tmp;
        uint8_t size = interaction_buffer.size();
        for (uint8_t i = 0; i < size; ++i)
        {
            tmp = interaction_buffer.shift();

            if (tmp.eeprom_config.type == eeprom_config.type && tmp.eeprom_config.act_num == eeprom_config.act_num)
            {
                // Serial.println(F("Remove"));
            }
            else
            {
                interaction_buffer.push(tmp);
            }
        }
    };

    void button_push_dim(const uint8_t num, const Bt_Action action);
    void button_push_rly(const uint8_t num, const Bt_Action action);
    void button_push_opt_rly(const uint8_t num, const Bt_Action action);
    void switch_dim(const uint8_t num, const Bt_Action action);
    void switch_rly(const uint8_t num, const Bt_Action action);
    void switch_opt_rly(const uint8_t num, const Bt_Action action);
    void two_button_push_dim(const uint8_t num, const Bt_Action action);
    void two_button_push_cover(const uint8_t num, const Bt_Action action);
    void two_button_push_portal(const uint8_t num, const Bt_Action action);

    dt_interaction_memory_config memory_config;
    dt_interaction_eeprom_config eeprom_config;
};

void DT_interation_init();
void DT_interation_loop();

void DT_interation_input_action(const uint8_t num, const Bt_Action action);
