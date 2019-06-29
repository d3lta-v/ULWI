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

/* TODO: Comment out the definition if in production!! */
#define DEVELOPMENT

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
static void timer_cb(void *arg) {
    static bool s_tick_tock = false;
    LOG(LL_INFO,
        ("uptime: %.2lf, RAM: %lu, %lu free",
        mgos_uptime(), (unsigned long) mgos_get_heap_size(),
        (unsigned long) mgos_get_free_heap_size()));
    s_tick_tock = !s_tick_tock;
    (void) arg;
}
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
    /* Retrieve pointer of the last character, in this case it's the newline */
    char *line_ending = (char *)mg_strchr(mg_mk_str_n(buffer.buf, buffer.len), '\n');
    if (line_ending == NULL)
    {
        /* Unable to find newline or line termination, ignoring input */
        return;
    }
    *line_ending = '\0'; /* Null terminate the string, replacing newline with NULL*/
    size_t line_length = line_ending - buffer.buf;
    /* Creates an mg_str (basically a string) from buffer.buf (which is a C array) */
    struct mg_str line = mg_mk_str_n(buffer.buf, line_length);
    /* Check for CR as well. Remember that valid commands are terminated with
     * \r\n, not \n, so that content can be terminated with \n */
    if (line_ending > buffer.buf && *(line_ending - 1) == '\r')
    {
        /* CR found, command is valid, remove the \r */
        *(line_ending - 1) = '\0';
        line.len--;
    }
    else
    {
        /* CR not found, command might be incomplete, but might be newline in content */
#ifdef DEVELOPMENT
        LOG(LL_WARN, ("Command might be incomplete!"));
#endif
        return;
    }

    /* Now we can process the line itself, because we've null terminated the
     * line correctly (it's a C string now) and removed newline characters */

    /* The sample line comparisons here are just for illustration purposes */

    /* Phase I: basic command sanitisation */
    if (line_length < (3 + 1)) /* Keep into account the null character for termination */
    {
        mgos_uart_printf(UART_NO, "short\r\n");
    }
    else
    {
        /* Phase II: string comparison to sort out the commands */
        if (mg_str_starts_with(line, COMMAND_NOP) && line.len == 3)
        {
            mgos_uart_printf(UART_NO, "\r\n");
        }
        else if (mg_str_starts_with(line, COMMAND_VER) && line.len == 3)
        {
            mgos_uart_printf(UART_NO, "v1.0-alpha1\r\n");
        } 
        else if (mg_str_starts_with(line, COMMAND_RST) && line.len == 3)
        {
            mgos_system_restart();
        }
        else if (mg_str_starts_with(line, COMMAND_LAP) && line.len == 3)
        {
            mgos_wifi_scan(wifi_scan_cb, NULL);
        }
        else if (mg_str_starts_with(line, COMMAND_CAP) && line.len > 4)
        {
            const size_t parameter_len = line.len - 4;
            char parameter_c_str[130] = {0}; /* 128 + 2 null termination chars */
            strncpy(parameter_c_str, line.p+4, parameter_len);

            const int max_params = 2;
            const int max_param_len = 65;
            char result[max_params][max_param_len];

            const int param_len = split_parameter_string(parameter_c_str, max_params, max_param_len, result);
            
            if (param_len == 2)
            {
                /* Initiate wifi */
                const struct mgos_config_wifi_sta wifi_config = { true, result[0], result[1] };
                mgos_wifi_setup_sta(&wifi_config);
            }
            else
            {
                mgos_uart_printf(UART_NO, "invalid\r\n");
            }
        }
        else if (mg_str_starts_with(line, COMMAND_SAP) && line.len == 3)
        {
            const enum mgos_wifi_status wifi_status = mgos_wifi_get_status();
            const char *ssid = mgos_wifi_get_connected_ssid();
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
    mgos_set_timer(10000, MGOS_TIMER_REPEAT, timer_cb, NULL); /* Enable debug timer */
    cs_log_set_level(LL_INFO); /* Enable logging, but not too verbose */
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

    return MGOS_APP_INIT_SUCCESS;
}
