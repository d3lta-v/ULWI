#include "http.h"
#include "common.h"
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

        mbuf_init(&state->content_buffer, HTTP_RX_CONTENT_MAX);
        break;
    case MG_EV_HTTP_CHUNK: {
        /* Chunked reply has arrived */
        state->progress = IN_PROGRESS;
        const size_t total_len = state->written + hm->body.len;
        if (total_len < HTTP_RX_CONTENT_MAX)
        {
            state->written += hm->body.len;
            // strncat(state->content, hm->body.p, hm->body.len);
            mbuf_append(&state->content_buffer, hm->body.p, hm->body.len);
        }
        /* TODO: show error upon detecting that total_len exceeded HTTP_RX_CONTENT_MAX! */
        nc->flags |= MG_F_DELETE_CHUNK; /* delete chunk right after it's read to conserve heap memory */
        break;
    }
    case MG_EV_HTTP_REPLY:
        /* Server has completed the reply*/
        state->status = hm->resp_code;
        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        break;
    case MG_EV_CLOSE:
        /* Connection fully closed */
        LOG(LL_INFO, ("status %d bytes %llu", state->status, state->written));
        /* Write buffer to string and discard buffer */
        const struct mg_str temp_string = MG_MK_STR_N(state->content_buffer.buf, state->content_buffer.len);
        mg_strfree(&state->content);
        state->content = mg_strdup_nul(temp_string);
        mbuf_free(&state->content_buffer);
        if (state->status >= 200 && state->status < 300)
        {
            state->progress = SUCCESS;
        }
        else
        {
            state->progress = FAILED;
            LOG(LL_ERROR, ("Connection closed with error code: %d", state->status));
        }
        /* NOTE: Manual memory management must be done to the state variable
           to prevent memory leaks. Alternatively, manual memory management
           is also possible */
        break;
    }
}

void ulwi_empty_state(struct state *s)
{
    s->progress = NONEXISTENT; /* Reset progress as this is a new request */
    s->status = 0;
    s->written = 0;
    mbuf_free(&s->content_buffer);
    mg_strfree(&s->content);
}

void ulwi_empty_request(struct http_request *r)
{
    r->method = '\0';
    r->url[0] = '\0';
    r->params[0] = '\0';
    r->content[0] = '\0';
    r->headers[0] = '\0';
}

void insert_field_http_request(enum http_data type, struct mg_str *line, struct http_request *http_array)
{
    const size_t parameter_len = line->len - 4;
    if (parameter_len > 0 && parameter_len < 2 + HTTP_TX_CONTENT_MAX)
    {
        char parameter_c_str[2+HTTP_TX_CONTENT_MAX] = "";
        strlcpy(parameter_c_str, line->p+4, parameter_len);

        char raw_handle[2] = "";
        int handle = 0;
        // struct state *state = &state_array[handle];

        char *token = strtok(parameter_c_str, ULWI_DELIMITER);
        int param_counter = 0;
        const int max_param_count = 2;
        while (token != NULL && param_counter < max_param_count)
        {
            switch (param_counter)
            {
            case 0:
                strncpy(raw_handle, token, 1);
                raw_handle[1] = '\0'; /* Forcibly null terminate it */
                handle = atoi(raw_handle);
                if (handle >= HTTP_HANDLES_MAX || handle < 0)
                {
                    /* Perform basic validation of handle */
                    handle = -1; /* invalidate the handle */
                }
                else if (http_array[handle].method == '\0')
                {
                    /* Perform deep validation of handle if it passed basic */
                    /* If method is null ('\0'), it means that this handle does not exist */
                    handle = -1; /* invalidate the handle */
                }
                break;
            case 1:
                /* Copy the parameters over */
                if (handle != -1)
                {
                    switch (type)
                    {
                    case PARAMETER:
                        LOG(LL_INFO, ("params: %s", token));
                        strcpy(http_array[handle].params, token);
                        break;
                    case CONTENT:
                        LOG(LL_INFO, ("content: %s", token));
                        strcpy(http_array[handle].content, token);
                        break;
                    case HEADER: {
                        char *buffer = repl_str(token, "\n", "\r\n");
                        LOG(LL_INFO, ("headers: %s", buffer));
                        strcpy(http_array[handle].headers, buffer);
                        free(buffer);
                        break;
                    }
                    case STATE:
                        break;
                    }
                    mgos_uart_printf(UART_NO, "S\r\n");
                }
                break;
            }
            token = strtok(NULL, ULWI_DELIMITER);
            param_counter++;
        }

        if (handle == -1)
        {
            mgos_uart_printf(UART_NO, "U\r\n");
        }
    }
    else
    {
        mgos_uart_printf(UART_NO, "long\r\n");
    }
}
