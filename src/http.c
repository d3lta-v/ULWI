#include "http.h"
#include "mgos_rpc.h"
#include "constants.h"

struct state {
    struct mg_rpc_request_info *ri; /* RPC request info */
    int status;                     /* Request status */
    int64_t written;                /* Number of bytes written */
    FILE *fp;                       /* File to write to */
};

void http_cb(struct mg_connection *c, int ev, void *ev_data, void *ud) {
    struct http_message *hm = (struct http_message *) ev_data;
    struct state *state = (struct state *) ud;

    switch (ev) {
    case MG_EV_CONNECT:
        state->status = *(int *) ev_data;
        break;
    case MG_EV_HTTP_CHUNK: {
        /*
        * Write data to file or UART. mgos_uart_write() blocks until
        * all data is written.
        */
        size_t n =
            (state->fp != NULL)
                ? fwrite(hm->body.p, 1, hm->body.len, state->fp)
                : mgos_uart_write(UART_NO, hm->body.p, hm->body.len);
        if (n != hm->body.len) {
        c->flags |= MG_F_CLOSE_IMMEDIATELY;
        state->status = 500;
        }
        state->written += n;
        c->flags |= MG_F_DELETE_CHUNK;
        break;
    }
    case MG_EV_HTTP_REPLY:
        /* Only when we successfully got full reply, set the status. */
        state->status = hm->resp_code;
        LOG(LL_INFO, ("Finished fetching"));
        c->flags |= MG_F_CLOSE_IMMEDIATELY;
        break;
    case MG_EV_CLOSE:
        LOG(LL_INFO, ("status %d bytes %llu", state->status, state->written));
        if (state->status == 200) {
        /* Report success only for HTTP 200 downloads */
        mg_rpc_send_responsef(state->ri, "{written: %llu}", state->written);
        } else {
        mg_rpc_send_errorf(state->ri, state->status, NULL);
        }
        if (state->fp != NULL) fclose(state->fp);
        free(state);
        break;
    }
}

void ev_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data)) {
  if (ev == MG_EV_HTTP_REPLY) {
    struct http_message *hm = (struct http_message *)ev_data;
    nc->flags |= MG_F_CLOSE_IMMEDIATELY;
    mgos_uart_printf(UART_NO, "%s", hm->message.p);
    // fwrite(hm->message.p, 1, (int)hm->message.len, stdout);
    // putchar('\n');
    // exit_flag = 1;
  } else if (ev == MG_EV_CLOSE) {
    // exit_flag = 1;
    LOG(LL_INFO, ("Connection closed"));
  };
  (void)user_data;
}
