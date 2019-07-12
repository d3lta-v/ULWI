/******************************************************************************
 *                                                                            *
 * NAME: constants.h                                                          *
 *                                                                            *
 * PURPOSE: Defines global constants such as commands                         *
 *                                                                            *
 * GLOBAL VARIABLES:                                                          *
 *                                                                            *
 * Variable Type Description                                                  *
 * -------- ---- -----------                                                  *
 *                                                                            *
 * DEVELOPMENT HISTORY:                                                       *
 *                                                                            *
 * Date Author Change Id Release Description Of Change                        *
 * ---- ------ --------- ------- ---------------------                        *
 *                                                                            *
 *****************************************************************************/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "mgos.h"

#define UART_NO 0

#define HTTP_HANDLES_MAX 3

static const char delimiter[] = "\x1f";

/* General operations commands */
static const struct mg_str COMMAND_NOP = MG_MK_STR("nop");
static const struct mg_str COMMAND_VER = MG_MK_STR("ver");
static const struct mg_str COMMAND_RST = MG_MK_STR("rst");
static const struct mg_str COMMAND_OTA = MG_MK_STR("ota");

/* Access Point commands */
static const struct mg_str COMMAND_LAP = MG_MK_STR("lap");
static const struct mg_str COMMAND_CAP = MG_MK_STR("cap");
static const struct mg_str COMMAND_SAP = MG_MK_STR("sap");
static const struct mg_str COMMAND_DAP = MG_MK_STR("dap");

/* IP related commands */
static const struct mg_str COMMAND_GIP = MG_MK_STR("gip");

/* HTTP Request commands */
static const struct mg_str COMMAND_IHR = MG_MK_STR("ihr");
static const struct mg_str COMMAND_PHR = MG_MK_STR("phr");
static const struct mg_str COMMAND_CHR = MG_MK_STR("chr");
static const struct mg_str COMMAND_HHR = MG_MK_STR("hhr");
static const struct mg_str COMMAND_THR = MG_MK_STR("thr");
static const struct mg_str COMMAND_SHR = MG_MK_STR("shr");
static const struct mg_str COMMAND_GHR = MG_MK_STR("ghr");

#endif