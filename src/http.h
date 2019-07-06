#ifndef HTTP_H
#define HTTP_H

#include "mgos.h"

struct http_request
{
    char method;            /* HTTP method of the request, G for GET, P for POST and so on */
    struct mg_str url;      /* Full URL of the request */
    struct mg_str params;   /* Parameters of the request, usually part of the URL */
    struct mg_str content;  /* Content of the request */
    struct mg_str headers;  /* HTTP headers of the request */
};

struct http_request http_array[3];

void http_cb(struct mg_connection *c, int ev, void *ev_data, void *ud);

#endif