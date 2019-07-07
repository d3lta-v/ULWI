#ifndef HTTP_H
#define HTTP_H

#include "mgos.h"

struct http_request
{
    char method;        /* HTTP method of the request, G for GET, P for POST and so on */
    char url[256];      /* Full URL of the request */
    char params[256];   /* Parameters of the request, usually part of the URL */
    char content[256];  /* Content of the request */
    char headers[256];  /* HTTP headers of the request */
};

void http_cb(struct mg_connection *c, int ev, void *ev_data, void *ud);

#endif