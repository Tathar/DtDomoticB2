#pragma once

#include <Arduino.h>

inline void debug(const int line, const char *func) __attribute__((always_inline));
inline void debug(const int line, const char *func, const char *data) __attribute__((always_inline));

void debug(const int line, const char *func)
{
    //snprintf_P(buf_debug, BUF_DEBUG_LEN, test, millis(), line, func);
    Serial.print(millis());
    Serial.print(F(";"));
    Serial.print(line);
    Serial.print(F(";"));
    Serial.println(func);
}

void debug(const int line, const char *func, const char *data)
{
    //snprintf_P(buf_debug, BUF_DEBUG_LEN, test, millis(), line, func);
    Serial.print(millis());
    Serial.print(F(";"));
    Serial.print(line);
    Serial.print(F(";"));
    Serial.print(func);
    Serial.print(F(";"));
    Serial.println(data);
}