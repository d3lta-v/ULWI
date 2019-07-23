#include "http.h"
#include "common.h"
#include "mgos_rpc.h"
#include "constants.h"

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: ev_handler                                                  *
 *                                                                            *
 * PURPOSE: Callback function that is called when the HTTP request has a      *
 *          change in state. A typical implementation of this function        *
 *          will have a switch for the ev variable that changes throughout    *
 *          the lifecycle of the HTTP request. The function then mutates the  *
 *          user_data variable to store data more permanently, such as to a   *
 *          FILE handle or a struct.                                          *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE           I/O DESCRIPTION                                    *
 * -------- -------------- --- -----------                                    *
 * nc        mg_connection  I  The current mg_connection struct for this      *
 *                             request                                        *
 * ev        int            I  The current state of the connection as defined *
 *                             by mongoose.h                                  *
 * ev_data   void *         I  Current event data, most likely a http_message *
 * user_data void *         I  User supplied variable that can be mutated     *
 *                             throughout different parts of the HTTP request *
 *                             lifecycle                                      *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
void ev_handler(struct mg_connection *nc, int ev, void *ev_data MG_UD_ARG(void *user_data)) {
    struct http_message *hm = (struct http_message *) ev_data;
    struct http_response *response = (struct http_response *) user_data;

    switch (ev)
    {
    case MG_EV_CONNECT:
        /* System just established connection, integer should be 0 */
        response->progress = IN_PROGRESS;
        response->status = *(int *) ev_data;

        mbuf_init(&response->content_buffer, HTTP_RX_CONTENT_MAX);
        break;
    case MG_EV_HTTP_CHUNK: {
        /* Chunked reply has arrived */
        response->progress = IN_PROGRESS;
        const size_t total_len = response->written + hm->body.len;
        if (total_len < HTTP_RX_CONTENT_MAX)
        {
            response->written += hm->body.len;
            // strncat(response->content, hm->body.p, hm->body.len);
            mbuf_append(&response->content_buffer, hm->body.p, hm->body.len);
        }
        else
        {
            LOG(LL_WARN, ("Warning: total length of HTTP content exceeded HTTP_RX_CONTENT_MAX"));
            /* TODO: show error upon detecting that total_len exceeded HTTP_RX_CONTENT_MAX! */
        }
        nc->flags |= MG_F_DELETE_CHUNK; /* delete chunk right after it's read to conserve heap memory */
        break;
    }
    case MG_EV_HTTP_REPLY:
        /* Server has completed the reply*/
        response->status = hm->resp_code;
        nc->flags |= MG_F_CLOSE_IMMEDIATELY;
        break;
    case MG_EV_CLOSE:
        /* Connection fully closed */
        LOG(LL_INFO, ("status %d bytes %llu", response->status, response->written));
        /* Write buffer to string and discard buffer */
        const struct mg_str temp_string = MG_MK_STR_N(response->content_buffer.buf, response->content_buffer.len);
        mg_strfree(&response->content);
        response->content = mg_strdup_nul(temp_string);
        mbuf_free(&response->content_buffer);
        if (response->status >= 200 && response->status < 300)
        {
            response->progress = SUCCESS;
        }
        else
        {
            response->progress = FAILED;
            LOG(LL_ERROR, ("Connection closed with error code: %d", response->status));
        }
        /* NOTE: Manual memory management must be done to the response variable
           to prevent memory leaks. Alternatively, manual memory management
           is also possible */
        break;
    }
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: ulwi_empty_response                                            *
 *                                                                            *
 * PURPOSE: Empties the http_response struct and frees all of its memory      *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE            I/O DESCRIPTION                                   *
 * -------- --------------- --- -----------                                   *
 * s        http_response *  I  The state struct variable to free             *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
void ulwi_empty_response(struct http_response *s)
{
    s->progress = NONEXISTENT; /* Reset progress as this is a new request */
    s->status = 0;
    s->written = 0;
    mbuf_free(&s->content_buffer);
    mg_strfree(&s->content);
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: ulwi_empty_request                                          *
 *                                                                            *
 * PURPOSE: Empties the http_request struct and frees all of its memory       *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE       I/O DESCRIPTION                                        *
 * -------- ---------- --- -----------                                        *
 * s        request *   I  The http_request struct variable to free           *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
void ulwi_empty_request(struct http_request *r)
{
    r->method = '\0';
    mg_strfree(&r->url);
    mg_strfree(&r->params);
    mg_strfree(&r->content);
    mg_strfree(&r->headers);
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: get_available_handle                                        *
 *                                                                            *
 * PURPOSE: Gets the next available HTTP handle for ULWI                      *
 *                                                                            *
 * ARGUMENTS: none                                                            *
 *                                                                            *
 * RETURNS: an integer representing the index of the next available HTTP      *
 *          handle, or -1 if there are no handles available                   *
 *                                                                            *
 *****************************************************************************/
int get_available_handle(struct http_request * request_array)
{
    size_t i;
    for (i = 0; i < HTTP_HANDLES_MAX; i++)
    {
        if (request_array[i].method == '\0')
        {
            break;
        }
    }
    if (i == HTTP_HANDLES_MAX)
    {
        return -1;
    }
    return i;
}

bool response_handle_readable(struct http_response * response_array, int handle)
{
    /* First stage of handle validation, if the handle is within parameters */
    if (handle < 0 || handle > HTTP_HANDLES_MAX)
    {
        return false;
    }

    /* Second stage, check if handle is not used. If it's not used, it's not readable */
    if (response_array[handle].status == 0)
    {
        return false;
    }

    return true;
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: insert_field_http_request                                   *
 *                                                                            *
 * PURPOSE: Inserts a field of string-based data into a http_request struct.  *
 *          This helper function was created to reduce duplicated code for    *
 *          the CHR, HHR and PHR commands.                                    *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT     TYPE       I/O DESCRIPTION                                    *
 * ------------ ---------- --- -----------                                    *
 * type         http_data   I  The http_data enum that specifies the type of  *
 *                             data to insert into the http_request struct    *
 * line         mg_str      I  A mg_str portable string that specifies the    *
 *                             data to insert into the http_request struct    *
 * http_request http_array  O  The http_array struct to insert said data into *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
void insert_field_http_request(enum http_data type, struct mg_str *line, struct http_request *http_array)
{
    const size_t parameter_len = line->len - 4;
    const enum str_len_state str_state = ulwi_validate_strlen(parameter_len, 0, 2 + HTTP_TX_CONTENT_MAX);
    if (str_state == STRING_OK)
    {
        struct mg_str parameters_string = mg_strdup_nul(mg_mk_str_n(line->p+4, line->len-4));

        char raw_handle[2] = "";
        int handle = -1;

        char *mutable_pointer = (char *)parameters_string.p; /* Explicit cast from const pointer to non-const. Hacky?? */

        char *token = strtok(mutable_pointer, ULWI_DELIMITER);
        int param_counter = 0;
        const int max_param_count = 2;
        while (token != NULL && param_counter < max_param_count)
        {
            switch (param_counter)
            {
            case 0:
                strncpy(raw_handle, token, 1);
                raw_handle[1] = '\0'; /* Forcibly null terminate it */
                handle = validate_handle_string(raw_handle);
                break;
            case 1:
                /* Copy the parameters over */
                if (handle != -1)
                {
                    switch (type)
                    {
                    case PARAMETER:
                        LOG(LL_INFO, ("params: %s", token));
                        http_array[handle].params = mg_strdup_nul(mg_mk_str(token));
                        break;
                    case CONTENT:
                        LOG(LL_INFO, ("content: %s", token));
                        http_array[handle].content = mg_strdup_nul(mg_mk_str(token));
                        break;
                    case HEADER: {
                        char *buffer = repl_str(token, "\n", "\r\n");
                        LOG(LL_INFO, ("headers: %s", buffer));
                        http_array[handle].headers = mg_strdup_nul(mg_mk_str(buffer));
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

        mg_strfree(&parameters_string);

        if (handle == -1)
        {
            mgos_uart_printf(UART_NO, "U\r\n");
        }
    }
    else if (str_state == STRING_LONG)
    {
        mgos_uart_printf(UART_NO, "long\r\n");
    }
    else if (str_state == STRING_SHORT)
    {
        mgos_uart_printf(UART_NO, "short\r\n");
    }
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: validate_handle_string                                      *
 *                                                                            *
 * PURPOSE: Validates a HTTP handle from a C-style string (char pointer).     *
 *          This function takes care of checking if the string is properly    *
 *          terminated and can be converted, and performs basic bounds        *
 *          checking afterwards to make sure that the string does describe a  *
 *          handle that exists.                                               *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT     TYPE       I/O DESCRIPTION                                    *
 * ------------ ---------- --- -----------                                    *
 * handle_char  char *      I  The HTTP handle as a C-style string (char *)   *
 *                                                                            *
 * RETURNS: The handle as a positive integer if valid, -1 if invalid          *
 *                                                                            *
 *****************************************************************************/
int validate_handle_string(char *handle_char)
{
    size_t i = 0;
    bool digit_check = true;
    char current_char = '0';

    /* Check if handle is within 3 characters excluding null termination and
       all characters are digits */
    while (current_char != '\0' && i < 4)
    {
        current_char = *(handle_char + i);
        if (!isdigit((int)current_char) && current_char != '\0')
        {
            /* as this function will eventually encounter a null character, 
               we must ignore null characters when iterating through */
            digit_check = false;
        }
        i++;
    }
    if (current_char != '\0' || digit_check == false)
    {
        return -1;
    }

    int handle_integer = atoi(handle_char);

    /* Check if converted handle is within limits */
    if (handle_integer < 0 || handle_integer > HTTP_HANDLES_MAX)
    {
        return -1;
    }
    return handle_integer;
}
