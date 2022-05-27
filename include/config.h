#ifndef CONFIG
#define CONFIG

#include <Arduino.h>
#include <config.h>

void debug(const char *var);
void debug(const __FlashStringHelper *var);
void memory(bool print);
void debug_wdt_reset();
void debug_wdt_reset(const char *var);
void debug_wdt_reset(const __FlashStringHelper *var);

#define SAVE_EEPROM 600000 // sauvegarde des données dans l eeprom toute les x ms
#define MQTT_REFRESH 10000 // temp de rafrechissement du MQTT

#define BOARD_MANUFACTURER "DOUET Touch Familly"
#define BOARD_MODEL "DTBoard02"
#define BOARD_NAME "Chauffage"
#define BOARD_SW_VERSION "0.2"
#define BOARD_IDENTIFIER "DTB02-001"

#define MAX_TOPIC 64
#define MAX_PAYLOAD 512

// INPUT
#define DEBOUNCE_TIME 50
#define LONG_PUSH_TIME 800
#define LONG_LONG_PUSH_TIME 5000 // for value save
#define XL_LONG_PUSH_TIME 10000
#define MULTIPLE_PUSH_TIME 100
#define INPUT_REFRESH DEBOUNCE_TIME / 2

// Dimmer
#define DIMMER_LIGHT_NUM 14 // max (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + (DIMMER_COVER_NUM * 2) ) = 14
#define DIMMER_SPEED 250    // en miliseconde
// #define MIN_CANDLE_TIME 250
// #define MAX_CANDLE_TIME 1000
#define CANDLE_OFSSET_PERCENTE_MIN 0  // en pourcentage
#define CANDLE_OFSSET_PERCENTE_MAX 30 // en pourcentage
#define CANDLE_SPEED_MIN 250          // en miliseconde
#define CANDLE_SPEED_MAX 1000         // en miliseconde

#define DIMMER_HEAT_NUM 0 // max (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + (DIMMER_COVER_NUM * 2) ) = 14

#define DIMMER_COVER_NUM 0 // nombre de volet connecté dimmer : max (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + (DIMMER_COVER_NUM * 2) ) = 14

// relais
#define RELAY_COVER_NUM 1 // nombre de volet connecté au relai

// PT100
#define TEMP_NUM 3 // 12 ok 13 ko // max 18
// #define TEMP_NUM 0

// watchdog
#define WATCHDOG_TIME WDTO_8S

#define MQTT
#ifdef MQTT
// buffer
#define BUFFER_SIZE 65
#define BUFFER_VALUE_SIZE 512
// network
#define MAC1 0xDE
#define MAC2 0xED
#define MAC3 0xBA
#define MAC4 0xFE
#define MAC5 0xFE
#define MAC6 0xED
//#define DHCP
// ip address
#define SOURCE_IP1 192
#define SOURCE_IP2 168
#define SOURCE_IP3 1
#define SOURCE_IP4 4
// dns server
#define DNS1 192
#define DNS2 168
#define DNS3 1
#define DNS4 1
// gateway
#define GW1 192
#define GW2 168
#define GW3 1
#define GW4 1
// sub net mask
#define MASK1 255
#define MASK2 255
#define MASK3 255
#define MASK4 0
// MQTT Server
#define MQTT_IP1 80
#define MQTT_IP2 11
#define MQTT_IP3 239
#define MQTT_IP4 99
// MQTT config
#define MQTT_CLIENT_ID "Board2"
#define MQTT_USER "test"
#define MQTT_PASSWORD "test"
#define MQTT_ROOT_TOPIC "DtBoard"
#define MQTT_WILL_TOPIC MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"
#define MQTT_WILL_QOS 1
#define MQTT_WILL_RETAIN true
#define MQTT_WILL_MESSAGE "offline"
// Advance option
#define NETWORK_RESET_TIME 60000 // temp avant reset de la carte reseau en qua d'imposibilité de se connecter (en miliseconde)
#define MQTT_UPDATE 1000         // in ms
#endif                           // MQTT

// Poele
#define POELE
#ifdef POELE
#define MIN_T4 0                     // en °C (fake NTC)
#define POELE_MAX_TEMPERATURE 85     // en °C (consigne temperature Balon)
#define TEMPERATURE_DEFAULT_POELE 85 // en °C (Temperature a la quelle le poele ce met en default)
#define TEMPS_DEFAULT_PT100_POELE 60000

#define PT100_H_BALON 1
#define PT100_M_BALON 2
#define PT100_B_BALON 3
#define PT100_ECS1 4
#define PT100_ECS2 5

#define RELAY_EV1 1
#endif // POELE

// Vanne 3 Voies
#define VANNES
#ifdef VANNES
#define TMP_EAU_PCBT_MAX 38 // valeur maximum de la consigne de temperature
#define TMP_EAU_MCBT_MAX 60 // valeur maximum de la consigne de temperature
#define DBMAC 0.5           // demi bamnde morte pour l'arret des circulateur (en °C)

#define PT100_EXT 6
#define PT100_3_VOIES_PCBT 7
#define PT100_3_VOIES_MCBT 8

#define CIRCULATEUR_PCBT 2
#define VANNE_PCBT_HOT 3
#define VANNE_PCBT_COLD 4

#define CIRCULATEUR_MCBT 5
#define VANNE_MCBT_HOT 6
#define VANNE_MCBT_COLD 7

// Planchée chauffant
#define MAX_TMP_PLANCHE 27 // en °C
#define NUM_PLANCHE 4
#define PT100_PLANCHE_SALON 9
#define RELAIS_PLANCHE_SALON 8
#define PT100_PLANCHE_CH_1 10
#define RELAIS_PLANCHE_CH_1 9
#define PT100_PLANCHE_CH_2 11
#define RELAIS_PLANCHE_CH_2 10
#define PT100_PLANCHE_SDB 12
#define RELAIS_PLANCHE_SDB 11

// const float PT100_PLANCHEE[NUM_PLANCHE] PROGMEM = {PT100_PLANCHE_SALON, PT100_PLANCHE_CH_1, PT100_PLANCHE_CH_2, PT100_PLANCHE_SDB};
// const uint8_t RELAI_PLANCHE[NUM_PLANCHE] PROGMEM = {RELAIS_PLANCHE_SALON, RELAIS_PLANCHE_CH_1, RELAIS_PLANCHE_CH_2, RELAIS_PLANCHE_SDB};

#endif // VANNES

// Circulateur boucle d eau chaude
#define CIRCULATEUR_ECS 12

// EV2 ECS
#define EV2_ECS 13

// Demmarage Poele
#define MARCHE_POELE 14

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// program variable, do not touch


#ifdef __GIT_HASH__
#define BOARD_SW_VERSION_PRINT BOARD_SW_VERSION " (" __GIT_HASH__ ")"
#else
#define BOARD_SW_VERSION_PRINT BOARD_SW_VERSION
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#if DIMMER_LIGHT_NUM >= 1
#define DIMMER_LIGHT_NUM_OPT DIMMER_LIGHT_NUM + 1
#else
#define DIMMER_LIGHT_NUM_OPT 0
#endif

#define COVER_NUM DIMMER_COVER_NUM + RELAY_COVER_NUM

#endif