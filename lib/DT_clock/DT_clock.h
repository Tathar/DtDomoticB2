#ifndef DT_CLOCK
#define DT_CLOCK
#include <RTClib.h>

class RTCNTP
{
public:
    RTCNTP();
    bool begin();
    bool syncOnce();
    void loop();
    DateTime now();
    uint32_t ToD();

    void printDateTime(const DateTime &dt);
    void ToDateTime(const DateTime &dt, char *buf, uint8_t len);
    void start_date_time(char *buf, uint8_t len);

    bool mqtt_publish(uint8_t sequance);

private:
    RTC_DS1307 _rtc;
    bool rtc_ok = false;
    DateTime old_date_time;
    uint32_t old_date_time_millis;
    // EthernetUDP &_udp;
    // long _tzOffset;
    unsigned long _lastSync;
    DateTime _started; // date de demmarage

    unsigned long getNTPTime();
};

// void DT_cover_set_callback(void (*callback)(const uint8_t num, const int8_t percent, const cover_state state));
extern RTCNTP rtcNtp;

#endif // DT_CLOCK