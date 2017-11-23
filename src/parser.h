#ifndef H_PARSER
#define H_PARSER 42

#include "error.h"
#include "symtable.h"
#include "scanner.h"
#include "tokens.h"
#include "stack.h"
#include "expressions.h"

/* Label names */
enum label_names
{
	label_func,
	label_if,
	label_else,
	label_end_if
}typedef enum_label_names;


/* Starts translation of code to assembler code */
void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal START rules. Generates relevant instructions */
void neterm_start(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal FUNCTION_DECLARATION rules. */
void neterm_function_dec(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal FUNCTION_DEFINITION rules. Generates relevant instructions */
void neterm_function_def(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal SCOPE rules. */
void neterm_scope(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal ARGS rules. Checks arguments of function */
void neterm_args(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record);

/* Nonterminal function describes nonterminal ARGS_CREATE rules. Creates relevant arguments for function */
void neterm_args_create(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * empty_func_record);

/* Nonterminal function describes nonterminal BODY rules. Generates relevant instructions */
void neterm_body(token_buffer * token_buff, htab_t * symtable, String * primal_code);

void neterm_body_func(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record);

/* Nonterminal function describes nonterminal BODY_DECLARATION rules. Generates relevant instructions */
void body_declaration(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal BODY_INPUT rules. Generates relevant instructions */
void body_input(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal BODY_PRINT rules. Generates relevant instructions */
void body_print(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal BODY_IF_THEN rules. Generates relevant instructions */
void body_if_then(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal BODY_DO_WHILE rules. Generates relevant instructions */
void body_do_while(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Nonterminal function describes nonterminal BODY_ASSIGNMENT rules. Generates relevant instructions */
void body_assignment(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * found_record);

/* Nonterminal function describes nonterminal BODY_RETURN rules. Generates relevant instructions */
void body_return(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record);

/* Nonterminal function describes nonterminal EXPRESSION rules. Generates relevant instructions */
void neterm_expression(token_buffer * token_buff, htab_t * symtable, String * primal_code, token_type end_token);

/* Nonterminal function describes nonterminal FUNCTION_CALL rules. Generates relevant instructions */
void function_call(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * found_record);

/* Generates relevant instructions for parameters */
void generate_prepare_params(String * primal_code, struct func_par * actual_param, unsigned int param_order, struct htab_listitem * param_caller, struct htab_listitem * found_record);

/* Nonterminal function describes nonterminal TYPE rules. */
unsigned int neterm_type(token_buffer * token_buff, htab_t * symtable, String * primal_code);

/* Checks if next token is the one we expected. If it is different exits program with relevant error code */
void expected_token(token_buffer * token_buff, int tok_type);

/* Checks if next token is the one we expected. Returns true if it is truth*/
bool is_peek_token(token_buffer * token_buff, int tok_type);

/* Generates order of if label */
unsigned int generate_if_label_order();

/* Generates relevant instructions of if label */
void generate_if_label(String * primal_code, enum_label_names prefix, unsigned int order);

void generate_func_label(String * primal_code, char* func_name);

/* Adds built-in functions into code with its instructions */
void add_build_in_functions(htab_t * symtalbe, String * primal_code);

/* Creates new function record */
struct htab_listitem * create_func_record(htab_t * symtable, char * name);

/* Generates instruction code for variable */
void copy_scope_layer(struct htab_listitem * item, htab_t * other_symtable, String * primal_code);

/* Generates code of hard value */
void generate_implicit_value(String * primal_code, char * name, enum_type type);

bool unique_parameter(struct func_par * first_par, char * str);

/* Generates relevant instrunctions for expression*/
void parse_semantic_expression(String * primal_code, struct htab_listitem *found_record, int variable_type, int expr_return_type);

void parse_semantic_expression_modified(String * primal_code, char * name, int variable_type, int expr_return_type);

#endif