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
#include "stack.h"
#include "garbage_collector.h"
#include "tokens.h"
#include "common.h"
#include "parser.h"

struct dynamic_stack * GARBAGE_COLLECTOR = NULL;


// AFTER TESTING LEXICAL_ANALYSIS THIS CAN BE REMOVED
char *key_words2[] = { "as", "asc", "declare", "dim", "do", "double", "else", "end", "chr", "function",
					  "if", "input", "integer", "length", "loop", "print", "return", "scope", "string",
					  "substr", "then", "while"};

char * tok_names [] = {"LEXICAL_ERROR", "IDENTIFIER" , "AS", "ASC", "DECLARE", "DIM", "DO", "DOUBLE", "ELSE", "END", "CHR", "FUNCTION", "IF", "INPUT",
		"INTEGER", "LENGTH", "LOOP", "PRINT", "RETURN", "SCOPE", "STRING", "SUBSTRING", "THEN", "WHILE", "AND", "BOOLEAN",
		"CONTINUE", "ELSEIF", "EXIT", "FALSE", "FOR", "NEXT", "NOT", "OR", "SHARED", "STATIC", "TRUE", "SEMICOLON", "ADD", "SUB", "MUL", "DIV", "DIV2",
		"LESS_THAN", "GREATER_THAN", "LESS_OR_EQUALS", "GREATER_OR_EQUALS", "EQUALS, NOT_EQUALS", "LEFT_PARANTHESIS", "RIGHT_PARANTHESIS",
		"DOUBLEE", "INT_WITH_EXP", "DOUBLE_WITH_EXP", "INT_VALUE", "STRING_VALUE", "COMA", "KEY_WORD", "INT_2", "INT_8", "INT_16", "NEW_LINE"};

/* For testing purposes */
void print_token(int tk, token *t);

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Wrong arguments\n");
		return ERR_CODE_LEXICAL;
	}

	const char * file_path = argv[1];
	token_buffer * token_buff = init_token_buffer();
	htab_t * symtable = initialization(file_path, token_buff);
	String primal_code;

	translate(token_buff, symtable, &primal_code);

	free_tokens(token_buff);
	htab_free(symtable);
	garbage_collect();

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

