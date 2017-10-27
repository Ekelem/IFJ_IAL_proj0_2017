#include "parser.h"

void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	init_string(primal_code);
	append_str_to_str(primal_code, ".IFJ17\n");		//Header
	neterm_start(token_buff, symtable, primal_code);
}

void neterm_start(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case SCOPE :
			neterm_scope(token_buff, symtable, primal_code);
			break;
		case DECLARE :
			neterm_function_dec(token_buff, symtable, primal_code);
			break;
		case FUNCTION :
			//neterm_function_def(token_buff, symtable, primal_code);
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 3, SCOPE, DECLARE, FUNCTION);
			break;
	}


}

void neterm_scope(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	neterm_body(token_buff, symtable, primal_code);
}

void neterm_function_dec(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);		//expect "FUNCTION"
	switch (actual_token->type){
		case FUNCTION :
			//Do nothing
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, FUNCTION);
			break;
	}

	actual_token = token_buffer_get_token(token_buff);		//expect "IDENTIFIER"
	switch (actual_token->type){
		case IDENTIFIER :
			//check if exist
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, IDENTIFIER);
			break;
	}

	actual_token = token_buffer_get_token(token_buff);		//expect "LEFT_PARANTHESIS"
	switch (actual_token->type){
		case LEFT_PARANTHESIS :
			//Do nothing
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, LEFT_PARANTHESIS);
			break;
	}
	neterm_args(token_buff, symtable, primal_code);
	actual_token = token_buffer_get_token(token_buff);		//expect "AS"
	switch (actual_token->type){
		case AS :
			//Do nothing
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, AS);
			break;
	}
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

}

void neterm_body(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case DIM :
			body_declaration(token_buff, symtable, primal_code);
			break;
		/*case INPUT :
			body_input(token_buff, symtable, primal_code);
			break;
		case IF :
			body_if_then(token_buff, symtable, primal_code);
			break;
		case DO :
			body_do_while(token_buff, symtable, primal_code);
			break;
		case IDENTIFIER :
			body_assignment(token_buff, symtable, primal_code);
			break;
		case RETURN :
			body_return(token_buff, symtable, primal_code);
			break;*/
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

	actual_token = token_buffer_get_token(token_buff);		//expect "AS"
	switch (actual_token->type){
		case AS :
			//Do nothing
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, AS);
			break;
	}
	neterm_type(token_buff, symtable, primal_code);
	//expression(token_buff, symtable, primal_code);
	neterm_body(token_buff, symtable, primal_code);
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

	actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case NEW_LINE :
			break;
		default :
			syntax_error_unexpexted(actual_token->type, 1, NEW_LINE);
			break;
	}
}