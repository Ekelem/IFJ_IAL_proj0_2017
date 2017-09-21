/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#include "error.h"

void error_msg(int err_code, const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	fprintf(stderr, "%s", ERRPREFIX );
	vfprintf(stderr, fmt, list);
	va_end(list);
	exit(err_code);
}

void warn_msg(const char *fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	fprintf(stderr, "%s", WARNPREFIX );
	vfprintf(stderr, fmt, list);
	va_end(list);
}