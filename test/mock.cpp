#include <mock.h>

using namespace fakeit;

void init_mock()
{
    When(Method(ArduinoFake(Serial), available)).Return(1);
    // virtual size_t println(const __FlashStringHelper *) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const __FlashStringHelper *))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const __FlashStringHelper *))).AlwaysReturn();
    // virtual size_t println(const String &s) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const String &s))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String &s))).AlwaysReturn();
    // virtual size_t println(const char[]) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const char[]))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char[]))).AlwaysReturn();
    // virtual size_t println(char) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(char))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(char))).AlwaysReturn();
    // virtual size_t println(unsigned char, int = DEC) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(unsigned char, int))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(unsigned char, int))).AlwaysReturn();
    // virtual size_t println(int, int = DEC) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(int, int))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(int, int))).AlwaysReturn();
    // virtual size_t println(unsigned int, int = DEC) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(unsigned int, int))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(unsigned int, int))).AlwaysReturn();
    // virtual size_t println(long, int = DEC) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(long, int))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(long, int))).AlwaysReturn();
    // virtual size_t println(unsigned long, int = DEC) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(unsigned long, int))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(unsigned long, int))).AlwaysReturn();
    // virtual size_t println(double, int = 2) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(double, int))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(double, int))).AlwaysReturn();
    // virtual size_t println(const Printable &) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), print, size_t(const Printable &))).AlwaysReturn();
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const Printable &))).AlwaysReturn();
    //    virtual size_t println(void) = 0;
    When(OverloadedMethod(ArduinoFake(Serial), println, size_t())).AlwaysReturn();

    When(Method(ArduinoFake(), randomSeed)).AlwaysReturn();
    When(Method(ArduinoFake(), analogRead)).Return(0);
}