/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#ifndef ERROR_H
#define ERROR_H 42

/*
 *chyba v programu v ramci lexikalni analyzy (chybna struktura aktualniho lexe-
 *mu)
*/
#define ERR_CODE_LEXICAL 1


/*
 *chyba v programu v ramci syntakticke analyzy (chybna syntaxe programu)
*/
#define ERR_CODE_SYNTAX 2


/*
 *semanticka chyba v programu – nedefinovana funkce/promenna, pokus o redefi-
 *nici funkce/promenne, atd.
*/
#define ERR_CODE_UNDEFINED 3


/*
 *semanticka chyba typove kompatibility v aritmetickych, retezcovych a relacnich
 *vyrazech, prip. spatny pocet ci typ parametru u volani funkce.
*/
#define ERR_CODE_TYPE 4


/*
 *ostatni semanticke chyby
*/
#define ERR_CODE_OTHERS 6


/*
 *interni chyba prekladace tj. neovlivnena vstupnim programem (napr. chyba alo-
 *kace pameti, atd.)
*/
#define ERR_CODE_INTERN 99

#define ERRPREFIX "Error: "
#define WARNPREFIX "Warning: "

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


void error_msg(int err_code, const char *fmt, ...);
void warn_msg(const char *fmt, ...);
void syntax_error_unexpexted(int line, int pos, int unexpected_type, int numb, ...);


#endif