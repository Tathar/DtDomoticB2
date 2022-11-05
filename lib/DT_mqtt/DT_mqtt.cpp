#include <DT_mqtt.h>

#include <DT_eeprom.h>
#include <DT_ha.h>
// #include <DT_buffer.h>
#include <CircularBuffer.h>

#define add0x2(s) 0x##s
#define toHEX(s) add0x2(s)

#ifdef MQTT

// Update these with values suitable for your hardware/network.
IPAddress server(MQTT_IP1, MQTT_IP2, MQTT_IP3, MQTT_IP4);
EthernetClient ethClient;
MQTTClient mqtt(512);
bool as_ethernet = false;
bool link_status;

bool (*_mqtt_update)(MQTTClient &mqtt, bool start);
bool (*_mqtt_subscribe)(MQTTClient &mqtt, bool start);
bool (*_mqtt_publish)(bool start);
void (*_mqtt_receve)(MQTTClient *client, const char topic[], const char bytes[], const int length);

// DT_buffer<MQTT_data> send_buffer;
// DT_buffer<MQTT_recv_msg> recv_buffer;
CircularBuffer<MQTT_data, 10> send_buffer;
CircularBuffer<MQTT_recv_msg, 10> recv_buffer;

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
    debug(F(AT));
    MQTT_recv_msg recv(topic, bytes, length);
    // debug(F(AT));
    recv_buffer.unshift(recv);
    // debug(F(AT));
    memory(true);
}

void init_ethernet()
{
    // 220502  debug(F(AT));
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
    }

    if (as_ethernet && Ethernet.linkStatus() == Unknown)
    {
        Serial.println(F("Link status unknown. Link status detection is only available with W5200 and W5500."));
    }
    else if (as_ethernet && Ethernet.linkStatus() == LinkON)
    {
        Serial.println(F("Link status: On"));
    }
    else if (as_ethernet && Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println(F("Link status: Off"));
    }
}

void DT_mqtt_init()
{
    // debug(AT);
    // auto Serial.println("start network");
    mem_config.MQTT_online = false;
    memory(true);
    pinMode(NETWORK_RESET, OUTPUT);
    digitalWrite(NETWORK_RESET, HIGH);

    Serial.println(F("start network"));
    init_ethernet();
    mqtt.begin(server, 1883, ethClient);
    mqtt.setWill(MQTT_WILL_TOPIC, MQTT_WILL_MESSAGE, MQTT_WILL_RETAIN, MQTT_WILL_QOS);
    // mqtt.setTimeout(1000);
    mqtt.setTimeout(250);
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
    _mqtt_publish = nullptr;
    //  mqtt.setCallback(&test_mqtt_receve);
    // send_buffer.reserve(6);
    // send_buffer.reserve(2);
    // rcv_topic.reserve(32);
    // rcv_payload.reserve(16);
    // send_buffer = nullptr;
    // send_buffer_len = 0;
    // send_buffer_read = 0;
    // send_buffer_write = 0;
    memory(false);
}

void DT_mqtt_loop()
{
    // debug(AT);
    // debug_wdt_reset();
    // wdt_reset();
    static uint32_t last_reconnection_time = 0;
    static uint32_t reset_time = 0; // for reset network device
    static bool reset = false;      // for reset network device
    bool ret = true;
    static uint32_t time = 0;
    // static bool old_link_status = false; // for reset network device
    uint32_t now = millis();
    // Serial.println("DT_mqtt_loop start");

    // Serial.println("DT_mqtt_loop 2");
    if (as_ethernet)
    {
        link_status = (Ethernet.linkStatus() == LinkON); // verification de l'etat de connection de la carte reseau
        if ((!mqtt.connected() || !link_status))         // si pas de connection reseau ou pas connecté au serveur MQTT
        {

            static uint32_t keep_alive_timout = 0;
            if (mem_config.MQTT_online)
            {
                mem_config.MQTT_online = false;
                keep_alive_timout = now;
                // Serial.println("DT_mqtt_loop 1");
            }

            // nettoyage du buffer d'envois si deconnecté depusi plus de 1 seconde
            if (now - keep_alive_timout > 1000)
            {
                send_buffer.clear();
                // send_buffer.reserve(6);
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
                Serial.println(F("reset network board"));
                digitalWrite(NETWORK_RESET, LOW);
                last_reconnection_time = now;
                reset = true;
                reset_time = now;
            }
            else if (now - last_reconnection_time > 5000)
            {
                // 220502  debug(F(AT));
                last_reconnection_time = now;
                if (reset && now - reset_time > 60000)
                {
                    Serial.println(F("restart network"));
                    digitalWrite(NETWORK_RESET, HIGH);
                    delay(10);
                    // debug_wdt_reset(F(AT));
                    // wdt_reset();
                    // debug_wdt_reset();
                    init_ethernet();
                    reset_time = 0;
                    reset = false;
                }
                else if (reset)
                {
                    return;
                }
                // Attempt to reconnect

                Serial.print(F("start MQTT connection "));

                if (!link_status)
                    Serial.println(F("(Link OFF)"));
                else
                    Serial.println(F("(Link ON)"));

                if (as_ethernet && link_status)
                {
                    // wdt_enable(WDTO_8S);
                    //  debug_wdt_reset(F(AT));
                    // wdt_reset();
                    //  debug_wdt_reset();
                    //  220502  debug(F(AT));
                    if (!mqtt.connected() && mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, false))
                    {
                        // wdt_enable(WDTO_8S);
                        //  debug_wdt_reset(F(AT));
                        // wdt_reset();
                        //  debug_wdt_reset();
                        //  220502  debug(F(AT));
                        reset_time = 0;
                        // 220502  debug(F(AT));
                        Serial.println(F("MQTT connected"));
                        mem_config.MQTT_online = true; // TODO : ne fonctionne pas si home assistant nes plus en ligne
                        memory(true);
                        // Once connected, publish an announcement and resubscribe...
                        if (_mqtt_subscribe != nullptr)
                            _mqtt_subscribe(mqtt, true);
                        reset_time = 0; // desactivation du compteur de reset

                        // debug(AT);
                    }
                    else
                    { // si echec affichage erreur
                        Serial.print(F(" ECHEC, rc="));
                        Serial.print(mqtt.returnCode());
                        Serial.print(F(" le="));
                        Serial.print(mqtt.lastError());
                        Serial.println(F(" nouvelle tentative dans 5 secondes"));
                    }
                }
                Serial.print(F("mqtt elapse time = "));
                Serial.println(millis() - now);
                // debug(AT);
            }
            // debug_wdt_reset(F(AT));
            // wdt_reset();
            // debug_wdt_reset();
            // wdt_enable(WATCHDOG_TIME);
        }
        else if (as_ethernet && link_status && mqtt.connected()) // si connecté au serveur MQTT
        {

            // wdt_reset();
            if (reset_time != 0)
                reset_time = 0;
            mqtt.loop();
            // 220502  debug(F(AT));
            if (_mqtt_receve != nullptr && recv_buffer.size() > 0) // traitement du buffer de reception de donnée
            {
                debug(F(AT));
                MQTT_recv_msg recv = recv_buffer.pop();
                _mqtt_receve(&mqtt, recv._topic, recv._payload, recv._length);
                recv.clean();
                // send_buffer.clean(2);
            }
            else if (send_buffer.size() > 0)
            {
                // 220502  debug(F(AT));
                char topic[MAX_TOPIC];
                char payload[MAX_PAYLOAD];
                MQTT_data data = send_buffer.shift();
                data.get(topic, 64, payload, MAX_PAYLOAD);
                Serial.print(F("send "));
                Serial.print(topic);
                Serial.print(F(" = "));
                Serial.println(payload);
                uint16_t len = strlen(payload);
                if (len < MAX_PAYLOAD)
                {
                    mqtt.publish(topic, payload, len);
                    // debug(F(AT));
                }
                else
                {
                    debug(F(AT));
                    Serial.print(F("len > MAX_PAYLOAD len = "));
                    Serial.println(len);
                }

                Serial.print(F("send buffer size = "));
                Serial.println(send_buffer.size());
                // }
                // Serial.print(F("Buffer capacity = "));
                // Serial.println(send_buffer.capacity());
                // Serial.print(F("Buffer size = "));
                // Serial.println(send_buffer.size());
                // Serial.print(F("Buffer available = "));
                // Serial.println(send_buffer.available());
                // if (send_buffer.size() == 0)
                // {
                // send_buffer.clean(6);
                // }
                // debug(F(AT));
            }
            else if (mem_config.HA_MQTT_CONFIG == false)
            {
                // 220502  debug(F(AT));
                mem_config.HA_MQTT_CONFIG = homeassistant(false);
                // ret_homeassistant = true;
                // 220502  debug(F(AT));
            }
            else
            {
                static uint8_t choix = 0;
                switch (choix++)
                {
                case 0:
                    if (_mqtt_subscribe != nullptr)
                    {
                        // 220502  debug(F(AT));
                        _mqtt_subscribe(mqtt, false);
                        // 220502  debug(F(AT));
                    }
                    break;
                case 1:
                    if (_mqtt_publish != nullptr)
                    {
                        // 220502  debug(F(AT));
                        _mqtt_publish(false);
                        // 220502  debug(F(AT));
                    }
                    break;
                case 2:
                    if (now - time >= MQTT_UPDATE)
                    {
                        time = now;
                        // 220502  debug(F(AT));
                        if (_mqtt_update != nullptr)
                            _mqtt_update(mqtt, false);
                        // 220502  debug(F(AT));
                    }
                    break;

                default:
                    break;
                }
                if (choix == 3)
                    choix = 0;
            }

            mem_config.MQTT_online = true; // TODO : ne fonctionne pas si home assistant nes plus en ligne
        }
    }

    memory(false);
}

#endif // MQTT