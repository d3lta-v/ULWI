#include "common.h"
#include "constants.h"

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
    char *token = strtok(target_str, ULWI_DELIMITER);

    int max_param_counter = 0;

    while (token != NULL && max_param_counter < max_params)
    {
        strncpy(result[max_param_counter], token, max_param_len);
        token = strtok(NULL, ULWI_DELIMITER);
        max_param_counter++;
    }

    return max_param_counter;
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: repl_str                                                    *
 *                                                                            *
 * PURPOSE: Replaces strings in a C char array string. 					      *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE    I/O DESCRIPTION                                           *
 * -------- ------- --- -----------                                           *
 * str      char *   I  The string to replace strings for. This function will *
 * 						not alter this string.								  *
 * from     char *   I  The target string to replace from.					  *
 * to       char *   I  The target string to replace to.					  *
 *                                                                            *
 * RETURNS: a char pointer that refers to a string. Needs to be manually      *
 * 			freed by calling free() on the pointer.                           *
 *                                                                            *
 *****************************************************************************/
char *repl_str(const char *str, const char *from, const char *to)
{
	/* Adjust each of the below values to suit your needs. */

	/* Increment positions cache size initially by this number. */
	size_t cache_sz_inc = 16;
	/* Thereafter, each time capacity needs to be increased,
	 * multiply the increment by this factor. */
	const size_t cache_sz_inc_factor = 3;
	/* But never increment capacity by more than this number. */
	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	#if (__STDC_VERSION__ >= 199901L)
	uintptr_t *pos_cache_tmp, *pos_cache = NULL;
	#else
	ptrdiff_t *pos_cache_tmp, *pos_cache = NULL;
	#endif
	size_t cache_sz = 0;
	size_t cpylen, orglen, retlen, tolen, fromlen = strlen(from);

	/* Find all matches and cache their positions. */
	while ((pstr2 = strstr(pstr, from)) != NULL) {
		count++;

		/* Increase the cache size when necessary. */
		if (cache_sz < count) {
			cache_sz += cache_sz_inc;
			pos_cache_tmp = realloc(pos_cache, sizeof(*pos_cache) * cache_sz);
			if (pos_cache_tmp == NULL) {
				goto end_repl_str;
			} else pos_cache = pos_cache_tmp;
			cache_sz_inc *= cache_sz_inc_factor;
			if (cache_sz_inc > cache_sz_inc_max) {
				cache_sz_inc = cache_sz_inc_max;
			}
		}

		pos_cache[count-1] = pstr2 - str;
		pstr = pstr2 + fromlen;
	}

	orglen = pstr - str + strlen(pstr);

	/* Allocate memory for the post-replacement string. */
	if (count > 0) {
		tolen = strlen(to);
		retlen = orglen + (tolen - fromlen) * count;
	} else	retlen = orglen;
	ret = malloc(retlen + 1);
	if (ret == NULL) {
		goto end_repl_str;
	}

	if (count == 0) {
		/* If no matches, then just duplicate the string. */
		strcpy(ret, str);
	} else {
		/* Otherwise, duplicate the string whilst performing
		 * the replacements using the position cache. */
		pret = ret;
		memcpy(pret, str, pos_cache[0]);
		pret += pos_cache[0];
		for (i = 0; i < count; i++) {
			memcpy(pret, to, tolen);
			pret += tolen;
			pstr = str + pos_cache[i] + fromlen;
			cpylen = (i == count-1 ? orglen : pos_cache[i+1]) - pos_cache[i] - fromlen;
			memcpy(pret, pstr, cpylen);
			pret += cpylen;
		}
		ret[retlen] = '\0';
	}

end_repl_str:
	/* Free the cache and return the post-replacement string,
	 * which will be NULL in the event of an error. */
	free(pos_cache);
	return ret;
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: ulwi_validate_strlen                                        *
 *                                                                            *
 * PURPOSE: Validates a string length by checking the raw length integer	  *
 * 			against an upper and a lower boundary. 							  *
 * 																			  *
 * NOTE: `mg_str` lengths typically do not include null termination			  *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE    I/O DESCRIPTION                                           *
 * -------- ------- --- -----------                                           *
 * length   size_t   I  The length of the string to check					  *
 * lower    size_t   I  Lower boundary (inclusive) length to check for		  *
 * upper    size_t   I  Upper boundary (inclusive) length to check for		  *
 *                                                                            *
 * RETURNS: a str_len_state enum that indicates that the string is within	  *
 * 			acceptable margins (STRING_OK), too short (STRING_SHORT) or too	  *
 * 			long (STRING_LONG).												  *
 *                                                                            *
 *****************************************************************************/
enum str_len_state ulwi_validate_strlen(size_t length, size_t lower, size_t upper)
{
	if (length >= lower && length <= upper)
	{
		return STRING_OK;
	}
	else if (length < lower)
	{
		return STRING_SHORT;
	}
	else
	{
		return STRING_LONG;
	}
}

/******************************************************************************
 *                                                                            *
 * FUNCTION NAME: ulwi_cpy_params_only                                        *
 *                                                                            *
 * PURPOSE: Almost functionally identical to strlcpy, except that it only	  *
 * 			copies the parameters from a command to a C style string. This	  *
 * 			function accomplishes this by advancing the source pointer by 4   *
 * 			indices (which covers the first 3 characters of the command + a	  *
 * 			space), and by reducing the maximum length by 3 (from the 4		  *
 * 			deducted characters minus 1 null termination char). This function *
 * 			performs NO bounds checking and it must be done elsewhere!		  *
 *                                                                            *
 * ARGUMENTS:                                                                 *
 *                                                                            *
 * ARGUMENT TYPE    I/O DESCRIPTION                                           *
 * -------- ------- --- -----------                                           *
 * target   char *   O  The target to copy the string _to_					  *
 * source   char *   I  The place to copy the string _from_				      *
 * len      size_t   I  The length of the entire command, not just the params *
 *                                                                            *
 * RETURNS: a boolean indicating whether the copy was successful			  *
 *                                                                            *
 *****************************************************************************/
bool ulwi_cpy_params_only(char *target, const char *source, const size_t len)
{
	if (len <= 3)
	{
		/* Not enough characters to copy */
		return false;
	}
	strlcpy(target, source + 4, len - 3); /* length is reduced by 3 because of the command length of 3 */
	return true;
}
