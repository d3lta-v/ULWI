#ifndef HTTP_H
#define HTTP_H

#include "mgos.h"

#define HTTP_TX_CONTENT_MAX 256
#define HTTP_RX_CONTENT_MAX 512

struct http_request
{
    char method;                        /* HTTP method of the request, G for GET, P for POST and so on */
    char url[HTTP_TX_CONTENT_MAX];      /* Full URL of the request */
    char params[HTTP_TX_CONTENT_MAX];   /* Parameters of the request, usually part of the URL */
    char content[HTTP_TX_CONTENT_MAX];  /* Content of the request */
    char headers[HTTP_TX_CONTENT_MAX];  /* HTTP headers of the request */
};

enum request_progress
{
    NONEXISTENT,    /* This request does not exist at all */
    IN_PROGRESS,    /* This request is being processed */
    SUCCESS,        /* Successful request */
    FAILED          /* Failed request */
};

struct state
{
    enum request_progress progress;              /* Current progress of the request */
    int status;                         /* Request status (may be HTTP status as well) */
    int64_t written;                    /* Number of bytes written */
    char content[HTTP_RX_CONTENT_MAX];  /* Content of the HTTP response */
};

// void http_cb(struct mg_connection *c, int ev, void *ev_data, void *ud);
void ev_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data));

void empty_state(struct state *s);
void empty_request(struct http_request *r);

#endif