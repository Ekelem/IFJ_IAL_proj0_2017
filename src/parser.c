#include "parser.h"

void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	init_string(primal_code);
	append_str_to_str(primal_code, ".IFJ17\n");		//Header
	neterm_start(token_buff, symtable, primal_code);
}

void neterm_start(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	static bool scope_found = 0;
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case SCOPE :
			if (scope_found)
				error_msg(ERR_CODE_OTHERS, "Scope block was already defined\n");
			scope_found = 1;
			neterm_scope(token_buff, symtable, primal_code);
			neterm_start(token_buff, symtable, primal_code);
			break;
		case DECLARE :
			neterm_function_dec(token_buff, symtable, primal_code);
			neterm_start(token_buff, symtable, primal_code);
			break;
		case FUNCTION :
			//neterm_function_def(token_buff, symtable, primal_code);
			break;
		case NEW_LINE :
			neterm_start(token_buff, symtable, primal_code);
			break;
		case EOF :
			if (scope_found == 0)
				error_msg(ERR_CODE_OTHERS, "Scope block wasnt found\n");
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 3, SCOPE, DECLARE, FUNCTION);
			break;
	}


}

void neterm_scope(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	while (!is_peek_token(token_buff, END))
	{
		neterm_body(token_buff, symtable, primal_code);
	}
	expected_token(token_buff, END);
	expected_token(token_buff, SCOPE);
}

void neterm_function_dec(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	expected_token(token_buff, FUNCTION);

	token * actual_token = token_buffer_get_token(token_buff);		//expect "IDENTIFIER"
	switch (actual_token->type){
		case IDENTIFIER :
			//check if exist
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, IDENTIFIER);
			break;
	}
	expected_token(token_buff, LEFT_PARANTHESIS);
	neterm_args(token_buff, symtable, primal_code);
	expected_token(token_buff, AS);
	neterm_type(token_buff, symtable, primal_code);
}

unsigned int neterm_type(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);		//type selection
	switch (actual_token->type){
		case INTEGER :
			return INTEGER_TYPE;
			break;
		case STRING :
			return STRING_TYPE;
			break;
		case DOUBLE :
			return DOUBLE_TYPE;
			break;
		case BOOLEAN :
			return BOOLEAN_TYPE;
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 3, INTEGER, STRING, DOUBLE);
			break;
	}
	return 0;
}

void neterm_args(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case IDENTIFIER :
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, IDENTIFIER);
			break;
	}
	expected_token(token_buff, AS);
	neterm_type(token_buff, symtable, primal_code);

	actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case COMA :
			neterm_args(token_buff, symtable, primal_code);
			break;
		case RIGHT_PARANTHESIS :
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 2, COMA, RIGHT_PARANTHESIS);
			break;
	}
}

void neterm_body(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case DIM :
			body_declaration(token_buff, symtable, primal_code);
			break;
		case INPUT :
			body_input(token_buff, symtable, primal_code);
			break;
		case IF :
			body_if_then(token_buff, symtable, primal_code);
			break;
		case DO :
			body_do_while(token_buff, symtable, primal_code);
			break;
		/*case IDENTIFIER :
			body_assignment(token_buff, symtable, primal_code);
			break;
		case RETURN :
			body_return(token_buff, symtable, primal_code);
			break;*/

		case NEW_LINE :
			neterm_body(token_buff, symtable, primal_code);
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 6, DIM, INPUT, IF, DO, IDENTIFIER, RETURN);
			break;
	}
}

void body_declaration(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case IDENTIFIER :
			//check if exists -> error
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, IDENTIFIER);
			break;
	}
	expected_token(token_buff, AS);
	neterm_type(token_buff, symtable, primal_code);
	//expression(token_buff, symtable, primal_code);
	expected_token(token_buff, NEW_LINE);
}

void body_input(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case IDENTIFIER :
			//check if exists
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, IDENTIFIER);
			break;
	}

	expected_token(token_buff, NEW_LINE);
}

void body_if_then(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	//Expression
	expected_token(token_buff, THEN);
	expected_token(token_buff, NEW_LINE);
	token * next_token = token_buffer_peek_token(token_buff);
	while (next_token->type != ELSE && next_token->type != END)
	{
		neterm_body(token_buff, symtable, primal_code);
		next_token = token_buffer_peek_token(token_buff);
	}
	switch (next_token->type){
	case ELSE :
		expected_token(token_buff, ELSE);
		expected_token(token_buff, NEW_LINE);

		while (((next_token = token_buffer_peek_token(token_buff))->type) != END)
		{
			neterm_body(token_buff, symtable, primal_code);
		}
	case END :
		expected_token(token_buff, END);
		expected_token(token_buff, IF);
		expected_token(token_buff, NEW_LINE);
		break;
	default :
		break;
	}
}

void body_do_while(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	expected_token(token_buff, WHILE);
	//Expression
	expected_token(token_buff, NEW_LINE);
	while (!is_peek_token(token_buff, LOOP))
	{
		neterm_body(token_buff, symtable, primal_code);
	}

	expected_token(token_buff, LOOP);
	expected_token(token_buff, NEW_LINE);
}

void body_assignment(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	//Unary operators needed (Erik)
	//expression
	expected_token(token_buff, NEW_LINE);
}

void expected_token(token_buffer * token_buff, int tok_type)
{
	token * actual_token = token_buffer_get_token(token_buff);
	if (actual_token->type != tok_type)
		syntax_error_unexpexted(actual_token->type, 1, tok_type);
}

bool is_peek_token(token_buffer * token_buff, int tok_type)
{
	token * next_token = token_buffer_peek_token(token_buff);
	if (next_token->type == tok_type)
		return 1;
	else
		return 0;
}