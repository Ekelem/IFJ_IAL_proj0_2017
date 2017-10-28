/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"
//#include "tokens.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define KEYWORD_COUNT 22

typedef union attribute {
	int int_value;
	double double_value;
	char *string_value;
} attribute;

typedef struct {
	int type;
	attribute attr;
} token;


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

typedef enum {
		LEXICAL_ERROR = 0, IDENTIFIER , AS, ASC, DECLARE, DIM, DO, DOUBLE, ELSE, END, CHR, FUNCTION, IF, INPUT,
		INTEGER, LENGTH, LOOP, PRINT, RETURN, SCOPE, STRING, SUBSTRING, THEN, WHILE, AND, BOOLEAN,
		CONTINUE, ELSEIF, EXIT, FALSE, FOR, NEXT, NOT, OR, SHARED, STATIC, TRUE, SEMICOLON, ADD, SUB, MUL, DIV, DIV2,
		LESS_THAN, GREATER_THAN, LESS_OR_EQUALS, GREATER_OR_EQUALS, EQUALS, NOT_EQUALS, LEFT_PARANTHESIS, RIGHT_PARANTHESIS,
		DOUBLEE, INT_WITH_EXP, DOUBLE_WITH_EXP, INT_VALUE, STRING_VALUE, COMA, KEY_WORD, INT_2, INT_8, INT_16, NEW_LINE,
		ADD_EQUALS, SUB_EQUALS, MUL_EQUALS, DIV_EQUALS, DIV2_EQUALS
} token_type ;

bool is_keyword(char *str, token *t);
bool is_validID(char *str);

int base_to_int(char *str, int base);

token * get_token(FILE *f);
token * save_token(token *t, String *str, int type);

#endif