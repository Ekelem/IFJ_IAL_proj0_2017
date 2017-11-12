#ifndef H_PARSER
#define H_PARSER 42

#include "error.h"
#include "symtable.h"
#include "scanner.h"
#include "tokens.h"
#include "stack.h"

enum label_names
{
	label_func,
	label_if,
	label_else,
	label_end_if
}typedef enum_label_names;

void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void neterm_start(token_buffer * token_buff, htab_t * symtable, String * primal_code);


void neterm_function_dec(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void neterm_function_def(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void neterm_scope(token_buffer * token_buff, htab_t * symtable, String * primal_code);

void neterm_args(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record);
void neterm_args_create(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * empty_func_record);

void neterm_body(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_declaration(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_input(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_print(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_if_then(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_do_while(token_buffer * token_buff, htab_t * symtable, String * primal_code);
void body_assignment(token_buffer * token_buff, htab_t * symtable, String * primal_code, token * identifier);

void neterm_expression(token_buffer * token_buff, htab_t * symtable, String * primal_code, token_type end_token);

unsigned int neterm_type(token_buffer * token_buff, htab_t * symtable, String * primal_code);

void expected_token(token_buffer * token_buff, int tok_type);
bool is_peek_token(token_buffer * token_buff, int tok_type);

unsigned int generate_if_label_order();
void generate_if_label(String * primal_code, enum_label_names prefix, unsigned int order);

#endif