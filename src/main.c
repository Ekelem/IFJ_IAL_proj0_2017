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

char * tok_names [] = {"LEXICAL_ERROR", "IDENTIFIER" , "AS", "DECLARE", "DIM", "DO", "DOUBLE", "ELSE", "END", "FUNCTION", "IF", "INPUT",
		"INTEGER", "LOOP", "PRINT", "RETURN", "SCOPE", "STRING", "THEN", "WHILE", "AND", "BOOLEAN",
		"CONTINUE", "ELSEIF", "EXIT", "FALSE", "FOR", "NEXT", "NOT", "OR", "SHARED", "STATIC", "TRUE", "SEMICOLON", "ADD", "SUB", "MUL", "DIV", "DIV2",
		"LESS_THAN", "GREATER_THAN", "LESS_OR_EQUALS", "GREATER_OR_EQUALS", "EQUALS", "NOT_EQUALS", "LEFT_PARANTHESIS", "RIGHT_PARANTHESIS",
		"DOUBLEE", "INT_WITH_EXP", "DOUBLE_WITH_EXP", "INT_VALUE", "STRING_VALUE", "COMA", "KEY_WORD", "INT_2", "INT_8", "INT_16", "NEW_LINE"};


int main(int argc, char const *argv[])
{
	token_buffer * token_buff = init_token_buffer();
	htab_t * symtable = initialization(token_buff);
	String primal_code;

	translate(token_buff, symtable, &primal_code);	//makes translation
	fputs(primal_code.str, stdout); 				//prints IFJ17code assembler

	free_tokens(token_buff);
	htab_free(symtable);
	garbage_collect();
	free_string(&primal_code);
	return 0;
}




