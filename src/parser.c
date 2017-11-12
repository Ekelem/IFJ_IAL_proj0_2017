#include "parser.h"

void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	init_string(primal_code);
	append_str_to_str(primal_code, ".IFJcode17\n");		//Header
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
			neterm_function_def(token_buff, symtable, primal_code);
			neterm_start(token_buff, symtable, primal_code);
			break;
		case NEW_LINE :
			neterm_start(token_buff, symtable, primal_code);
			break;
		case EOF :
			if (scope_found == 0)
				error_msg(ERR_CODE_OTHERS, "Scope block wasnt found\n");
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 3, SCOPE, DECLARE, FUNCTION);
			break;
	}


}

void neterm_scope(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	append_str_to_str(primal_code, "CREATEFRAME\n");
	append_str_to_str(primal_code, "PUSHFRAME\n");
	while (!is_peek_token(token_buff, END))
	{
		neterm_body(token_buff, symtable, primal_code);
	}
	expected_token(token_buff, END);
	expected_token(token_buff, SCOPE);
	append_str_to_str(primal_code, "POPFRAME\n");
}

void neterm_function_dec(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	expected_token(token_buff, FUNCTION);

	token * actual_token = token_buffer_get_token(token_buff);		//expect "IDENTIFIER"
	struct htab_listitem *found_record = NULL;
	switch (actual_token->type){
		case IDENTIFIER :
			found_record = htab_find(symtable, actual_token->attr.string_value);
			if (found_record != NULL)
			{
				if (id_is_function(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' was declared before as function\n", found_record->key);
				if (!id_is_declared(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' is a variable\n", found_record->key);
			}
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}
	expected_token(token_buff, LEFT_PARANTHESIS);
	if (found_record == NULL)
	{
		found_record = make_item(actual_token->attr.string_value);
		set_id_declared(found_record);
		set_id_function(found_record);
		htab_append(found_record, symtable);
		set_func_par_count(found_record, 0);
	}
	while (!is_peek_token(token_buff, RIGHT_PARANTHESIS))
	{
		neterm_args_create(token_buff, symtable, primal_code, found_record);
	}
	expected_token(token_buff, RIGHT_PARANTHESIS);
	expected_token(token_buff, AS);
	found_record->data.type = neterm_type(token_buff, symtable, primal_code);
}

void neterm_function_def(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);		//expect "IDENTIFIER"
	struct htab_listitem * found = NULL;
	switch (actual_token->type){
		case IDENTIFIER :
			found = htab_find(symtable, actual_token->attr.string_value);
			if (id_is_defined(found))
				error_msg(ERR_CODE_OTHERS, "redefinition of function %s.\n", found->key);
			else
				set_id_defined(found);
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}
	expected_token(token_buff, LEFT_PARANTHESIS);
	if (found == NULL)	//implicit declaration
	{
		found = make_item(actual_token->attr.string_value);
		htab_append(found, symtable);
		set_id_function(found);
		set_id_declared(found);
		set_id_defined(found);
		set_func_par_count(found, 0);
		while (!is_peek_token(token_buff, RIGHT_PARANTHESIS))
		{
			neterm_args_create(token_buff, symtable, primal_code, found);
		}
		expected_token(token_buff, RIGHT_PARANTHESIS);
		expected_token(token_buff, AS);
		found->data.type = neterm_type(token_buff, symtable, primal_code);
	}
	else
	{
		while (!is_peek_token(token_buff, RIGHT_PARANTHESIS))
		{
			neterm_args(token_buff, symtable, primal_code, found);
		}
		expected_token(token_buff, RIGHT_PARANTHESIS);
		expected_token(token_buff, AS);
		if (found->data.type != neterm_type(token_buff, symtable, primal_code))
			error_msg(ERR_CODE_OTHERS, "return type in function %s do not match declaration.\n", found->key);
	}
	expected_token(token_buff, NEW_LINE);
	struct htab_t * new_symtable = htab_init(symtable->arr_size);
	while (!is_peek_token(token_buff, END))
	{
		neterm_body(token_buff, new_symtable, primal_code);
	}
	htab_free(new_symtable);
	expected_token(token_buff, END);
	expected_token(token_buff, FUNCTION);
	expected_token(token_buff, NEW_LINE);
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
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 3, INTEGER, STRING, DOUBLE);
			break;
	}
	return 0;
}

void neterm_args(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	struct func_par * actual_param = func_record->data.u_argconst.first_par;
	unsigned int param_order = 0;
	//printf("%d\n", func_record->data.par_count);
	while (42)
	{
		token * actual_token = token_buffer_get_token(token_buff);
		switch (actual_token->type){
			case IDENTIFIER :
				if (param_order < func_record->data.par_count)
				{
					if (strcmp(actual_token->attr.string_value, actual_param->par_name))
					{
						error_msg(ERR_CODE_OTHERS, "%d. param (%s) in function %s was declared before as %s.\n", param_order+1, actual_token->attr.string_value, func_record->key, actual_param->par_name);
					}
				}
				else
				{
					error_msg(ERR_CODE_OTHERS, "function %s take only %d parameters.\n", func_record->key, func_record->data.par_count);
				}
				break;
			default :
				syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
				break;
		}
		expected_token(token_buff, AS);
		if (actual_param->par_type != neterm_type(token_buff, symtable, primal_code))
			error_msg(ERR_CODE_OTHERS, "type of %d. parameter in function %s do not match type in declaration.\n", param_order+1, func_record->key);
	
		actual_token = token_buffer_peek_token(token_buff);
		if (actual_token->type == COMA)
		{
			expected_token(token_buff, COMA);
			param_order++;
			actual_param=actual_param->par_next;
			continue;
		}
		else if (actual_token->type == RIGHT_PARANTHESIS)
		{
			if (++param_order != func_record->data.par_count)
				error_msg(ERR_CODE_OTHERS, "function %s expect %d parameters.\n", func_record->key, func_record->data.par_count);
			break;
		}
		else
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 2, COMA, RIGHT_PARANTHESIS);
	}
}

void neterm_args_create(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * new_func_record)
{
	unsigned int param_order = 0;
	struct func_par ** actual_param = &(new_func_record->data.u_argconst.first_par);

	while (42)
	{
		token * actual_token = token_buffer_get_token(token_buff);
		switch (actual_token->type){
			case IDENTIFIER :
				add_func_par_count(new_func_record);
				*actual_param=malloc(sizeof(struct func_par));
				(*actual_param)->par_name=actual_token->attr.string_value;
				break;
			default :
				syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
				break;
		}
		expected_token(token_buff, AS);
		(*actual_param)->par_type=neterm_type(token_buff, symtable, primal_code);
	
		actual_token = token_buffer_peek_token(token_buff);
		if (actual_token->type == COMA)
		{
			expected_token(token_buff, COMA);
			param_order++;
			actual_param=&((*actual_param)->par_next);
			continue;
		}
		else if (actual_token->type == RIGHT_PARANTHESIS)
			break;
		else
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 2, COMA, RIGHT_PARANTHESIS);
	}
}

void neterm_body(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	struct htab_t * new_symtable;
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case DIM :
			body_declaration(token_buff, symtable, primal_code);
			break;
		case INPUT :
			body_input(token_buff, symtable, primal_code);
			break;
		case PRINT :
			body_print(token_buff, symtable, primal_code);
			break;
		case IF :
			body_if_then(token_buff, symtable, primal_code);
			break;
		case DO :
			body_do_while(token_buff, symtable, primal_code);
			break;
		case IDENTIFIER :
			body_assignment(token_buff, symtable, primal_code, actual_token);
			break;
		/*case RETURN :
			body_return(token_buff, symtable, primal_code);
			break;*/
		case SCOPE :
			expected_token(token_buff, NEW_LINE);
			new_symtable = htab_move(symtable->arr_size, symtable);
			neterm_scope(token_buff, new_symtable, primal_code);
			htab_free(new_symtable);
			break;

		case NEW_LINE :
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 6, DIM, INPUT, IF, DO, IDENTIFIER, RETURN);
			break;
	}
}

void body_declaration(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	struct htab_listitem * found_record = NULL;
	switch (actual_token->type){
		case IDENTIFIER :
			found_record = htab_find(symtable, actual_token->attr.string_value);
			if (found_record != NULL)
			{
				if (id_is_function(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' is function\n", found_record->key);
				if (id_is_declared(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' was declared before\n", found_record->key);
			}
			else
			{
				found_record = make_item(actual_token->attr.string_value);
				htab_append(found_record, symtable);
			}
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}
	expected_token(token_buff, AS);
	token *var_type = token_buffer_peek_token(token_buff);
	set_id_type(found_record, neterm_type(token_buff, symtable, primal_code));
	set_id_declared(found_record);
	append_str_to_str(primal_code, "DEFVAR LF@");
	append_str_to_str(primal_code, found_record->key);
	append_char_to_str(primal_code, '\n');

	actual_token = token_buffer_peek_token(token_buff);
	if (actual_token->type == NEW_LINE){
		//TODO: init var
		expected_token(token_buff, NEW_LINE);
	}
	else {
		expression_EQ(token_buff, symtable, primal_code, found_record->key, var_type->type);
	}
}

void body_input(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_get_token(token_buff);
	switch (actual_token->type){
		case IDENTIFIER :
			//check if exists
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}

	expected_token(token_buff, NEW_LINE);
}

void body_if_then(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	expression_EQ(token_buff, symtable, primal_code, NULL, e_if);

	expected_token(token_buff, NEW_LINE);
	unsigned int order =generate_if_label_order();
	generate_if_label(primal_code, label_if, order);
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
		generate_if_label(primal_code, label_else, order);

		while (((next_token = token_buffer_peek_token(token_buff))->type) != END)
		{
			neterm_body(token_buff, symtable, primal_code);
		}
	case END :
		expected_token(token_buff, END);
		expected_token(token_buff, IF);
		expected_token(token_buff, NEW_LINE);
		generate_if_label(primal_code, label_end_if, order);
		break;
	default :
		break;
	}
}

void body_do_while(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	expected_token(token_buff, WHILE);
	expression_EQ(token_buff, symtable, primal_code, NULL, e_while);
	while (!is_peek_token(token_buff, LOOP))
	{
		neterm_body(token_buff, symtable, primal_code);
	}

	expected_token(token_buff, LOOP);
	expected_token(token_buff, NEW_LINE);
}

void body_assignment(token_buffer * token_buff, htab_t * symtable, String * primal_code, token * identifier)
{
	struct htab_listitem * found_record = htab_find(symtable, identifier->attr.string_value);
	if (found_record == NULL)
		error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' is not declared\n", identifier->attr.string_value);

	int type;
	if (found_record->data.type == 1)
		type = INTEGER;
	else if (found_record->data.type == 2)
		type = STRING;
	else if (found_record->data.type == 4)
		type = DOUBLE;
	else
		type = BOOLEAN;
	expression_EQ(token_buff, symtable, primal_code, identifier->attr.string_value, type);
}

void body_print(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	//expression
	expected_token(token_buff, SEMICOLON);
	if (is_peek_token(token_buff, NEW_LINE))
		expected_token(token_buff, NEW_LINE);
	else
		body_print(token_buff, symtable, primal_code);
}

void expected_token(token_buffer * token_buff, int tok_type)
{
	token * actual_token = token_buffer_get_token(token_buff);
	if (actual_token->type != tok_type)
		syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, tok_type);
}

bool is_peek_token(token_buffer * token_buff, int tok_type)
{
	token * next_token = token_buffer_peek_token(token_buff);
	if (next_token->type == tok_type)
		return 1;
	else
		return 0;
}

void neterm_expression(token_buffer * token_buff, htab_t * symtable, String * primal_code, token_type end_token)
{
	struct dynamic_stack * expr_stack=dynamic_stack_init();
	token * actual_token;
	char buffer[128];
	while (!is_peek_token(token_buff, end_token))
	{
		actual_token = token_buffer_get_token(token_buff);
		switch(actual_token->type)
		{
			case INT_VALUE:		//operand type int
				//dynamic_stack_push(expr_stack, actual_token)
				append_str_to_str(primal_code, "PUSHS int@");
				snprintf(buffer, 128, "%d", actual_token->attr.int_value);
				append_str_to_str(primal_code, buffer);
				append_char_to_str(primal_code, '\n');
				break;

			case ADD:		//operator +
				if (dynamic_stack_empty(expr_stack))	//rules postfix
				{
					dynamic_stack_push(expr_stack, actual_token);
				}
				else
				{
					token * previous_oper = dynamic_stack_top(expr_stack);
					if (previous_oper->type <= ADD)
					{
						dynamic_stack_pop(expr_stack);
						switch (previous_oper->type)
						{
							case ADD:
								append_str_to_str(primal_code, "ADDS\n");
								break;
							default:
								break;
						}
						dynamic_stack_push(expr_stack, actual_token);
					}
					else
					{

					}

				}
		}
	}
	if (!dynamic_stack_empty(expr_stack))
	{
		actual_token = dynamic_stack_top(expr_stack);
		switch (actual_token->type)
		{
		case ADD:
			append_str_to_str(primal_code, "ADDS\n");
			break;
		default:
			break;
		}
	}
}

//void expression_operator_push()

unsigned int generate_if_label_order()
{
	static unsigned int counter = 0;
	return counter++;
}

void generate_if_label(String * primal_code, enum_label_names prefix, unsigned int order)
{
	char buffer[4] = "0000";
	append_str_to_str(primal_code, "LABEL ");
	switch (prefix)
	{
		case label_if:
			snprintf(buffer, 4, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "IF\n");
			break;
		case label_else:
			snprintf(buffer, 4, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ELSE\n");
			break;
		case label_end_if:
			snprintf(buffer, 4, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ENDIF\n");
			break;
		default:
			break;
	}
}

void generate_if_jump(String * primal_code, enum_label_names prefix, unsigned int order)
{
	char buffer[4] = "0000";
	append_str_to_str(primal_code, "JUMP ");
	switch (prefix)
	{
		case label_if:
			snprintf(buffer, 4, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "IF\n");
			break;
		case label_else:
			snprintf(buffer, 4, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ELSE\n");
			break;
		case label_end_if:
			snprintf(buffer, 4, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ENDIF\n");
			break;
		default:
			break;
	}
}