/******************************************************************************
 *                                                                            *
 * FILE NAME: main.c                                                          *
 *                                                                            *
 * PURPOSE: Contains the starting point of the entire Mongoose OS application *
 *                                                                            *
 * FILE REFERENCES:                                                           *
 *                                                                            *
 * Name I/O Description                                                       *
 * ---- --- -----------                                                       *
 * none                                                                       *
 *                                                                            *
 * EXTERNAL VARIABLES:                                                        *
 * Source: .h                                                                 *
 *                                                                            *
 * Name Type I/O Description                                                  *
 * ---- ---- --- -----------                                                  *
 *                                                                            *
 * EXTERNAL REFERENCES:                                                       *
 *                                                                            *
 * Name Description                                                           *
 * ---- -----------                                                           *
 *                                                                            *
 * ABNORMAL TERMINATION CONDITIONS, ERROR AND WARNING MESSAGES: none          *
 *                                                                            *
 * ASSUMPTIONS, CONSTRAINTS, RESTRICTIONS: none                               *
 *                                                                            *
 * NOTES: none                                                                *
 *                                                                            *
 * REQUIREMENTS/FUNCTIONAL SPECIFICATIONS REFERENCES: N/A                     *
 *                                                                            *
 * ALGORITHM (PDL)                                                            *
 *                                                                            *
 *****************************************************************************/

#include "mgos.h"

#include "constants.h"
#include "common.h"
#include "wifi.h"
#include "http.h"
#include "mqtt.h"

/* TODO: Comment out the definition if in production!! */
#define DEVELOPMENT

static struct http_request http_array[HTTP_HANDLES_MAX];
static struct http_response response_array[HTTP_HANDLES_MAX];

#ifdef DEVELOPMENT
/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: timer_cb                                                    *
 *                                                                            *
 * PURPOSE: A simple timer that prints debug information regarding the        *
 *          current state of the system periodically                          *
 *                                                                            *
 * ARGUMENTS: N/A                                                             *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
// static void timer_cb(void *arg) {
//     LOG(LL_INFO,
//         ("uptime: %.2lf, RAM: %lu, %lu free",
//         mgos_uptime(), (unsigned long) mgos_get_heap_size(),
//         (unsigned long) mgos_get_free_heap_size()));
//     (void) arg;
// }
#endif /* DEVELOPMENT */

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: uart_dispatcher                                             *
 *                                                                            *
 * PURPOSE: A UART Dispatcher to be invoked by a mgos_uart_set_dispatcher     *
 *          function, which is called whenever there is input data available  *
 *          to process from a UART.                                           *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE    I/O DESCRIPTION                                           *
 * -------- ------- --- -----------                                           *
 * uart_no  int      I  The UART port's identification number. Typically 0    *
 *                      for the ESP8266-01 board.                             *
 * arg      void*    I  Arguments to be passed into the UART dispatcher       *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
static void uart_dispatcher(int uart_no, void *arg)
{
    /* Phase 1: allocate buffer */
    static struct mbuf buffer = {0}; /* Make an empty mbuf (memory buffer) struct */
    assert(uart_no == UART_NO);      /* Just to make sure that we're reading on the correct UART */

    /* Phase 2: Check input size, return if size is 0 */
    size_t available_size = mgos_uart_read_avail(uart_no);
    if (available_size == 0)
    {
        return;
    }

    /* Phase 3: Read input into buffer and appropriately terminate the line */
    mgos_uart_read_mbuf(uart_no, &buffer, available_size);
    /* Retrieve pointer of the last character, in this case it's the CR character */
    char *line_ending = (char *)mg_strchr(mg_mk_str_n(buffer.buf, buffer.len), '\r');
    if (line_ending == NULL)
    {
        /* Unable to find newline or line termination, ignoring input */
        return;
    }
    if (*(line_ending + 1) != '\n')
    {
        /* Check if the next character in sequence is NL which is a full
           modern Windows style CRLF (\r\n) */
        /* CAVEAT: This will cause any input containing \r only to be ignored */
        return;
    }
    *(line_ending + 1) = '\0'; /* Null terminate the string, replacing NL (NOT CR!) with NULL */
    size_t line_length = line_ending + 1 - buffer.buf;
    /* Creates an mg_str (basically a string) from buffer.buf (which is a C array) */
    struct mg_str line = MG_MK_STR_N(buffer.buf, line_length);
    /* Manually strip CR from line after NL is processed as the termination character */
    /* This is to fully flush the serial buffer */
    *line_ending = '\0';
    line.len--;

    /* Now we can process the line itself, because we've null terminated the
     * line correctly (it's a C string now) and removed newline characters */

    /* Phase I: basic command sanitisation */
    // LOG(LL_INFO, ("input length: %d, input: %s", line.len, line.p));
    if (line.len < 3)
    {
        mgos_uart_printf(UART_NO, "short\r\n");
    }
    else
    {
        /* Phase II: string comparison to sort out the commands */
        if (mg_str_starts_with(line, COMMAND_NOP) && line.len == 3)
        {
            /* No Operation */
            mgos_uart_printf(UART_NO, "\r\n");
        }
        else if (mg_str_starts_with(line, COMMAND_VER) && line.len == 3)
        {
            /* Print Version */
            mgos_uart_printf(UART_NO, "v1.0-alpha1\r\n");
        }
        else if (mg_str_starts_with(line, COMMAND_RST) && line.len == 3)
        {
            /* Reset device */
            mgos_system_restart();
        }
        else if (mg_str_starts_with(line, COMMAND_LAP) && line.len == 3)
        {
            /* List Access Points */
            /* We need to ensure that Wi-Fi radio is enabled, even temporarily,
               for this purpose */
            float rand = mgos_rand_range(0.0f, 100.0f);
            char buf[10];
            snprintf(buf, 10, "%f", rand);
            const struct mgos_config_wifi_ap ap_config = 
            { 
                .enable = true,
                .ssid = buf,
                .dhcp_start = "10.1.0.2",
                .dhcp_end = "10.1.0.9",
                .ip = "10.1.0.1",
                .netmask = "255.255.255.0",
                .gw = "10.1.0.1",
                .hidden = true
            };
            mgos_wifi_setup_ap(&ap_config);
            mgos_wifi_scan(wifi_scan_cb, NULL);
        }
        else if (mg_str_starts_with(line, COMMAND_CAP))
        {
            /* Connect to Access Point */
            const size_t min_len = 4;               /* 3 for command, 1 space */
            const size_t max_len = min_len + 255;
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, min_len, max_len);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[256] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                const int max_params = 5;
                const int max_param_len = 65;
                char result[max_params][max_param_len];

                const int param_len = split_parameter_string(parameter_c_str, max_params, max_param_len, result);
                
                if (param_len == 2)
                {
                    const struct mgos_config_wifi_sta wifi_config = 
                    { 
                        .enable = true, 
                        .ssid = result[0],
                        .pass = result[1]
                    };
                    mgos_wifi_setup_sta(&wifi_config);
                    mgos_uart_printf(UART_NO, "\r\n");
                }
                else if (param_len == 3)
                {
                    const struct mgos_config_wifi_sta wifi_config = 
                    { 
                        .enable = true, 
                        .ssid = result[0],
                        .user = result[1],
                        .pass = result[2]
                    };
                    mgos_wifi_setup_sta(&wifi_config);
                    mgos_uart_printf(UART_NO, "\r\n");
                }
                else if (param_len == 5)
                {
                    const struct mgos_config_wifi_sta wifi_config = 
                    { 
                        .enable = true,
                        .ssid = result[0],
                        .pass = result[1],
                        .ip = result[2],
                        .gw = result[3],
                        .netmask = result[4]
                    };
                    mgos_wifi_setup_sta(&wifi_config);
                    mgos_uart_printf(UART_NO, "\r\n");
                }
                else
                {
                    mgos_uart_printf(UART_NO, "invalid\r\n");
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
        else if (mg_str_starts_with(line, COMMAND_SAP) && line.len == 3)
        {
            /* Status of Access Point */
            const enum mgos_wifi_status wifi_status = mgos_wifi_get_status();
            char *ssid = mgos_wifi_get_connected_ssid();
            switch (wifi_status)
            {
            case MGOS_WIFI_DISCONNECTED:
                mgos_uart_printf(UART_NO, "N\r\n");
                break;
            case MGOS_WIFI_CONNECTING:
                mgos_uart_printf(UART_NO, "P\r\n");
                break;
            case MGOS_WIFI_CONNECTED:
                /* TODO: need to implement differentiation between connected and IP acquired,
                         as some Wi-Fi environments do NOT use DHCP */
                /* this condition is fallthrough */
            case MGOS_WIFI_IP_ACQUIRED:
                if (ssid)
                {
                    mgos_uart_printf(UART_NO, "S\x1f%s\r\n", ssid);
                }
                else
                {
                    /* This case occurs when Wi-Fi connection was JUST established and
                       might not constitute an error */
                    mgos_uart_printf(UART_NO, "P\r\n", ssid);
                    LOG(LL_DEBUG, ("Wifi IP acquired but SSID is null"));
                }
                break;
            default:
                mgos_uart_printf(UART_NO, "U\r\n");
                break;
            }
            free(ssid);
        }
        else if (mg_str_starts_with(line, COMMAND_DAP) && line.len == 3)
        {
            /* Disconnect from Access Point */
            const char *ssid = mgos_wifi_get_connected_ssid();
            if (ssid)
            {
                LOG(LL_DEBUG, ("attempting disconnect from wifi"));
                const struct mgos_config_wifi_sta wifi_config = { false };
                mgos_wifi_setup_sta(&wifi_config);
            }
            else
            {
                LOG(LL_DEBUG, ("not attempting disconnect from wifi"));
            }
        }
        else if (mg_str_starts_with(line, COMMAND_GIP) && line.len == 3)
        {
            /* Get IP */
            struct mgos_net_ip_info ip_information;
            mgos_net_get_ip_info(MGOS_NET_IF_TYPE_WIFI, MGOS_NET_IF_WIFI_STA, &ip_information);
            char ip[16];
            mgos_net_ip_to_str(&ip_information.ip, ip);
            mgos_uart_printf(UART_NO, "%s\r\n", ip);
        }
        else if (mg_str_starts_with(line, COMMAND_IHR))
        {
            /* Initialise HTTP Request */
            const int max_param_count = 2;
            const size_t min_len = 4;
            const size_t max_len = min_len + 258;
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, min_len, max_len);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[258] = {0}; /* 1 (Get/Post) + 255 (URL length) + 2 null termination */
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                const int handle = get_available_handle(http_array); /* Returns -1 if no handle available */

                if (handle != -1)
                {
                    struct http_request *request = &http_array[handle];
                    struct http_response *response = &response_array[handle];

                    char *token = strtok(parameter_c_str, ULWI_DELIMITER);
                    int param_counter = 0;
                    while (token != NULL && param_counter < max_param_count)
                    {
                        switch (param_counter)
                        {
                        case 0:
                            request->method = token[0];
                            break;
                        case 1:
                            request->url = mg_strdup_nul(mg_mk_str(token));
                            break;
                        }
                        token = strtok(NULL, ULWI_DELIMITER);
                        param_counter++;
                    }

                    if (param_counter == max_param_count)
                    {
                        /* Ensure that the relevant http_response struct is empty before finishing */
                        ulwi_empty_response(response);
                        mgos_uart_printf(UART_NO, "%i\r\n", handle);
                        LOG(LL_INFO, ("request type: %c, url: %s", request->method, request->url.p));
                    }
                    else
                    {
                        /* Reset request if there are not enough parameters given,
                        reason being that the request is filled before checking
                        for max parameter count */
                        ulwi_empty_request(request);
                        mgos_uart_printf(UART_NO, "short\r\n");
                    }
                }
                else
                {
                    /* Failed to create handle, most likely it ran out */
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
        else if (mg_str_starts_with(line, COMMAND_PHR) && line.len > 4)
        {
            /* Parameter of HTTP request */
            insert_field_http_request(POST_FIELD, &line, http_array);
        }
        else if (mg_str_starts_with(line, COMMAND_HHR) && line.len > 4)
        {
            /* Header of HTTP request */
            insert_field_http_request(HEADER, &line, http_array);
        }
        else if (mg_str_starts_with(line, COMMAND_THR))
        {
            /* Transmit HTTP request */
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[2] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                const int handle = validate_handle_string(parameter_c_str);
                if (handle >= 0)
                {
                    struct http_request *request = &http_array[handle];
                    struct http_response *response = &response_array[handle];

                    /* Check if handle refers to a null by checking the method char for the null char */
                    if (request->method)
                    {
                        /* Clear the previous response */
                        ulwi_empty_response(response);
                        char *headers = NULL;
                        if (request->headers.len > 0)
                        {
                            headers = (char *)request->headers.p;
                        }
                        if (request->method == 'G')
                        {
                            LOG(LL_INFO, ("GET HTTP url: %s", request->url.p));
                            mg_connect_http(mgos_get_mgr(), &ev_handler, response, request->url.p, headers, NULL);
                        }
                        else if (request->method == 'P')
                        {
                            if (request->post_field.len > 0)
                            {
                                LOG(LL_INFO, ("POST HTTP url: %s, data: %s", request->url.p, request->post_field.p));
                                mg_connect_http(mgos_get_mgr(), &ev_handler, response, request->url.p, headers, request->post_field.p);
                            }
                            else
                            {
                                /* TODO: Check if the behavior of this condition actually even performs POST, since an empty string
                                   is effectively \0 (which, while different from a NULL, which is a pointer, may be the same
                                   thing depending on the system) */
                                LOG(LL_INFO, ("POST HTTP url: %s", request->url.p));
                                mg_connect_http(mgos_get_mgr(), &ev_handler, response, request->url.p, headers, "");
                            }
                        }
                    }
                    else
                    {
                        mgos_uart_printf(UART_NO, "U\r\n");
                    }
                }
                else
                {
                    /* Invalid handle */
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
        else if (mg_str_starts_with(line, COMMAND_SHR))
        {
            /* Status of HTTP request */
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[2] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                const int handle = validate_handle_string(parameter_c_str);
                if (handle >= 0)
                {
                    struct http_response *response = &response_array[handle];
                    mgos_uart_printf(UART_NO, "%c\r\n", (char)response->progress);
                }
                else
                {
                    /* Invalid handle */
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
        else if (mg_str_starts_with(line, COMMAND_GHR))
        {
            /* Get HTTP request */
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 9, 9);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[6] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                int handle = -1;
                enum http_data command_type;
                enum boolean purge;

                char *token = strtok(parameter_c_str, ULWI_DELIMITER);
                int param_counter = 0;
                const int max_param_count = 3;
                while (token != NULL && param_counter < max_param_count)
                {
                    switch (param_counter)
                    {
                    case 0:
                        /* Handle */
                        handle = validate_handle_string(token);
                        LOG(LL_INFO, ("GHR handle parsed: %d", handle));
                        break;
                    case 1:
                        /* S/H/C (enum http_data) */
                        LOG(LL_INFO, ("GHR type parsed: %c", token[0]));
                        switch (token[0])
                        {
                        case 'S':
                            command_type = STATE;
                            break;
                        case 'H':
                            command_type = HEADER;
                            break;
                        case 'C':
                            command_type = CONTENT;
                            break;
                        default:
                            break;
                        }
                        break;
                    case 2:
                        /* Boolean for whether to purge the request */
                        LOG(LL_INFO, ("GHR bool parsed: %c", token[0]));
                        switch (token[0])
                        {
                        case 'T':
                            purge = ULWI_TRUE;
                            break;
                        case 'F':
                            purge = ULWI_FALSE;
                            break;
                        default:
                            break;
                        }
                        break;
                    }
                    token = strtok(NULL, ULWI_DELIMITER);
                    param_counter++;
                }
                LOG(LL_INFO, ("GHR param count: %d", param_counter));
                if (param_counter == max_param_count)
                {
                    /* Correct number of arguments, validate the handle again but this time through checking if state is available to read from */
                    if (response_handle_readable(response_array, handle) == true)
                    {
                        struct http_response *http_response = &response_array[handle];

                        switch (command_type)
                        {
                        case STATE:
                            /* Get http_response */
                            mgos_uart_printf(UART_NO, "%i\r\n", http_response->status);
                            break;
                        case HEADER:
                            /* Get headers of the HTTP response */
                            mgos_uart_printf(UART_NO, "%s\r\n", http_response->headers);
                            break;
                        case CONTENT:
                            /* Get content of the HTTP response */
                            /* TODO: Ensure proper null termination here */
                            mgos_uart_printf(UART_NO, "%s\r\n", http_response->content.p);
                            break;
                        default:
                            break;
                        }

                        /* See whether or not to purge the request */
                        if (purge == ULWI_TRUE)
                        {
                            /* Purge said request */
                            ulwi_empty_response(http_response);
                            ulwi_empty_request(&http_array[handle]);
                        }
                    }
                    else
                    {
                        mgos_uart_printf(UART_NO, "U\r\n");
                    }
                }
                else
                {
                    mgos_uart_printf(UART_NO, "short\r\n");
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
        else if (mg_str_starts_with(line, COMMAND_DHR))
        {
            /* Delete HTTP Request */
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[2] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                const int handle = validate_handle_string(parameter_c_str);
                if (handle >= 0)
                {
                    struct http_request *request = &http_array[handle];
                    struct http_response *response = &response_array[handle];

                    /* Check if handle number is pointing to an populated handle */
                    if (request->method || response->status != 0)
                    {
                        ulwi_empty_request(request);
                        ulwi_empty_response(response);
                        mgos_uart_printf(UART_NO, "\r\n");
                    }
                    else
                    {
                        mgos_uart_printf(UART_NO, "U\r\n");
                    }
                }
                else
                {
                    /* Invalid handle */
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
        else if (mg_str_starts_with(line, COMMAND_MCG))
        {
            /* MQTT Configure */
            // 5 arguments max, 3 arguments standard

            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5 + 320);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[320] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                const int max_params = 5;
                const int max_param_len = 65;
                char result[max_params][max_param_len];

                const int param_len = split_parameter_string(parameter_c_str, max_params, max_param_len, result);
                
                struct mgos_config_mqtt mqtt_conf = *mgos_sys_config_get_mqtt();

                if (param_len == 1)
                {
                    /* Enable or disable only */
                    bool enable = result[0][0] == 'T'; //TODO: does this need additional sanitisation?
                    if (!enable)
                    {
                        /* Allow disable under any circumstances */
                        mqtt_conf.enable = enable;
                        ulwi_mqtt_unsub_all();
                        mgos_mqtt_set_config(&mqtt_conf);
                        mgos_uart_printf(UART_NO, "\r\n");
                    }
                    else if (mqtt_conf.server != NULL)
                    {
                        /* Enable is true in this if statement, only allow enable if server field is not empty */
                        mqtt_conf.enable = enable;
                        mgos_mqtt_set_config(&mqtt_conf);
                        mgos_uart_printf(UART_NO, "\r\n");
                    }
                    else
                    {
                        /* Cannot enable if server field is empty */
                        mgos_uart_printf(UART_NO, "U\r\n");
                    }
                }
                else if (param_len == 3)
                {
                    //TODO: add input sanitisation in here
                    const bool enable_tls = result[2][0] == 'T' ? true : false;
                    mqtt_conf.enable = result[0][0] == 'T' ? true : false;
                    mqtt_conf.server = result[1];
                    mqtt_conf.ssl_ca_cert = enable_tls ? "rootca.pem" : NULL;
                    mgos_mqtt_set_config(&mqtt_conf);
                    mgos_uart_printf(UART_NO, "\r\n");
                }
                else if (param_len == 5)
                {
                    const bool enable_tls = result[2][0] == 'T' ? true : false;
                    mqtt_conf.enable = result[0][0] == 'T' ? true : false;
                    mqtt_conf.server = result[1];
                    mqtt_conf.user = result[3];
                    mqtt_conf.pass = result[4];
                    mqtt_conf.ssl_ca_cert = enable_tls ? "rootca.pem" : NULL;
                    
                    if (mgos_mqtt_set_config(&mqtt_conf))
                    {
                        if (mgos_mqtt_global_connect())
                        {
                            mgos_uart_printf(UART_NO, "\r\n");
                        }
                        else
                        {
                            mgos_uart_printf(UART_NO, "U\r\n");
                        }
                    }
                    else
                    {
                        mgos_uart_printf(UART_NO, "U\r\n");
                    }
                }
                else
                {
                    mgos_uart_printf(UART_NO, "invalid\r\n");
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
        else if (mg_str_starts_with(line, COMMAND_MIC) && line.len == 3)
        {
            /* MQTT Is Connected? */
            if (mgos_mqtt_global_is_connected())
            {
                //TODO WARNING: this function returns true even setting MQTT enable to false. will need to investigate further
                mgos_uart_printf(UART_NO, "T\r\n");
            }
            else
            {
                mgos_uart_printf(UART_NO, "F\r\n");
            }
        }
        else if (mg_str_starts_with(line, COMMAND_MSB))
        {
            /* MQTT Subscribe */
            // 1 argument
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5 + 127);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[128] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);
                if (!ulwi_mqtt_sub_exists(parameter_c_str))
                {
                    ulwi_mqtt_sub(parameter_c_str, mqtt_sub_handler, NULL);
                    mgos_uart_printf(UART_NO, "S\r\n");
                }
                else mgos_uart_printf(UART_NO, "U\r\n"); /* Subscription already exists */
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
        else if (mg_str_starts_with(line, COMMAND_MUS))
        {
            /* MQTT Unsubscribe */
            // 1 argument
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5 + 127);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[128] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);
                if (ulwi_mqtt_sub_exists(parameter_c_str))
                {
                    ulwi_mqtt_unsub(parameter_c_str);
                    mgos_uart_printf(UART_NO, "S\r\n");
                }
                else mgos_uart_printf(UART_NO, "U\r\n"); /* Subscription does not exist */
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
        else if (mg_str_starts_with(line, COMMAND_MND))
        {
            /* MQTT check subscription for New Data */
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5 + 127);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[128] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);
                if (ulwi_mqtt_sub_exists(parameter_c_str))
                {
                    ulwi_mqtt_new_data_arrived(parameter_c_str) ? mgos_uart_printf(UART_NO, "T\r\n") : mgos_uart_printf(UART_NO, "F\r\n");
                }
                else mgos_uart_printf(UART_NO, "U\r\n"); /* Subscription does not exist */
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
        else if (mg_str_starts_with(line, COMMAND_MGS))
        {
            /* MQTT Get Subscribed message */
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5 + 127);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[128] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);
                struct mg_str *message = ulwi_mqtt_get_sub_message(parameter_c_str);
                if (message != NULL)
                {
                    struct mg_str buffer = mg_strdup_nul(*message);
                    mgos_uart_write(UART_NO, buffer.p, buffer.len);
                    mgos_uart_printf(UART_NO, "\r\n");
                    mg_strfree(&buffer);
                }
                else mgos_uart_printf(UART_NO, "U\r\n"); /* Subscription does not exist */
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
        else if (mg_str_starts_with(line, COMMAND_MPB))
        {
            /* MQTT Publish */
            // 4 arguments
            const enum str_len_state str_state = ulwi_validate_strlen(line.len, 5, 5 + 255);
            if (str_state == STRING_OK)
            {
                char parameter_c_str[256] = {0};
                ulwi_cpy_params_only(parameter_c_str, line.p, line.len);

                const int max_params = 4;
                const int max_param_len = 64;
                char result[max_params][max_param_len];

                const int param_len = split_parameter_string(parameter_c_str, max_params, max_param_len, result);
                if (param_len == 4)
                {
                    if (mgos_mqtt_pub(result[0], result[1], strlen(result[1]), 0, false))
                    {
                        mgos_uart_printf(UART_NO, "S\r\n");
                    }
                    else
                    {
                        mgos_uart_printf(UART_NO, "U\r\n");
                    }
                }
                else
                {
                    mgos_uart_printf(UART_NO, "invalid\r\n");
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
        else
        {
            mgos_uart_printf(UART_NO, "invalid\r\n");
        }
    }

    mbuf_remove(&buffer, line_length + 1); /* Release the buffer */
    (void)arg;
}

enum mgos_app_init_result mgos_app_init(void)
{
    /* Enable or disable logging based on the build environment */
#ifdef DEVELOPMENT
    // mgos_set_timer(10000, MGOS_TIMER_REPEAT, timer_cb, NULL); /* Enable debug timer */
    cs_log_set_level(LL_VERBOSE_DEBUG); /* Full verbose logging */
#else
    mgos_set_stdout_uart(-1);  /* Disables stdout */
    mgos_set_stderr_uart(-1);  /* Disables stderr */
    cs_log_set_level(LL_NONE); /* Disables all logging*/
#endif

    /* Configure UART port */
    struct mgos_uart_config ucfg;
    mgos_uart_config_set_defaults(UART_NO, &ucfg);
#ifndef DEVELOPMENT
    ucfg.baud_rate = 9600; /* Defaults to 115200 in development, 9600 in production */
#endif
    ucfg.num_data_bits = 8;
    ucfg.rx_buf_size = 1024;
    ucfg.tx_buf_size = 1024;
    ucfg.parity = MGOS_UART_PARITY_NONE;
    ucfg.stop_bits = MGOS_UART_STOP_BITS_1;
    if (!mgos_uart_configure(UART_NO, &ucfg))
    {
        return MGOS_APP_INIT_ERROR;
    }

    /* Set uart_dispatcher to be the dispatcher when there's data in the input
     * buffer or space available in the output buffer */
    mgos_uart_set_dispatcher(UART_NO, uart_dispatcher, NULL /* arg */);
    mgos_uart_set_rx_enabled(UART_NO, true); /* Enable UART receiver */

    /* Setup Wi-Fi event handlers */
    mgos_event_add_group_handler(MGOS_EVENT_GRP_NET, wifi_cb, NULL);

    /* Setup MQTT handlers */
    mgos_mqtt_add_global_handler(mqtt_ev_handler, NULL);

    return MGOS_APP_INIT_SUCCESS;
}
