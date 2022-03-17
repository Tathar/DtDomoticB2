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
    byte mac[] = {MAC1, MAC2, MAC3, MAC4, MAC5, MAC6};
#ifdef DHCP
    Ethernet.begin(mac, 5000);
#else
    IPAddress ip(SOURCE_IP1, SOURCE_IP2, SOURCE_IP3, SOURCE_IP4);
    IPAddress dns(DNS1, DNS2, DNS3, DNS4);
    IPAddress gateway(GW1, GW2, GW3, GW4);
    IPAddress mask(MASK1, MASK2, MASK3, MASK4);
    Ethernet.begin(mac, ip, dns, gateway, mask);
#endif

    // while (Ethernet.linkStatus() == LinkOFF)
    // {
    //     delay(10);
    // }

    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Serial.println(F("Ethernet shield was not found."));
    }
    else if (Ethernet.hardwareStatus() == EthernetW5100)
    {
        Serial.println(F("W5100 Ethernet controller detected."));
    }
    else if (Ethernet.hardwareStatus() == EthernetW5200)
    {
        Serial.println(F("W5200 Ethernet controller detected."));
    }
    else if (Ethernet.hardwareStatus() == EthernetW5500)
    {
        Serial.println(F("W5500 Ethernet controller detected."));
    }

    if (Ethernet.linkStatus() == Unknown)
    {
        Serial.println(F("Link status unknown. Link status detection is only available with W5200 and W5500."));
    }
    else if (Ethernet.linkStatus() == LinkON)
    {
        Serial.println(F("Link status: On"));
    }
    else if (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println(F("Link status: Off"));
    }

    // IPAddress ping(192, 168, 1, 1);
    // EthernetClient client;
    // if (client.connect(ping, 80))
    //     Serial.println(F("ping OK"));
    // else
    //     Serial.println(F("ping KO"));
}

void DT_mqtt_init()
{
    // auto Serial.println("start network");
    pinMode(NETWORK_RESET, OUTPUT);
    digitalWrite(NETWORK_RESET, HIGH);
    Serial.println(F("start network"));
    init_ethernet();
    mqtt.setServer(server, 1883);
    //  if (!mqtt.connected())
    //  {
    //      if (mqtt.connect("test", "test", "test"))
    //      {
    //          Serial.println("MQTT connected");
    //          mqtt.loop();
    //      }
    //      else
    //      {
    //          Serial.println("MQTT not connected");
    //      }
    //  }
    //  else
    //  {
    //      mqtt.loop();
    //  }
    _mqtt_update = nullptr;
    _mqtt_subscribe = nullptr;
    //  mqtt.setCallback(&test_mqtt_receve);
}

void DT_mqtt_loop()
{
    static uint32_t last_reconnection_time = 0;
    static uint32_t reset_time = 0; // for reset network device
    static bool reset = false;      // for reset network device
    uint32_t now = millis();
    if (!mqtt.connected() || Ethernet.linkStatus() == LinkOFF)
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
            Serial.println("reset network board");
            digitalWrite(NETWORK_RESET, LOW);
            last_reconnection_time = now;
            reset = true;
        }
        else if (now - last_reconnection_time > 5000)
        {
            last_reconnection_time = now;
            if (reset)
            {
                Serial.println(F("restart network"));
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
            Serial.println("start MQTT conection");
            //  if (mqtt.connect(clientId.c_str(), "DtBoard", "1MotdePasse"))

            if (Ethernet.linkStatus() == LinkOFF)
            {
                wdt_reset();
                Serial.println(F("Link status: Off"));
            }
            else if (Ethernet.linkStatus() == LinkON && mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_WILL_TOPIC, MQTT_WILL_QOS, MQTT_WILL_RETAIN, MQTT_WILL_MESSAGE))
            {
                wdt_reset();
                Serial.println(F("MQTT Connected"));
                // Once connected, publish an announcement and resubscribe...
                if (_mqtt_subscribe != nullptr)
                    _mqtt_subscribe(mqtt);
                reset_time = 0; // desactivation du compteur de reset
                                // auto Serial.println("MQTT connected");
            }
            else
            { // si echec affichage erreur
                wdt_reset();
                Serial.print(F("ECHEC, rc="));
                Serial.print(mqtt.state());
                Serial.println(F(" nouvelle tentative dans 5 secondes"));
            }
        }

        // delay(50);
        // wdt_enable(WATCHDOG_TIME);
    }
    else
    {
        wdt_reset();
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

#endif // MQTT