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

bool (*_mqtt_update)(MQTTClient &mqtt, bool start);
bool (*_mqtt_subscribe)(MQTTClient &mqtt, bool start);
bool (*_mqtt_publish)(bool start);
void (*_mqtt_receve)(MQTTClient *client, const char topic[], const char bytes[], const int length);

String rcv_topic;
String rcv_payload;
String *send_topic;
String *send_payload;
// void (*_mqtt_receve)(char *, uint8_t *, unsigned int);

void DT_mqtt_set_update_callback(bool (*mqtt_update)(MQTTClient &mqtt, bool start))
{
    _mqtt_update = mqtt_update;
}

void DT_mqtt_set_subscribe_callback(bool (*mqtt_subscribe)(MQTTClient &mqtt, bool start))
{
    _mqtt_subscribe = mqtt_subscribe;
}

void DT_mqtt_set_publish_callback(bool (*mqtt_publish)(bool start))
{
    _mqtt_publish = mqtt_publish;
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

    memory(true);
}

bool DT_mqtt_send(const char *topic, const float value)
{
    Serial.print(F("DT_mqtt_send "));
    Serial.print(topic);
    Serial.print(F(" -> "));
    Serial.println(value);
    memory(false);
    debug(AT);
    if (mqtt.connected())
    {
        debug(AT);
        char buffer[32];
        dtostrf(value, 1, 2, buffer);
        memory(false);
        debug_wdt_reset();
        bool ret = mqtt.publish(topic, buffer, strlen(buffer));
        debug(AT);
        memory(false);
        // debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
        debug(AT);
    }
    memory(false);
    return false;
}

bool DT_mqtt_send(const char *topic, const unsigned int value)
{
    Serial.print(F("DT_mqtt_send "));
    Serial.print(topic);
    Serial.print(F(" -> "));
    Serial.println(value);
    memory(false);
    debug(AT);
    if (mqtt.connected())
    {
        debug(AT);
        char buffer[32];
        sprintf(buffer, "%u", value);
        memory(false);
        // debug(AT);
        debug_wdt_reset();
        bool ret = mqtt.publish(topic, buffer, strlen(buffer));
        memory(false);
        debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
        debug(AT);
    }
    memory(false);
    return false;
}

bool DT_mqtt_send(const char *topic, const int value)
{
    Serial.print(F("DT_mqtt_send "));
    Serial.print(topic);
    Serial.print(F(" -> "));
    Serial.println(value);
    memory(false);
    debug(AT);
    if (mqtt.connected())
    {
        debug(AT);
        char buffer[32];
        sprintf(buffer, "%i", value);
        memory(false);
        // debug(AT);
        debug_wdt_reset();
        bool ret = mqtt.publish(topic, buffer, strlen(buffer));
        memory(false);
        debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
        debug(AT);
    }
    memory(false);
    return false;
}

bool DT_mqtt_send(const char *topic, const uint32_t value)
{
    memory(false);
    Serial.print(F("DT_mqtt_send "));
    Serial.print(topic);
    Serial.print(F(" -> "));
    Serial.println(value);
    debug(AT);

    if (mqtt.connected())
    {
        debug(AT);
        char buffer[32];
        sprintf(buffer, "%" PRIu32, value);
        memory(false);
        // debug(AT);
        debug_wdt_reset();
        bool ret = mqtt.publish(topic, buffer, strlen(buffer));
        memory(false);
        debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
        debug(AT);
    }

    memory(false);
    return false;
}

bool DT_mqtt_send(const char *topic, const char *value)
{
    memory(false);
    Serial.print(F("DT_mqtt_send "));
    Serial.print(topic);
    Serial.print(F(" -> "));
    Serial.println(value);

    debug(AT);
    if (mqtt.connected())
    {
        debug(AT);
        memory(false);
        bool ret = mqtt.publish(topic, value, strlen(value));
        debug_wdt_reset();
        memory(false);
        debug(AT);
        return ret;
    }
    else
    {
        mem_config.MQTT_online = false;
        debug(AT);
    }
    memory(false);
    return false;
}

void init_ethernet()
{
    debug(AT);
    memory(true);
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
    // debug(AT);
    // auto Serial.println("start network");
    memory(true);
    pinMode(NETWORK_RESET, OUTPUT);
    digitalWrite(NETWORK_RESET, HIGH);

    // Serial.print(millis());
    Serial.println(F("start network"));
    init_ethernet();
    mqtt.begin(server, 1883, ethClient);
    mqtt.setWill(MQTT_WILL_TOPIC, MQTT_WILL_MESSAGE, MQTT_WILL_RETAIN, MQTT_WILL_QOS);
    mqtt.setTimeout(200);
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
    _mqtt_publish = nullptr,
    //  mqtt.setCallback(&test_mqtt_receve);
        rcv_topic.reserve(32);
    rcv_payload.reserve(16);
    send_topic = nullptr;
    send_payload = nullptr;
    memory(false);
}

void DT_mqtt_loop()
{
    // debug(AT);
    // debug_wdt_reset();
    static uint32_t last_reconnection_time = 0;
    static uint32_t reset_time = 0; // for reset network device
    static bool reset = false;      // for reset network device
    static bool ret_homeassistant = false;
    bool ret = true;
    // static bool old_link_status = false; // for reset network device
    uint32_t now = millis();
    // Serial.println("DT_mqtt_loop start");
    link_status = (Ethernet.linkStatus() == LinkON); // verification de l'etat de connection de la carte reseau

    // Serial.println("DT_mqtt_loop 2");
    if (as_ethernet && (!mqtt.connected() || !link_status)) // si pas de connection reseau ou pas connecté au serveur MQTT
    {
        if (mem_config.MQTT_online)
        {
            mem_config.MQTT_online = false;
            // Serial.println("DT_mqtt_loop 1");
        }

        // wdt_enable(WDTO_8S); // watchdog at 8 secdons
        //  wdt_disable();
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
            debug(AT);
            // Serial.println("DT_mqtt_loop 4");
            last_reconnection_time = now;
            if (reset)
            {
                // Serial.print(millis());
                Serial.println(F("restart network"));
                digitalWrite(NETWORK_RESET, HIGH);
                delay(10);
                debug_wdt_reset();
                init_ethernet();
                // debug_wdt_reset();
                reset_time = 0;
                reset = false;

                // Serial.println("DT_mqtt_loop 5");
            }
            // Attempt to reconnect
            // String clientId = "Board01";

            // Serial.print(millis());
            Serial.print(F("start MQTT connection "));

            if (!link_status)
                Serial.println(F("(Link OFF)"));
            else
                Serial.println(F("(Link ON)"));
            //  if (mqtt.connect(clientId.c_str(), "DtBoard", "1MotdePasse"))

            // if (Ethernet.linkStatus() == LinkOFF)
            // {
            //     debug_wdt_reset();
            //     Serial.println(F("Link status: Off"));
            // }
            // else
            if (as_ethernet && link_status)
            {
                debug_wdt_reset();
                debug(AT);
                // old_link_status = true;
                if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, false))
                {
                    debug(AT);
                    debug_wdt_reset();
                    // Serial.print(millis());
                    Serial.println(F("MQTT connected"));
                    memory(true);
                    // Once connected, publish an announcement and resubscribe...
                    if (_mqtt_subscribe != nullptr)
                        _mqtt_subscribe(mqtt, true);
                    reset_time = 0; // desactivation du compteur de reset
                                    // auto Serial.println("MQTT connected");
                }
                else
                { // si echec affichage erreur
                    // debug_wdt_reset();
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
        // wdt_enable(WATCHDOG_TIME);
    }
    else if (as_ethernet && link_status && mqtt.connected()) // si connecté au serveur MQTT
    {
        // debug_wdt_reset();

        if (reset_time != 0)
            reset_time = 0;

        // Serial.println(F("loop1"));
        mqtt.loop();
        if (_mqtt_receve != nullptr && rcv_topic.length() > 0) // traitement du buffer de reception de donnée
        {
            debug(AT);
            int topic_index = rcv_topic.indexOf(F("|"));
            if (topic_index == -1) // only one data in rcv_topic
            {
                _mqtt_receve(&mqtt, rcv_topic.c_str(), rcv_payload.c_str(), rcv_payload.length());
                rcv_topic = "";
                rcv_payload = "";
            }
            else // many data in rcv_topic
            {
                int payload_index = rcv_payload.indexOf(F("|"));
                _mqtt_receve(&mqtt, rcv_topic.substring(0, topic_index - 1).c_str(), rcv_payload.substring(0, payload_index - 1).c_str(), rcv_payload.substring(0, payload_index - 1).length());
                rcv_topic.remove(0, topic_index + 1);
                rcv_payload.remove(0, payload_index + 1);
            }
        }

        // Serial.println(F("loop2"));
        static uint32_t time = 0;
        if (ret_homeassistant == false)
        {
            // while (mqtt.connected() && mem_config.MQTT_online && ret_homeassistant == false)
            // {
            ret = ret_homeassistant = homeassistant(false);
            // debug_wdt_reset();
            // mqtt.loop();
            // }
        }

        if (_mqtt_subscribe != nullptr && ret)
        {
            ret = _mqtt_subscribe(mqtt, false);
            // debug_wdt_reset();
        }

        if (_mqtt_publish != nullptr && ret)
        {
            ret = _mqtt_publish(false);
            // debug_wdt_reset();
        }

        mem_config.MQTT_online = true; // TODO : ne fonctionne pas si home assistant nes plus en ligne

        if (now - time >= MQTT_UPDATE)
        {
            time = now;
            debug(AT);
            if (_mqtt_update != nullptr)
                _mqtt_update(mqtt, false);
        }
        // delay(100);
    }

    memory(false);
}

#endif // MQTT