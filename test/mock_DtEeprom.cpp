
#include <DT_eeprom.h>
#include <ArduinoFake.h>

void chargeEEPROM()
{
    uint8_t i = 0; //for loop

    // Détection d'une mémoire non initialisée
    byte erreur = eeprom_config.magic != STRUCT_MAGIC;

    // Valeurs par défaut struct_version == 1
    if (eeprom_config.struct_version < 1 || erreur)
    {
        eeprom_config.struct_version = 1;
    }
};

void sauvegardeEEPROM()
{
    printf("sauvegardeEEPROM()\n");
}