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

#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"
#include "tokens.h"
#include "stack.h"

/* Expression operators */
typedef enum {
	E_ADD = 0,
	E_SUB,
	E_MUL,
	E_DIV,
	E_DIV2,
	E_LS,
	E_GT,
	E_LSE,
	E_GRE,
	E_NEQ,
	E_EQ,
	E_LPAR,
	E_RPAR,
	E_ID,
	E_LIT,
	E_AND,
	E_OR,
	E_NOT,
} t_expressions;

/* Expression states */
typedef enum {
	E_SEARCH = 0,
	E_OPERAND,
	E_DELETE
} expr_state;

/* Semantic expression states */
typedef enum {
	sem_value = 0,
	sem_LP,
	sem_operand,
	sem_RP,
	sem_operand_not
} semantic_expr_states;


/* Returns priority of operators using precedence table*/
bool has_higher_priority(t_expressions op1, t_expressions op2);

/* Parses the given expression */
int parse_expression(token_buffer * token_buff, htab_t * symtable, String * primal_code, int end_token);

/* Pops everything from stack until token is left parenthesis */
void untilLeftPar ( TStack *sTemp, TStack  *sOut);

/* Does relevant operation depending on type of actual token  */
void doOperation ( TStack *sTemp, TStack  *sOut, token *t);

/* Converts expression from infix to postfix using stack */
TStack infix2postfix (token_buffer * token_buff, htab_t * symtable, String * primal_code, int end_token);

/* Checks semantic of given expression. Exits the program with different error codes depending on error.*/ 
void semantic_expr_check(token_buffer * token_buff, htab_t * symtable, String * primal_code, int type);

/* Counts value of expression. Checks semantics and generates relevant instructions*/
int get_expr_value(token_buffer * token_buff, htab_t * symtable, String * primal_code, TStack *s);

/* Converts operator type */
int convert_operand_type(int operand);

/* Generates relevant PUSH instructions depending on operand type*/
bool e_push(htab_t *symtable, String *primal_code, TSElem *t, String *str, BStack *value_stack);

/* Checks type identity */
bool is_token(TSElem *s, int type);

/* Chceks if given token is value or not. */
bool is_value(token *token_type);

/* Checks if given token is operator */
bool is_operand(token *token_type, bool in_condition);

/* Checks semantic of given expression. Exits the program with different error codes depending on error.*/
void semantic_expr_check_order(token_buffer * token_buff, htab_t * symtable, String * primal_code, int end_token);

/* Checks token type of found record in symtable. */
bool is_token_type(htab_t * symtable, TSElem *actual_token, int type);

void semantic_expr_check_second(token_buffer * token_buff, htab_t * symtable, String * primal_code, int type, TStack *Out);

/* Checks if given type of token is valid. Eventually exits program with relevant error code. */
bool is_valid_token_type(htab_t * symtable, TSElem *actual_token, int type);

/* Checks semantic in using operator NOT. */
void sem_check_not(TSElem *actual_token, htab_t *symtable);

/* Returns relevant semantic type. */
int return_semantic_type(TStack *Out, htab_t *symtable);

#endif