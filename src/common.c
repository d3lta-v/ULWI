#include "common.h"

static const char delimiter[] = "\x1f";

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: split_parameter_string                                      *
 *                                                                            *
 * PURPOSE: Split up a string (char array) into multiple strings, using the   *
 *          ASCII Unit Separator character as a delimiter, returning a        *
 *          multidimensional array and actual number of parameters present    *
 *          in that array.                                                    *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT       TYPE    I/O DESCRIPTION                                     *
 * -------------- ------- --- -----------                                     *
 * target_str     char *   I  The target string to split                      *
 * max_params     int      I  The maximum number of parameters expected, no   *
 *                            more than this no. of parameters will be split  *
 * max_param_len  int      I  Maximum length expected for each parameter      *
 * result         char **  O  Splitted strings as a two-dimensional array     *
 *                                                                            *
 * RETURNS: the actual number of parameters presented by the target string,   *
 *          useful for bounds checking                                        *
 *                                                                            *
 *****************************************************************************/
int split_parameter_string(char *target_str, const int max_params, const int max_param_len, char result[max_params][max_param_len])
{
    char *token = strtok(target_str, delimiter);

    int max_param_counter = 0;

    while (token != NULL && max_param_counter < max_params)
    {
        strncpy(result[max_param_counter], token, max_param_len);
        token = strtok(NULL, delimiter);
        max_param_counter++;
    }

    return max_param_counter;
}
