#include "expressions.h"

#define TABLE_SIZE 20
#define ERROR false

const bool  precedence_tab[TABLE_SIZE][TABLE_SIZE] = {
//			  ADD    SUB    MUL	   DIV    DIV2   LS	    GR	   LSE	  GRE 	 NEQ 	EQ 	   LPAR   RPAR   ID	    LIT    AND    OR     NOT
/*ADD*/ 	{ true,  true,  true,  true,  true,  false, false, false, false, false, false, false, true,  true,  true,  false, false, true   },
/*SUB*/		{ true,  true,  true,  true,  true,  false, false, false, false, false, false, false, true,  true,  true,  false, false, true   },
/*MUL*/		{ false, false, true,  true,  false, false, false, false, false, false, false, false, true,  true,  true,  false, false, true   },
/*DIV1*/	{ false, false, true,  true,  false, false, false, false, false, false, false, false, true,  true,  true,  false, false, true   },
/*DIV2*/	{ false, false, true,  true,  true,  false, false, false, false, false, false, false, true,  true,  true,  false, false, true   },
/*LESS*/	{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  false, false, true   },
/*GRT*/		{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  false, false, true   },
/*LESSEQ*/	{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  false, false, true   },
/*GRTEQ*/	{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  false, false, true   },
/*NOTEQ*/	{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  false, false, true   },
/*EQUAL*/	{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  false, false, true   },
/*LPAR*/	{ false, false, false, false, false, false, false, false, false, false, false, false, ERROR, ERROR, ERROR, false, false, false  },
/*RPAR*/	{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  ERROR, true,  true,  true,  true,  true,  true   },
/*ID*/		{ false, false, false, false, false, false, false, false, false, false, false, false, ERROR, ERROR, ERROR, false, false, false  },
/*LITERAL*/	{ false, false, false, false, false, false, false, false, false, false, false, false, ERROR, ERROR, ERROR, false, false, false  },
/*AND*/		{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true   },
/*OR*/		{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true   },
/*NOT*/		{ false, false, false, false, false, false, false, false, false, false, false, false, true,  ERROR, ERROR, false, false, false  }
};




int expression_EQ(token_buffer * token_buff, htab_t * symtable, String * primal_code, char *key, int type) {
	if (type == INTEGER || type == DOUBLE || type == BOOLEAN || type == STRING)
		expected_token(token_buff, EQUALS);

	//1.step: check, TODO
	semantic_expr_check(token_buff, symtable, primal_code, key, type);

	//2.step: create a double linked list(postfix expr)
	TStack s = infix2postfix(token_buff, symtable, primal_code);

	//3.step: calculate the value
	get_expr_value(primal_code, &s, type, key);

	return 1;
}


bool has_higher_priority(t_expressions op1, t_expressions op2) {
	return (precedence_tab[op1][op2]);
}

bool is_operand(int token_type, bool in_condition) {

	switch(token_type) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case DIV2:
			return true;
	}
	if (in_condition) {
		switch(token_type) {
			case AND:
			case OR:
			case NOT:
			case LESS_THAN:
			case GREATER_THAN:
			case LESS_OR_EQUALS:
			case GREATER_OR_EQUALS:
			case EQUALS:
			case NOT_EQUALS:
				return true;
		}
	}
	return false;
}

bool is_value(int token_type) {
	switch(token_type) {
		case IDENTIFIER:
		case DOUBLEE:
		case INT_WITH_EXP:
		case DOUBLE_WITH_EXP:
		case INT_VALUE:
		case STRING_VALUE:
		case TRUE:
		case FALSE:
			return true;
	}
	return false;
}



void untilLeftPar ( TStack *sTemp, TStack  *sOut) {
	token *actual_token;

	while (!SEmpty(sTemp)) {
		actual_token = peek_last_expr(sTemp);

		if (actual_token->type == LEFT_PARANTHESIS) {
			pop_last_expr(sTemp);
			return;
		}
		else {
			push_expr_token(sOut, actual_token);
			pop_last_expr(sTemp);
		}
	}
}

void doOperation ( TStack *sTemp, TStack  *sOut, token *t) {

	token *actual_token = peek_last_expr(sTemp);
	while(!SEmpty(sTemp)) {
		int operand1 = convert_operand_type(t->type);
		int operand2 = convert_operand_type(actual_token->type);
		if (actual_token->type == LEFT_PARANTHESIS ||
			(!has_higher_priority(operand1, operand2)) || SEmpty(sTemp)) {
				push_expr_token(sTemp, t);
				return ;
		}
		else {
			push_expr_token(sOut, actual_token);
			pop_last_expr(sTemp);
		}

		actual_token = peek_last_expr(sTemp);
	}

	if (SEmpty(sTemp)) {
		push_expr_token(sTemp, t);
	}
}

TStack infix2postfix (token_buffer * token_buff, htab_t * symtable, String * primal_code) {

	TStack sTemp;
	stack_init(&sTemp);

	TStack sOut;
	stack_init(&sOut);

	token *t = token_buffer_get_token(token_buff);

	while (42) {
		if (t->type == NEW_LINE || t->type == EOF || t->type == THEN)
			break;

		if ( is_operand(t->type, true)) {
			doOperation(&sTemp, &sOut, t);
		}

		else if(is_value(t->type)){
			push_expr_token(&sOut, t);
		}

		else if ( t->type == LEFT_PARANTHESIS) {
			push_expr_token(&sTemp, t);
		}

		else if ( t->type == RIGHT_PARANTHESIS) {
			untilLeftPar(&sTemp, &sOut);
		}
		else {
			fprintf(stderr, "Ooopps, we are fucked :/\n");
		}

		t = token_buffer_get_token(token_buff);
	}
	while(!SEmpty(&sTemp)) {
		t = peek_last_expr(&sTemp);
		push_expr_token(&sOut, t);
		pop_last_expr(&sTemp);
	}

	return (sOut);
}

void semantic_expr_check(token_buffer * token_buff, htab_t * symtable, String * primal_code, char *key, int type) {
	//TODO
}

void get_expr_value(String * primal_code, TStack *s, int type, char *key) {
	TSElem *actual_token = s->First;
	TSElem *next_token = s->First->next;
	String str;
	init_string(&str);

	if (type == STRING)
		e_move(primal_code, actual_token, key, &str);

	if (stack_counter(s) == 1 && type != STRING)
		e_push(primal_code, actual_token, key, &str);


	int state = E_SEARCH;
	// 1.Search
	// 2.Do operand
	// 3.Delete
	while(stack_counter(s) != 1) {
		switch (state) {
			//step 1: Search
			case E_SEARCH:
				if (is_value(actual_token->t_elem->type) && is_value(next_token->t_elem->type) && is_operand(next_token->next->t_elem->type,true)) {
					state = E_OPERAND;
				}
				else {
					actual_token = next_token;
					next_token = next_token->next;
				}
				break;

			//step 2: Do the job
			case E_OPERAND:
				if (type != STRING) {
					e_push(primal_code, actual_token, key, &str);

					e_push(primal_code, next_token, key, &str);
				}

				if (type == STRING) {
					if (next_token->next->t_elem->type == ADD) {
						append_str_to_str(primal_code, "CONCAT ");
						append_str_to_str(primal_code, "LF@");
						append_str_to_str(primal_code, key);

						append_str_to_str(primal_code, " LF@");
						append_str_to_str(primal_code, key);

						if (next_token->t_elem->type == IDENTIFIER) {
							append_str_to_str(primal_code, " LF@");
							append_str_to_str(primal_code, next_token->t_elem->attr.string_value);
						}
						else {
							append_str_to_str(primal_code, " string@");
							append_str_to_str(primal_code, next_token->t_elem->attr.string_value);
						}
						append_char_to_str(primal_code, '\n');
					}
				}

				if (type != STRING) {
					if (next_token->next->t_elem->type == ADD)
						append_str_to_str(primal_code, "ADDS\n");
					else if (next_token->next->t_elem->type == SUB)
						append_str_to_str(primal_code, "SUBS\n");
					else if (next_token->next->t_elem->type == MUL)
						append_str_to_str(primal_code, "MULS\n");
					else if (next_token->next->t_elem->type == DIV)
						append_str_to_str(primal_code, "DIVS\n");
					else if (next_token->next->t_elem->type == AND)
						append_str_to_str(primal_code, "ANDS\n");
					else if (next_token->next->t_elem->type == OR)
						append_str_to_str(primal_code, "ORS\n");
					else if (next_token->next->t_elem->type == NOT){
						append_str_to_str(primal_code, "NOTS\n");
					}
					else if (next_token->next->t_elem->type == DIV2){
						operand_module(primal_code, actual_token);
					}else if (next_token->next->t_elem->type == LESS_THAN){
						append_str_to_str(primal_code, "LTS\n");
					}
					else if (next_token->next->t_elem->type == GREATER_THAN){
						append_str_to_str(primal_code, "GTS\n");
					}
					else if (next_token->next->t_elem->type == LESS_OR_EQUALS) {
						append_str_to_str(primal_code, "GTS\n");
						append_str_to_str(primal_code, "NOTS\n");
					}
					else if (next_token->next->t_elem->type == GREATER_OR_EQUALS) {
						append_str_to_str(primal_code, "LTS\n");
						append_str_to_str(primal_code, "NOTS\n");
					}
					else if (next_token->next->t_elem->type == EQUALS){
						append_str_to_str(primal_code, "EQS\n");
					}
					else if (next_token->next->t_elem->type == NOT_EQUALS){
						append_str_to_str(primal_code, "EQS\n");
						append_str_to_str(primal_code, "NOTS\n");
					}
				}

				state = E_DELETE;
				break;

			//step 3: Delete unnecessary tokens
			case E_DELETE:
				actual_token->is_valid = false;
				actual_token = next_token->next;
				delete_current_expr(next_token);
				delete_current_expr(actual_token);

				state = E_SEARCH;
				actual_token = s->First;
				next_token = s->First->next;
				break;
		}
	}
	if (type != STRING && type != e_if && type != e_while) {
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, key);
		append_char_to_str(primal_code, '\n');
	}

	free_string(&str);
}




int convert_operand_type(int operand) {
	switch(operand) {
		case ADD:  return E_ADD;
		case SUB:  return E_SUB;
		case MUL:  return E_MUL;
		case DIV:  return E_DIV;
		case DIV2: return E_DIV2;
		case LESS_THAN: return E_LS;
		case GREATER_THAN: return E_GT;
		case LESS_OR_EQUALS: return E_LSE;
		case GREATER_OR_EQUALS: return E_GRE;
		case NOT_EQUALS: return E_NEQ;
		case EQUALS: return E_EQ;
		case LEFT_PARANTHESIS: return E_LPAR;
		case RIGHT_PARANTHESIS: return E_RPAR;
		case IDENTIFIER: return E_ID;
		case E_LIT: break;
		case AND: return E_AND;
		case OR: return E_OR;
		case NOT: return E_NOT;
	}
	return operand;
}


void e_move(String *primal_code, TSElem *t, char *key, String *str) {
		clear_string(str);
		append_str_to_str(primal_code, "MOVE LF@");
		append_str_to_str(primal_code, key);
		switch(t->t_elem->type) {
			case INT_VALUE:
				append_str_to_str(primal_code, " int@");
				sprintf(str->str, "%d",t->t_elem->attr.int_value);
				append_str_to_str(primal_code, str->str);
				break;

			case DOUBLEE:
			case DOUBLE_WITH_EXP:
			case INT_WITH_EXP:
				append_str_to_str(primal_code, " float@");
				sprintf(str->str, "%g",t->t_elem->attr.double_value);
				append_str_to_str(primal_code, str->str);
				break;

			case FALSE:
			case TRUE:
				append_str_to_str(primal_code, " bool@");
				append_str_to_str(primal_code, t->t_elem->attr.int_value != 0 ? "true" : "false");
				break;

			case STRING_VALUE:
				append_str_to_str(primal_code, " string@");
				append_str_to_str(primal_code, t->t_elem->attr.string_value);
				break;

			case IDENTIFIER:
				append_str_to_str(primal_code, "LF@");
				append_str_to_str(primal_code, t->t_elem->attr.string_value);
				break;
		}
		append_char_to_str(primal_code, '\n');
}

void e_push(String *primal_code, TSElem *t, char *key, String *str) {
	if (t->t_elem->type == STRING)
		return;
	clear_string(str);
	if (t->is_valid) {
		switch(t->t_elem->type) {
			case INT_VALUE:
				append_str_to_str(primal_code, "PUSHS int@");
				sprintf(str->str, "%d",t->t_elem->attr.int_value);
				append_str_to_str(primal_code, str->str);
				break;

			case IDENTIFIER:
				append_str_to_str(primal_code, "PUSHS LF@");
				append_str_to_str(primal_code, t->t_elem->attr.string_value);
				break;

			case DOUBLEE:
			case DOUBLE_WITH_EXP:
			case INT_WITH_EXP:
				append_str_to_str(primal_code, "PUSHS float@");
				sprintf(str->str, "%g",t->t_elem->attr.double_value);
				append_str_to_str(primal_code, str->str);
				break;

			case TRUE:
			case FALSE:
				append_str_to_str(primal_code, "PUSHS bool@");
				append_str_to_str(primal_code, (t->t_elem->type) == TRUE ? "TRUE" : "FALSE");
				break;
		}
		append_char_to_str(primal_code, '\n');
	}
}

void operand_module(String *primal_code, TSElem *t) {
	String str;
	init_string(&str);

	e_push(primal_code, t, NULL, &str);
	append_str_to_str(primal_code, "INT2FLOATS\n");
	e_push(primal_code, t->next, NULL, &str);
	append_str_to_str(primal_code, "INT2FLOATS\nDIVS\n");
	append_str_to_str(primal_code, "PUSHS float@0.5\nSUBS\nFLOAT2INTS\nMULS\nSUBS\n");

	free_string(&str);
}