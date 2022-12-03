#include <ArduinoFake.h>
#include <unity.h>

using namespace fakeit;
// #include "ArduinoFake.h"
#include <DT_eeprom.h>
#include <DT_poele.h>
#include <mock.h>
#include <config.h>
#include <pinout.h>

Config config;

unsigned long my_millis = 0;

inline void sleep_ms(uint32_t msec)
{
    When(Method(ArduinoFake(), millis)).AlwaysReturn(my_millis += msec);
}

inline void sleep(uint32_t sec)
{
    When(Method(ArduinoFake(), millis)).AlwaysReturn(my_millis += sec * 1000);
}

// void BASE_TEST_FG_STORE_2_PUSH_BTN(uint8_t btn, Bt_Action action, uint8_t num_dimmer)
// {
//     uint8_t btn_up = btn;
//     uint8_t btn_down = btn + 1;
//     // printf("btn = %i\n", btn);
//     // printf("btn2 = %i\n", btn2);
//     Bt_Action long_push = Bt_Action(int(action) + 1);
//     Bt_Action long_long_push = Bt_Action(int(action) + 2);
//     // printf("action = %i\n", action);
//     // printf("long_push = %i\n", long_push);
//     uint8_t motor_up = num_dimmer;
//     uint8_t motor_down = num_dimmer + 1;
//     my_millis += 1000000;

//     //test up
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(1);
//     btn_push(btn_up, action);
//     wait_ms(100);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     for (uint8_t i = 1; i <= 60; i += 1)
//     {
//         wait(1);
//         TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//         TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//         printf("wait = %d ms = %d\n", i, my_millis);
//     }
//     for (uint8_t i = 0; i < 9; ++i)
//     {
//         wait(1);
//         TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//         TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     }
//     wait(2);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //test down

//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     sleep(1);
//     FG_Button(btn_down, action);
//     wait_ms(50);
//     FG_Button(btn_down, IN_RELEASE);
//     wait_ms(50);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     wait(30);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     wait(30);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     wait(10);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //test up and stop
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     sleep(1);
//     btn_push(btn_up, action);
//     sleep_ms(50);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     sleep(30);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     printf("btn_push(2 button)\n");
//     FG_Button(btn_up, IN_PUSHED);
//     wait_ms(50);
//     FG_Button(btn_down, IN_PUSHED);
//     wait_ms(50);

//     FG_Button(btn_up, IN_RELEASE);
//     wait_ms(50);
//     FG_Button(btn_down, IN_RELEASE);
//     wait_ms(50);

//     FG_Button(btn_up, action);
//     wait_ms(50);
//     FG_Button(btn_down, action);
//     wait_ms(50);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(1);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(1);

//     printf("btn_push(up)\n");
//     btn_push(btn_up, action);
//     wait_ms(100);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(30);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(30);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(10);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //test down and stop
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     sleep(1);
//     FG_Button(btn_down, action);
//     sleep_ms(50);
//     FG_loop();
//     sleep_ms(50);
//     FG_Button(btn_down, IN_RELEASE);
//     sleep_ms(50);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     sleep(30);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     btn_push2(btn_down, btn_up, action, action);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(1);
//     btn_push(btn_down, action);
//     wait(1);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     sleep(30);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     sleep(30);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     sleep(10);
//     FG_loop();
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //changement de sens en cours de monté / dessente (up/down/up/down)
//     printf("\nnew test up/down/up/down \n");
//     btn_push(btn_up, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 2; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push(btn_down, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 4; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     btn_push(btn_up, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 4; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push(btn_down, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 2; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }

//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     btn_push(btn_up, action);

//     wait(60);

//     //changement de sens en cours de monté / dessente (down/up/down/up)
//     printf("\nnew test down/up/down/up \n");
//     btn_push(btn_down, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 2; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     btn_push(btn_up, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 4; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push(btn_down, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 4; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     btn_push(btn_up, action);
//     for (uint32_t i = 0; i <= config.FG[0].store_time / 2; i += config.FG[0].store_time / 100)
//     {
//         sleep_ms(config.FG[0].store_time / 100);
//         FG_loop();
//     }

//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     wait(60);

//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //set memory
//     btn_push(btn_down, action);
//     wait_ms(config.FG[0].store_time / 4);

//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     btn_push2(btn_down, btn_up, action, action); //stop
//     wait_ms(500);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push2(btn_down, btn_up, long_long_push, long_long_push); //save position in memory
//     wait_ms(1100);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     wait_ms(200);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     wait_ms(1200);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push(btn_up, action);
//     wait(71);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //test going to memory
//     printf("test going to memory\n");
//     btn_push2(btn_down, btn_up, action, action); //going memory (down)
//     wait_ms(500);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     wait(60);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push(btn_down, action);
//     wait(120);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     printf("test going to memory\n");
//     btn_push2(btn_down, btn_up, action, action); //going memory (up)
//     wait_ms(500);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     wait(60);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //config time up
//     printf("config time up\n");
//     btn_xllpush(btn_down, action);
//     wait_ms(50);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));
//     wait(120);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     btn_lpush_REALSE(btn_down, action);
//     wait_ms(50);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push(btn_up, action);
//     wait_ms(100);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     wait(120);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     wait(30);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     //stage 2
//     btn_xllpush(btn_up, action);
//     wait_ms(50);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
//     wait(60);
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_lpush_REALSE(btn_up, action);
//     wait_ms(50);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     btn_push(btn_down, action);
//     wait_ms(100);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     wait(60);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(100, get_dimmer(motor_down));

//     wait(30);
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));

//     //end of test
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_up));
//     TEST_ASSERT_EQUAL_UINT8(0, get_dimmer(motor_down));
// }

void TEST_DT_3_VOIE(void)
{
    //set configuration
    my_millis = 0;
    init_mock();
    chargeEEPROM();

    DT_poele_init()
    sleep(1);
    DT_poele_loop()
    sleep(1);
    DT_poele_loop()
    sleep(1);
    DT_poele_loop()
    sleep(1);
    DT_poele_loop()
    TEST_ASSERT_EQUAL(32, 32);

}


int main(int argc, char *argv[])
{
    UNITY_BEGIN();
    RUN_TEST(TEST_DT_3_VOIE);
    UNITY_END();
    return 0;
};