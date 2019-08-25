#include "mqtt.h"

struct mqtt_subscription *ulwi_mqtt_subscriptions = NULL;

struct mqtt_subscription *ulwi_mqtt_get_sub(const char *topic)
{
    struct mqtt_subscription *s;
    HASH_FIND_STR(ulwi_mqtt_subscriptions, topic, s);
    return s;
}

bool ulwi_mqtt_sub_exists(const char *topic)
{
    struct mqtt_subscription *s;
    HASH_FIND_STR(ulwi_mqtt_subscriptions, topic, s);
    if (s != NULL) return true;
    else return false;
}

void mqtt_ev_handler(struct mg_connection *c, int ev, void *p, void *user_data) {
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *) p;

    // if (ev != 0) LOG(LL_DEBUG, ("Global MQTT handler received: %d", ev));
    if (ev == MG_EV_MQTT_CONNACK)
    {
        LOG(LL_INFO, ("CONNACK: %d", msg->connack_ret_code));
    }
    else if (ev == MG_EV_MQTT_UNSUBSCRIBE) /*UNSUBACK is also here*/
    {
        /* Clean up MQTT subscription details if unsubscribing from a channel */
        //HASH_DEL(users, s);
    }
    (void) user_data;
    (void) c;
}

void mqtt_sub_handler(struct mg_connection *nc, const char *topic, int topic_len, const char *msg, int msg_len, void *ud)
{
    /* Free previous string and reset new counter */
    struct mg_str topic_str = mg_strdup_nul(mg_mk_str_n(topic, topic_len));
    struct mqtt_subscription *sub = ulwi_mqtt_get_sub(topic_str.p);
    if (sub == NULL)
    {
        /* NULL pointer checking, clean up heap allocated stuff */
        LOG(LL_ERROR, ("MQTT Topic %s is not found in hash table!!", topic_str.p));
        mg_strfree(&topic_str);
        return;
    }

    mg_strfree(&sub->message);
    sub->new = true;
    sub->active = true;
    sub->message = mg_strdup_nul(mg_mk_str_n(msg, msg_len));
    mg_strfree(&topic_str);
    (void) nc;
    (void) ud;
}

struct sub_data {
    sub_handler_t handler;
    void *user_data;
};

static void mqttsubtrampoline(struct mg_connection *c, int ev, void *ev_data, void *user_data) {
    if (ev != MG_EV_MQTT_PUBLISH) return;
    struct sub_data *sd = (struct sub_data *) user_data;
    struct mg_mqtt_message *mm = (struct mg_mqtt_message *) ev_data;
    sd->handler(c, mm->topic.p, mm->topic.len, mm->payload.p, mm->payload.len, sd->user_data);
}

void ulwi_mqtt_sub(const char *topic, sub_handler_t handler, void *user_data)
{
    struct sub_data *sd = (struct sub_data *) malloc(sizeof(*sd));
    sd->handler = handler;
    sd->user_data = user_data;
    mgos_mqtt_global_subscribe(mg_mk_str(topic), mqttsubtrampoline, sd);

    /* Add subscription to hash table */
    struct mqtt_subscription *sub = malloc(sizeof *sub);
    strlcpy(sub->topic, topic, 128);
    sub->new = false;
    sub->active = false;
    sub->message = mg_strdup_nul(mg_mk_str("")); /* Initialise an empty message here */
    LOG(LL_DEBUG, ("Added MQTT topic %s to hash table", sub->topic));
    HASH_ADD_STR(ulwi_mqtt_subscriptions, topic, sub);
}

bool ulwi_mqtt_unsub(char *topic)
{
    struct mg_connection* nc = mgos_mqtt_get_global_conn();
    const bool sub_exists = ulwi_mqtt_sub_exists(topic);
    if (nc && sub_exists)
    {
        struct mqtt_subscription *s;
        s = ulwi_mqtt_get_sub(topic);

        if (s->message.len > 0) mg_strfree(&s->message);
        HASH_DEL(ulwi_mqtt_subscriptions, s);
        free(s);

        mg_mqtt_unsubscribe(nc, &topic, 1, mgos_mqtt_get_packet_id());
        return true;
    }
    else return false;
}

void ulwi_mqtt_unsub_all()
{
    struct mg_connection* nc = mgos_mqtt_get_global_conn();
    struct mqtt_subscription *current_sub, *tmp;
    char topic_buffer[128];
    char *topic_ptr = topic_buffer;
    HASH_ITER(hh, ulwi_mqtt_subscriptions, current_sub, tmp)
    {
        strlcpy(topic_buffer, current_sub->topic, 128);

        if (current_sub->message.len > 0) mg_strfree(&current_sub->message);
        HASH_DEL(ulwi_mqtt_subscriptions, current_sub);
        free(current_sub);

        mg_mqtt_unsubscribe(nc, &topic_ptr, 1, mgos_mqtt_get_packet_id());
    }
}

bool ulwi_mqtt_new_data_arrived(const char *topic)
{
    struct mqtt_subscription *sub = ulwi_mqtt_get_sub(topic);
    if (sub == NULL) return false;
    return sub->new;
}

struct mg_str *ulwi_mqtt_get_sub_message(const char *topic)
{
    struct mqtt_subscription *sub = ulwi_mqtt_get_sub(topic);
    if (sub == NULL) return NULL;
    sub->new = false;
    return &sub->message;
}
