/*
 * IFJ17 Compiler Project, FIT VUT Brno 2017
 *
 * Authors:
 * Erik Kelemen    - xkelem01
 * Attila Lakatos  - xlakat01
 * Patrik Sober    - xsober00
 * Tomas Zubrik    - xzubri00
 *
 */

#include "error.h"

extern char * tok_names [];

/* Writes error message and exits program with corresponding error code */
void error_msg(int err_code, const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	fprintf(stderr, "%s", ERRPREFIX );
	vfprintf(stderr, fmt, list);
	va_end(list);
	exit(err_code);
}

/* Writes error warning */
void warn_msg(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	fprintf(stderr, "%s", WARNPREFIX );
	vfprintf(stderr, fmt, list);
	va_end(list);
}

/* Writes number of line and position of error. Exits program with corresponding error code */
void syntax_error_unexpected(int line, int pos, int unexpected_type, int numb, ...)
{
	va_list list;
	va_start(list, numb);
	fprintf(stderr, "Error on line:%d from position:%d : unexpected symbol '%s', expected '%s'", line, pos,tok_names[unexpected_type], tok_names[va_arg(list, int)]);
	for (int i = 0; i < (numb-1); ++i)
	{
		fprintf(stderr, " or '%s'", tok_names[va_arg(list, int)]);
	}
	fputc('\n', stderr);
	exit(ERR_CODE_SYNTAX);
}