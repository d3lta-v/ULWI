/******************************************************************************
 *                                                                            *
 * NAME: wifi.h                                                               *
 *                                                                            *
 * PURPOSE: Provides Wi-Fi helper functions and data                          *
 *                                                                            *
 * GLOBAL VARIABLES:                                                          *
 *                                                                            *
 * Variable Type Description                                                  *
 * -------- ---- -----------                                                  *
 *                                                                            *
 *                                                                            *
 *****************************************************************************/

#ifndef WIFI_H
#define WIFI_H

#include "mgos.h"

void wifi_cb(int ev, void *evd, void *arg);
void wifi_scan_cb(int n, struct mgos_wifi_scan_result *res, void *arg);

#endif