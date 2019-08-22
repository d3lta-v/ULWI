#ifndef MQTT_H
#define MQTT_H

#include "mgos.h"
#include "mgos_mqtt.h"

void mqtt_ev_handler(struct mg_connection *c, int ev, void *p, void *user_data);
void mqtt_sub_handler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud);

#endif