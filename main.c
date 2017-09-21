/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#include "error.h"

int main(int argc, char const *argv[])
{
	//TODO: check argument
	
	//TODO: open file (safely) for scanner
	
	//TODO: symbol table
	
	//TODO: scanner (lexical analysis) + parser (semantic, syntax)

	error_msg(ERR_CODE_INTERN, "line %d, missing operand.\n", 15);
	return 0;
}