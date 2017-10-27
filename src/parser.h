#ifndef H_PARSER
#define H_PARSER 42

#include "error.h"
#include "symtable.h"
#include "scanner.h"
#include "tokens.h"

void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void neterm_start(token_buffer * token_buff, htab_t * symtable, String * primal_code);


void neterm_function_dec(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void neterm_function_def(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void neterm_scope(token_buffer * token_buff, htab_t * symtable, String * primal_code);

void neterm_args(token_buffer * token_buff, htab_t * symtable, String * primal_code);

void neterm_body(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_declaration(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_input(token_buffer * token_buff, htab_t * symtable, String * primal_code);

unsigned int neterm_type(token_buffer * token_buff, htab_t * symtable, String * primal_code);

//void expected_token(FILE * fptr, int tok_type);	//maybe later

#endif