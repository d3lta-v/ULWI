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

// static struct http_message http_array[3];

#endif