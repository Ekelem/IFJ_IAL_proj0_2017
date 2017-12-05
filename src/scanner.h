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

#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>

#define KEYWORD_COUNT 31

/* Attribute of token structure */
typedef union attribute {
	int int_value;
	double double_value;
	char *string_value;
} attribute;

/* Token structure */
typedef struct {
	int type;
	int line, pos;
	attribute attr;
} token;

/* States enumeration */
typedef enum {
	WHITE_SPACE = 0,
	IDENTIFICATOR,
	COMPARE_LESS,
	COMPARE_GREATER,
	NUMBER,
	DOUBLE_1,
	DOUBLE_2,
	DOUBLE_3,
	EXCLAMATION_MARK,
	STRING_LITERAL_BEGINS,
	STRING_LITERAL_ENDS,
	UNUSUAL_CHAR,
	UNUSUAL_CHAR_2,
	LINE_COMMENT,
	DIV_OR_COMMENT,
	BLOCK_COMMENT,
	BLOCK_COMMENT_END,
	BASE,
	BASE_2,
	BASE_8,
	BASE_16,
	ADD_EQ,
	SUB_EQ,
	MUL_EQ,
	DIV_EQ,
	DIV2_EQ
} states;

/* Token types enumeration */
typedef enum {
		LEXICAL_ERROR = 0, IDENTIFIER , AS, DECLARE, DIM, DO, DOUBLE, ELSE, END, FUNCTION, IF, INPUT,
		INTEGER, LOOP, PRINT, RETURN, SCOPE, STRING, THEN, WHILE, AND, OR, BOOLEAN,
		CONTINUE, ELSEIF, EXIT, FALSE, FOR, NEXT, NOT, SHARED, STATIC, TRUE, SEMICOLON, ADD, SUB, MUL, DIV, DIV2,
		LESS_THAN, GREATER_THAN, LESS_OR_EQUALS, GREATER_OR_EQUALS, EQUALS, NOT_EQUALS, LEFT_PARANTHESIS, RIGHT_PARANTHESIS,
		DOUBLEE, INT_WITH_EXP, DOUBLE_WITH_EXP, INT_VALUE, STRING_VALUE, COMA, KEY_WORD, INT_2, INT_8, INT_16, NEW_LINE,
} token_type ;

/* Checks if sequence is keyword */
bool is_keyword(char *str, token *t);

/* Checks if sequence is valid identifier sequence*/
bool is_validID(char *str);

/* Converts number from base 2,8,16 to integer*/
int base_to_int(char *str, int base);

/* Gets next token*/
token * get_token(FILE *f, int *err_line, int *err_pos);

/* Saves token*/
token * save_token(token *t, String *str, int type, int line, int pos);

#endif