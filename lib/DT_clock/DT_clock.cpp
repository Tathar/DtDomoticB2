#include "pinout.h"
#include "config.h"
#include "DT_clock.h"
#include "DT_eeprom.h"
#include "DT_mqtt.h"

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Dns.h"
#include <Wire.h>

#ifdef CLOCK

RTCNTP::RTCNTP()
    : _lastSync(0) {}

bool RTCNTP::begin()
{
  Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
  Wire.write(i2c_channel_to_multiplexer(1));
  Wire.endTransmission();

  if (!_rtc.begin())
  {
    Serial.println("RTC DS1307 non detectee");
    return false;
  }
  if (!_rtc.isrunning())
  {
    Serial.println("RTC DS1307 n'est pas en cours d'execution (verifier pile)");
  }
  return true;
}

bool RTCNTP::syncOnce()
{
  if (as_ethernet)
  {

    unsigned long epoch = getNTPTime();
    if (epoch == 0)
    {
      Serial.println("Echec recup NTP");
      return false;
    }

    DateTime dt((uint32_t)epoch);

    Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
    Wire.write(i2c_channel_to_multiplexer(1));
    Wire.endTransmission();

    _rtc.adjust(dt);
    _lastSync = millis();
    printDateTime(dt);
    return true;
  }
  else
  {
    return false;
  }
}

void RTCNTP::loop()
{
  if (millis() - _lastSync >= NTP_SYNC_INTERVAL)
  {
    _lastSync = millis();
    syncOnce();
  }

  // static unsigned long old = 0;
  // if (millis() - old >= 5000)
  // {
  //   old = millis();
  //   printDateTime(now());
  // }
}

DateTime RTCNTP::now()
{
  Wire.beginTransmission(I2C_MULTIPLEXER_ADDRESS); // change I2C channel
  Wire.write(i2c_channel_to_multiplexer(1));
  Wire.endTransmission();
  DateTime dt = _rtc.now();
  DateTime ret;

  DateTime d1 = DateTime(dt.year(), CLOCK_MONTH_1, CLOCK_DAY_1, CLOCK_HOUR_1, 0);
  int day = d1.dayOfTheWeek();
  if (day != 0)
  {
    d1 = DateTime(dt.year(), CLOCK_MONTH_1, CLOCK_DAY_1 - day, CLOCK_HOUR_1, 0);
  }

  DateTime d2 = DateTime(dt.year(), CLOCK_MONTH_2, CLOCK_DAY_2, CLOCK_HOUR_2, 0);
  day = d2.dayOfTheWeek();
  if (day != 0)
  {
    d2 = DateTime(dt.year(), CLOCK_MONTH_2, CLOCK_DAY_2 - day, CLOCK_HOUR_2, 0);
  }

  if (dt > d1 && dt < d2)
  {
    Serial.println("UTC+2");
    return dt + CLOCK_OFSFET_1;
  }
  else
  {
    Serial.println("UTC+1");
    return dt + CLOCK_OFSFET_2;
  }

  // return dt; // TODO: heure hiver/ete
}

unsigned long RTCNTP::getNTPTime()
{
  const int NTP_PACKET_SIZE = 48;
  byte packetBuffer[NTP_PACKET_SIZE];
  IPAddress ntpIP;
  DNSClient dns;
  dns.begin(Ethernet.dnsServerIP());

  if (!dns.getHostByName(NTP_SERVER, ntpIP))
  {
    Serial.println("Impossible de resoudre le serveur NTP");
    ntpIP = IPAddress(172, 232, 44, 156);
  }
  else
  {
    Serial.println("resolution NTP OK");
  }

  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;

  EthernetUDP _udp;
  _udp.begin(NTP_LOCAL_PORT);
  _udp.beginPacket(ntpIP, 123);
  _udp.write(packetBuffer, NTP_PACKET_SIZE);
  _udp.endPacket();

  unsigned long start = millis();
  while (millis() - start < 3000)
  {
    int size = _udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      _udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = (highWord << 16) | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      _udp.stop();
      return secsSince1900 - seventyYears;
    }
    delay(10);
  }
  _udp.stop();
  return 0;
}

void RTCNTP::printDateTime(const DateTime &dt)
{
  char buf[30];
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
           dt.year(), dt.month(), dt.day(),
           dt.hour(), dt.minute(), dt.second());
  // Serial.print("RTC regle a: ");
  Serial.println(buf);
}

void RTCNTP::ToDateTime(const DateTime &dt, char *buf, uint8_t len)
{
  snprintf(buf, len, "%04u-%02u-%02u %02u:%02u:%02u",
           dt.year(), dt.month(), dt.day(),
           dt.hour(), dt.minute(), dt.second());
}

RTCNTP rtcNtp;

#endif