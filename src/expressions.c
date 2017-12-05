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

#include "expressions.h"
#include "parser.h"

#define TABLE_SIZE 20
#define ERROR false

/* Structure of precedence table using true and false, that represent priority of operation */
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
/*AND*/		{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  false, true   },
/*OR*/		{ true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true,  true   },
/*NOT*/		{ false, false, false, false, false, false, false, false, false, false, false, false, true,  ERROR, ERROR, false, false, false  }
};



/* Parses the given expression */
int parse_expression(token_buffer * token_buff, htab_t * symtable, String * primal_code, int end_token) {
	//1.step: check
	int actual_end = end_token;
	semantic_expr_check_order(token_buff, symtable, primal_code, end_token);

	if (end_token == RIGHT_PARANTHESIS)
		actual_end=NEW_LINE;

	//2.step: create a double linked list(postfix expr)
	TStack s = infix2postfix(token_buff, symtable, primal_code, actual_end);

	//3.step: calculate the value
	int return_type = get_expr_value(token_buff, symtable, primal_code, &s);

	token_buff->actual -= 1;
	return return_type;
}

/* Checks semantic of given expression. Exits the program with different error codes depending on error.*/
void semantic_expr_check_order(token_buffer * token_buff, htab_t * symtable, String * primal_code, int end_token) {
	int start_token = token_buff->actual;
	token *actual_token = token_buffer_get_token(token_buff);
	token *next_token = token_buffer_peek_token(token_buff);
	int state = actual_token->type;
	int par_count = 0;

	if (end_token == RIGHT_PARANTHESIS)
	{
		state = sem_LP;
		par_count = 1;
	}

	if (is_value(actual_token)) {
		state = sem_value;
	}
	else if (actual_token->type == LEFT_PARANTHESIS){
		par_count++;
		state = sem_LP;
	}
	else if (actual_token->type == NOT)
		state = sem_operand_not;
	else if (actual_token->type == IDENTIFIER) {
		state = sem_value;
	}
	else if (actual_token->type == LEXICAL_ERROR)
		error_msg(ERR_CODE_LEXICAL, "Lexical error detected\n");
	else {
		error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");
	}
	while (actual_token->type != end_token) {
		next_token = token_buffer_peek_token(token_buff);
		switch(state) {
			case sem_value:
				if (next_token->type == LEFT_PARANTHESIS && (actual_token->type != TRUE && actual_token->type != FALSE)){
					par_count++;
					state = sem_LP;
				}
				else if (is_operand(next_token, TRUE))
					state = sem_operand;
				else if (next_token->type == RIGHT_PARANTHESIS){
					par_count--;
					state = sem_RP;
				}
				else if (next_token->type == LEXICAL_ERROR)
					error_msg(ERR_CODE_LEXICAL, "Lexical error detected\n");
				else if (next_token->type != end_token)
				{
					int counter = 0;
					token *actual_tok= token_buffer_peek_token(token_buff);
					actual_tok->type = end_token;

					while(actual_tok->type != WHILE && actual_tok->type != IF && actual_tok->type != PRINT && actual_tok->type != EQUALS)
					{
						if(actual_tok->type == IDENTIFIER)
							break;
						actual_tok = token_buffer_get_prev(token_buff, counter++);

					}
					if(actual_tok->type == IDENTIFIER)
					{
						htab_listitem *found = htab_find(symtable, actual_tok->attr.string_value);

						if(!found)
							error_msg(ERR_CODE_SEM, "Undeclared variable '%s' in expression\n", actual_tok->attr.string_value);
						else
							error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");
					break;
					}
						else
							error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");

				}
				break;
			case sem_LP:
				if (next_token->type == LEFT_PARANTHESIS){
					par_count++;
					state = sem_LP;
				}
				else if (next_token->type == NOT)
					state = sem_operand_not;
				else if (is_value(next_token))
					state = sem_value;
				else if (next_token->type == LEXICAL_ERROR)
					error_msg(ERR_CODE_LEXICAL, "Lexical error detected\n");
				else if (next_token->type != end_token)
					error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");
				break;
			case sem_operand:
				if (next_token->type == LEFT_PARANTHESIS){
					par_count++;
					state = sem_LP;
				}
				else if (next_token->type == NOT)
					state = sem_operand_not;
				else if (is_value(next_token)) {
					state = sem_value;
				}
				else if (next_token->type == LEXICAL_ERROR)
					error_msg(ERR_CODE_LEXICAL, "Lexical error detected\n");
				else if (next_token->type != end_token)
				{
					int counter = 0;
					token *actual_tok= token_buffer_peek_token(token_buff);
					actual_tok->type = end_token;

					while(actual_tok->type != WHILE && actual_tok->type != IF && actual_tok->type != PRINT && actual_tok->type != EQUALS)
					{
						if(actual_tok->type == IDENTIFIER)
							break;
						actual_tok = token_buffer_get_prev(token_buff, counter++);

					}

					if(actual_tok->type == IDENTIFIER)
					{
						htab_listitem *found = htab_find(symtable, actual_tok->attr.string_value);

						if(!found)
							error_msg(ERR_CODE_SEM, "Undeclared variable '%s' in expression\n", actual_tok->attr.string_value);
						break;
					}
					else
						error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");
				}
				break;
			case sem_RP:
				if (next_token->type == RIGHT_PARANTHESIS){
					par_count--;
					state = sem_RP;
				}
				else if (is_operand(next_token, true))
					state = sem_operand;
				else if (next_token->type == LEXICAL_ERROR)
					error_msg(ERR_CODE_LEXICAL, "Lexical error detected\n");
				else if (next_token->type != end_token)
					error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");
				break;
			case sem_operand_not:
				if (is_value(next_token))
					state = sem_value;
				else if (next_token->type == LEFT_PARANTHESIS){
					par_count++;
					state = sem_LP;
				}
				else if (next_token->type == NOT)
					state = sem_operand_not;
				else if (next_token->type == LEXICAL_ERROR)
					error_msg(ERR_CODE_LEXICAL, "Lexical error detected\n");
				else if (next_token->type != end_token)
					error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");
				break;
			default:
				error_msg(ERR_CODE_TYPE, "Semantic error detected\n");
				break;
		}

		actual_token = token_buffer_get_token(token_buff);
	}

	if ((state != sem_value && state != sem_RP) || (par_count != 0) || (next_token->type != end_token))
	{
		int counter = 0;
		token *actual_tok= next_token;
		//actual_tok->type = end_token;

		while(actual_tok->type != WHILE && actual_tok->type != IF && actual_tok->type != PRINT && actual_tok->type != EQUALS)
		{
			actual_tok = token_buffer_get_prev(token_buff, ++counter);
		}

		actual_tok = token_buffer_next_token(token_buff);
		if(actual_tok->type == IDENTIFIER)
		{
				htab_listitem *found = htab_find(symtable, actual_tok->attr.string_value);
				if(!found)
					error_msg(ERR_CODE_SEM, "Undeclared variable in expression\n");
		}

		error_msg(ERR_CODE_SYNTAX, "Syntactic error detected\n");
	}


	token_buff->actual = start_token;
}

/* Pops everything from stack until token is left parenthesis */
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

/* Does relevant operation depending on type of actual token  */
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

/* Converts expression from infix to postfix using stack */
TStack infix2postfix (token_buffer * token_buff, htab_t * symtable, String * primal_code, int end_token) {

	TStack sTemp;
	stack_init(&sTemp);

	TStack sOut;
	stack_init(&sOut);

	token *t = token_buffer_get_token(token_buff);

	while (42) {
		if (t->type == end_token || t->type == EOF)
			break;

		if ( is_operand(t, true)) {
			doOperation(&sTemp, &sOut, t);
		}

		else if(is_value(t)){
			push_expr_token(&sOut, t);
		}

		else if ( t->type == LEFT_PARANTHESIS) {
			push_expr_token(&sTemp, t);
		}

		else if ( t->type == RIGHT_PARANTHESIS) {
			untilLeftPar(&sTemp, &sOut);
		}
		//else {
		//	fprintf(stderr, "Ooopps, we are fucked :/\n");
		//}

		t = token_buffer_get_token(token_buff);
	}
	while(!SEmpty(&sTemp)) {
		t = peek_last_expr(&sTemp);
		push_expr_token(&sOut, t);
		pop_last_expr(&sTemp);
	}

	return (sOut);
}

/* Counts value of expression. Checks semantics and generates relevant instructions*/
int get_expr_value(token_buffer * token_buff, htab_t * symtable, String * primal_code, TStack *s) {
	TSElem *actual_token = s->First;
	TSElem *next_token = s->First->next;
	String str;
	init_string(&str);
	bool conv_first = false, conv_second = false;
	BStack value_stack;
	BInit(&value_stack);
	int return_type = return_semantic_type(s, symtable);

	if (stack_counter(s) == 1){
		e_push(symtable, primal_code,actual_token, &str, &value_stack);
	}


	int state = E_SEARCH;
	// 1.Search
	// 2.Do operand
	// 3.Delete
	// print_stack(s);
	while(stack_counter(s) != 1) {

		switch (state) {
			//step 1: Search
			case E_SEARCH:
				if (is_value(actual_token->t_elem) && is_token(next_token, NOT)) {
					sem_check_not(actual_token, symtable);
					e_push(symtable, primal_code, actual_token, &str, &value_stack);
					append_str_to_str(primal_code, "NOTS\n");
					actual_token->is_valid = false;
					delete_current_expr(next_token);
					actual_token = s->First;
					next_token = s->First->next;
					state = E_SEARCH;
					BPush(&value_stack, true);
				}
				else if ( is_value(next_token->t_elem) && is_token(next_token->next, NOT)) {
					sem_check_not(next_token, symtable);
					e_push(symtable, primal_code, next_token, &str, &value_stack);
					append_str_to_str(primal_code, "NOTS\n");
					next_token->is_valid = false;
					delete_current_expr(next_token->next);
					actual_token = s->First;
					next_token = s->First->next;
					state = E_SEARCH;
					BPush(&value_stack, true);
				}
				else if (is_value(actual_token->t_elem) && is_value(next_token->t_elem) && is_operand(next_token->next->t_elem,true)) {
					state = E_OPERAND;
				}
				else {
					actual_token = next_token;
					next_token = next_token->next;
				}
				break;

			//step 2: Do the job
			case E_OPERAND:
				//Semantic check starts
				switch(next_token->next->t_elem->type) {
					case ADD:
						if (is_valid_token_type(symtable, actual_token, STRING_TYPE) && (is_valid_token_type(symtable, next_token, STRING_TYPE))) {

						}
						else if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
								if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
									conv_first = true;
									actual_token->conv_double = true;
								}
								if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
									conv_second = true;
									next_token->conv_double = true;
								}
							}
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand ADD can be combined only with (INTEGER AND DOUBLE) OR STRING values\n");
						}
						break;

					case SUB:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand SUB can be combined only with INTEGER OR DOUBLE values\n");
						}
						if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
								if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
									conv_first = true;
									actual_token->conv_double = true;
								}
								if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
									conv_second = true;
									next_token->conv_double = true;
								}
						}
						break;

					case MUL:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand MUL can be combined only with INTEGER OR DOUBLE values\n");
						}
						if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
								if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
									conv_first = true;
									actual_token->conv_double = true;
								}
								if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
									conv_second = true;
									next_token->conv_double = true;
								}
						}
						break;

					case DIV:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand DIV can be combined only with INTEGER OR DOUBLE values\n");
						}
						if (!(is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && !actual_token->conv_double) {
							conv_first = true;
							actual_token->conv_double = true;
						}
						if(!(is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) && !next_token->conv_double){
							next_token->conv_double = true;
							conv_second = true;
						}
						if (next_token->t_elem->type == DOUBLEE || next_token->t_elem->type == DOUBLE_WITH_EXP || next_token->t_elem->type == INT_WITH_EXP) {
							if (next_token->t_elem->attr.double_value == 0.0) {
								error_msg(ERR_CODE_TYPE, "Can not divide by Zero\n");
							}
						}
						else if (next_token->t_elem->type == INT_VALUE) {
							if (next_token->t_elem->attr.int_value == 0) {
								append_str_to_str(primal_code, "\n\n");
							}
						}
						else if (next_token->t_elem->type == IDENTIFIER) {
							struct htab_listitem *found_record = htab_find(symtable, next_token->t_elem->attr.string_value);

							if(is_function(found_record))
								error_msg(ERR_CODE_SEM, "Function '%s' used in expression\n", found_record->key);


							if (found_record->data.type == INTEGER_TYPE) {
								if (next_token->t_elem->attr.int_value == 0) {
									error_msg(ERR_CODE_TYPE, "Can not divide by Zero\n");
								}
							}
							else if (found_record->data.type == DOUBLE_TYPE) {
								if (next_token->t_elem->attr.double_value == 0.0) {
									error_msg(ERR_CODE_TYPE, "Can not divide by Zero\n");
								}
							}
						}
						break;

					case DIV2:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand DIV can be combined only with INTEGER or DOUBLE values\n");
						}
						if (!(is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && !actual_token->conv_double) {
							conv_first = true;
							actual_token->conv_double = true;
						}
						if(!(is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) && !next_token->conv_double){
							next_token->conv_double = true;
							conv_second = true;
						}
						if (next_token->t_elem->type == DOUBLEE || next_token->t_elem->type == DOUBLE_WITH_EXP || next_token->t_elem->type == INT_WITH_EXP) {
							if (next_token->t_elem->attr.double_value == 0.0) {
								error_msg(ERR_CODE_TYPE, "Can not divide by Zero\n");
							}
						}
						else if (next_token->t_elem->type == INT_VALUE) {
							if (next_token->t_elem->attr.int_value == 0) {
								error_msg(ERR_CODE_TYPE, "Can not divide by Zero\n");
							}
						}
						else if (next_token->t_elem->type == IDENTIFIER) {
							struct htab_listitem *found_record = htab_find(symtable, next_token->t_elem->attr.string_value);

							if(is_function(found_record))
								error_msg(ERR_CODE_SEM, "Function '%s' used in expression\n", found_record->key);

							if (found_record->data.type == INTEGER_TYPE) {
								if (next_token->t_elem->attr.int_value == 0) {
									error_msg(ERR_CODE_TYPE, "Can not divide by Zero\n");
								}
							}
							else if (found_record->data.type == DOUBLE_TYPE) {
								if (next_token->t_elem->attr.double_value == 0.0) {
									error_msg(ERR_CODE_TYPE, "Can not divide by Zero\n");
								}
							}
						}
						break;

					case AND:
						break;
					case OR:
						break;

					case LESS_THAN:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, STRING_TYPE) && is_valid_token_type(symtable, next_token, STRING_TYPE)) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, BOOLEAN_TYPE) && (is_valid_token_type(symtable, next_token, BOOLEAN_TYPE))) {
							//Type OK
						}
						else if (BTop_equals(&value_stack)){
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand < can be combined only with INTEGER OR DOUBLE values\n");
						}
						if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
							if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
								conv_first = true;
								actual_token->conv_double = true;
							}
							if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
								conv_second = true;
								next_token->conv_double = true;
							}
						}
						break;

					case GREATER_THAN:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, STRING_TYPE) && is_valid_token_type(symtable, next_token, STRING_TYPE)) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, BOOLEAN_TYPE) && (is_valid_token_type(symtable, next_token, BOOLEAN_TYPE))) {
							//Type OK
						}
						else if (BTop_equals(&value_stack)){
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand > can be combined only with INTEGER OR DOUBLE values\n");
						}
						if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
							if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
								conv_first = true;
								actual_token->conv_double = true;
							}
							if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
								conv_second = true;
								next_token->conv_double = true;
							}
						}
						break;

					case LESS_OR_EQUALS:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, STRING_TYPE) && is_valid_token_type(symtable, next_token, STRING_TYPE)) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, BOOLEAN_TYPE) && (is_valid_token_type(symtable, next_token, BOOLEAN_TYPE))) {
							//Type OK
						}
						else if (BTop_equals(&value_stack)){
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand <= can be combined only with INTEGER OR DOUBLE values\n");
						}
						if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
							if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
								conv_first = true;
								actual_token->conv_double = true;
							}
							if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
								conv_second = true;
								next_token->conv_double = true;
							}
						}
						break;

					case GREATER_OR_EQUALS:
						if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, STRING_TYPE) && is_valid_token_type(symtable, next_token, STRING_TYPE)) {
							//Type OK
						}
						else if (is_valid_token_type(symtable, actual_token, BOOLEAN_TYPE) && (is_valid_token_type(symtable, next_token, BOOLEAN_TYPE))) {
							//Type OK
						}
						else if (BTop_equals(&value_stack)){
							//Type OK
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand >= can be combined only with INTEGER OR DOUBLE values\n");
						}
						if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
							if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
								conv_first = true;
								actual_token->conv_double = true;
							}
							if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
								conv_second = true;
								next_token->conv_double = true;
							}
						}
						break;

					case EQUALS:
						if (is_valid_token_type(symtable, actual_token, STRING_TYPE) && (is_valid_token_type(symtable, next_token, STRING_TYPE))) {

						}
						else if (is_valid_token_type(symtable, actual_token, BOOLEAN_TYPE) && (is_valid_token_type(symtable, next_token, BOOLEAN_TYPE))) {
							//Type OK
						}
						else if (BTop_equals(&value_stack)){
							//Type OK
						}
						else if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
								if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
									conv_first = true;
									actual_token->conv_double = true;
								}
								if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
									conv_second = true;
									next_token->conv_double = true;
								}
							}
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand = can be combined only with (INTEGER AND DOUBLE) OR STRING values\n");
						}
						break;

					case NOT_EQUALS:
						if (is_valid_token_type(symtable, actual_token, STRING_TYPE) && (is_valid_token_type(symtable, next_token, STRING_TYPE))) {

						}
						else if (is_valid_token_type(symtable, actual_token, BOOLEAN_TYPE) && (is_valid_token_type(symtable, next_token, BOOLEAN_TYPE))) {
							//Type OK
						}
						else if (BTop_equals(&value_stack)){
							//Type OK
						}
						else if (BTop_equals(&value_stack)){
							//Type OK
						}
						else if ((is_valid_token_type(symtable, actual_token, INTEGER_TYPE) || is_valid_token_type(symtable, actual_token, DOUBLE_TYPE)) && (is_valid_token_type(symtable, next_token, INTEGER_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE))) {
							if ((is_valid_token_type(symtable, actual_token, DOUBLE_TYPE) || is_valid_token_type(symtable, next_token, DOUBLE_TYPE)) || actual_token->conv_double || next_token->conv_double ) {
								if (is_valid_token_type(symtable, actual_token, INTEGER_TYPE) && !actual_token->conv_double) {
									conv_first = true;
									actual_token->conv_double = true;
								}
								if (is_valid_token_type(symtable, next_token, INTEGER_TYPE) && !next_token->conv_double) {
									conv_second = true;
									next_token->conv_double = true;
								}
							}
						}
						else {
							error_msg(ERR_CODE_TYPE, "Operand <> can be combined only with (INTEGER AND DOUBLE) OR STRING values\n");
						}
						break;

					default:
						error_msg(ERR_CODE_TYPE, "Something went wrong\n");
						break;
				}
				// SEM_CHECK ENDS

				if (!(actual_token->t_elem->type == STRING_VALUE || is_token_type(symtable, actual_token, STRING_TYPE))) {
					if (actual_token->is_valid == true && next_token->is_valid == false && next_token->next->t_elem != NULL) {
						switch(next_token->next->t_elem->type) {
							case LESS_THAN:
								next_token->next->t_elem->type = GREATER_THAN;
								break;
							case GREATER_THAN:
								next_token->next->t_elem->type = LESS_THAN;
								break;
							case LESS_OR_EQUALS:
								next_token->next->t_elem->type = GREATER_OR_EQUALS;
								break;
							case GREATER_OR_EQUALS:
								next_token->next->t_elem->type = LESS_OR_EQUALS;
								break;
						}
					}
				}

				bool pushed = false, pushed2 = true;
				pushed = e_push(symtable, primal_code, actual_token, &str, &value_stack);
				if (!(is_token_type(symtable, actual_token, STRING_TYPE))){
					if (conv_first) {
						if (!pushed && !next_token->is_valid){
							append_str_to_str(primal_code, "POPS GF@%SWAP\n");
							append_str_to_str(primal_code, "INT2FLOATS\n");
							append_str_to_str(primal_code, "PUSHS GF@%SWAP\n");
						}
						else {
							append_str_to_str(primal_code, "INT2FLOATS\n");
						}
						actual_token->conv_double = true;
					}
				}
				pushed2 = e_push(symtable, primal_code, next_token, &str, &value_stack);
				if (!(is_token_type(symtable, next_token, STRING_TYPE))) {
					if (next_token->t_elem->type != NOT){
						if (conv_second) {
							if (pushed && !pushed2){
								append_str_to_str(primal_code, "POPS GF@%SWAP\n");
								append_str_to_str(primal_code, "INT2FLOATS\nPUSHS GF@%SWAP\n");
							}
							else
								append_str_to_str(primal_code, "INT2FLOATS\n");
							next_token->conv_double = true;
						}
					}
				}

				if (actual_token->t_elem->type == STRING_VALUE || is_token_type(symtable, actual_token, STRING_TYPE)) {
					switch(next_token->next->t_elem->type){
						case AND:
							if (!BTop_equals(&value_stack)){
								error_msg(ERR_CODE_TYPE, "Operand AND can work only with BOOLEAN values\n");
							}
							append_str_to_str(primal_code, "ANDS\n");
							break;

						case OR:
							if (!BTop_equals(&value_stack)){
								error_msg(ERR_CODE_TYPE, "Operand AND can work only with BOOLEAN values\n");
							}
							append_str_to_str(primal_code, "ORS\n");
							break;

						case GREATER_THAN:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "GTS\n");
							break;

						case GREATER_OR_EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "LTS\n");
							append_str_to_str(primal_code, "NOTS\n");
							break;

						case LESS_THAN:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "LTS\n");
							break;

						case LESS_OR_EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "GTS\n");
							append_str_to_str(primal_code, "NOTS\n");
							break;

						case EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "EQS\n");
							break;

						case NOT_EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "EQS\nNOTS\n");
							break;

						case ADD:
							BPop(&value_stack);
							if (pushed && !pushed2){
								append_str_to_str(primal_code, "POPS GF@%SWAP\n");
								append_str_to_str(primal_code, "POPS GF@%SWAP2\n");
								append_str_to_str(primal_code, "CONCAT GF@%SWAP GF@%SWAP GF@%SWAP2\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP\n");
							}
							else {
								append_str_to_str(primal_code, "POPS GF@%SWAP2\n");
								append_str_to_str(primal_code, "POPS GF@%SWAP\n");
								append_str_to_str(primal_code, "CONCAT GF@%SWAP GF@%SWAP GF@%SWAP2\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP\n");
							}
							break;
					}
				}
				else {
					switch(next_token->next->t_elem->type){
						case ADD:
							BPop(&value_stack);
							append_str_to_str(primal_code, "ADDS\n");
							break;
						case SUB:
							if (pushed && !pushed2){
								append_str_to_str(primal_code, "POPS GF@%SWAP\n");
								append_str_to_str(primal_code, "POPS GF@%SWAP2\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP2\n");
							}
							BPop(&value_stack);
							append_str_to_str(primal_code, "SUBS\n");
							break;
						case MUL:
							BPop(&value_stack);
							append_str_to_str(primal_code, "MULS\n");
							break;
						case DIV:
							BPop(&value_stack);
							if (pushed && !pushed2){
								append_str_to_str(primal_code, "POPS GF@%SWAP\n");
								append_str_to_str(primal_code, "POPS GF@%SWAP2\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP2\n");
							}
							append_str_to_str(primal_code, "DIVS\n");
							break;
						case AND:
							if (!BTop_equals(&value_stack)){
								error_msg(ERR_CODE_TYPE, "Operand AND can work only with BOOLEAN values\n");
							}
							append_str_to_str(primal_code, "ANDS\n");
							break;
						case OR:
							if (!BTop_equals(&value_stack)){
								error_msg(ERR_CODE_TYPE, "Operand OR can work only with BOOLEAN values\n");
							}
							append_str_to_str(primal_code, "ORS\n");
							break;
						case DIV2:
							BPop(&value_stack);
							if (pushed && !pushed2){
								append_str_to_str(primal_code, "POPS GF@%SWAP\n");
								append_str_to_str(primal_code, "POPS GF@%SWAP2\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP\n");
								append_str_to_str(primal_code, "PUSHS GF@%SWAP2\n");
							}

							append_str_to_str(primal_code, "POPS GF@%SWAP\n");
							append_str_to_str(primal_code, "FLOAT2R2EINTS\nINT2FLOATS\n");
							append_str_to_str(primal_code, "PUSHS GF@%SWAP\n");

							append_str_to_str(primal_code, "FLOAT2R2EINTS\nINT2FLOATS\n");

							append_str_to_str(primal_code, "DIVS\nFLOAT2INTS\n");
							actual_token->conv_double=false;
							next_token->conv_double=false;
							break;
						case LESS_THAN:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "LTS\n");
							break;
						case GREATER_THAN:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "GTS\n");
							break;
						case LESS_OR_EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "GTS\n");
							append_str_to_str(primal_code, "NOTS\n");
							break;
						case GREATER_OR_EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "LTS\n");
							append_str_to_str(primal_code, "NOTS\n");
							break;
						case EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "EQS\n");
							break;
						case NOT_EQUALS:
							if (!is_top_ok(&value_stack)){
								error_msg(ERR_CODE_SYNTAX, "Invalid argument assignment\n");
							}
							BPop(&value_stack); BPop(&value_stack); BPush(&value_stack, true);
							append_str_to_str(primal_code, "EQS\n");
							append_str_to_str(primal_code, "NOTS\n");
							break;
					}
				}
				conv_first = false;
				conv_second = false;
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

	dealloc_BStack(&value_stack);
	//free_string(&str);
	return return_type;
}



/********************* Helpful functions **********************/

/* Converts operator type */
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

/* Generates relevant PUSH instructions depending on operand type*/
bool e_push(htab_t *symtable, String *primal_code, TSElem *t, String *str, BStack *value_stack) {
	bool was_pushed = false;
	clear_string(str);
	struct htab_listitem *found_record;
	if (t->is_valid) {
		switch(t->t_elem->type) {
			case INT_VALUE:
				was_pushed = true;
				append_str_to_str(primal_code, "PUSHS int@");
				sprintf(str->str, "%d",t->t_elem->attr.int_value);
				append_str_to_str(primal_code, str->str);
				BPush(value_stack, false);
				break;

			case IDENTIFIER:
				found_record = htab_find(symtable, t->t_elem->attr.string_value);

				if(is_function(found_record))
					error_msg(ERR_CODE_SEM, "Function '%s' used in expression\n", found_record->key);

				if (found_record != NULL) {
					if (found_record->data.type == BOOLEAN_TYPE)
						BPush(value_stack, true);
					else
						BPush(value_stack, false);
				}
				was_pushed = true;
				append_str_to_str(primal_code, "PUSHS LF@");
				append_str_to_str(primal_code, t->t_elem->attr.string_value);
				break;

			case DOUBLEE:
			case DOUBLE_WITH_EXP:
			case INT_WITH_EXP:
				BPush(value_stack, false);
				was_pushed = true;
				append_str_to_str(primal_code, "PUSHS float@");
				sprintf(str->str, "%g",t->t_elem->attr.double_value);
				append_str_to_str(primal_code, str->str);
				break;

			case STRING_VALUE:
				BPush(value_stack, false);
				was_pushed = true;
				append_str_to_str(primal_code, "PUSHS string@");
				append_str_to_str(primal_code, t->t_elem->attr.string_value);
				break;

			case TRUE:
			case FALSE:
				BPush(value_stack, true);
				was_pushed = true;
				append_str_to_str(primal_code, "PUSHS bool@");
				append_str_to_str(primal_code, (t->t_elem->type) == TRUE ? "TRUE" : "FALSE");
				break;
		}
		append_char_to_str(primal_code, '\n');
	}
	return was_pushed;
}


/* Checks type identity */
bool is_token(TSElem *s, int type) {
	if (s == NULL) {
		return false;
	}
	if (s->t_elem != NULL) {
		if (s->t_elem->type == type)
			return true;
	}
	return false;
}

/* Returns priority of operators using precedence table*/
bool has_higher_priority(t_expressions op1, t_expressions op2) {
	return (precedence_tab[op1][op2]);
}

/* Checks if given token is operator */
bool is_operand(token *token_type, bool in_condition) {
	if (token_type == NULL){
		return false;
	}
	switch(token_type->type) {
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case DIV2:
			return true;
	}
	if (in_condition) {
		switch(token_type->type) {
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

/* Chceks if given token is value or not. */
bool is_value(token *token_type) {
	if (token_type == NULL){
		return false;
	}
	switch(token_type->type) {
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

/* Checks token type of found record in symtable. */
bool is_token_type(htab_t * symtable, TSElem *actual_token, int type) {
	if (symtable == NULL || actual_token->t_elem == NULL)
		return false;

	if (actual_token != NULL) {
		if (actual_token->t_elem->type == IDENTIFIER) {
			struct htab_listitem *found_record = htab_find(symtable, actual_token->t_elem->attr.string_value);

			if(is_function(found_record))
				error_msg(ERR_CODE_SEM, "Function '%s' used in expression\n", found_record->key);

			if (found_record != NULL){
				if (found_record->data.type == type)
					return true;
			}
		}
	}

	return false;
}

/* Checks if given type of token is valid. Eventually exits program with relevant error code. */
bool is_valid_token_type(htab_t * symtable, TSElem *actual_token, int type) {
	if (symtable == NULL || actual_token->t_elem == NULL)
		return false;

	struct htab_listitem *found_record = NULL;
	if (actual_token->t_elem->type == IDENTIFIER) {
		found_record = htab_find(symtable, actual_token->t_elem->attr.string_value);

		if(is_function(found_record))
			error_msg(ERR_CODE_SEM, "Function '%s' used in expression\n", found_record->key);
	}

	switch(type) {
		case INTEGER_TYPE:
			if (actual_token->t_elem->type == IDENTIFIER) {
				if (found_record == NULL) {
					error_msg(ERR_CODE_SEM, "Undeclared ID detected\n");
				}
				if ((found_record->data.type != INTEGER_TYPE)) {
					return false;
				}
			}
			else if (actual_token->t_elem->type != INT_VALUE) {
				return false;
			}
			break;
		case DOUBLE_TYPE:
			if (actual_token->t_elem->type == IDENTIFIER) {
				if (found_record == NULL) {
					error_msg(ERR_CODE_SEM, "Undeclared ID detected\n");
				}
				if ((found_record->data.type != DOUBLE_TYPE)) {
					return false;
				}
			}
			else if (actual_token->t_elem->type == DOUBLEE || actual_token->t_elem->type == DOUBLE_WITH_EXP || actual_token->t_elem->type == INT_WITH_EXP) {
				return true;
			}
			else {
				return false;
			}
			break;
		case BOOLEAN_TYPE:
			if (actual_token->t_elem->type == IDENTIFIER) {
				if (found_record == NULL) {
					error_msg(ERR_CODE_SEM, "Undeclared ID detected\n");
				}
				if ((found_record->data.type != BOOLEAN_TYPE)) {
					return false;
				}
			}
			else if (actual_token->t_elem->type == TRUE || actual_token->t_elem->type == FALSE ) {
			}
			else {
				return false;
			}
			break;
		case STRING_TYPE:
			if (actual_token->t_elem->type == IDENTIFIER) {
				if (found_record == NULL) {
					error_msg(ERR_CODE_SEM, "Undeclared ID detected\n");
				}
				if ((found_record->data.type != STRING_TYPE)) {
					return false;
				}
			}
			else if (actual_token->t_elem->type != STRING_VALUE) {
				return false;
			}
			break;
	}

	return true;
}

/* Checks semantic in using operator NOT. */
void sem_check_not(TSElem *actual_token, htab_t *symtable) {
	if (is_value(actual_token->t_elem)) {
		if (actual_token->t_elem->type == TRUE || actual_token->t_elem->type == FALSE) {
		}
		else if (actual_token->t_elem->type == IDENTIFIER) {
			if (is_token_type(symtable, actual_token, BOOLEAN_TYPE)) {
			}
			else {
				error_msg(ERR_CODE_TYPE, "NOT operand can be used with BOOLEAN values\n");
			}
		}
		else {
			error_msg(ERR_CODE_TYPE, "NOT operand can be used with BOOLEAN values\n");
		}
	}
	else {
		error_msg(ERR_CODE_TYPE, "Wrong type\n");
	}
}

/* Returns relevant semantic type. */
int return_semantic_type(TStack *Out, htab_t *symtable) {
	TSElem *tmp = Out->First;
	bool is_string = false;
	bool is_double = false;
	struct htab_listitem *found_record = NULL;

	while (tmp != NULL) {
		switch(tmp->t_elem->type) {
			case AND:
			case OR:
			case NOT:
			case TRUE:
			case FALSE:
			case LESS_THAN:
			case GREATER_THAN:
			case LESS_OR_EQUALS:
			case GREATER_OR_EQUALS:
			case EQUALS:
			case NOT_EQUALS:
				return BOOLEAN_TYPE;
				break;

			case IDENTIFIER:
				found_record = htab_find(symtable, tmp->t_elem->attr.string_value);
				if (found_record != NULL) {

					if(is_function(found_record))
						error_msg(ERR_CODE_SEM, "Function '%s' used in expression\n", found_record->key);

					if (found_record->data.type == BOOLEAN_TYPE)
						return BOOLEAN_TYPE;
					else if (found_record->data.type == DOUBLE_TYPE)
						is_double = true;
					else if (found_record->data.type == STRING_TYPE)
						is_string = true;
				}
				else {
					error_msg(ERR_CODE_SEM, "Undeclared ID %s\n", tmp->t_elem->attr.string_value);
				}
				break;

			case STRING_VALUE:
				is_string = true;
				break;

			case DOUBLEE:
			case DOUBLE_WITH_EXP:
			case INT_WITH_EXP:
			case DIV:
				is_double = true;
				break;

		}
		tmp = tmp->next;
	}
	if (is_string)
		return STRING_TYPE;
	else if (is_double)
		return DOUBLE_TYPE;
	else
		return INTEGER_TYPE;
}
