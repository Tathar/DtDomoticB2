#include <DT_mqtt.h>

#include <avr/wdt.h> //watchdog
#include <DT_eeprom.h>

#define add0x2(s) 0x##s
#define toHEX(s) add0x2(s)

#ifdef MQTT
// float to str
//  char *dtostrf(double val, signed char width, unsigned char prec, char *s);
// str to float
// if (sscanf(value, "%d", &ret) != 1)

// Update these with values suitable for your hardware/network.
IPAddress server(MQTT_IP1, MQTT_IP2, MQTT_IP3, MQTT_IP4);
EthernetClient ethClient;
PubSubClient mqtt(ethClient);

void (*_mqtt_update)(PubSubClient &mqtt);
void (*_mqtt_subscribe)(PubSubClient &mqtt);
// void (*_mqtt_receve)(char *, uint8_t *, unsigned int);

void DT_mqtt_set_update_callback(void (*mqtt_update)(PubSubClient &mqtt))
{
    _mqtt_update = mqtt_update;
}

void DT_mqtt_set_subscribe_callback(void (*mqtt_subscribe)(PubSubClient &mqtt))
{
    _mqtt_subscribe = mqtt_subscribe;
}

void DT_mqtt_set_receve_callback(void (*mqtt_receve)(char *, uint8_t *, unsigned int))
{
    mqtt.setCallback(mqtt_receve);
}

bool DT_mqtt_send(const char *tag, const float value)
{
    char buffer[32];
    dtostrf(value, 1, 2, buffer);
    return mqtt.publish(tag, buffer, strlen(buffer));
}

bool DT_mqtt_send(const char *tag, const unsigned int value)
{

    char buffer[32];
    sprintf(buffer, "%u", value);
    return mqtt.publish(tag, buffer, strlen(buffer));
}

bool DT_mqtt_send(const char *tag, const int value)
{
    char buffer[32];
    sprintf(buffer, "%i", value);
    return mqtt.publish(tag, buffer, strlen(buffer));
}

bool DT_mqtt_send(const char *tag, const uint32_t value)
{
    char buffer[32];
    sprintf(buffer, "%" PRIu32, value);
    return mqtt.publish(tag, buffer, strlen(buffer));
}

bool DT_mqtt_send(const char *tag, const char *value)
{
    return mqtt.publish(tag, value, strlen(value));
}

void init_ethernet()
{
    Ethernet.init(NETWORK_CS);
#ifdef DHCP
    Ethernet.begin(mac, 5000);
#else
    byte mac[] = {MAC1, MAC2, MAC3, MAC4, MAC5, MAC6};
    IPAddress ip(SOURCE_IP1, SOURCE_IP2, SOURCE_IP3, SOURCE_IP4);
    IPAddress dns(DNS1, DNS2, DNS3, DNS4);
    IPAddress gateway(GW1, GW2, GW3, GW4);
    IPAddress mask(MASK1, MASK2, MASK3, MASK4);
    Ethernet.begin(mac, ip, dns, gateway, mask);
#endif
    // delay(1500);
}

void DT_mqtt_init()
{
    // auto Serial.println("start network");
    pinMode(NETWORK_RESET, OUTPUT);
    digitalWrite(NETWORK_RESET, HIGH);
    mqtt.setServer(server, 1883);
    // _mqtt_update = nullptr;
    // _mqtt_subscribe = nullptr;
    // mqtt.setCallback(&test_mqtt_receve);
    init_ethernet();
}

void DT_mqtt_loop()
{
    static uint32_t last_reconnection_time = 0;
    static uint32_t reset_time = 0; // for reset network device
    static bool reset = false;      // for reset network device
    uint32_t now = millis();
    if (!mqtt.connected())
    {
        if (mem_config.MQTT_online)
        {
            mem_config.MQTT_online = false;
        }

        wdt_enable(WDTO_8S); // watchdog at 8 secdons
        if (reset_time == 0)
            reset_time = now;
        else if (reset_time != 0 && !reset && now - reset_time > NETWORK_RESET_TIME)
        {
            // auto Serial.println("reset network board");
            digitalWrite(NETWORK_RESET, LOW);
            last_reconnection_time = now;
            reset = true;
        }
        else if (now - last_reconnection_time > 5000)
        {
            last_reconnection_time = now;
            if (reset)
            {
                // auto Serial.println("restart network");
                digitalWrite(NETWORK_RESET, HIGH);
                delay(2);
                wdt_reset();
                init_ethernet();
                wdt_reset();
                reset_time = 0;
                reset = false;
            }
            // Attempt to reconnect
            // String clientId = "Board01";
            // auto Serial.println("start MQTT conection");
            //  if (mqtt.connect(clientId.c_str(), "DtBoard", "1MotdePasse"))

            wdt_reset();
            if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_WILL_TOPIC, MQTT_WILL_QOS, MQTT_WILL_RETAIN, MQTT_WILL_MESSAGE))
            {
                wdt_reset();
                // Once connected, publish an announcement and resubscribe...
                if (_mqtt_subscribe != nullptr)
                    _mqtt_subscribe(mqtt);
                reset_time = 0; // desactivation du compteur de reset
                                // auto Serial.println("MQTT connected");
            }
            else
            { // si echec affichage erreur
                wdt_reset();
                // auto Serial.print("ECHEC, rc=");
                // auto Serial.print(mqtt.state());
                // auto Serial.println(" nouvelle tentative dans 5 secondes");
            }
        }

        // delay(50);
        wdt_enable(WATCHDOG_TIME);
    }
    else
    {
        mqtt.loop();
        static uint32_t time = 0;
        if (now - time >= MQTT_UPDATE)
        {
            if (!mem_config.MQTT_online)
            {
                mem_config.MQTT_online = true;
            }
            if (_mqtt_update != nullptr)
                _mqtt_update(mqtt);
        }
        // delay(100);
    }
}

#endif //MQTT