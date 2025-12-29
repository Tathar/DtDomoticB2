#ifndef _ECS_RAPH
#define _ECS_RAPH

#include <config.h>
#include <DT_eeprom.h>

#ifdef ECS_RAPH

struct MQTTClient;



void DT_ECS_RAPH_init();
void DT_ECS_RAPH_loop();

DT_ECS_RAPH_mode DT_ECS_RAPH_get_mode();
void DT_ECS_RAPH_set_mode(DT_ECS_RAPH_mode mode);
void DT_ECS_RAPH_mode_callback(const DT_ECS_RAPH_mode mode);

float DT_ECS_RAPH_get_init_consigne();
void DT_ECS_RAPH_set_init_consigne(float cons);
void DT_ECS_RAPH_init_consigne_callback(float cons);

uint8_t DT_ECS_RAPH_get_nb_personne();
void DT_ECS_RAPH_set_nb_personne(uint8_t nb);
void DT_ECS_RAPH_nb_personne_callback(uint8_t nb);

float DT_ECS_RAPH_get_coef();
void DT_ECS_RAPH_set_coef(float coef);
void DT_ECS_RAPH_coef_callback(float coef);

float DT_ECS_RAPH_get_secours();
void DT_ECS_RAPH_set_secours(float coef);
void DT_ECS_RAPH_secours_callback(float coef);

float DT_ECS_RAPH_get_consigne();
void DT_ECS_RAPH_consigne_callback(float cons);

bool DT_ECS_RAPH_mqtt_subscribe(MQTTClient &mqtt, uint8_t sequance);
bool DT_ECS_RAPH_mqtt_receve(const char topic[], const char buffer[]);
bool DT_ECS_RAPH_mqtt_publish(uint8_t sequance);


#endif //ECS_RAPH
#endif //_ECS_RAPH