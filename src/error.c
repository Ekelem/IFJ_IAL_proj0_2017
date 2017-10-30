/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#include "error.h"

extern char * tok_names [];

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

void syntax_error_unexpexted(int line, int pos, int unexpected_type, int numb, ...)
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