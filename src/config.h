#ifndef CONFIG
#define CONFIG

#define str(s) #s
#define defstr(s) str(s)

#define add0x(s) 0x##s
#define concat6(a, b, c, d, e, f) a##b##c##d##e##f
#define toMAC (a, b, c, d, e, f) #concat6(a, b, c, d, e, f)

#include <Arduino.h>

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

#define HEX(var) "0x"##str(var)
#define MAC MAC1##MAC2##MAC3##MAC4##MAC5##MAC6

#define BOARD_UID "FE:FE:ED"
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