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


bool has_higher_priority(t_expressions op1, t_expressions op2);
int expression_EQ(token_buffer * token_buff, htab_t * symtable, String * primal_code, char *key, int type);

void untilLeftPar ( TStack *sTemp, TStack  *sOut);
void doOperation ( TStack *sTemp, TStack  *sOut, token *t);
TStack infix2postfix (token_buffer * token_buff, htab_t * symtable, String * primal_code);
void semantic_expr_check(token_buffer * token_buff, htab_t * symtable, String * primal_code, char *key, int type);
void get_expr_value(String * primal_code, TStack *s, int type, char *key);
int convert_operand_type(int operand);
void e_move(String *primal_code, TSElem *t, char *key, String *str);
void e_push(String *primal_code, TSElem *t, char *key, String *str);
void operand_module(String *primal_code, TSElem *t);

#endif