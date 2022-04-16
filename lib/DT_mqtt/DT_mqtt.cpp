#include <DT_mqtt.h>

#include <avr/wdt.h> //watchdog
#include <DT_eeprom.h>
#include <DT_ha.h>

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
MQTTClient mqtt(512);
bool as_ethernet;
bool link_status;

void (*_mqtt_update)(MQTTClient &mqtt);
void (*_mqtt_subscribe)(MQTTClient &mqtt);
void (*_mqtt_receve)(MQTTClient *client, const char topic[], const char bytes[], const int length);

String rcv_topic;
String rcv_payload;
// void (*_mqtt_receve)(char *, uint8_t *, unsigned int);

void DT_mqtt_set_update_callback(void (*mqtt_update)(MQTTClient &mqtt))
{
    _mqtt_update = mqtt_update;
}

void DT_mqtt_set_subscribe_callback(void (*mqtt_subscribe)(MQTTClient &mqtt))
{
    _mqtt_subscribe = mqtt_subscribe;
}

void DT_mqtt_set_receve_callback(void (*mqtt_receve)(MQTTClient *client, const char topic[], const char bytes[], const int length))
{
    _mqtt_receve = mqtt_receve;
}

void DT_receve_callback(MQTTClient *client, char topic[], char bytes[], int length)
{
    if (rcv_topic.length() > 0)
    {
        rcv_topic += F("|");
        rcv_topic += topic;
        rcv_payload += F("|");
        rcv_payload += bytes;
    }
    else
    {
        rcv_topic = topic;
        rcv_payload = bytes;
    }

    memory();
}

bool DT_mqtt_send(const char *tag, const float value)
{
    debug(AT);
    if (mqtt.connected())
    {
        debug(AT);
        char buffer[32];
        dtostrf(value, 1, 2, buffer);
        memory();
        // Serial.println(buffer);
        debug(AT);
        bool ret = mqtt.publish(tag, buffer, strlen(buffer));
        memory();
        debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
    }
    return false;
}

bool DT_mqtt_send(const char *tag, const unsigned int value)
{
    debug(AT);
    if (mqtt.connected())
    {
    debug(AT);
        char buffer[32];
        sprintf(buffer, PSTR("%u"), value);
        memory();
    debug(AT);
        bool ret = mqtt.publish(tag, buffer, strlen(buffer));
        memory();
    debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
    }
    return false;
}

bool DT_mqtt_send(const char *tag, const int value)
{
    debug(AT);
    if (mqtt.connected())
    {
        char buffer[32];
        sprintf(buffer, PSTR("%i"), value);
    debug(AT);
        memory();
        bool ret = mqtt.publish(tag, buffer, strlen(buffer));
        memory();
    debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
    }
    return false;
}

bool DT_mqtt_send(const char *tag, const uint32_t value)
{
    debug(AT);
    if (mqtt.connected())
    {
        char buffer[32];
        sprintf(buffer, "%" PRIu32, value);
    debug(AT);
        memory();
        bool ret = mqtt.publish(tag, buffer, strlen(buffer));
        memory();
    debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
    }
    return false;
}

bool DT_mqtt_send(const char *tag, const char *value)
{
    debug(AT);
    if (mqtt.connected())
    {
        Serial.println(value);
        memory();
        debug(AT);
        bool ret = mqtt.publish(tag, value, strlen(value));
        debug(AT);
        memory();
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
    }
    debug(AT);
    return false;
}

void init_ethernet()
{
    debug(AT);
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
        as_ethernet = false;
    }
    else
    {
        as_ethernet = true;
        if (Ethernet.hardwareStatus() == EthernetW5100)
        {

            // Serial.print(millis());
            Serial.println(F("W5100 Ethernet controller detected."));
        }
        else if (Ethernet.hardwareStatus() == EthernetW5200)
        {
            // Serial.print(millis());
            Serial.println(F("W5200 Ethernet controller detected."));
        }
        else if (Ethernet.hardwareStatus() == EthernetW5500)
        {
            // Serial.print(millis());
            Serial.println(F("W5500 Ethernet controller detected."));
        }
    }

    if (as_ethernet && Ethernet.linkStatus() == Unknown)
    {
        // Serial.print(millis());
        Serial.println(F("Link status unknown. Link status detection is only available with W5200 and W5500."));
    }
    else if (as_ethernet && Ethernet.linkStatus() == LinkON)
    {
        // Serial.print(millis());
        Serial.println(F("Link status: On"));
    }
    else if (as_ethernet && Ethernet.linkStatus() == LinkOFF)
    {
        // Serial.print(millis());
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
    debug(AT);
    // auto Serial.println("start network");
    pinMode(NETWORK_RESET, OUTPUT);
    digitalWrite(NETWORK_RESET, HIGH);

    // Serial.print(millis());
    Serial.println(F("start network"));
    init_ethernet();
    mqtt.begin(server, 1883, ethClient);
    mqtt.setWill(PSTR(MQTT_WILL_TOPIC), PSTR(MQTT_WILL_MESSAGE), MQTT_WILL_RETAIN, MQTT_WILL_QOS);
    mqtt.setTimeout(100);
    mqtt.onMessageAdvanced(DT_receve_callback);
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
    rcv_topic.reserve(32);
    rcv_payload.reserve(16);
    memory();
}

void DT_mqtt_loop()
{
    // debug(AT);
    wdt_reset();
    static uint32_t last_reconnection_time = 0;
    static uint32_t reset_time = 0; // for reset network device
    static bool reset = false;      // for reset network device
    // static bool old_link_status = false; // for reset network device
    uint32_t now = millis();
    // Serial.println("DT_mqtt_loop start");
    link_status = (Ethernet.linkStatus() == LinkON);

    // if (old_link_status && !link_status)
    // {
    //     Serial.println("reset network board");
    //     digitalWrite(NETWORK_RESET, LOW);
    //     delay(500);
    //     Serial.println(F("restart network"));
    //     digitalWrite(NETWORK_RESET, HIGH);
    //     delay(10);
    //     wdt_reset();
    //     init_ethernet();
    //     wdt_reset();
    //     reset_time = 0;
    //     reset = false;
    //     old_link_status = link_status = false;
    // }

    // Serial.println("DT_mqtt_loop 2");
    if (as_ethernet && (!mqtt.connected() || !link_status))
    {
        if (mem_config.MQTT_online)
        {
            mem_config.MQTT_online = false;
            // Serial.println("DT_mqtt_loop 1");
        }

        wdt_enable(WDTO_8S); // watchdog at 8 secdons
        // wdt_disable();
        if (reset_time == 0)
        {
            // Serial.println("DT_mqtt_loop 2");
            reset_time = now;
        }
        else if (reset_time != 0 && !reset && now - reset_time > NETWORK_RESET_TIME)
        {
            // Serial.print(millis());
            Serial.println(F("reset network board"));
            digitalWrite(NETWORK_RESET, LOW);
            last_reconnection_time = now;
            reset = true;
            // Serial.println("DT_mqtt_loop 3");
        }
        else if (now - last_reconnection_time > 5000)
        {
            // Serial.println("DT_mqtt_loop 4");
            last_reconnection_time = now;
            if (reset)
            {
                // Serial.print(millis());
                Serial.println(F("restart network"));
                digitalWrite(NETWORK_RESET, HIGH);
                delay(10);
                wdt_reset();
                init_ethernet();
                wdt_reset();
                reset_time = 0;
                reset = false;

                // Serial.println("DT_mqtt_loop 5");
            }
            // Attempt to reconnect
            // String clientId = "Board01";

            // Serial.print(millis());
            Serial.print(F("start MQTT conection "));

            if (!link_status)
                Serial.println(F("(Link OFF)"));
            else
                Serial.println(F("(Link ON)"));
            //  if (mqtt.connect(clientId.c_str(), "DtBoard", "1MotdePasse"))

            // if (Ethernet.linkStatus() == LinkOFF)
            // {
            //     wdt_reset();
            //     Serial.println(F("Link status: Off"));
            // }
            // else
            if (as_ethernet && link_status)
            {
                // old_link_status = true;
                if (mqtt.connect(PSTR(MQTT_CLIENT_ID), PSTR(MQTT_USER), PSTR(MQTT_PASSWORD), false))
                {
                    wdt_reset();
                    // Serial.print(millis());
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
                    // Serial.print(millis());
                    Serial.print(F(" ECHEC, rc="));
                    Serial.print(mqtt.returnCode());
                    Serial.print(F(" le="));
                    Serial.print(mqtt.lastError());
                    Serial.println(F(" nouvelle tentative dans 5 secondes"));
                }
            }
            // Serial.print(millis());
            Serial.print(F("mqtt elapse time = "));
            Serial.println(millis() - now);
        }

        // delay(50);
        wdt_enable(WATCHDOG_TIME);
    }
    else if (as_ethernet && link_status && mqtt.connected())
    {
        if (reset_time != 0)
            reset_time = 0;
        wdt_reset();
        // Serial.println(F("loop1"));
        mqtt.loop();
        if (_mqtt_receve != nullptr && rcv_topic.length() > 0)
        {
            int topic_index = rcv_topic.indexOf(PSTR("|"));
            if (topic_index == -1) // only one data in rcv_topic
            {
                _mqtt_receve(&mqtt, rcv_topic.c_str(), rcv_payload.c_str(), rcv_payload.length());
                rcv_topic = "";
                rcv_payload = "";
            }
            else // many data in rcv_topic
            {
                int payload_index = rcv_payload.indexOf(PSTR("|"));
                _mqtt_receve(&mqtt, rcv_topic.substring(0, topic_index - 1).c_str(), rcv_payload.substring(0, payload_index - 1).c_str(), rcv_payload.substring(0, payload_index - 1).length());
                rcv_topic.remove(0, topic_index + 1);
                rcv_payload.remove(0, payload_index + 1);
            }
        }
        // Serial.println(F("loop2"));
        static uint32_t time = 0;
        homeassistant(false);
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

    memory();
}

#endif // MQTT