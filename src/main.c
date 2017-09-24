/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "scanner.h"
#include "string.h"
#include "error.h"

/* For testing purposes */
void print_token(int token);

int main(int argc, char const *argv[])
{
	//TODO: check argument

	//TODO: open file (safely) for scanner

	//TODO: symbol table

	//TODO: scanner (lexical analysis) + parser (semantic, syntax)

	/*
	error_msg(ERR_CODE_INTERN, "line %d, missing operand.\n", 15); */

	if (argc != 2) {
		fprintf(stderr, "Wrong arguments\n");
		exit(100);
	}

	FILE *f;
	if ((f = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "We could not open the file: %s\n", argv[1]);
	}

	int state = 0;
	while ( state != EOF) {
		token t;
		state = get_token(f, &t);
		print_token(state);
	}

	printf("\n");

	fclose(f);
	return 0;
}

/*** Testing lexical analysis ***/
void print_token(int token)
{
	switch (token)
	{
		case ADD : printf("ADD "); break;
		case SUB : printf("SUB "); break;
		case DIV : printf("DIV "); break;
		case DIV2 : printf("DIV2 "); break;
		case MUL: printf("MUL "); break;
		case IDENTIFICATOR : printf("IDENTIFICATOR "); break;
		case SEMICOLON : printf("SEMICOLON "); break;
		case LESS_THAN: printf("LESS_THAN "); break;
		case GREATER_THAN : printf("GREATER_THAN "); break;
		case LESS_OR_EQUALS : printf("LESS_OR_EQUALS "); break;
		case GREATER_OR_EQUALS : printf("GREATER_OR_EQUALS "); break;
		case EQUALS: printf("EQUALS "); break;
		case NOT_EQUALS : printf("NOT_EQUALS "); break;
		case LEFT_PARANTHESIS : printf("LEFT_PARANTHESIS "); break;
		case RIGHT_PARANTHESIS : printf("RIGTH_PARANTHESIS "); break;
		case DOUBLEE : printf("DOUBLE "); break;
		case INT_WITH_EXP : printf("INT_WITH_EXP "); break;
		case DOUBLE_WITH_EXP : printf("DOUBLE_WITH_EXP "); break;
		case INT_VALUE : printf("INT_VALUE "); break;
		case STRING_VALUE : printf("STRING_VALUE "); break;
		case LEXICAL_ERROR: printf("LEXICAL_ERROR "); break;
		case COMA : printf("COMA "); break;
		case EOF : printf("EOF"); break;
	}
}