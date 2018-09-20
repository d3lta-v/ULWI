#include "wifi.h"

#include "constants.h"

int compare_larger_rssi(const void *s1, const void *s2)
{
  struct mgos_wifi_scan_result *e1 = (struct mgos_wifi_scan_result *)s1;
  struct mgos_wifi_scan_result *e2 = (struct mgos_wifi_scan_result *)s2;
  return e2->rssi - e1->rssi;
}

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
    qsort(res, n, sizeof(struct mgos_wifi_scan_result), compare_larger_rssi); /* Sort by RSSI descending */
    mgos_uart_printf(UART_NO, "%d hotspots detected: ", n, arg);
    for (int i = 0; i < n; i++)
    {
        if (i > 0)
        {
            mgos_uart_printf(UART_NO, ",");
        }
        mgos_uart_printf(UART_NO, "%s", res[i].ssid);
    }
    mgos_uart_printf(UART_NO, "\r\n");
}
