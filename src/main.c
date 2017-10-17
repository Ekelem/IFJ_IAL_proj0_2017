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
#include "symtable.h"


// AFTER TESTING LEXICAL_ANALYSIS THIS CAN BE REMOVED
char *key_words2[] = { "as", "asc", "declare", "dim", "do", "double", "else", "end", "chr", "function",
					  "if", "input", "integer", "length", "loop", "print", "return", "scope", "string",
					  "substr", "then", "while"};

/* For testing purposes */
void print_token(int tk, token *t);

int main(int argc, char const *argv[])
{

	if (argc != 2) {
		fprintf(stderr, "Wrong arguments\n");
		return 100;
	}

	FILE *f;
	if ((f = fopen(argv[1], "r")) == NULL) {
		fprintf(stderr, "We could not open the file: %s\n", argv[1]);
		return 100;
	}
	htab_t * symtable=htab_init(1024);

	int state = 0;
	while ( state != EOF) {
		token t;
		t.attr.string_value = NULL;
		state = get_token(f, &t);
		print_token(state, &t);

	}

	printf("\n");

	fclose(f);
	htab_free(symtable);
	return 0;
}

/*** Testing lexical analysis ***/
void print_token(int tk, token *t)
{
	switch (tk)
	{
		case ADD : printf("ADD "); break;
		case SUB : printf("SUB "); break;
		case DIV : printf("DIV "); break;
		case DIV2 : printf("DIV2 "); break;
		case MUL: printf("MUL "); break;
		case IDENTIFICATOR : printf("IDENTIFICATOR :%s: ", t->attr.string_value); break;
		default : printf("KEY_WORD :%s: ", key_words2[t->type - 2]); break;
		case SEMICOLON : printf("SEMICOLON "); break;
		case LESS_THAN: printf("LESS_THAN "); break;
		case GREATER_THAN : printf("GREATER_THAN "); break;
		case LESS_OR_EQUALS : printf("LESS_OR_EQUALS "); break;
		case GREATER_OR_EQUALS : printf("GREATER_OR_EQUALS "); break;
		case EQUALS: printf("EQUALS "); break;
		case NOT_EQUALS : printf("NOT_EQUALS "); break;
		case LEFT_PARANTHESIS : printf("LEFT_PARANTHESIS "); break;
		case RIGHT_PARANTHESIS : printf("RIGTH_PARANTHESIS "); break;
		case DOUBLEE : printf("DOUBLE :%f: ", t->attr.double_value); break;
		case INT_WITH_EXP : printf("INT_WITH_EXP :%f: ", t->attr.double_value); break;
		case DOUBLE_WITH_EXP : printf("DOUBLE_WITH_EXP :%f: ", t->attr.double_value); break;
		case INT_VALUE : printf("INT_VALUE :%d :", t->attr.int_value); break;
		case STRING_VALUE : printf("STRING_VALUE :%s: ", t->attr.string_value); break;
		case LEXICAL_ERROR: printf("LEXICAL_ERROR "); break;
		case COMA : printf("COMA "); break;
		case NEW_LINE: printf("NEW_LINE\n"); break;
		case EOF : printf("EOF "); break;
	}
}

