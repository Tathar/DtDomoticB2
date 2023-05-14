#include <DT_interaction.h>
#include <DT_Dimmer.h>
#include <DT_relay.h>
#include <DT_opt_relay.h>
#include <DT_cover.h>
#include <DT_portal.h>

CircularBuffer<dt_interaction_config, 10> interaction_buffer;

void DT_interation_init(void)
{
}

void DT_interation_loop(void)
{
}

void DT_interation_input_action(const uint8_t num, const Bt_Action action)
{
    dt_interaction_eeprom_config eeprom_config;
    Bt_Action new_action = action;

#if PUSH_1_NUM > 0
    if (num < PUSH_1_NUM)
    {
        memcpy_P(&eeprom_config, interaction_input_1_push_config + num, sizeof(eeprom_config));
        if (eeprom_config.type != dt_interaction_eeprom_config::dt_interaction_type_t::dt_no_action)
        {
            dt_interaction_t interaction(eeprom_config);
            interaction.call(num, new_action);
        }
    }
#endif // PUSH_2_NUM > 0

#if PUSH_2_NUM > 0
    if (num < PUSH_2_NUM)
    {
        memcpy_P(&eeprom_config, interaction_input_2_push_config + num, sizeof(eeprom_config));
        if (eeprom_config.type != dt_interaction_eeprom_config::dt_interaction_type_t::dt_no_action)
        {
            dt_interaction_t interaction(eeprom_config);
            if (action < Bt_Action::IN_PUSH)
            {
                interaction.call(num, action);
            }
            else if (action >= Bt_Action::IN_2PUSH)
            {
                new_action = (Bt_Action)((int)new_action - 4);
                interaction.call(num, new_action);
            }
        }
    }
#endif // PUSH_2_NUM > 0

#if PUSH_3_NUM > 0
    if (num < PUSH_3_NUM)
    {
        memcpy_P(&eeprom_config, interaction_input_3_push_config + num, sizeof(eeprom_config));
        if (eeprom_config.type != dt_interaction_eeprom_config::dt_interaction_type_t::dt_no_action)
        {
            dt_interaction_t interaction(eeprom_config);
            if (action < Bt_Action::IN_PUSH)
            {
                interaction.call(num, action);
            }
            else if (action >= Bt_Action::IN_2PUSH)
            {
                new_action = (Bt_Action)((int)new_action - 4);
                interaction.call(num, new_action);
            }
        }
    }

#endif // PUSH_3_NUM > 0

#if PUSH_4_NUM > 0
    if (num < PUSH_4_NUM)
    {
        memcpy_P(&eeprom_config, interaction_input_4_push_config + num, sizeof(eeprom_config));
        if (eeprom_config.type != dt_interaction_eeprom_config::dt_interaction_type_t::dt_no_action)
        {
            dt_interaction_t interaction(eeprom_config);
            if (action < Bt_Action::IN_PUSH)
            {
                interaction.call(num, action);
            }
            else if (action >= Bt_Action::IN_2PUSH)
            {
                new_action = (Bt_Action)((int)new_action - 4);
                interaction.call(num, new_action);
            }
        }
    }
#endif // PUSH_4_NUM > 0
}

void dt_interaction_t::switch_rly(const uint8_t num, const Bt_Action action)
{
#if RELAY_NUM > 0
    switch (action)
    {
    case Bt_Action::IN_PUSHED:
    case Bt_Action::IN_RELEASE:
        if (DT_relay_get(eeprom_config.act_num) == true)
        {
            DT_relay(eeprom_config.act_num, false);
            debug(F(AT "_switch_rly_false"));
            Serial.println(eeprom_config.act_num);
        }
        else
        {
            DT_relay(eeprom_config.act_num, true);
            debug(F(AT "_switch_rly_true"));
            Serial.println(eeprom_config.act_num);
        }
        break;
    default:
        break;
    }
#endif // RELAY_NUM > 0
}

void dt_interaction_t::switch_opt_rly(const uint8_t num, const Bt_Action action)
{
#if OPT_RELAY_NUM > 0
    switch (action)
    {
    case Bt_Action::IN_PUSHED:
    case Bt_Action::IN_RELEASE:
        if (DT_opt_relay_get(num) == true)
        {
            DT_opt_relay(num, false);
            debug(F(AT "_switch_opt_rly_false"));
            Serial.println(eeprom_config.act_num);
        }
        else
        {
            DT_opt_relay(num, true);
            debug(F(AT "_switch_opt_rly_true"));
            Serial.println(num);
        }
        break;
    default:
        break;
    }
#endif // OPT_RELAY_NUM > 0
}

void dt_interaction_t::switch_dim(const uint8_t num, const Bt_Action action)
{
    switch (action)
    {
    case Bt_Action::IN_PUSHED:
    case Bt_Action::IN_RELEASE:
        if (get_dimmer(eeprom_config.act_num) != 0)
        {
            dimmer_set(eeprom_config.act_num, false, DIMMER_ON_OFF_SPEED);
        }
        else
        {
            dimmer_set(eeprom_config.act_num, true, DIMMER_ON_OFF_SPEED);
            debug(F(AT));
        }
        break;
    default:
        break;
    }
}

void dt_interaction_t::button_push_rly(const uint8_t num, const Bt_Action action)
{
    debug(F(AT));
#if RELAY_NUM > 0
    switch (action)
    {
    case Bt_Action::IN_PUSH:
        debug(F(AT));
        if (DT_relay_get(eeprom_config.act_num) == true)
        {
            DT_relay(eeprom_config.act_num, false);
            debug(F(AT "_button_push_rly_false"));
            Serial.println(eeprom_config.act_num);
        }
        else
        {
            DT_relay(eeprom_config.act_num, true);
            debug(F(AT "_button_push_rly_true"));
            Serial.println(eeprom_config.act_num);
        }
        break;
    default:
        break;
    }
#endif // RELAY_NUM > 0
}

void dt_interaction_t::button_push_opt_rly(const uint8_t num, const Bt_Action action)
{
    debug(F(AT));
#if OPT_RELAY_NUM > 0
    switch (action)
    {
    case Bt_Action::IN_PUSH:
        debug(F(AT));
        if (DT_opt_relay_get(num) == true)
        {
            DT_opt_relay(num, false);
            debug(F(AT "_button_push_opt_rly_false"));
            Serial.println(num);
        }
        else
        {
            DT_opt_relay(num, true);
            debug(F(AT "_button_push_opt_rly_true"));
            Serial.println(num);
        }
        break;
    default:
        break;
    }
#endif // OPT_RELAY_NUM > 0
}

void dt_interaction_t::button_push_dim(const uint8_t num, const Bt_Action action)
{
#if DIMMER_LIGHT_NUM > 0
    uint8_t actual_power = get_dimmer(eeprom_config.act_num);
    switch (action)
    {

    case Bt_Action::IN_RELEASE:
        dimmer_stop(eeprom_config.act_num);
        break;

    case Bt_Action::IN_PUSH:
        if (get_dimmer(eeprom_config.act_num) != 0)
        {
            dimmer_set(eeprom_config.act_num, false, DIMMER_ON_OFF_SPEED);
        }
        else
        {
            dimmer_set(eeprom_config.act_num, true, DIMMER_ON_OFF_SPEED);
        }
        break;

    case Bt_Action::IN_LPUSH:
        restore();

        if (actual_power == 255)
        {
            dimmer_set(eeprom_config.act_num, (uint8_t)0, DIMMER_SETTING_SPEED);
            memory_config.down = false;
        }
        else if (actual_power == 0)
        {
            dimmer_set(eeprom_config.act_num, (uint8_t)255, DIMMER_SETTING_SPEED);
            memory_config.down = true;
        }
        else if (memory_config.down)
        {
            uint16_t time = SCALE(actual_power, 0, 255, 0, DIMMER_SETTING_SPEED);
            dimmer_set(eeprom_config.act_num, (uint8_t)0, time);
            memory_config.down = false;
        }
        else
        {
            uint16_t time = SCALE((255 - actual_power), 0, 255, 0, DIMMER_SETTING_SPEED);
            dimmer_set(eeprom_config.act_num, (uint8_t)255, time);
            memory_config.down = true;
        }
        store();
        break;

    default:
        break;
    }

#endif // DIMMER_LIGHT_NUM > 0
}

void dt_interaction_t::two_button_push_dim(const uint8_t num, const Bt_Action action)
{
#if DIMMER_LIGHT_NUM > 0
    if (restore())
        Serial.println(F("2BPD restore"));

    if (memory_config.run == true)
        Serial.println(F("2BPD run == true"));
    // if (action != IN_RELEASE && millis() - memory[num_fg].D2PB.interlock_time < FG_DEBOUNCE_TIME) //debounce (antirebond)
    // {
    //     // Serial.println(F("FG_DEBOUNCE_TIME"));
    //     return;
    // }

    if (is_locked()) // si l'interlock est actif traite uniquement les info du bouton verouille
    {
        Serial.println(F("2BPD Locked"));
        return;
    }
    else if (action == IN_PUSHED)
    {
        Serial.println(F("2BPD Lock"));
        memory_config.interlock = true;
        memory_config.run = false;
        store();
        return;
    }
    else if (action >= IN_RELEASE && memory_config.interlock == false)
    {
        Serial.println(F("2BPD Locked 2"));
        return;
    }
    //     else
    //     else if (action >= Bt_Action::IN_PUSH )
    //     {
    //         memory_config.interlock = false;
    //         remove();
    //     }
    //

    else if (action == Bt_Action::IN_PUSH) // short push
    {
        if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::up || eeprom_config.act_type == dt_interaction_eeprom_config::action_t::down) // short push on button up or down
        {
            // interlock
            // memory_config.interlock = true;
            // store();

            // if (config.FG[num_fg].in[num_in].type == FG_BTN_UP)
            //     Serial.println("FG_BTN_UP");
            // else if (config.FG[num_fg].in[num_in].type == FG_BTN_DOWN)
            //     Serial.println("FG_BTN_DOWN");

            if (get_dimmer(eeprom_config.act_num) == 0) // si dimmer off
            {
                dimmer_set(eeprom_config.act_num, true, DIMMER_ON_OFF_SPEED, false); // Turn ON
            }
            else // si dimmer on
            {
                dimmer_set(eeprom_config.act_num, false, DIMMER_ON_OFF_SPEED, false); // Turn OFF
            }

            // memory_config.interlock = false;
            // memory_config.run = false;
            // store();
            remove();
            Serial.println(F("2BPD unLock"));
            return;
        }
    }
    else if (action == Bt_Action::IN_LPUSH && memory_config.interlock == true) // long push
    {

        memory_config.run = true;
        store();

        if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::up) // long push on button up
        {
            // interlock
            // memory_config.interlock = true;
            // store();

            // Serial.println("FG_BTN_UP (long)");
            if (get_dimmer(eeprom_config.act_num) < 255) // si dimmer inferieur a 100%
            {
                dimmer_set(eeprom_config.act_num, (uint8_t)255, (uint16_t)DIMMER_SETTING_SPEED, false); // incrementé le dimmer;
            }
        }
        else if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::down) // long push on button down
        {
            // interlock
            // memory_config.interlock = true;
            // store();

            // Serial.println("FG_BTN_DOWN (long)");
            if (get_dimmer(eeprom_config.act_num) > DIMMER_SETTING_MIN) // si dimmer superieur a 0%
            {
                dimmer_set(eeprom_config.act_num, (uint8_t)DIMMER_SETTING_MIN, (uint16_t)DIMMER_SETTING_SPEED, false); // décrémenté le dimmer;
            }
        }
    }
    else if (action == IN_RELEASE && memory_config.interlock == true && memory_config.run == true)
    {
        dimmer_stop(eeprom_config.act_num);
        // memory_config.interlock = false;
        // memory_config.run = false;
        // store();
        remove();
        Serial.println(F("2BPD unLock"));
        return;
    }
    else
    {
        Serial.println(F("2BPD not Locked"));
        if (memory_config.run == true)
            Serial.println(F("2BPD run == true 2"));
    }

#endif // DIMMER_LIGHT_NUM > 0
}

void dt_interaction_t::two_button_push_cover(const uint8_t num, const Bt_Action action)
{
#if COVER_NUM > 0
    restore();
    // printf("FG_Func_STORE num_fg=%d\n", num_fg);
    // printf("FG_Func_STORE\n");

    if (action == IN_PUSHED)
    {
        Serial.println(F("Lock"));
        memory_config.interlock = true;
        store();
        // return;
    }
    else if (action == IN_RELEASE && !is_locked() && memory_config.interlock == true && memory_config.run == true) // save up or down time and reset interlock
    {
        // printf("DIM_STORE_TIME_CONFIG_UP ou DIM_STORE_TIME_CONFIG_DOWN\n");
        // memory_config.interlock = false;               // on relache l interlock
        DT_cover_stop_register(eeprom_config.act_num); // validation de la configuration du cover
        remove();                                      // on libere la memoire
        // return;
    }
    // else if (action == IN_PUSHED && is_locked()) // push 2 button a same time
    // {
    //     //   printf("push 2 button a same time\n");
    //     DT_cover_stop(eeprom_config.act_num);
    // }
    // else if (is_locked()) // if a locked butuon
    // {
    //     //   printf("interlock");
    //     return;
    // }
    else if (action == IN_PUSH && memory_config.interlock == true) // short push
    {
        if (is_locked() == false) // short push on buttons
        {
            if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::up) // short push on button up
            {
                // printf("FG_Func_STORE FG_BTN_UP\n");
                //   printf("btn push UP\n");
                DT_cover_up(eeprom_config.act_num);
                remove();
            }
            else if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::down) // short push on button down
            {
                // printf("FG_Func_STORE FG_BTN_DOWN\n");
                //   printf("btn push down\n");
                DT_cover_down(eeprom_config.act_num);
                remove();
            }
        }
        else // short push on 2 buttons (STOP or memory position)
        {
            // printf("FG_Func_STORE push 2 btn\n");
            //   printf("btn 2 push btn\n");
            if (DT_cover_get_state(eeprom_config.act_num) >= 8) // if store mouve up or down
            {
                Serial.println(F("btn 2 push btn DIM_STORE_STOP"));
                DT_cover_stop(eeprom_config.act_num);
                remove_all();
            }
            else // if store is stoped
            {
                Serial.println(F("btn 2 push btn DIM_STORE_RESTORE_POS"));
                DT_cover_restore_pos(eeprom_config.act_num);
                remove_all();
            }
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
    else if (action == IN_LLPUSH && memory_config.interlock == true) // long long push
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
        if (is_locked()) // long push on 2 buttons (Memory position)
        {
            // printf("FG_Func_STORE long push 2 btn\n");
            cover_state state = DT_cover_get_state(eeprom_config.act_num);
            if (state != cover_closed && state != cover_open)
            {
                DT_cover_backup_pos(eeprom_config.act_num);
                remove_all();
            }
        }
    }
    else if (action == IN_XLLPUSH && memory_config.interlock == true) // XL long push
    {
        //   printf("xl_long_push\n");
        if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::up && memory_config.interlock == true) // XL long push on button up
        {
            Serial.println(F("DIM_STORE_TIME_CONFIG_UP"));
            // TODO: FG_Func_STORE_2_PUSH_BTN: enregistrement du temps de montée
            DT_cover_start_register(eeprom_config.act_num, true);
            memory_config.run = true;
            store();
            // remove();
        }
        else if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::down && memory_config.interlock == true) // XL long push on button down
        {
            Serial.println(F("DIM_STORE_TIME_CONFIG_DOWN"));
            // TODO: FG_Func_STORE_2_PUSH_BTN: enregistrement du temps de déssente
            DT_cover_start_register(eeprom_config.act_num, false);
            memory_config.run = true;
            store();
            // remove();
        }
    }
    else if (is_locked()) // for debug
    {
        Serial.println(F("Locked"));
        Serial.println(F("No action"));
        // return;
    }
    else if (!is_locked()) // for debug
    {
        Serial.println(F("Not Locked"));
        Serial.println(F("No action"));
        // return;
    }
#endif // COVER_NUM > 0
};

void dt_interaction_t::two_button_push_portal(const uint8_t num, const Bt_Action action)
{
#if PORTAL_NUM > 0
    restore();

    if (action == IN_PUSH) // short push
    {
        if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::up) // short push on button up
        {
            // printf("FG_Func_STORE FG_BTN_UP\n");
            //   printf("btn push UP\n");
            if (DT_portal_get_state(eeprom_config.act_num) == portal_closed || DT_portal_get_state(eeprom_config.act_num) == portal_stopped)
            {
                DT_portal_open(eeprom_config.act_num);
            }
            else
            {
                DT_portal_stop(eeprom_config.act_num);
            }
            remove();
        }
        else if (eeprom_config.act_type == dt_interaction_eeprom_config::action_t::down) // short push on button down
        {
            // printf("FG_Func_STORE FG_BTN_DOWN\n");
            //   printf("btn push down\n");
            // printf("FG_Func_STORE FG_BTN_UP\n");
            //   printf("btn push UP\n");
            if (DT_portal_get_state(eeprom_config.act_num) == portal_open || DT_portal_get_state(eeprom_config.act_num) == portal_stopped)
            {
                DT_portal_close(eeprom_config.act_num);
            }
            else
            {
                DT_portal_stop(eeprom_config.act_num);
            }
            remove();
        }
    }
#endif // PORTAIL_NUM > 0
};