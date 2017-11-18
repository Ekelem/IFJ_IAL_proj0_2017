#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "parser.h"
#include "tokens.h"
#include "stack.h"

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

typedef enum {
	E_SEARCH = 0,
	E_OPERAND,
	E_DELETE
} expr_state;

typedef enum {
	sem_value = 0,
	sem_LP,
	sem_operand,
	sem_RP,
	sem_operand_not
} semantic_expr_states;


bool has_higher_priority(t_expressions op1, t_expressions op2);
int parse_expression(token_buffer * token_buff, htab_t * symtable, String * primal_code, char *key, int type, int end_token);

void untilLeftPar ( TStack *sTemp, TStack  *sOut);
void doOperation ( TStack *sTemp, TStack  *sOut, token *t);
TStack infix2postfix (token_buffer * token_buff, htab_t * symtable, String * primal_code);
void semantic_expr_check(token_buffer * token_buff, htab_t * symtable, String * primal_code, char *key, int type);
int get_expr_value(token_buffer * token_buff, htab_t * symtable, String * primal_code, TStack *s, int type, char *key);
int convert_operand_type(int operand);
bool e_push(htab_t *symtable, String *primal_code, TSElem *t, char *key, String *str, BStack *value_stack);
void operand_module(htab_t *symtable, String *primal_code, TSElem *t, BStack *value_stack);
bool is_token(TSElem *s, int type);
bool is_value(token *token_type);
bool is_operand(token *token_type, bool in_condition);
void semantic_expr_check_order(token_buffer * token_buff, htab_t * symtable, String * primal_code, int type, int end_token);
bool is_token_type(htab_t * symtable, TSElem *actual_token, int type);
void semantic_expr_check_second(token_buffer * token_buff, htab_t * symtable, String * primal_code, int type, TStack *Out);
bool is_valid_token_type(htab_t * symtable, TSElem *actual_token, int type);
void sem_check_not(TSElem *actual_token, htab_t *symtable);
int return_semantic_type(TStack *Out, htab_t *symtable);

#endif