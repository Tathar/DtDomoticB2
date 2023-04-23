#include <DT_teleinfo.h>

#ifdef TIC
#include <LibTeleinfo.h>

void (*_DT_teleinfo_callback)(const char *name, const char *value);
TInfo tinfo; // Teleinfo object


/* ======================================================================
Function: UpdatedFrame
Purpose : callback when we received a complete teleinfo frame
Input   : linked list pointer on the concerned data
Output  : -
Comments: it's called only if one data in the frame is different than
          the previous frame
====================================================================== */
void UpdatedFrame(ValueList *me)
{

    // Envoyer les val
    if (me)
    {
        // Loop thru the node
        while (me->next)
        {
            // go to next node
            me = me->next;

            // uniquement sur les nouvelles valeurs ou celles modifiées
            // sauf si explicitement demandé toutes
            if (me->flags & (TINFO_FLAGS_UPDATED | TINFO_FLAGS_ADDED))
            {
                // First elemement, no comma

                Serial.print(F("Telereleve = "));
                Serial.print(me->name);
                Serial.print(F(" -> "));
                Serial.println(me->value);
                if (_DT_teleinfo_callback != nullptr)
                {
                    _DT_teleinfo_callback(me->name, me->value);
                }
            }
        }
    }
}


void DT_teleinfo_init()
{

    Serial.println(F("starting TIC"));
    _DT_teleinfo_callback = nullptr;

    Serial3.begin(1200);
    // Serial3.begin(9600);

    // Init teleinfo
    tinfo.init();

    // Attacher les callback dont nous avons besoin
    // pour cette demo, ADPS et TRAME modifiée
    tinfo.attachUpdatedFrame(UpdatedFrame);
}

void DT_teleinfo_loop()
{
    char c;
    // On a reçu un caractère ?
    if (Serial3.available())
    {
        // Le lire
        c = Serial3.read();
        Serial.print(F("Telereleve -> "));
        Serial.println(c);
        // Gerer
        tinfo.process(c);
    }
}


void DT_teleinfo_set_callback(void (*callback)(const char *name, const char *value))
{
    _DT_teleinfo_callback = callback;
}
#endif //TIC