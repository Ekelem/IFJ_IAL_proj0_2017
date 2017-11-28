#ifndef H_PARSER
#define H_PARSER 42

#include "error.h"
#include "symtable.h"
#include "scanner.h"
#include "tokens.h"
#include "stack.h"
#include "expressions.h"

enum label_names
{
	label_func,
	label_if,
	label_else,
	label_end_if
}typedef enum_label_names;

typedef enum {
	e_init,
	e_assign,
	e_if,
	e_while
} expr_states;

void printf_fun_par_types(htab_listitem *fun);
void add_build_in_functions(htab_t * symtable, String * primal_code);
void parse_semantic_expression(String * primal_code, struct htab_listitem *found_record, int variable_type, int expr_return_type);
char* get_string_from_type(int type);

void create_param(htab_t * symtable, char *name, int type);


void here();
/* Generates order of if label */
unsigned int generate_if_label_order();

void parse_semantic_expression_modified(String * primal_code, char * frame, char * name, int variable_type, int expr_return_type);


void generate_chr(String *primal_code);
void generate_length(String *primal_code);
void generate_substr(String *primal_code);
void generate_asc(String *primal_code);

/* Generates relevant instructions of if label */
void generate_if_label(String * primal_code, enum_label_names prefix, unsigned int order);

void generate_func_label(String * primal_code, char* func_name);

/* Adds built-in functions into code with its instructions */
void add_build_in_functions(htab_t * symtalbe, String * primal_code);

/* Generates instruction code for variable */
void copy_scope_layer(struct htab_listitem * item, htab_t * other_symtable, String * primal_code);

/* Generates code of hard value */
void generate_implicit_value(String * primal_code, char * name, enum_type type);

/* Generates relevant instrunctions for expression*/
void parse_semantic_expression(String * primal_code, struct htab_listitem *found_record, int variable_type, int expr_return_type);

void expected_token(token_buffer * token_buff, int tok_type);
void skip_expression(token_buffer * token_buff,  htab_t * symtable);

struct htab_listitem *create_fun_record(htab_t * symtable, char *name);
void create_param(htab_t * symtable, char *name, int type);



void check_variable_name(htab_listitem *record, char * var_name);
void check_funcall_params(htab_t * symtable, htab_listitem *actual_fun, htab_listitem *called_fun, char *var_name, String *primal_code);
void check_fun_declaration_params(token_buffer * token_buff, htab_t * symtable, htab_t * new_symtable, struct htab_listitem *record, String *primal_code);
void check_fun_definiton_params(token_buffer * token_buff, htab_listitem *record, int par_counter);

void compare_with_params(htab_t * symtable, htab_listitem *record, char * var_name);
void fundec_check_param_name(htab_listitem *new_fun_record, htab_t *global_symtable, char *par_name, bool first_par);
void check_new_lines_after_scopeblock(token_buffer * token_buff);
void check_params_names(struct htab_listitem * item, token *token);
void check_all_functions_definitions(htab_t *symtable);
void expected_token(token_buffer * token_buff, int tok_type);
void translate(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void funblock_rule_continue(token_buffer * token_buff, htab_t * symtable, int token_type, String *primal_code);
void neterm_start(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void neterm_funblock(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void neterm_scopeblock(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void neterm_fundecparams(token_buffer * token_buff, htab_t * symtable,  htab_listitem *new_fun_record, String *primal_code);
void neterm_fundecparamsnext(token_buffer * token_buff, htab_t * symtable, htab_listitem *new_fun_record, String *primal_code);
 int neterm_type(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void neterm_statementblock(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void neterm_decORasign(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void neterm_decasign(token_buffer * token_buff, htab_t * symtable, htab_listitem *found, String *primal_code);
void neterm_funcallORasign(token_buffer * token_buff, htab_t * symtable, String *primal_code);
void neterm_funcallORasign2(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_var, String *primal_code);
void neterm_funcallparams(token_buffer * token_buff, htab_t * symtable, htab_listitem *found, String *primal_code);
void neterm_funcallparam(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_fun, String *primal_code);
void neterm_funcallparamsnext(token_buffer * token_buff, htab_t * symtable, htab_listitem *found, String *primal_code);
void neterm_constvalue(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_fun, String *primal_code);
void neterm_elsestatement(token_buffer * token_buff, htab_t * symtable, String *primal_code, int order);
void neterm_exprnext(token_buffer * token_buff, htab_t * symtable, String *primal_code);

#endif