/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#ifndef SCANNER_H
#define SCANNER_H

#include "string.h"

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
	LINE_COMMENT,
	DIV_OR_COMMENT,
	BLOCK_COMMENT,
	BLOCK_COMMENT_END
} states;

typedef enum {
		LEXICAL_ERROR = 0, IDENTIFIER , AS, ASC, DECLARE, DIM, DO, DOUBLE, ELSE, END, CHR, FUNCTION, IF, INPUT,
		INTEGER, LENGTH, LOOP, PRINT, RETURN, SCOPE, STRING, SUBSTRING, THEN, WHILE, AND, BOOLE,
		CONTINUE, ELSEIF, EXIT, FALSE, FOR, NEXT, NOT, OR, SHARED, STATIC, TRUE, SEMICOLON, ADD, SUB, MUL, DIV, DIV2,
		LESS_THAN, GREATER_THAN, LESS_OR_EQUALS, GREATER_OR_EQUALS, EQUALS, NOT_EQUALS, LEFT_PARANTHESIS, RIGHT_PARANTHESIS,
		DOUBLEE, INT_WITH_EXP, DOUBLE_WITH_EXP, INT_VALUE, STRING_VALUE, COMA, KEY_WORD
} token_type ;

bool is_keyword(char *str);
bool is_validID(char *str);

int get_token(FILE *f, token *t);
int save_token(token *t, String *str, int type);

#endif