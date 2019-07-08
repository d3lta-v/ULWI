#include "http.h"
#include "mgos_rpc.h"
#include "constants.h"

void ev_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data)) {
    struct http_message *hm = (struct http_message *) ev_data;
    struct state *state = (struct state *) user_data;

    switch (ev)
    {
    case MG_EV_CONNECT:
        /* System just established connection, integer should be 0 */
        state->progress = IN_PROGRESS;
        state->status = *(int *) ev_data;
        break;
    case MG_EV_HTTP_CHUNK: {
        /* Chunked reply has arrived */
        state->progress = IN_PROGRESS;
        const size_t total_len = state->written + hm->body.len;
        if (total_len < HTTP_RX_CONTENT_MAX)
        {
            state->written += hm->body.len;
            strncat(state->content, hm->body.p, hm->body.len);
        }
        nc->flags |= MG_F_DELETE_CHUNK; /* immediately delete chunk after it's read to conserve memory */
        break;
    }
    case MG_EV_HTTP_REPLY:
        /* Server has completed the reply*/
        state->status = hm->resp_code;
        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        break;
    case MG_EV_CLOSE:
        LOG(LL_INFO, ("status %d bytes %llu", state->status, state->written));
        if (state->status >= 200 && state->status < 300)
        {
            state->progress = SUCCESS;
        }
        else
        {
            state->progress = FAILED;
            LOG(LL_ERROR, ("Connection closed with error code: %d\r\n", state->status));
        }
        /* NOTE: Manual memory management must be done to the state variable to prevent memory leaks */
        break;
    }
}

void empty_state(struct state *s)
{
    s->progress = NONEXISTENT; /* Reset progress as this is a new request */
    s->status = 0;
    s->written = 0;
    s->content[0] = '\0';
}

void empty_request(struct http_request *r)
{
    r->method = '\0';
    r->url[0] = '\0';
    r->params[0] = '\0';
    r->content[0] = '\0';
    r->headers[0] = '\0';
}
