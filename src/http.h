#ifndef HTTP_H
#define HTTP_H

#include "mgos.h"

#define HTTP_TX_CONTENT_MAX 256
#define HTTP_RX_CONTENT_MAX 512

enum http_data
{
    POST_FIELD, /* Specifies parameters for a HTTP request. Only applicable for the PHR command */
    CONTENT,    /* Retrieves content for HTTP data */
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
    struct mg_str url;
    struct mg_str post_field;   /* Parameters of the request, usually part of the URL */
    struct mg_str headers;      /* HTTP headers of the request */
};

struct http_response
{
    int status;                         /* Request status (may be HTTP status as well) */
    enum request_progress progress;     /* Current progress of the request */
    int64_t written;                    /* Number of bytes written */
    char headers[HTTP_RX_CONTENT_MAX];  /* Headers of the HTTP response */
    struct mbuf content_buffer;
    struct mg_str content;
};

void ev_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data));

bool response_handle_readable(struct http_response * response_array, int handle);

void insert_field_http_request(enum http_data type, struct mg_str *line, struct http_request *http_array);
int get_available_handle(struct http_request * request_array);
void ulwi_empty_response(struct http_response *s);
void ulwi_empty_request(struct http_request *r);
int validate_handle_string(char *handle_char);

#endif
