#include "parser.h"

#define DEBUG_MSG 42

/* Starts translation of code to assembler code */
void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: started translation\n");
	#endif

	init_string(primal_code);
	append_str_to_str(primal_code, ".IFJcode17\n");		//Header
	append_str_to_str(primal_code, "JUMP %MAIN\n");		//jump to scope
	
	//add_build_in_functions(symtable, primal_code);

	neterm_start(token_buff, symtable, primal_code);
}

/* Nonterminal function describes nonterminal START rules. Generates relevant instructions */
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
				error_msg(ERR_CODE_UNDEFINED, "Scope block was already defined\n");
			scope_found = 1;
			append_str_to_str(primal_code, "LABEL %MAIN\n");
			append_str_to_str(primal_code, "DEFVAR GF@%SWAP\n");
			append_str_to_str(primal_code, "DEFVAR GF@%SWAP2\n");
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
				error_msg(ERR_CODE_SYNTAX, "Scope block wasnt found\n");
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type,
                                    3, SCOPE, DECLARE, FUNCTION);
			break;
		}
	}
}

/* Nonterminal function describes nonterminal SCOPE rules. */
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

/* Nonterminal function describes nonterminal FUNCTION_DECLARATION rules. */
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
					error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' was declared before as variable\n", found_record->key);
				if (!id_is_declared(found_record))
					error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' was declared before.\n", found_record->key);
			}
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}
	expected_token(token_buff, LEFT_PARANTHESIS);
	if (found_record == NULL)
	{
		found_record = create_func_record(symtable, actual_token->attr.string_value);
	}
	while (!is_peek_token(token_buff, RIGHT_PARANTHESIS))
	{
		neterm_args_create(token_buff, symtable, primal_code, found_record);
	}
	expected_token(token_buff, RIGHT_PARANTHESIS);
	expected_token(token_buff, AS);
	found_record->data.type = neterm_type(token_buff, symtable, primal_code);
}

/* Nonterminal function describes nonterminal FUNCTION_DEFINITION rules. Generates relevant instructions */
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
					error_msg(ERR_CODE_UNDEFINED, "redefinition of function %s.\n", found->key);
				else
					set_id_defined(found);

				expected_token(token_buff, LEFT_PARANTHESIS);

				while (!is_peek_token(token_buff, RIGHT_PARANTHESIS))
				{
					neterm_args(token_buff, symtable, primal_code, found);
				}
				expected_token(token_buff, RIGHT_PARANTHESIS);
				expected_token(token_buff, AS);
				if (found->data.type != neterm_type(token_buff, symtable, primal_code))
					error_msg(ERR_CODE_TYPE, "return type in function %s do not match declaration.\n", found->key);
			}
			else
			{
				//implicit declaration

				expected_token(token_buff, LEFT_PARANTHESIS);

				found = create_func_record(symtable, actual_token->attr.string_value);
				set_id_defined(found);
				while (!is_peek_token(token_buff, RIGHT_PARANTHESIS))
				{
					neterm_args_create(token_buff, symtable, primal_code, found);
				}
				expected_token(token_buff, RIGHT_PARANTHESIS);
				expected_token(token_buff, AS);
				found->data.type = neterm_type(token_buff, symtable, primal_code);

				//error_msg(ERR_CODE_UNDEFINED, "function must be declared before definition.\n");
			}
			break;
		default :
			syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}

	expected_token(token_buff, NEW_LINE);
	append_str_to_str(primal_code, "LABEL ");
	append_str_to_str(primal_code, found->key);
	append_char_to_str(primal_code, '\n');
	append_str_to_str(primal_code, "DEFVAR LF@%returnval\n");
	struct htab_t * new_symtable = htab_init(symtable->arr_size);

	#ifdef DEBUG_MSG
	fprintf(stderr, "log: adding globals\n");
	#endif

	copy_parameters(found->data.first_par, new_symtable);		//add parameters
	htab_foreach(symtable, new_symtable, primal_code, (*copy_general_layer)); //add globals

	while (!is_peek_token(token_buff, END))
	{
		neterm_body_func(token_buff, new_symtable, primal_code, found);
	}
	htab_free(new_symtable);
	expected_token(token_buff, END);
	expected_token(token_buff, FUNCTION);
	expected_token(token_buff, NEW_LINE);

	//implicit return value, in 99 percent of time it is a dead code
	generate_implicit_value(primal_code, "%returnval", found->data.type);
	append_str_to_str(primal_code, "PUSHS LF@%returnval\n");
	append_str_to_str(primal_code, "RETURN\n");
}

/* Nonterminal function describes nonterminal TYPE rules. */
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

/* Nonterminal function describes nonterminal ARGS rules. Checks arguments of function */
void neterm_args(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm args\n");
	#endif

	struct func_par * actual_param = func_record->data.first_par;
	while (42)
	{
		token * actual_token = token_buffer_get_token(token_buff);
		switch (actual_token->type){
			case IDENTIFIER :
				if (actual_param!=NULL)
				{
					if (strcmp(actual_token->attr.string_value, actual_param->par_name))
					{
						error_msg(ERR_CODE_UNDEFINED, "param (%s) from function %s was declared before as %s.\n",
                                  actual_token->attr.string_value, func_record->key,
                                  actual_param->par_name);
					}
				}
				else
				{
					error_msg(ERR_CODE_UNDEFINED, "function %s do not take so much parameters.\n", func_record->key);
				}
				break;
			default :
				syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
				break;
		}
		expected_token(token_buff, AS);
		if (actual_param->par_type != neterm_type(token_buff, symtable, primal_code))
			error_msg(ERR_CODE_TYPE, "type of parameter %s in function %s do not match type in declaration.\n",
                      actual_param->par_name, func_record->key);

		actual_token = token_buffer_peek_token(token_buff);
		if (actual_token->type == COMA)
		{
			expected_token(token_buff, COMA);
			actual_param=actual_param->par_next;
			continue;
		}
		else if (actual_token->type == RIGHT_PARANTHESIS)
		{
			if (actual_param->par_next != NULL)
				error_msg(ERR_CODE_TYPE, "function %s expect more parameters.\n",
                          func_record->key);
			break;
		}
		else
			syntax_error_unexpexted(actual_token->line, actual_token->pos,
                                    actual_token->type, 2, COMA, RIGHT_PARANTHESIS);
	}
}

/* Nonterminal function describes nonterminal ARGS_CREATE rules. Creates relevant arguments for function */
void neterm_args_create(token_buffer * token_buff, htab_t * symtable, String * primal_code,
                        struct htab_listitem * new_func_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm args create\n");
	#endif

	struct func_par ** actual_param = &(new_func_record->data.first_par);

	while (42)
	{
		token * actual_token = token_buffer_get_token(token_buff);
		switch (actual_token->type){
			case IDENTIFIER :
				if (!unique_parameter(new_func_record->data.first_par, actual_token->attr.string_value))
					error_msg(ERR_CODE_UNDEFINED, "multiple parameters in function %s are called %s.\n",
							new_func_record->key, actual_token->attr.string_value);

				*actual_param=malloc(sizeof(struct func_par));
				(*actual_param)->par_name=actual_token->attr.string_value;
				(*actual_param)->par_next=NULL; //Added parameter is the last one;
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


/* Nonterminal function describes nonterminal BODY rules. Generates relevant instructions */
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
			error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' is not declared\n",
                      actual_token->attr.string_value);	//Does not exist.. Too bad
	
			if (id_is_function(found_record))	//is it function or variable name? ..can not be both, sorry.
				function_call(token_buff, symtable, primal_code, found_record);
			else
				body_assignment(token_buff, symtable, primal_code, found_record);
			break;
		case SCOPE :
			expected_token(token_buff, NEW_LINE);
			new_symtable = htab_move(symtable->arr_size, symtable);
			append_str_to_str(primal_code, "CREATEFRAME\n");
			htab_foreach(new_symtable, new_symtable, primal_code, (*copy_scope_layer));
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

void neterm_body_func(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: neterm func_body\n");
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
			body_func_if_then(token_buff, symtable, primal_code, func_record);
			break;
		case DO :
			body_func_do_while(token_buff, symtable, primal_code, func_record);
			break;
		case IDENTIFIER :
			found_record = htab_find(symtable, actual_token->attr.string_value);	//search in symtable for identifier
			if (found_record == NULL)
			error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' is not declared\n",
                      actual_token->attr.string_value);	//Does not exist.. Too bad
	
			if (id_is_function(found_record))	//is it function or variable name? ..can not be both, sorry.
				function_call(token_buff, symtable, primal_code, found_record);
			else
				body_assignment(token_buff, symtable, primal_code, found_record);
			break;
		case RETURN :
			body_return(token_buff, symtable, primal_code, func_record);
			break;
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

/* Nonterminal function describes nonterminal BODY_DECLARATION rules. Generates relevant instructions */
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
					error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' is function.\n", found_record->key);
				if (id_is_declared(found_record))
					error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' was declared before.\n", found_record->key);
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
	if (!id_is_shadow(found_record))
	{
		append_str_to_str(primal_code, "DEFVAR LF@");
		append_str_to_str(primal_code, found_record->key);
		append_char_to_str(primal_code, '\n');
	}

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
		int expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
		parse_semantic_expression(primal_code, found_record, variable_type, expr_return_type);
		set_id_defined(found_record);
		expected_token(token_buff, NEW_LINE);
	}
	else {
		syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 2, NEW_LINE, EQUALS);
	}
}

/* Nonterminal function describes nonterminal BODY_INPUT rules. Generates relevant instructions */
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
				error_msg(ERR_CODE_UNDEFINED, "identifier %s was not declared in this scope as variable.", actual_token->attr.string_value);

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

/* Nonterminal function describes nonterminal BODY_IF_THEN rules. Generates relevant instructions */
void body_if_then(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body if then\n");
	#endif

	bool else_branch=0;

	int variable_type = BOOLEAN_TYPE;
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, THEN);
	parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);
	expected_token(token_buff, THEN);


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

void body_func_if_then(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body if then\n");
	#endif

	bool else_branch=0;

	int variable_type = BOOLEAN_TYPE;
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, THEN);
	parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);
	expected_token(token_buff, THEN);


	append_str_to_str(primal_code, "PUSHS bool@true\n");
	append_str_to_str(primal_code, "JUMPIFNEQS ");
	unsigned int order =generate_if_label_order();
	generate_if_label(primal_code, label_else, order);
	token * next_token = token_buffer_peek_token(token_buff);
	while (next_token->type != ELSE && next_token->type != END)
	{
		neterm_body_func(token_buff, symtable, primal_code, func_record);
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
                neterm_body_func(token_buff, symtable, primal_code, func_record);
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

/* Nonterminal function describes nonterminal BODY_DO_WHILE rules. Generates relevant instructions */
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
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
	parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);
	expected_token(token_buff, NEW_LINE);

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

void body_func_do_while(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body do while\n");
	#endif

	expected_token(token_buff, WHILE);

	unsigned int order = generate_if_label_order();
	append_str_to_str(primal_code, "LABEL ");
	generate_if_label(primal_code, label_if, order);

	int variable_type = BOOLEAN_TYPE;
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
	parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);
	expected_token(token_buff, NEW_LINE);

	append_str_to_str(primal_code, "PUSHS bool@true\n");
	append_str_to_str(primal_code, "JUMPIFNEQS ");
	generate_if_label(primal_code, label_else, order);

	while (!is_peek_token(token_buff, LOOP))
	{
		neterm_body_func(token_buff, symtable, primal_code, func_record);
	}

	append_str_to_str(primal_code, "JUMP ");
	generate_if_label(primal_code, label_if, order);
	append_str_to_str(primal_code, "LABEL ");
	generate_if_label(primal_code, label_else, order);

	expected_token(token_buff, LOOP);
	expected_token(token_buff, NEW_LINE);
}

/* Nonterminal function describes nonterminal BODY_ASSIGNMENT rules. Generates relevant instructions */
void body_assignment(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * found_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body assignment\n");
	#endif

	expected_token(token_buff, EQUALS);


	token * next_token = token_buffer_peek_token(token_buff);
	if (next_token->type == IDENTIFIER)
	{
		struct htab_listitem * record = htab_find(symtable, next_token->attr.string_value);
		if (record != NULL)
		{

			if (id_is_function(record))
			{
				expected_token(token_buff, IDENTIFIER);
				function_call(token_buff, symtable, primal_code, record);
				parse_semantic_expression(primal_code, found_record, get_id_type(found_record), get_id_type(record));
				return;
			}

		}
	}

	int variable_type = get_id_type(found_record);
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
	parse_semantic_expression(primal_code, found_record, variable_type, expr_return_type);
	set_id_defined(found_record);
	expected_token(token_buff, NEW_LINE);
}

/* Nonterminal function describes nonterminal BODY_RETURN rules. Generates relevant instructions */
void body_return(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * func_record)
{
	//only callable from funcbody
	int variable_type = get_id_type(func_record);
	int expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
	parse_semantic_expression_modified(primal_code, "%returnval", variable_type, expr_return_type);

	append_str_to_str(primal_code, "PUSHS LF@%returnval\n");
	append_str_to_str(primal_code, "RETURN\n");
}

/* Generates relevant instrunctions for expression*/
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
		error_msg(ERR_CODE_TYPE, "The expression value does not match the variable type\n");
	}
}

/*Erik

second parameter as (char *) would be better
in return you dont have htab_listitem
only  name constant name "%returnvar"

switch please..

*/

void parse_semantic_expression_modified(String * primal_code, char * name, int variable_type, int expr_return_type) {

	if (variable_type == DOUBLE_TYPE && expr_return_type == INTEGER_TYPE) {
		append_str_to_str(primal_code, "INT2FLOATS\nPOPS LF@");
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == DOUBLE_TYPE && expr_return_type == DOUBLE_TYPE){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == INTEGER_TYPE && expr_return_type == DOUBLE_TYPE){
		append_str_to_str(primal_code, "FLOAT2INTS\nPOPS LF@");
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == INTEGER_TYPE && expr_return_type == INTEGER_TYPE){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}
	
	else if (variable_type == BOOLEAN_TYPE && expr_return_type == BOOLEAN_TYPE){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}

	else if (variable_type == STRING_TYPE && expr_return_type == STRING_TYPE){
		append_str_to_str(primal_code, "POPS LF@");
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}

	else {
		error_msg(ERR_CODE_TYPE, "The expression value does not match the variable type\n");
	}
}

/* Nonterminal function describes nonterminal FUNCTION_CALL rules. Generates relevant instructions */
void function_call(token_buffer * token_buff, htab_t * symtable, String * primal_code, struct htab_listitem * found_record)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: function call\n");
	#endif

	append_str_to_str(primal_code, "CREATEFRAME\n");	//prepare Temporary frame

	struct func_par * actual_param = found_record->data.first_par;
	struct htab_listitem * param_caller = NULL;
	expected_token(token_buff, LEFT_PARANTHESIS);
	while ((token_buffer_peek_token(token_buff)->type)!=RIGHT_PARANTHESIS)
	{
		token * actual_token = token_buffer_get_token(token_buff);
		switch (actual_token->type){
			case IDENTIFIER :
				param_caller = htab_find(symtable, actual_token->attr.string_value);
				if (param_caller == NULL)
					error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' does not exist.\n",
                              actual_token->attr.string_value);
				if (id_is_function(param_caller))
					error_msg(ERR_CODE_UNDEFINED, "IDENTIFIER '%s' is function, which can not be used as parameter.\n",
                              actual_token->attr.string_value);

				if (actual_param!=NULL)
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
								error_msg(ERR_CODE_TYPE, "parameter in function %s expect integer or double.\n",
                                          found_record->key);
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
                                          "parameter in function %s expect integer or double value.\n",
                                          found_record->key);
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
								error_msg(ERR_CODE_TYPE, "parameter in function %s expect string value.\n",
                                          found_record->key);
							
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
								error_msg(ERR_CODE_TYPE, "parameter in function %s expect boolean value.\n",
                                          found_record->key);
							
							break;
						default:
							error_msg(ERR_CODE_TYPE, "parameter in function calling %s have unexpected value.\n",
                                      found_record->key);
						break;
					}
				}
				else
				{
					error_msg(ERR_CODE_TYPE, "function %s do not takes so much parameters.\n",
                              found_record->key);
				}
				break;
			default :
				syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
				break;
		}
		actual_param=actual_param->par_next;
		if (actual_param!=NULL)
			expected_token(token_buff, COMA);
	}
	if (actual_param != NULL)
		error_msg(ERR_CODE_TYPE, "function %s expect more parameters.\n",
                  found_record->key);

	expected_token(token_buff, RIGHT_PARANTHESIS);
	append_str_to_str(primal_code, "PUSHFRAME\n");
	append_str_to_str(primal_code, "CALL ");
	append_str_to_str(primal_code, found_record->key);
	append_char_to_str(primal_code, '\n');
	append_str_to_str(primal_code, "POPFRAME\n");
}

/* Nonterminal function describes nonterminal BODY_PRINT rules. Generates relevant instructions */
void body_print(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	#ifdef DEBUG_MSG
	fprintf(stderr, "log: body print\n");
	#endif

	parse_expression(token_buff, symtable, primal_code, SEMICOLON);
	append_str_to_str(primal_code, "POPS GF@%SWAP\n");
	append_str_to_str(primal_code, "WRITE GF@%SWAP\n");

	if (is_peek_token(token_buff, SEMICOLON)){
		expected_token(token_buff, SEMICOLON);
		if (!is_peek_token(token_buff, NEW_LINE))
			body_print(token_buff, symtable, primal_code);
	}
	else {
		token *actual_token = token_buffer_peek_token(token_buff);
		syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 2, NEW_LINE, SEMICOLON);
	}
}

/* Checks if next token is the one we expected. If it is different exits program with relevant error code */
void expected_token(token_buffer * token_buff, int tok_type)
{
	token * actual_token = token_buffer_get_token(token_buff);
	if (actual_token->type != tok_type)
		syntax_error_unexpexted(actual_token->line, actual_token->pos ,actual_token->type, 1, tok_type);
}
/* Checks if next token is the one we expected. Returns true if it is truth*/
bool is_peek_token(token_buffer * token_buff, int tok_type)
{
	token * next_token = token_buffer_peek_token(token_buff);
	if (next_token->type == tok_type)
		return 1;
	else
		return 0;
}

/* Nonterminal function describes nonterminal EXPRESSION rules. Generates relevant instructions */
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

/* Generates order of if label */
unsigned int generate_if_label_order()
{
	static unsigned int counter = 0;
	return counter++;
}

/* Generates relevant instructions of if label */
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

/* Generates relevant instructions for jumps */
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

/* Generates relevant instructions for parameters */
/*void generate_prepare_params(String * primal_code, struct func_par * actual_param, unsigned int param_order,
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
}*/

/* Adds built-in functions into code with its instructions */
/*void add_build_in_functions(htab_t * symtable, String * primal_code)
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
    append_str_to_str(primal_code,  "JUMPIFNEQ Snzero LF@supbool boo@true\n");
    append_str_to_str(primal_code,  "LABEL SubstrEnd\n"
                                    "MOVE LF@%returnval string@d \n"
                                    "RETURN\n");
    append_str_to_str(primal_code,  "LABEL Snzero\n"
                                    "EQ LF@supbool LF@i int@0\n");
    append_str_to_str(primal_code,  "JUMPIFEQ SubstrEnd LF@supbool boo@true\n");
    append_str_to_str(primal_code,  "LT LF@supbool LF@i int@0\n"
                                    "JUMPIFEQ SubstrEnd LF@supbool boo@true\n");
    append_str_to_str(primal_code,  "LT LF@supbool LF@n int@0\n");
    append_str_to_str(primal_code,  "JUMPIFEQ Sublen LF@supbool boo@true\n"
                                    "SUB LF@strindex LF@slen LF@i\n");
    append_str_to_str(primal_code,  "GT LF@supbool LF@n LF@strindex\n");
    append_str_to_str(primal_code,  "JUMPIFEQ Sublen LF@supbool boo@true\n"
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
                                    "JUMPIFEQ Scycle LF@supbool boo@true\n");
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
    append_str_to_str(primal_code,  "MOVE LF@%returnval string@0 \n"
                                    "RETURN\n"
                                    "\n");
}

/* Creates new function record */
struct htab_listitem * create_func_record(htab_t * symtable, char * name)
{
	struct htab_listitem * new_record = make_item(name);
	set_id_declared(new_record);
	set_id_function(new_record);
	htab_append(new_record, symtable);
	new_record->data.first_par=NULL;
	return new_record;
}

/* Generates instruction code for variable */
void copy_scope_layer(struct htab_listitem * item, htab_t * other_symtable, String * primal_code)
{
	if (!id_is_function(item))
	{
		set_id_shadow(item);	//set shadow and unsed declared
		append_str_to_str(primal_code, "DEFVAR TF@");
		append_str_to_str(primal_code, item->key);
		append_str_to_str(primal_code, "\nMOVE TF@");
		append_str_to_str(primal_code, item->key);
		append_str_to_str(primal_code, " LF@");
		append_str_to_str(primal_code, item->key);
		append_char_to_str(primal_code, '\n');
	}
}

/* Appends global variables to new_symtable */
void copy_general_layer(struct htab_listitem * item, htab_t * other_symtable, String * primal_code)
{
	if (id_is_function(item))
	{
		struct htab_listitem * copy_record = create_func_record(other_symtable, item->key);
		copy_record->data.first_par = item->data.first_par;
		copy_record->data.type = item->data.type;
	}
}

/* Generates code of hard value */
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

bool unique_parameter(struct func_par * first_par, char * str)
{
	struct func_par * loop_param = first_par;
	while (loop_param != NULL)
	{
		if (!strcmp(str, loop_param->par_name))
		{
			return 0;
		}
		loop_param = loop_param->par_next;
	}
	return true;
}

void copy_parameters(struct func_par * first_par, htab_t * symtable)
{
	struct func_par * loop_param = first_par;
	while (loop_param != NULL)
	{
		struct htab_listitem * new_param = make_item(loop_param->par_name);
		new_param->data.type=loop_param->par_type;
		htab_append(new_param , symtable);
		loop_param = loop_param->par_next;
	}
}