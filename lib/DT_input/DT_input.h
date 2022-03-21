#ifndef DT_INPUT
#define DT_INPUT
#include <Arduino.h>

enum __attribute__((__packed__)) Bt_Action // l'utilisation de "__attribute__((__packed__))" indique au compilateur de codé l'enum sur 1 octet
{
    IN_NULL,
    IN_PUSHED,
    IN_RELEASE,
    IN_PUSH = 4,
    IN_LPUSH,
    IN_LLPUSH,
    IN_XLLPUSH,
    IN_2PUSH = 8,
    IN_L2PUSH,
    IN_LL2PUSH,
    IN_XLL2PUSH,
    IN_3PUSH = 12,
    IN_L3PUSH,
    IN_LL3PUSH,
    IN_XLL3PUSH,
    IN_4PUSH = 16,
    IN_L4PUSH,
    IN_LL4PUSH,
    IN_XLL4PUSH,
    IN_5PUSH = 20,
    IN_L5PUSH,
    IN_LL5PUSH,
    IN_XLL5PUSH,
    IN_6PUSH = 24,
    IN_L6PUSH,
    IN_LL6PUSH,
    IN_XLL6PUSH,
    IN_7PUSH = 28,
    IN_L7PUSH,
    IN_LL7PUSH,
    IN_XLL7PUSH,
    IN_8PUSH = 32,
    IN_L8PUSH,
    IN_LL8PUSH,
    IN_XLL8PUSH
};

void DT_input_init();
void DT_input_loop();
uint8_t DT_input_get_pin_stats(int num);
Bt_Action DT_input_get_stats(int num);
void DT_input_set_callback(void (*callback)(const uint8_t num, const Bt_Action action));

#endif