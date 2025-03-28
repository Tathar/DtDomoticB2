#ifndef TOOLS_H
#define TOOLS_H

#ifdef __GIT_HASH__
#define BOARD_SW_VERSION_PRINT BOARD_SW_VERSION " (" __GIT_HASH__ ")"
#else
#define BOARD_SW_VERSION_PRINT BOARD_SW_VERSION
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#if DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM > 0
#define DIMMER_LIGHT_HEAT_NUM_OPT (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + 1)
#else
#define DIMMER_LIGHT_HEAT_NUM_OPT 0
#endif

// #define DIMMER_RELAY_NUM (18 - (DIMMER_LIGHT_HEAT_NUM_OPT + DIMMER_COVER_NUM +  PT100_NUM + TIC_NUM))
#define DIMMER_RELAY_FIRST_NUM DIMMER_LIGHT_HEAT_NUM_OPT + DIMMER_COVER_NUM
// #define DIMMER_RELAY_LAST_NUM DIMMER_RELAY_FIRST_NUM + DIMMER_RELAY_NUM

#define COVER_NUM (DIMMER_COVER_NUM + RELAY_COVER_NUM)

#define RADIATOR_NUM (DIMMER_RADIATOR_NUM + RELAY_RADIATOR_NUM)
#define DIMMER_RADIATOR_FIRST_NUM DIMMER_COVER_NUM
#define RELAY_RADIATOR_FIRST_NUM (RELAY_COVER_NUM * 2)

#if RADIATOR_NUM > 0
#ifndef PT100_EXT
#error "need define PT100_EXT"
#endif
#endif

//#define DIMMER_RELAY_RESERVED ((DIMMER_COVER_NUM * 2) + DIMMER_RADIATOR_NUM)
//#define RELAY_RESERVED ((RELAY_COVER_NUM * 2) + RELAY_RADIATOR_NUM)

#if (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM) > 13
#error "you can use only 13 dimmer"
#endif

#ifdef TIC
#define TIC_NUM  1
#endif

#if (DIMMER_LIGHT_HEAT_NUM_OPT + (DIMMER_COVER_NUM * 2) + DIMMER_RADIATOR_NUM + DIMMER_RELAY_NUM + PT100_NUM + TIC_NUM) > 18
#error "this board as only 18 OPT connector"
#endif

#ifndef I2C_Multiplexeur
#undef INTERNAL_INPUT_I2C
#undef INTERNAL_OUTPUT_I2C
#endif

#ifndef INTERNAL_INPUT_I2C
#undef INPUT_NUM
#define INPUT_NUM 16
#endif

#ifndef INTERNAL_OUTPUT_I2C
#undef OUTPUT_NUM
#define OUTPUT_NUM 16
#endif

int i2c_channel_to_multiplexer(int channel);

#define P0 (OPT_P0 - 1)
#define P0_INF (OPT_P0 - 2)

void init_tools();

void debug_wdt_reset(void);
void debug_wdt_reset(const char *var);
void debug_wdt_reset(const __FlashStringHelper *var);

#endif // TOOLS_H