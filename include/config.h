#ifndef CONFIG
#define CONFIG

#include <Arduino.h>
#include <config.h>

#define STRINGIFY(x) #x
#define TO_C_STR(x) STRINGIFY(x)

#define BOARD_MANUFACTURER "DOUET Touch Familly"
#define BOARD_MODEL "DTBoard02"
#define BOARD_NAME "Chauffage"
#define BOARD_SW_VERSION "0.1"
#define BOARD_IDENTIFIER "DTB02-001"

// input
#define DEBOUNCE_TIME 50

#define BUFFER_SIZE 65
#define BUFFER_VALUE_SIZE 512

// watchdog
#define WATCHDOG_TIME WDTO_1S

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
#define SOURCE_IP4 201
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

#define MQTT_IP1 90
#define MQTT_IP2 78
#define MQTT_IP3 191
#define MQTT_IP4 235

#define MQTT_CLIENT_ID "Board1"
#define MQTT_USER "DtBoard"
#define MQTT_PASSWORD "1MotdePasse"

#define NETWORK_RESET_TIME 20000 // temp avant reset de la carte reseau en qua d'imposibilité de se connecter (en miliseconde)
#define MQTT_UPDATE 1000         // in ms

// Poele

#define PT100_BALON 1
#define PT100_ECS1 2
#define PT100_ECS2 3

// Vanne 3 Voies

#define TMP_PCBT_MAX 38 // valeur maximum de la consigne de temperature
#define TMP_MCBT_MAX 60 // valeur maximum de la consigne de temperature
#define DBMAC 0.5       // demi bamnde morte pour l'arret des circulateur (en °C)

#define PT100_EXT 4
#define PT100_3_VOIES_PCBT 5
#define PT100_3_VOIES_MCBT 6

#define CIRCULATEUR_PCBT 1
#define VANNE_PCBT_HOT 2
#define VANNE_PCBT_COLD 3

#define CIRCULATEUR_MCBT 4
#define VANNE_MCBT_HOT 5
#define VANNE_MCBT_COLD 6

// Planchée chauffant
#define MAX_TMP_PLANCHE 27 // en °C
#define NUM_PT100_PLANCHE 6
#define PT100_PLANCHE_CUISINE 7
#define PT100_PLANCHE_SALON 8
#define PT100_PLANCHE_CH_1 9
#define PT100_PLANCHE_CH_2 10
#define PT100_PLANCHE_SDB 11
#define PT100_PLANCHE_CELIER 12

const float pt100_planchee[NUM_PT100_PLANCHE] PROGMEM = {PT100_PLANCHE_CUISINE, PT100_PLANCHE_SALON, PT100_PLANCHE_CH_1, PT100_PLANCHE_CH_2, PT100_PLANCHE_SDB, PT100_PLANCHE_CELIER};

// Circulateur
#define CIRCULATEUR

#endif