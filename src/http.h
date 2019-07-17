#ifndef HTTP_H
#define HTTP_H

#include "mgos.h"

#define HTTP_TX_CONTENT_MAX 256
#define HTTP_RX_CONTENT_MAX 512

enum http_data
{
    PARAMETER,  /* Specifies parameters for a HTTP request. Only applicable for the PHR command */
    CONTENT,    /* Specifies or retrieves content for HTTP data */
    HEADER,     /* Specifies or retrieves headers for HTTP data */
    STATE       /* Retrieves the current state of the HTTP data. Only applicable for the GHR command */
};

enum request_progress
{
    NONEXISTENT = 'N',    /* This request does not exist at all */
    IN_PROGRESS = 'P',    /* This request is being processed */
    SUCCESS = 'S',        /* Successful request */
    FAILED = 'U'          /* Failed (unsuccessful) request */
};

struct http_request
{
    char method;                        /* HTTP method of the request, G for GET, P for POST and so on */
    char url[HTTP_TX_CONTENT_MAX];      /* Full URL of the request */
    char params[HTTP_TX_CONTENT_MAX];   /* Parameters of the request, usually part of the URL */
    char content[HTTP_TX_CONTENT_MAX];  /* Content of the request */
    char headers[HTTP_TX_CONTENT_MAX];  /* HTTP headers of the request */
};

struct state
{
    int status;                         /* Request status (may be HTTP status as well) */
    enum request_progress progress;     /* Current progress of the request */
    int64_t written;                    /* Number of bytes written */
    char headers[HTTP_RX_CONTENT_MAX];  /* Headers of the HTTP response */
    struct mbuf content_buffer;
    struct mg_str content;
};

void ev_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data));

bool is_state_handle_readable(struct state * state_array, int handle);

void insert_field_http_request(enum http_data type, struct mg_str *line, struct http_request *http_array);
int get_available_handle(struct http_request * request_array);
void ulwi_empty_state(struct state *s);
void ulwi_empty_request(struct http_request *r);

#endif