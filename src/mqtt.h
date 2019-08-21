#ifndef MQTT_H
#define MQTT_H

#include "mgos.h"
#include "mgos_mqtt.h"

void mqtt_ev_handler(struct mg_connection *c, int ev, void *p, void *user_data);

#endif