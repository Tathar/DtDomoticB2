#ifndef DT_CLOCK
#define DT_CLOCK
#include "RTClib.h" 

class RTCNTP
{
public:
    RTCNTP();
    bool begin();
    bool syncOnce();
    void loop();
    DateTime now();

    void printDateTime(const DateTime &dt);

private:
    RTC_DS1307 _rtc;
    // EthernetUDP &_udp;
    // long _tzOffset;
    unsigned long _lastSync;

    unsigned long getNTPTime();

};

// void DT_cover_set_callback(void (*callback)(const uint8_t num, const int8_t percent, const cover_state state));
extern RTCNTP rtcNtp;

#endif //DT_CLOCK