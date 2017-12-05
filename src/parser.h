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


/*********************************Nonterminal functions************************************/

/* First nonterminal function. Calls other neterminals to check syntax*/
void neterm_start(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/* Nonterminal function funblock*/
void neterm_funblock(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/* Nonterminal function scopeblock*/
void neterm_scopeblock(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/* Nonterminal function fundecparams*/
void neterm_fundecparams(token_buffer * token_buff, htab_t * symtable,  htab_listitem *new_fun_record, String *primal_code);

/* Nonterminal function fundecparamsnext*/
void neterm_fundecparamsnext(token_buffer * token_buff, htab_t * symtable, htab_listitem *new_fun_record, String *primal_code);

/* Nonterminal function type*/
 int neterm_type(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/* Nonterminal function statementblock*/
void neterm_statementblock(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/* Nonterminal function decORasign*/
void neterm_decORasign(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/* Nonterminal function decasign*/
void neterm_decasign(token_buffer * token_buff, htab_t * symtable, htab_listitem *found, String *primal_code);

/* Nonterminal function funcallORasign*/
void neterm_funcallORasign(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/* Nonterminal function funcallORasign*/
void neterm_funcallORasign2(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_var, String *primal_code);

/* Nonterminal function funcallparams*/
void neterm_funcallparams(token_buffer * token_buff, htab_t * symtable, htab_listitem *found, String *primal_code);

/* Nonterminal function funcallparam*/
void neterm_funcallparam(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_fun, String *primal_code);

/* Nonterminal function funcallparamsnext*/
void neterm_funcallparamsnext(token_buffer * token_buff, htab_t * symtable, htab_listitem *found, String *primal_code);

/* Nonterminal function constvalue*/
void neterm_constvalue(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_fun, String *primal_code);

/* Nonterminal function elsestatement*/
void neterm_elsestatement(token_buffer * token_buff, htab_t * symtable, String *primal_code, int order);

/* Nonterminal function exprnext*/
void neterm_exprnext(token_buffer * token_buff, htab_t * symtable, String *primal_code);


/**********************************Checking functions ***********************************/

/* Checks record name and prints error message*/
void check_variable_name(htab_listitem *record, char * var_name);

/* Checks funcall paramaters */
void check_funcall_params(htab_t * symtable, htab_listitem *actual_fun, htab_listitem *called_fun, char *var_name, String *primal_code);

/* Checks and compares parameters from declaration with new function definition*/
void check_fun_declaration_params(token_buffer * token_buff, htab_t * symtable, htab_t * new_symtable, struct htab_listitem *record, String *primal_code);

/* Checks function definiciton parameters*/
void check_fun_definiton_params(token_buffer * token_buff, htab_listitem *record, int par_counter);

/* Compares found identifier with params */
void funcall_compare_with_params(htab_t * global_symtable, htab_listitem *record, char * var_name);

/* Compares found identifier with functions*/
void funcall_compare_with_functions(htab_t *global_symtable, char *var_name);

/* Compares variable name with parameters*/
void compare_with_params(htab_t * symtable, htab_listitem *record, char * var_name);

/* Checks param name with functions and its others params*/
void fundec_check_param_name(htab_listitem *new_fun_record, htab_t *global_symtable, char *par_name, bool first_par);

/*	Checks new lines put after scopeblock till EOF.	If somethhing unexpected appears, kills program with lex error*/
void check_new_lines_after_scopeblock(token_buffer * token_buff);

/* Checks params names*/
void check_params_names(struct htab_listitem * item, token *token);

/* Checks if all declared functions were defined */
void check_all_functions_definitions(htab_t *symtable);


/********************************Generating functions***********************************/

/* Generates chr function */
void generate_chr(String *primal_code);

/* Generates length function */
void generate_length(String *primal_code);

/* Generates substr function */
void generate_substr(String *primal_code);

/* Generates asc function */
void generate_asc(String *primal_code);

/* Generates order of if label */
unsigned int generate_if_label_order();

/* Generates relevant instructions of if label */
void generate_if_label(String * primal_code, enum_label_names prefix, unsigned int order);

void generate_func_label(String * primal_code, char* func_name);

/* Adds built-in functions into code with its instructions */
void add_build_in_functions(htab_t * symtalbe, String * primal_code);

/* Generates code of hard value */
void generate_implicit_value(String * primal_code, char * name, enum_type type);

/* Generates relevant instrunctions for expression*/
void parse_semantic_expression(String * primal_code, struct htab_listitem *found_record, int variable_type, int expr_return_type);

/* Parses semantic expression and generates relevant instructions*/
void parse_semantic_expression_modified(String * primal_code, char * frame, char * name, int variable_type, int expr_return_type);


/****************************** Other functions ***********************************/

/* Creates function record */
struct htab_listitem *create_fun_record(htab_t * symtable, char *name);

/* Creates new param as record */
void create_param(htab_t * symtable, char *name, int type);

/* Makes implicit type conversion*/
int make_type_conversion(int type_1, int type_2);

/* Converts type as integer into relevant string*/
char* get_string_from_type(int type);

/* Adds built in functions into instructon list*/
void add_build_in_functions(htab_t * symtable, String * primal_code);

/*	Starts the translation of IFJ2017 to assembler generating instructions */
void translate(token_buffer * token_buff, htab_t * symtable, String *primal_code);

/*	Chooses next rule from neterm_funblock	*/
void funblock_rule_continue(token_buffer * token_buff, htab_t * symtable, int token_type, String *primal_code);

/*	Eats next token and compares with the one we expect */
void expected_token(token_buffer * token_buff, int tok_type);

/*	Skips expressions just for a case expressions havent been done yet */
void skip_expression(token_buffer * token_buff,  htab_t * symtable);

/* Prints function paramteres types */
void printf_fun_par_types(htab_listitem *fun);

/* Prints that im here :D */
void here();

#endif