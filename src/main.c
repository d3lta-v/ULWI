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
 * DEVELOPMENT HISTORY:                                                       *
 *                                                                            *
 * Date Author Change Id Release Description Of Change                        *
 * ---- ------ --------- ------- ---------------------                        *
 *                                                                            *
 * ALGORITHM (PDL)                                                            *
 *                                                                            *
 *****************************************************************************/



#include "mgos.h"

#define UART_NO 0

static void timer_cb(void *arg)
{
    /*
     * Note: do not use mgos_uart_write to output to console UART (0 in our case).
     * It will work, but output may be scrambled by console debug output.
     */
    mgos_uart_printf(UART_NO, "Hello, UART1!\r\n");
    (void)arg;
}

/*
 * Dispatcher can be invoked with any amount of data (even none at all) and
 * at any time. Here we demonstrate how to process input line by line.
 */
static void uart_dispatcher(int uart_no, void *arg)
{
    /* Phase 1: allocate buffer */
    static struct mbuf buffer = {0}; // Make an empty mbuf (memory buffer) struct
    assert(uart_no == UART_NO);      // Just to make sure that we're reading on the correct UART

    /* Phase 2: Check input size */
    size_t available_size = mgos_uart_read_avail(uart_no);
    if (available_size == 0)
        return; // No need to process anything if the number of bytes is 0

    /* Phase 3: Read input into buffer */
    mgos_uart_read_mbuf(uart_no, &buffer, available_size);                            // Reads the data into an mbuf struct (don't use the non-mbuf variant as it lacks information!)
    char *line_ending = (char *)mg_strchr(mg_mk_str_n(buffer.buf, buffer.len), '\n'); // Locates the pointer to the last character in a string, in this case it's the newline
    if (line_ending == NULL)
        return; // Newline character not found, ignoring input
    *line_ending = '\0'; // Null terminate the line by replacing newline with NULL
    size_t line_length = line_ending - buffer.buf;
    struct mg_str line = mg_mk_str_n(buffer.buf, line_length); // Creates an mg_str (basically a string) from buffer.buf (which is a C array)
    /* Because Windows exists and we love it so much, check for CR as well. */
    if (line_ending > buffer.buf && *(line_ending - 1) == '\r')
    {
        *(line_ending - 1) = '\0';
        line.len--;
    }

    /* Now we can process the line itself, because we've null terminated the
     * line (it's a C string now) */
    if (mg_vcasecmp(&line, "hi") == 0)
    { 
        /* Check if line says "hi" */
        mgos_uart_printf(UART_NO, "Hello!\r\n");
    }
    else
    {
        /* Read back the line itself */
        mgos_uart_printf(UART_NO, "You said '%.*s'.\r\n", (int)line.len, line.p);
    }

    mbuf_remove(&buffer, line_length + 1); /* Release the buffer */
    (void)arg;
}

enum mgos_app_init_result mgos_app_init(void)
{
    /* Comment the 3 lines below to enable logging */
    mgos_set_stdout_uart(-1);
    mgos_set_stderr_uart(-1);
    cs_log_set_level(LL_NONE);

    /* Configure UART port */
    struct mgos_uart_config ucfg;
    mgos_uart_config_set_defaults(UART_NO, &ucfg);
    /*
     * At this point it is possible to adjust baud rate, pins and other settings.
     * 9600 8-N-1 is the default mode, but we set it anyway
     */
    ucfg.baud_rate = 9600;
    ucfg.num_data_bits = 8;
    ucfg.parity = MGOS_UART_PARITY_NONE;
    ucfg.stop_bits = MGOS_UART_STOP_BITS_1;
    if (!mgos_uart_configure(UART_NO, &ucfg))
    {
        return MGOS_APP_INIT_ERROR;
    }

    /* Timer to repeat timer_cb every 1s which simply prints "Hello UART1!" */
    mgos_set_timer(1000 /* ms */, true /* repeat */, timer_cb, NULL /* arg */);

    /* Set uart_dispatcher to be the dispatcher when there's data in the input
     * buffer or space available in the output buffer */
    mgos_uart_set_dispatcher(UART_NO, uart_dispatcher, NULL /* arg */);
    mgos_uart_set_rx_enabled(UART_NO, true); // Enable UART receiver

    return MGOS_APP_INIT_SUCCESS;
}
