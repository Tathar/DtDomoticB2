#ifndef CONFIG
#define CONFIG

#include <Arduino.h>
#include <pinout.h>

void debug(const char *var);
void debug(const __FlashStringHelper *var);
void memory(bool print);
// void debug_wdt_reset();
// void debug_wdt_reset(const char *var);
// void debug_wdt_reset(const __FlashStringHelper *var);

#define SAVE_EEPROM 600000 // sauvegarde des données dans l eeprom toute les x ms
#define MQTT_REFRESH 10000 // temp de rafrechissement du MQTT

#define BOARD_MANUFACTURER "DOUET Touch Familly"
#define BOARD_MODEL "DTBoard02"
#define BOARD_NAME "Chauffage"
#define BOARD_SW_VERSION "0.2"
#define BOARD_IDENTIFIER "DTB02-001"

#define MAX_TOPIC 64
#define MAX_PAYLOAD 260

// INPUT
#define DEBOUNCE_TIME 50
#define LONG_PUSH_TIME 800
#define LONG_LONG_PUSH_TIME 5000 // for value save
#define XL_LONG_PUSH_TIME 10000
#define MULTIPLE_PUSH_TIME 100
#define INPUT_REFRESH DEBOUNCE_TIME / 2

// Dimmer
#define DIMMER_LIGHT_NUM 0 // max (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + (DIMMER_COVER_NUM * 2) ) = 14
#define DIMMER_SPEED 250   // en miliseconde
// #define MIN_CANDLE_TIME 250
// #define MAX_CANDLE_TIME 1000
#define CANDLE_OFSSET_PERCENTE_MIN 0  // en pourcentage
#define CANDLE_OFSSET_PERCENTE_MAX 30 // en pourcentage
#define CANDLE_SPEED_MIN 250          // en miliseconde
#define CANDLE_SPEED_MAX 1000         // en miliseconde

#define DIMMER_HEAT_NUM 0     // Fil pilote : max (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + (DIMMER_COVER_NUM * 2) ) = 14
#define DIMMER_COVER_NUM 0    // nombre de volet connecté au dimmer : max (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + (DIMMER_COVER_NUM * 2) ) = 14
#define DIMMER_RADIATOR_NUM 0 // nombre de radiateur connecté au dimmer : max (DIMMER_LIGHT_NUM + DIMMER_HEAT_NUM + (DIMMER_COVER_NUM * 2) + DIMMER_RADIATOR_NUM ) = 14
const uint8_t DIMMER_RADIATOR_PT100_ARRAY[DIMMER_RADIATOR_NUM] PROGMEM = {};
// PT100
#define TEMP_NUM 18 // max 18 //TODO: bug if TEMP_NUM != 18 
#define PT100_EXT 5

// BME_280
#define BME280_NUM 1
#if BME280_NUM > 0
const uint8_t BME280_ADDRESS_ARRAY[BME280_NUM] PROGMEM = {0x76};
const uint8_t BME280_CHANNEL_ARRAY[BME280_NUM] PROGMEM = {2};
#endif

// CCS_811
#define CCS811_NUM 2
#if CCS811_NUM > 0
const uint8_t CCS811_ADDRESS_ARRAY[CCS811_NUM] PROGMEM = {0x5A,0X5B};
const uint8_t CCS811_CHANNEL_ARRAY[CCS811_NUM] PROGMEM = {2,2};
#endif

// SCD4X
#define SCD4X_NUM 1
#if SCD4X_NUM > 0
const uint8_t SCD4X_CHANNEL_ARRAY[SCD4X_NUM] PROGMEM = {2};
#endif


// TIC
// téléreléve information client
#define TIC_NUM 0 // number of TIC : max 1

// relais
#define RELAY_COVER_NUM 0    // nombre de volet connecté au relai
#define RELAY_RADIATOR_NUM 0 // nombre de radiateur connecté au relai
const uint8_t RELAY_RADIATOR_PT100_ARRAY[RELAY_RADIATOR_NUM] PROGMEM = {};

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
#define MQTT_IP1 192
#define MQTT_IP2 168
#define MQTT_IP3 1
#define MQTT_IP4 2
// MQTT config
#define MQTT_CLIENT_ID "Board_chauffage"
#define MQTT_USER "dtboard"
#define MQTT_PASSWORD "1MotdePasse"
#define MQTT_ROOT_TOPIC "DtBoard"
#define MQTT_WILL_TOPIC MQTT_ROOT_TOPIC "/" BOARD_IDENTIFIER "/status"
#define MQTT_WILL_QOS 1
#define MQTT_WILL_RETAIN true
#define MQTT_WILL_MESSAGE "offline"
// Advance option
#define NETWORK_RESET_TIME 60000 // temp avant reset de la carte reseau en qua d'imposibilité de se connecter (en miliseconde)
#define MQTT_UPDATE 1000         // in ms
#endif                           // MQTT

// Home Assistant

#define RADIATOR_HA_MAX_TEMP 22
#define RADIATOR_HA_MIN_TEMP 15

// Poele
#define POELE
#ifdef POELE
#define MIN_T4 0                     // en °C (fake NTC)
#define POELE_MAX_TEMPERATURE 85     // en °C (consigne temperature Balon)
#define TEMPERATURE_DEFAULT_POELE 85 // en °C (Temperature a la quelle le poele ce met en default)
#define TEMPS_DEFAULT_PT100_POELE 60000

#define PT100_H_BALON 0
#define PT100_M_BALON 1
#define PT100_B_BALON 2
#define PT100_ECS1 3
#define PT100_ECS2 4

#define RELAY_EV1 0
#endif // POELE

// Vanne 3 Voies
#define VANNES
#ifdef VANNES
#define TMP_EAU_PCBT_MAX 38 // valeur maximum de la consigne de temperature
#define TMP_EAU_MCBT_MAX 60 // valeur maximum de la consigne de temperature
#define DBMAC 0.5           // demi bamnde morte pour l'arret des circulateur (en °C)

// #define PT100_EXT 5
#define PT100_3_VOIES_PCBT 6
#define PT100_3_VOIES_MCBT 7

#define CIRCULATEUR_PCBT 1
#define VANNE_PCBT_HOT 2
#define VANNE_PCBT_COLD 3

#define CIRCULATEUR_MCBT 4
#define VANNE_MCBT_HOT 5
#define VANNE_MCBT_COLD 6

// Planchée chauffant
#define MAX_TMP_PLANCHE 27 // en °C
#define NUM_PLANCHE 4
#define PT100_PLANCHE_SALON 8
//#define RELAIS_PLANCHE_SALON 
#define PT100_PLANCHE_CH_1 9
// #define RELAIS_PLANCHE_CH_1 
#define PT100_PLANCHE_CH_2 10
// #define RELAIS_PLANCHE_CH_2 
#define PT100_PLANCHE_SDB 11
// #define RELAIS_PLANCHE_SDB 
#define PT100_PLANCHE_CELIER 14
// #define RELAIS_PLANCHE_CELIER 
#define PT100_MUR_CHAUFANT 15
// #define RELAIS_MUR_CHAUFANT 

#define PT100_SALON 13
#define PT100_CH1 14
#define PT100_SDB 15

// const float PT100_PLANCHEE[NUM_PLANCHE] PROGMEM = {PT100_PLANCHE_SALON, PT100_PLANCHE_CH_1, PT100_PLANCHE_CH_2, PT100_PLANCHE_SDB};
// const uint8_t RELAI_PLANCHE[NUM_PLANCHE] PROGMEM = {RELAIS_PLANCHE_SALON, RELAIS_PLANCHE_CH_1, RELAIS_PLANCHE_CH_2, RELAIS_PLANCHE_SDB};

#endif // VANNES

// Circulateur boucle d eau chaude
#define CIRCULATEUR_ECS 11

// EV2 ECS
#define EV2_ECS 12

// Demmarage Poele
#define MARCHE_POELE 13

//jeux de lumière
#define JEUX_LUMIERE_1 8
#define JEUX_LUMIERE_2 9
#define JEUX_LUMIERE_3 10
#define JEUX_LUMIERE_4 14
#define JEUX_LUMIERE_5 15





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// program variable, do not touch
#include <tools.h>
#endif // CONFIG