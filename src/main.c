/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"

#define UART_NO 1

enum mgos_app_init_result mgos_app_init(void)
{
    // Set log level to VERBOSE DEBUG, which is the highest level
    cs_log_set_level(LL_VERBOSE_DEBUG);
    LOG(LL_INFO, ("Hello World"));

    // Configure UART port
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

    // mgos_set_timer(1000 /* ms */, true /* repeat */, timer_cb, NULL /* arg */);

    // mgos_uart_set_dispatcher(UART_NO, uart_dispatcher, NULL /* arg */);
    // mgos_uart_set_rx_enabled(UART_NO, true);

    LOG(LL_INFO,
    ("* Send some data to UART%d (don't forget to press Enter) *", UART_NO));

    return MGOS_APP_INIT_SUCCESS;
}
