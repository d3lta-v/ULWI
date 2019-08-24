#ifndef MQTT_H
#define MQTT_H

#include "mgos.h"
#include "mgos_mqtt.h"
#include "uthash.h"

// struct mg_str mqtt_subscribed_topics[3];        /* Array of strings containing topics for MQTT subscriptions */
// struct mg_str mqtt_subscribed_messages[3];      /* Array of strings containing messages for MQTT subscriptions. Only caches the latest  */
// bool mqtt_subscribed_active[3] = { false };     /* Boolean flag for if the current MQTT subscription is active */
// bool mqtt_subscribed_new[3] = { false };        /* Boolean flag for indicating whether the MQTT subscription has new data inbound */

struct mqtt_subscription
{
    char topic[128];        /* Topic of the MQTT subscription which also acts as the key */
    struct mg_str message;  /* Message of the MQTT subscription. Needs to be freed manually!!! Please guarentee that it will be alloc'd upon struct creation */
    bool new;               /* Boolean flag for indicating whether the MQTT subscription has new data inbound */
    bool active;            /* Boolean flag for indicating whether this subscription is active (i.e. after the first message is received) */
    UT_hash_handle hh;
};

void mqtt_ev_handler(struct mg_connection *c, int ev, void *p, void *user_data);
void mqtt_sub_handler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud);

void ulwi_mqtt_sub(const char *topic, sub_handler_t handler, void *user_data);
bool ulwi_mqtt_unsub(char *topic);
void ulwi_mqtt_unsub_all();
bool ulwi_mqtt_sub_exists(const char *topic);

#endif