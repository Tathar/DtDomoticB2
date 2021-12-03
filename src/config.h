#ifndef CONFIG
#define CONFIG

#include <Arduino.h>

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

#endif