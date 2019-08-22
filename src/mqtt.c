#include "mqtt.h"

void mqtt_ev_handler(struct mg_connection *c, int ev, void *p, void *user_data) {
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

    if (ev == MG_EV_MQTT_CONNACK)
    {
        LOG(LL_INFO, ("CONNACK: %d", msg->connack_ret_code));
    }
    else if (ev == MG_EV_MQTT_SUBACK)
    {
        LOG(LL_INFO, ("Subscription %u acknowledged", msg->message_id));
    }
    (void) user_data;
    (void) c;
}

void mqtt_sub_handler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud)
{
    struct mg_str topic_str = mg_strdup_nul(mg_mk_str_n(topic, topic_len));
    struct mg_str msg_str = mg_strdup_nul(mg_mk_str_n(msg, msg_len));
    LOG(LL_INFO, ("Topic: %s", topic_str.p));
    LOG(LL_INFO, ("Message: %s", msg_str.p));
    mg_strfree(&topic_str);
    mg_strfree(&msg_str);
    (void) nc;
    (void) ud;
    (void) topic_len;
    (void) msg_len;
}
