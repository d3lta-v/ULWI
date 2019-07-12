/******************************************************************************
 *                                                                            *
 * NAME: common.h                                                             *
 *                                                                            *
 * PURPOSE: Provides common helper functionality                              *
 *                                                                            *
 * GLOBAL VARIABLES:                                                          *
 *                                                                            *
 * Variable Type Description                                                  *
 * -------- ---- -----------                                                  *
 *                                                                            *
 *                                                                            *
 *****************************************************************************/

#ifndef COMMON_H
#define COMMON_H

#include "mgos.h"

enum boolean
{
    ULWI_TRUE = 'T',
    ULWI_FALSE = 'F'
};

int split_parameter_string(char *target_str, const int max_params, const int max_param_len, char result[max_params][max_param_len]);
char *repl_str(const char *str, const char *from, const char *to);

#endif