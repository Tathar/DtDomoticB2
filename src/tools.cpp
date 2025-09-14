#include <config.h>
#include <tools.h>
#include <avr/wdt.h> //watchdog
#include <DT_eeprom.h>

void init_wdt()
{
// debug_str.reserve(64);
#ifdef WATCHDOG_TIME
  // WDTCSR = (1 << WDCE) | (1 << WDE);                 // Déverrouille les bits de configuration du Watchdog (procédure spécifiée par le fabricant)
  // WDTCSR = WATCHDOG_TIME | (1 << WDE) | (1 << WDIE); // Mise à 1 des bits WDIE, WDE, WDP2,WDP1 pour activer les interruption puis le reset toutes les 1 secondes
  wdt_enable(WATCHDOG_TIME);
#endif
}

int i2c_channel_to_multiplexer(int channel)
{
  if (channel == 1)
    return 0b10;
  else if (channel == 2)
    return 0b1;
  else
    return 1 << (channel - 1);
}

volatile uint32_t watchdog_reset = 0;
volatile uint32_t old_call = 0;
String debug_str;

void debug(const char *var)
{
  // static uint32_t old_call = 0;
  uint32_t now = millis();
  debug_str = var;
  debug_str += F(",");
  debug_str += (now - watchdog_reset);
  debug_str += F(",");
  debug_str += (now - old_call);
  // Serial.println(debug_str);
  old_call = now;
}

void debug(const __FlashStringHelper *var)
{
  // static uint32_t old_call = 0;
  uint32_t now = millis();
  debug_str = var;
  debug_str += F(",");
  debug_str += (now - watchdog_reset);
  debug_str += F(",");
  debug_str += (now - old_call);
  // Serial.println(debug_str);
  old_call = now;
}

void debug_wdt_reset(void)
{
#ifdef WATCHDOG_TIME
  wdt_reset();
  __asm__("nop\n\t");
  watchdog_reset = millis();
#endif
}

void debug_wdt_reset(const char *var)
{
  // debug(var);
#ifdef WATCHDOG_TIME
  wdt_reset();
  watchdog_reset = millis();
#endif
}

void debug_wdt_reset(const __FlashStringHelper *var)
{
  // debug(var);
#ifdef WATCHDOG_TIME
  wdt_reset();
  watchdog_reset = millis();
#endif
}

// =========================================
// Routine d'interruption (liée au watchdog)
// =========================================
// ISR(WDT_vect) {
//  strncpy(eeprom_config.debug_str,debug_str.c_str(),64);
//  sauvegardeEEPROM(); fait planté
//}
