#include "wifi.h"

#include "constants.h"

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: wifi_cb                                                     *
 *                                                                            *
 * PURPOSE: Callback function that is called when a change in Wi-Fi state is  *
 *          detected.                                                         *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE    I/O DESCRIPTION                                           *
 * -------- ------- --- -----------                                           *
 * ev       int      I                                                        *
 * evd      void     O                                                        *
 * arg      void     O                                                        *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
void wifi_cb(int ev, void *evd, void *arg)
{
    const struct mgos_net_event_data *ev_data =
        (const struct mgos_net_event_data *)evd;
    if (ev_data->if_type != MGOS_NET_IF_TYPE_WIFI)
        return;
    // LOG(LL_INFO, ("WiFi change event: %d, arg %p", (int)ev, arg));
    (void)&ev;
    (void)arg;
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: wifi_scan_cb                                                *
 *                                                                            *
 * PURPOSE: Callback function that is called when the device has completed    *
 *          scanning for Wi-Fi devices                                        *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE    I/O DESCRIPTION                                           *
 * -------- ------- --- -----------                                           *
 * n        int      I  Number of Wi-Fi hotspots scanned                      *
 * res      struct   O  Result of the scan in an mgos_wifi_scan_result struct *
 * arg      void     O                                                        *
 *                                                                            *
 * RETURNS: none                                                              *
 *                                                                            *
 *****************************************************************************/
void wifi_scan_cb(int n, struct mgos_wifi_scan_result *res, void *arg)
{
    mgos_uart_printf(UART_NO, "WiFi scan result: SSIDs %d, arg %p, results:", n, arg);
    for (int i = 0; i < n; i++)
    {
        mgos_uart_printf(UART_NO, "  SSID: %-32s, auth: %2d, channel: %2d, RSSI: %2d",
                      res[i].ssid, res[i].auth_mode, res[i].channel, res[i].rssi);
    }
    mgos_uart_printf(UART_NO, "WiFi scan completed\r\n");
}
