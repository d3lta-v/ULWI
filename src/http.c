#include "http.h"
#include "mgos_rpc.h"
#include "constants.h"

void ev_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data)) {
    struct http_message *hm = (struct http_message *) ev_data;
    struct state *state = (struct state *) user_data;

    switch (ev)
    {
    case MG_EV_CONNECT:
        /* System just established connection */
        state->status = *(int *) ev_data;
        break;
    case MG_EV_HTTP_CHUNK: {
        /* Chunked reply has arrived */
        size_t n = mgos_uart_write(UART_NO, hm->body.p, hm->body.len);
        if (n != hm->body.len) {
            /* UART written size and body reported length mismatch */
            LOG(LL_WARN, ("Warning: UART written size does not match HTTP body length! Closing connection..."));
            nc->flags |= MG_F_CLOSE_IMMEDIATELY;
            state->status = 500;
        }
        state->written += n;
        nc->flags |= MG_F_DELETE_CHUNK; /* immediately delete chunk after it's read to conserve memory */
        break;
    }
    case MG_EV_HTTP_REPLY:
        /* Reply has arrived */
        // Get response code with hm->resp_code
        state->status = hm->resp_code;
        LOG(LL_INFO, ("Finished fetching"));
        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        break;
    case MG_EV_CLOSE:
        LOG(LL_INFO, ("status %d bytes %llu", state->status, state->written));
        if (state->status == 200)
        {
            /* Connection closed with success */
            LOG(LL_INFO, ("Connection closed"));
        }
        else
        {
            LOG(LL_ERROR, ("Connection closed with error"));
        }
        /* NOTE: Manual memory management must be done to the state variable to prevent memory leaks */
        break;
    }



    // if (ev == MG_EV_HTTP_REPLY) {
    //     struct http_message *hm = (struct http_message *)ev_data;
    //     nc->flags |= MG_F_CLOSE_IMMEDIATELY;
    //     mgos_uart_printf(UART_NO, "%s", hm->message.p);
    //     // fwrite(hm->message.p, 1, (int)hm->message.len, stdout);
    //     // putchar('\n');
    //     // exit_flag = 1;
    // } else if (ev == MG_EV_CLOSE) {
    //     // exit_flag = 1;
    //     LOG(LL_INFO, ("Connection closed"));
    // };
    (void)hm;
    (void)user_data;
}
