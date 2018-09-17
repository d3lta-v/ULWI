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

static const struct mg_str COMMAND_NOP = MG_MK_STR("nop");
static const struct mg_str COMMAND_VER = MG_MK_STR("ver");
static const struct mg_str COMMAND_RST = MG_MK_STR("rst");
static const struct mg_str COMMAND_LAP = MG_MK_STR("lap");

#endif