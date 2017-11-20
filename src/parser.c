#include "parser.h"

#define DEBUG_MSG 42

void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: started translation\n");
	#endif

	init_string(primal_code);
	append_str_to_str(primal_code, ".IFJcode17\n");		//Header
	append_str_to_str(primal_code, "JUMP %MAIN\n");		//jump to scope
	
	add_build_in_functions(symtable, primal_code);

	neterm_start(token_buff, symtable, primal_code);
}

void neterm_start(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm start\n");
	#endif

	static bool scope_found = 0;

	while ((token_buffer_peek_token(token_buff)->type)!=EOF)
	{
		token * actual_token = token_buffer_get_token(token_buff);
		switch (actual_token->type){
		case SCOPE :
			if (scope_found)
				error_msg(ERR_CODE_OTHERS, "Scope block was already defined\n");
			scope_found = 1;
			append_str_to_str(primal_code, "LABEL %MAIN\n");
			append_str_to_str(primal_code, "CREATEFRAME\n");
			append_str_to_str(primal_code, "PUSHFRAME\n");
			neterm_scope(token_buff, symtable, primal_code);
			append_str_to_str(primal_code, "POPFRAME\n");
			break;
		case DECLARE :
			neterm_function_dec(token_buff, symtable, primal_code);
			break;
		case FUNCTION :
			neterm_function_def(token_buff, symtable, primal_code);
			break;
		case NEW_LINE :
			break;
		case EOF :
			if (scope_found == 0)
				error_msg(ERR_CODE_OTHERS, "Scope block wasnt found\n");
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type,
                                    3, SCOPE, DECLARE, FUNCTION);
			break;
		}
	}
}

void neterm_scope(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm scope\n");
	#endif

	while (!is_peek_token(token_buff, END))
	{
		neterm_body(token_buff, symtable, primal_code);
	}
	expected_token(token_buff, END);
	expected_token(token_buff, SCOPE);
}

void neterm_function_dec(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: function declaration\n");
	#endif

	expected_token(token_buff, FUNCTION);

	token * actual_token = token_buffer_get_token(token_buff);		//expect "IDENTIFIER"
	struct htab_listitem *found_record = NULL;
	switch (actual_token->type){
		case IDENTIFIER :
			found_record = htab_find(symtable, actual_token->attr.string_value);
			if (found_record != NULL)
			{
				if (!id_is_function(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' was declared before as variable\n", found_record->key);
				if (!id_is_declared(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' was declared before.\n", found_record->key);
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
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: function definition\n");
	#endif

	token * actual_token = token_buffer_get_token(token_buff);		//expect "IDENTIFIER"
	struct htab_listitem * found = NULL;
	switch (actual_token->type){
		case IDENTIFIER :
			found = htab_find(symtable, actual_token->attr.string_value);
			if (found!=NULL)
			{
				if (id_is_defined(found))
					error_msg(ERR_CODE_OTHERS, "redefinition of function %s.\n", found->key);
				else
					set_id_defined(found);
			}
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
	append_str_to_str(primal_code, "LABEL ");
	append_str_to_str(primal_code, found->key);
	append_char_to_str(primal_code, '\n');
	append_str_to_str(primal_code, "DEFVAR LF@%returnval\n");
	struct htab_t * new_symtable = htab_init(symtable->arr_size);
	while (!is_peek_token(token_buff, END))
	{
		neterm_body(token_buff, new_symtable, primal_code);
	}
	htab_free(new_symtable);
	expected_token(token_buff, END);
	expected_token(token_buff, FUNCTION);
	expected_token(token_buff, NEW_LINE);
	append_str_to_str(primal_code, "MOVE LF@%returnval ");
	append_str_to_str(primal_code, "RETURN\n");
}

unsigned int neterm_type(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm type\n");
	#endif

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
			syntax_error_unexpexted(actual_token->line, actual_token->pos , actual_token->type,
                                    3, INTEGER, STRING, DOUBLE);
			break;
	}
	return 0;
}

void neterm_args(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm args\n");
	#endif

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
						error_msg(ERR_CODE_OTHERS, "%d. param (%s) in function %s was declared before as %s.\n",
                                  param_order+1, actual_token->attr.string_value, func_record->key,
                                  actual_param->par_name);
					}
				}
				else
				{
					error_msg(ERR_CODE_OTHERS, "function %s take only %d parameters.\n", func_record->key,
                              func_record->data.par_count);
				}
				break;
			default :
				syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
				break;
		}
		expected_token(token_buff, AS);
		if (actual_param->par_type != neterm_type(token_buff, symtable, primal_code))
			error_msg(ERR_CODE_OTHERS, "type of %d. parameter in function %s do not match type in declaration.\n",
                      param_order+1, func_record->key);
	
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
				error_msg(ERR_CODE_OTHERS, "function %s expect %d parameters.\n",
                          func_record->key, func_record->data.par_count);
			break;
		}
		else
			syntax_error_unexpexted(actual_token->line, actual_token->pos,
                                    actual_token->type, 2, COMA, RIGHT_PARANTHESIS);
	}
}

void neterm_args_create(token_buffer * token_buff, htab_t * symtable, String * primal_code,
                        struct htab_listitem * new_func_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm args create\n");
	#endif

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
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,
                                    actual_token->type, 2, COMA, RIGHT_PARANTHESIS);
	}
}

void neterm_body(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm body\n");
	#endif

	struct htab_t * new_symtable;
	struct htab_listitem * found_record;
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
			found_record = htab_find(symtable, actual_token->attr.string_value);	//search in symtable for identifier
			if (found_record == NULL)
			error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' is not declared\n",
                      actual_token->attr.string_value);	//Does not exist.. Too bad
	
			if (id_is_function(found_record))	//is it function or variable name? ..can not be both, sorry.
				function_call(token_buff, symtable, primal_code, found_record);
			else
				body_assignment(token_buff, symtable, primal_code, found_record);
			break;
		/*case RETURN :
			body_return(token_buff, symtable, primal_code);
			break;*/
		case SCOPE :
			expected_token(token_buff, NEW_LINE);
			new_symtable = htab_move(symtable->arr_size, symtable);
			append_str_to_str(primal_code, "CREATEFRAME\n");
			htab_foreach(symtable, new_symtable, primal_code, (*copy_scope_layer));
			append_str_to_str(primal_code, "PUSHFRAME\n");
			neterm_scope(token_buff, new_symtable, primal_code);
			append_str_to_str(primal_code, "POPFRAME\n");
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
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body declaration\n");
	#endif

	token * actual_token = token_buffer_get_token(token_buff);
	struct htab_listitem * found_record = NULL;
	switch (actual_token->type){
		case IDENTIFIER :
			found_record = htab_find(symtable, actual_token->attr.string_value);
			if (found_record != NULL)
			{
				if (id_is_function(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' is function.\n", found_record->key);
				if (id_is_declared(found_record))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' was declared before.\n", found_record->key);
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
	set_id_type(found_record, neterm_type(token_buff, symtable, primal_code));
	set_id_declared(found_record);
	append_str_to_str(primal_code, "DEFVAR LF@");
	append_str_to_str(primal_code, found_record->key);
	append_char_to_str(primal_code, '\n');

	actual_token = token_buffer_peek_token(token_buff);
	if (actual_token->type == NEW_LINE)		//implicit value
	{
		generate_implicit_value(primal_code, found_record->key, found_record->data.type);
		expected_token(token_buff, NEW_LINE);
	}
	else if (actual_token->type == EQUALS)
	{
		expected_token(token_buff, EQUALS);
		int variable_type = get_id_type(found_record);
		int expr_return_type = parse_expression(token_buff, symtable, primal_code, found_record->key, INTEGER_TYPE, NEW_LINE);
		parse_semantic_expression(primal_code, found_record, variable_type, expr_return_type);
		set_id_defined(found_record);
	}
	else {
		syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 2, NEW_LINE, EQUALS);
	}
}

void body_input(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body input\n");
	#endif

	token * actual_token = token_buffer_get_token(token_buff);
	struct htab_listitem * found_record = NULL;
	switch (actual_token->type){
		case IDENTIFIER :
			//check if exists
			found_record = htab_find(symtable, actual_token->attr.string_value);
			if (found_record == NULL || id_is_function(found_record))
				error_msg(ERR_CODE_OTHERS, "identifier %s was not declared in this scope as variable.", actual_token->attr.string_value);

			append_str_to_str(primal_code, "READ LF@");
			append_str_to_str(primal_code, actual_token->attr.string_value);

			switch (get_id_type(found_record))
			{
				case INTEGER_TYPE:
					append_str_to_str(primal_code, " int\n");
				break;
				case DOUBLE_TYPE:
					append_str_to_str(primal_code, " float\n");
				break;
				case STRING_TYPE:
					append_str_to_str(primal_code, " string\n");
				break;
				case BOOLEAN_TYPE:
					append_str_to_str(primal_code, " bool\n");
				break;
			}
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}

	expected_token(token_buff, NEW_LINE);
}

void body_if_then(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body if then\n");
	#endif

	bool else_branch=0;

	int variable_type = BOOLEAN_TYPE;
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, NULL, variable_type, THEN);
	parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);


	append_str_to_str(primal_code, "PUSHS bool@true\n");
	append_str_to_str(primal_code, "JUMPIFNEQS ");
	unsigned int order =generate_if_label_order();
	generate_if_label(primal_code, label_else, order);
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
            append_str_to_str(primal_code, "JUMP ");
            generate_if_label(primal_code, label_end_if, order);
            append_str_to_str(primal_code, "LABEL ");
            generate_if_label(primal_code, label_else, order);

            else_branch=1;

            while (((next_token = token_buffer_peek_token(token_buff))->type) != END)
            {
                neterm_body(token_buff, symtable, primal_code);
            }
        case END :
            if(else_branch)
            {
                append_str_to_str(primal_code, "LABEL ");
                generate_if_label(primal_code, label_end_if, order);
            }
            else
            {
                append_str_to_str(primal_code, "LABEL ");
                generate_if_label(primal_code, label_else, order);
            }
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
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body do while\n");
	#endif

	expected_token(token_buff, WHILE);

	unsigned int order = generate_if_label_order();
	append_str_to_str(primal_code, "LABEL ");
	generate_if_label(primal_code, label_if, order);

	int variable_type = BOOLEAN_TYPE;
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, NULL, BOOLEAN_TYPE, NEW_LINE);
	parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);

	append_str_to_str(primal_code, "PUSHS bool@true\n");
	append_str_to_str(primal_code, "JUMPIFNEQS ");
	generate_if_label(primal_code, label_else, order);

	while (!is_peek_token(token_buff, LOOP))
	{
		neterm_body(token_buff, symtable, primal_code);
	}

	append_str_to_str(primal_code, "JUMP ");
	generate_if_label(primal_code, label_if, order);
	append_str_to_str(primal_code, "LABEL ");
	generate_if_label(primal_code, label_else, order);

	expected_token(token_buff, LOOP);
	expected_token(token_buff, NEW_LINE);
}

void body_assignment(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * found_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body assignment\n");
	#endif

	expected_token(token_buff, EQUALS);
	int variable_type = get_id_type(found_record);
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, found_record->key, 420, NEW_LINE);
	parse_semantic_expression(primal_code, found_record, variable_type, expr_return_type);
	set_id_defined(found_record);
}

void body_return(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	//only callable from funcbody
	int variable_type = get_id_type(func_record);
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, func_record->key, 420, NEW_LINE);
	parse_semantic_expression(primal_code, func_record, variable_type, expr_return_type);

	append_str_to_str(primal_code, "POPS %returnval\n");
	append_str_to_str(primal_code, "RETURN\n");
}

void parse_semantic_expression(String * primal_code, struct htab_listitem *found_record, int variable_type, int expr_return_type) {

	if (variable_type == DOUBLE_TYPE && expr_return_type == INTEGER_TYPE) {
		append_str_to_str(primal_code, "INT2FLOATS\nPOPS LF@");
		append_str_to_str(primal_code, found_record->key);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == DOUBLE_TYPE && expr_return_type == DOUBLE_TYPE){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, found_record->key);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == INTEGER_TYPE && expr_return_type == DOUBLE_TYPE){
		append_str_to_str(primal_code, "FLOAT2INTS\nPOPS LF@");
		append_str_to_str(primal_code, found_record->key);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == INTEGER_TYPE && expr_return_type == INTEGER_TYPE){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, found_record->key);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == BOOLEAN_TYPE && expr_return_type == BOOLEAN_TYPE && found_record == NULL){

	}
	else if (variable_type == BOOLEAN_TYPE && expr_return_type == BOOLEAN_TYPE && found_record != NULL){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, found_record->key);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == STRING_TYPE && expr_return_type == STRING_TYPE){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, found_record->key);
		append_char_to_str(primal_code, '\n');
	}

	else {
		error_msg(ERR_CODE_OTHERS, "The expression value does not match the variable type\n");
	}
}

void function_call(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * found_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: function call\n");
	#endif

	append_str_to_str(primal_code, "CREATEFRAME\n");	//prepare Temporary frame

	struct func_par * actual_param = found_record->data.u_argconst.first_par;
	unsigned int param_order = 0;
	struct htab_listitem * param_caller = NULL;
	expected_token(token_buff, LEFT_PARANTHESIS);
	while ((token_buffer_peek_token(token_buff)->type)!=RIGHT_PARANTHESIS)
	{
		if (param_order != 0)
			expected_token(token_buff, COMA);

		token * actual_token = token_buffer_get_token(token_buff);
		switch (actual_token->type){
			case IDENTIFIER :
                //printf("Prom:%s\n", actual_token->attr.string_value);
				param_caller = htab_find(symtable, actual_token->attr.string_value);
				if (param_caller == NULL)
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' does not exist.\n",
                              actual_token->attr.string_value);

				/*if (!id_is_defined(param_caller))
					error_msg(ERR_CODE_OTHERS, "IDENTIFIER '%s' is used undefined.\n", param_caller->key);*/

				if (param_order < found_record->data.par_count)
				{
					switch (actual_param->par_type)
					{
						case INTEGER_TYPE:
							switch (get_id_type(param_caller))
							{
								case INTEGER_TYPE:
								append_str_to_str(primal_code, "DEFVAR TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, "\nMOVE TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, " LF@");
								append_str_to_str(primal_code, param_caller->key);
								append_char_to_str(primal_code, '\n');
								break;
								case DOUBLE_TYPE:
								append_str_to_str(primal_code, "DEFVAR TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, "\nFLOAT2INT TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, " LF@");
								append_str_to_str(primal_code, param_caller->key);
								append_str_to_str(primal_code, "\n");
								break;
								default:
								error_msg(ERR_CODE_TYPE, "%d. parameter in function %s expect integer or double.\n",
                                          param_order+1, found_record->key);
								break;
							}
						break;
						case DOUBLE_TYPE:
							switch (get_id_type(param_caller))
							{
								case DOUBLE_TYPE:
								append_str_to_str(primal_code, "DEFVAR TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, "\nMOVE TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, " LF@");
								append_str_to_str(primal_code, param_caller->key);
								append_char_to_str(primal_code, '\n');
								break;
								case INTEGER_TYPE:
								append_str_to_str(primal_code, "DEFVAR TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, "\nINT2FLOAT TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, " LF@");
								append_str_to_str(primal_code, param_caller->key);
								append_str_to_str(primal_code, "\n");
								break;
								default:
								error_msg(ERR_CODE_TYPE,
                                          "%d. parameter in function %s expect integer or double value.\n",
                                          param_order+1, found_record->key);
								break;
							}
						break;
						case STRING_TYPE:
							if (get_id_type(param_caller) == STRING_TYPE)
							{
								append_str_to_str(primal_code, "DEFVAR TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, "\nMOVE TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, " LF@");
								append_str_to_str(primal_code, param_caller->key);
								append_char_to_str(primal_code, '\n');
							}
							else
								error_msg(ERR_CODE_TYPE, "%d. parameter in function %s expect string value.\n",
                                          param_order+1, found_record->key);
							
							break;
						case BOOLEAN_TYPE:
							if (get_id_type(param_caller) == BOOLEAN_TYPE)
							{
								append_str_to_str(primal_code, "DEFVAR TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, "\nMOVE TF@");
								append_str_to_str(primal_code, actual_param->par_name);
								append_str_to_str(primal_code, " LF@");
								append_str_to_str(primal_code, param_caller->key);
								append_char_to_str(primal_code, '\n');
							}
							else
								error_msg(ERR_CODE_TYPE, "%d. parameter in function %s expect boolean value.\n",
                                          param_order+1, found_record->key);
							
							break;
						default:
							error_msg(ERR_CODE_TYPE, "%d. parameter in function calling %s have unexpected value.\n",
                                      param_order+1, found_record->key);
						break;
					}
				}
				else
				{
					error_msg(ERR_CODE_TYPE, "function %s takes only %d parameters.\n",
                              found_record->key, found_record->data.par_count);
				}
				break;
			default :
				syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
				break;
		}
		param_order++;
		actual_param=actual_param->par_next;
	}
	//printf("%d\n", found_record->data.par_count);
	if (param_order != found_record->data.par_count)
		error_msg(ERR_CODE_OTHERS, "function %s expect %d parameters.\n",
                  found_record->key, found_record->data.par_count);

	expected_token(token_buff, RIGHT_PARANTHESIS);
	append_str_to_str(primal_code, "PUSHFRAME\n");
	append_str_to_str(primal_code, "CALL ");
	append_str_to_str(primal_code, found_record->key);
	append_char_to_str(primal_code, '\n');
	append_str_to_str(primal_code, "POPFRAME\n");
}

void body_print(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body print\n");
	#endif

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
	//append_str_to_str(primal_code, "LABEL %");
	switch (prefix)
	{
		case label_if:
			snprintf(buffer, 4, "%d", order);
			append_char_to_str(primal_code, '%');
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "IF\n");
			break;
		case label_else:
			snprintf(buffer, 4, "%d", order);
			append_char_to_str(primal_code, '%');
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ELSE\n");
			break;
		case label_end_if:
			snprintf(buffer, 4, "%d", order);
			append_char_to_str(primal_code, '%');
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

void generate_prepare_params(String * primal_code, struct func_par * actual_param, unsigned int param_order,
                             struct htab_listitem * param_caller, struct htab_listitem * found_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: generate prepare params\n");
	#endif

	switch (actual_param->par_type)
	{
		case INTEGER_TYPE:
			switch (get_id_type(param_caller))
			{
				case INTEGER_TYPE:
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, "\nMOVE TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, " LF@");
				append_str_to_str(primal_code, param_caller->key);
				append_char_to_str(primal_code, '\n');
				break;
				case DOUBLE_TYPE:
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, "\nFLOAT2INT TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, " LF@");
				append_str_to_str(primal_code, param_caller->key);
				append_str_to_str(primal_code, "\n");
				break;
				default:
				error_msg(ERR_CODE_TYPE, "%d. parameter in function %s expect integer or double.",
                          param_order+1, found_record->key);
				break;
			}
		case DOUBLE_TYPE:
			switch (get_id_type(param_caller))
			{
				case DOUBLE_TYPE:
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, "\nMOVE TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, " LF@");
				append_str_to_str(primal_code, param_caller->key);
				append_char_to_str(primal_code, '\n');
				break;
				case INTEGER_TYPE:
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, "\nINT2FLOAT TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, " LF@");
				append_str_to_str(primal_code, param_caller->key);
				append_str_to_str(primal_code, "\n");
				break;
				default:
				error_msg(ERR_CODE_TYPE, "%d. parameter in function %s expect integer or double value.",
                          param_order+1, found_record->key);
				break;
			}
		case STRING_TYPE:
			if (get_id_type(param_caller) == STRING_TYPE)
			{
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, "\nMOVE TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, " LF@");
				append_str_to_str(primal_code, param_caller->key);
				append_char_to_str(primal_code, '\n');
			}
			else
				error_msg(ERR_CODE_TYPE, "%d. parameter in function %s expect string value.",
                          param_order+1, found_record->key);
			
			break;
		case BOOLEAN_TYPE:
			if (get_id_type(param_caller) == BOOLEAN_TYPE)
			{
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, "\nMOVE TF@");
				append_str_to_str(primal_code, actual_param->par_name);
				append_str_to_str(primal_code, " LF@");
				append_str_to_str(primal_code, param_caller->key);
				append_char_to_str(primal_code, '\n');
			}
			else
				error_msg(ERR_CODE_TYPE, "%d. parameter in function %s expect boolean value.",
                          param_order+1, found_record->key);
			
			break;
		default:
			error_msg(ERR_CODE_TYPE, "%d. parameter in function calling %s have unexpected value.",
                      param_order+1, found_record->key);
		break;
	}
}

void add_build_in_functions(htab_t * symtable, String * primal_code)
{
	struct htab_listitem * record = NULL;

	//length(s as string) as integer
	record = create_func_record(symtable, "length");
	record->data.type=INTEGER_TYPE;
	record->data.u_argconst.first_par=malloc(sizeof(struct func_par));
	record->data.u_argconst.first_par->par_name="s";
	record->data.u_argconst.first_par->par_type=STRING_TYPE;
	set_func_par_count(record, 1);

	append_str_to_str(primal_code, "LABEL length\n"
									"DEFVAR LF@%returnval\n"
									"STRLEN LF@%returnval LF@s\n"
									"RETURN\n"
                                    "\n");


	//substr(s as string, i as integer, n as integer) as string
	record = create_func_record(symtable, "substr");
	record->data.type=STRING_TYPE;

	record->data.u_argconst.first_par=malloc(sizeof(struct func_par));
	record->data.u_argconst.first_par->par_name="s";
	record->data.u_argconst.first_par->par_type=STRING_TYPE;

	record->data.u_argconst.first_par->par_next=malloc(sizeof(struct func_par));
	record->data.u_argconst.first_par->par_next->par_name="i";
	record->data.u_argconst.first_par->par_next->par_type=INTEGER_TYPE;

	record->data.u_argconst.first_par->par_next->par_next=malloc(sizeof(struct func_par));
	record->data.u_argconst.first_par->par_next->par_next->par_name="n";
	record->data.u_argconst.first_par->par_next->par_next->par_type=INTEGER_TYPE;
	set_func_par_count(record, 3);

	append_str_to_str(primal_code,  "LABEL substr\n"
                                    "DEFVAR LF@%returnval\n"
                                    "DEFVAR LF@supbool\n");
    append_str_to_str(primal_code,  "DEFVAR LF@strindex\n"
                                    "DEFVAR LF@slen\n"
                                    "DEFVAR LF@onechr\n");
    append_str_to_str(primal_code,  "STRLEN LF@slen LF@s\n"
									"EQ LF@supbool LF@slen int@0\n");
    append_str_to_str(primal_code,  "JUMPIFNEQ Snzero LF@supbool bool@true\n"
    								"LABEL SubstrEnd\n");
	append_str_to_str(primal_code,  "MOVE LF@%returnval string@\\000 \n");
	append_str_to_str(primal_code,  "RETURN\n");
    append_str_to_str(primal_code,  "LABEL Snzero\n"
									"EQ LF@supbool LF@i int@0\n");
	append_str_to_str(primal_code,  "JUMPIFEQ SubstrEnd LF@supbool bool@true\n");
    append_str_to_str(primal_code,  "LT LF@supbool LF@i int@0\n");
	append_str_to_str(primal_code,  "JUMPIFEQ SubstrEnd LF@supbool bool@true\n");
    append_str_to_str(primal_code,  "LT LF@supbool LF@n int@0\n");
    append_str_to_str(primal_code,  "JUMPIFEQ Sublen LF@supbool bool@true\n"
                                    "SUB LF@strindex LF@slen LF@i\n");
    append_str_to_str(primal_code,  "GT LF@supbool LF@n LF@strindex\n");
    append_str_to_str(primal_code,  "JUMPIFEQ Sublen LF@supbool bool@true\n"
                                    "JUMP Scontinue\n");
    append_str_to_str(primal_code,  "LABEL Sublen\n");
    append_str_to_str(primal_code,  "MOVE LF@n LF@slen\n"
                                    "LABEL Scontinue\n"
                                    "MOVE LF@strindex LF@i\n");
    append_str_to_str(primal_code,  "SUB LF@strindex int@1\n"
                                    "LABEL Scycle\n");
    append_str_to_str(primal_code,  "GETCHAR LF@onechr LF@s LF@strindex\n");
    append_str_to_str(primal_code,  "CONCAT LF@%returnval LF@%returnval LF@onechr\n");
    append_str_to_str(primal_code, "LT LF@supbool LF@strindex LF@n\n");
    append_str_to_str(primal_code,  "ADD LF@strindex int@1\n"
                                    "JUMPIFEQ Scycle LF@supbool bool@true\n");
    append_str_to_str(primal_code,  "RETURN\n"
                                    "\n");

	//asc(s as string, i as integer) as integer
	record = create_func_record(symtable, "asc");
	record->data.type=INTEGER_TYPE;

	record->data.u_argconst.first_par = malloc(sizeof(struct func_par));
	record->data.u_argconst.first_par->par_name = "s";
	record->data.u_argconst.first_par->par_type=STRING_TYPE;

	record->data.u_argconst.first_par->par_next=malloc(sizeof(struct func_par));
	record->data.u_argconst.first_par->par_next->par_name="i";
	record->data.u_argconst.first_par->par_next->par_type=INTEGER_TYPE;
	set_func_par_count(record, 2);

	append_str_to_str(primal_code,  "LABEL asc\n"
									"DEFVAR LF@%returnval\n"
									"STRLEN LF@%returnval LF@s\n");
    append_str_to_str(primal_code,  "PUSHS LF@i\n"
									"PUSHS int@0\n"
									"LTS\n");
    append_str_to_str(primal_code,  "PUSHS bool@true\n"
									"JUMPIFEQS ATrue\n"
									"PUSHS LF@i\n");
    append_str_to_str(primal_code,  "PUSHS LF@%returnval\n"
									"GTS\n"
									"PUSHS bool@true\n");
    append_str_to_str(primal_code,  "JUMPIFEQS ATrue\n"
									"STRI2INT LF@%returnval LF@s LF@i\n"
									"RETURN\n");
    append_str_to_str(primal_code,  "LABEL A_True\n"
									"MOVE LF@%returnval int@0\n"
									"RETURN\n"
                                    "\n");


	//chr(i as integer) as string
	record = create_func_record(symtable, "chr");
	record->data.type=STRING_TYPE;

	record->data.u_argconst.first_par = malloc(sizeof(struct func_par));
	record->data.u_argconst.first_par->par_name = "i";
	record->data.u_argconst.first_par->par_type = INTEGER_TYPE;
	set_func_par_count(record, 1);

	append_str_to_str(primal_code,  "LABEL chr\n"
                                    "DEFVAR LF@%returnval\n"
                                    "PUSHS LF@i\n");
    append_str_to_str(primal_code,  "PUSHS int@0\n"
                                    "LTS\n"
                                    "PUSHS bool@true\n");
    append_str_to_str(primal_code,  "JUMPIFEQS CTrue\n"
                                    "PUSHS LF@i\n"
                                    "PUSHS int@255\n");
    append_str_to_str(primal_code,  "GTS\n"
                                    "PUSHS bool@true\n"
                                    "JUMPIFEQS CTrue\n");
    append_str_to_str(primal_code,  "INT2CHAR LF@%returnval LF@i\n"
                                    "RETURN\n"
                                    "LABEL CTrue\n");
    append_str_to_str(primal_code,  "MOVE LF@%returnval bool@false \n"
                                    "RETURN\n"
                                    "\n");
}

struct htab_listitem * create_func_record(htab_t * symtable, char * name)
{
	struct htab_listitem * new_record = make_item(name);
	set_id_declared(new_record);
	set_id_function(new_record);
	htab_append(new_record, symtable);
	set_func_par_count(new_record, 0);
	return new_record;
}

void copy_scope_layer(struct htab_listitem * item, htab_t * other_symtable, String * primal_code)
{
	if (!id_is_function(item))
	{
		append_str_to_str(primal_code, "DEFVAR TF@");
		append_str_to_str(primal_code, item->key);
		append_str_to_str(primal_code, "\nMOVE TF@");
		append_str_to_str(primal_code, item->key);
		append_str_to_str(primal_code, " LF@");
		append_str_to_str(primal_code, item->key);
		append_char_to_str(primal_code, '\n');
	}
}

void copy_general_layer(struct htab_listitem * item, htab_t * other_symtable, String * primal_code)
{
	if (id_is_function(item))
	{
		htab_append(item, other_symtable);
	}
}

void generate_implicit_value(String * primal_code, char * name, enum_type type)
{
	append_str_to_str(primal_code, "MOVE LF@");
	append_str_to_str(primal_code, name);
	switch (type)
	{
		case INTEGER_TYPE:
		append_str_to_str(primal_code, " int@0\n");
		break;

		case DOUBLE_TYPE:
		append_str_to_str(primal_code, " float@0.0\n");
		break;

		case STRING_TYPE:
		append_str_to_str(primal_code, " string@\n");
		break;

		case BOOLEAN_TYPE:
		append_str_to_str(primal_code, " bool@false\n");
		break;
	}
}