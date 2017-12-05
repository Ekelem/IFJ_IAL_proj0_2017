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

#ifndef ERROR_H
#define ERROR_H 42

/* Program error in lexical analysis (wrong structure of actual lexem)*/
#define ERR_CODE_LEXICAL 1

/* Program error in syntactic analysis (wrong syntax of program)*/
#define ERR_CODE_SYNTAX 2

/* Program semantic error (undefined function/varaible, attempt to redefine function/variable etc.)*/
#define ERR_CODE_SEM 3

/* Semantic errors of type compatibility in arithmetic, string and relational expressions. 
   Eventually, wrong number or type of parameters in function call.*/
#define ERR_CODE_TYPE 4

/* Other semantic errors*/
#define ERR_CODE_OTHERS 6

/* Internal error (f.e. unsuccesful allocation..) */
#define ERR_CODE_INTERN 99

#define ERRPREFIX "Error: "
#define WARNPREFIX "Warning: "

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Writes error message and exits program with corresponding error code */
void error_msg(int err_code, const char *fmt, ...);

/* Writes error warning */
void warn_msg(const char *fmt, ...);

/* Writes number of line and position of error. Exits program with corresponding error code */
void syntax_error_unexpexted(int line, int pos, int unexpected_type, int numb, ...);


#endif