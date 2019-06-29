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
void wifi_cb(int ev, void *evd, void *arg) {
    switch (ev) {
    case MGOS_WIFI_EV_STA_DISCONNECTED: {
        struct mgos_wifi_sta_disconnected_arg *da =
            (struct mgos_wifi_sta_disconnected_arg *) evd;
        LOG(LL_INFO, ("WiFi STA disconnected, reason %d", da->reason));
        break;
    }
    case MGOS_WIFI_EV_STA_CONNECTING:
        LOG(LL_INFO, ("WiFi STA connecting %p", arg));
        break;
    case MGOS_WIFI_EV_STA_CONNECTED:
        LOG(LL_INFO, ("WiFi STA connected %p", arg));
        break;
    case MGOS_WIFI_EV_STA_IP_ACQUIRED:
        LOG(LL_INFO, ("WiFi STA IP acquired %p", arg));
        break;
    case MGOS_WIFI_EV_AP_STA_CONNECTED: {
        struct mgos_wifi_ap_sta_connected_arg *aa =
            (struct mgos_wifi_ap_sta_connected_arg *) evd;
        LOG(LL_INFO, ("WiFi AP STA connected MAC %02x:%02x:%02x:%02x:%02x:%02x",
                    aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
                    aa->mac[5]));
        break;
    }
    case MGOS_WIFI_EV_AP_STA_DISCONNECTED: {
        struct mgos_wifi_ap_sta_disconnected_arg *aa =
            (struct mgos_wifi_ap_sta_disconnected_arg *) evd;
        LOG(LL_INFO,
            ("WiFi AP STA disconnected MAC %02x:%02x:%02x:%02x:%02x:%02x",
            aa->mac[0], aa->mac[1], aa->mac[2], aa->mac[3], aa->mac[4],
            aa->mac[5]));
        break;
    }
    }
    (void) arg;
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
    for (int i = 0; i < n; i++)
    {
        if (i > 0)
        {
            mgos_uart_printf(UART_NO, ",");
        }
        mgos_uart_printf(UART_NO, "%s", res[i].ssid);
    }
    mgos_uart_printf(UART_NO, "\r\n");
    (void)arg;
}
