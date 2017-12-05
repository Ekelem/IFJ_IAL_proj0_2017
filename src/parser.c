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

#include "parser.h"

static unsigned inside_if = 0; 					// to check if we are in if statement
static unsigned inside_while = 0;				// to check if we are in While loop
static bool inside_fun = false;					// to check if we are in function
static bool inside_scopeblock = false;			// to check if first parameter was made
static bool first_par_made = false; 			// to block if first parameter was made
static bool first_par = true;					// to check if paramter is first
static struct fun_par ** actual_param;			// to change pars in record
static struct htab_listitem *fun_record_actual;	// to compare variable with parameters
static struct htab_t *global_symtable;			// to check names of declared and defined functions with newly defined variable
static bool first_par_taken = false;			// to check if first par was taken
static bool fun_call_param = false;				// to block if first par was taken
static unsigned global_par_count = 0;			// to count par count
static struct fun_par *global_actual_param;		// actual param


/*	Starts the translation of IFJ2017 to assembler generating instructions */
void translate(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	init_string(primal_code);
	append_str_to_str(primal_code, ".IFJcode17\n");
	append_str_to_str(primal_code, "JUMP %MAIN\n");
	add_build_in_functions(symtable, primal_code);
	neterm_start(token_buff, symtable, primal_code);
}

/* First nonterminal function. Calls other neterminals to check syntax*/
void neterm_start(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token * actual_token = token_buffer_peek_token(token_buff);
	funblock_rule_continue(token_buff, symtable, actual_token->type,primal_code);
	check_new_lines_after_scopeblock(token_buff);
	expected_token(token_buff, EOF);
}

/* Nonterminal function scopeblock*/
void neterm_scopeblock(token_buffer * token_buff, htab_t * symtable, String *primal_code)
{
	struct htab_t * new_symtable;

	check_all_functions_definitions(symtable);
	token * actual_token = token_buffer_get_token(token_buff);

	switch(actual_token->type){

		case SCOPE:
			global_symtable = symtable;
			new_symtable = htab_move(symtable->arr_size, symtable);

			inside_scopeblock = true;
			expected_token(token_buff, NEW_LINE);
			neterm_statementblock(token_buff, new_symtable,primal_code);
			expected_token(token_buff, END);
			expected_token(token_buff, SCOPE);
			htab_free(new_symtable);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, SCOPE);
			break;
	}
}

/* Nonterminal function funblock*/
void neterm_funblock(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{

	struct htab_listitem *found;
	token * actual_token = token_buffer_get_token(token_buff);

	switch(actual_token->type){

		case DECLARE:
			expected_token(token_buff, FUNCTION);
			actual_token = token_buffer_get_token(token_buff);
			if(actual_token->type != IDENTIFIER)
			{
				syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			}

			found = htab_find(symtable, actual_token->attr.string_value);
			if(found)
			{
				if(!is_function(found))
				{
					error_msg(ERR_CODE_SEM, "FUNCTION '%s' was declared before as variable\n", found->key);
				}

				if(is_declared(found))
				{
					error_msg(ERR_CODE_SEM, "FUNCTION '%s' was declared before\n", found->key);
				}
			}

			expected_token(token_buff, LEFT_PARANTHESIS);
			found = create_fun_record(symtable, actual_token->attr.string_value);
			neterm_fundecparams(token_buff, symtable,found,primal_code);

			check_fun_definiton_params(token_buff, found, found->data.par_count);

			expected_token(token_buff, RIGHT_PARANTHESIS);
			expected_token(token_buff, AS);
			found->data.type = neterm_type(token_buff, symtable,primal_code);
			expected_token(token_buff, NEW_LINE);

			fun_record_actual = found;
			global_symtable = symtable;

			actual_token = token_buffer_peek_token(token_buff);
			funblock_rule_continue(token_buff, symtable, actual_token->type,primal_code);
			break;

		case FUNCTION:
			inside_fun = true;
			actual_token = token_buffer_get_token(token_buff);

			if(actual_token->type != IDENTIFIER)
			{
				syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			}

			found = htab_find(symtable, actual_token->attr.string_value);

			if(found)
			{
				if (is_defined(found))
				{
					error_msg(ERR_CODE_SEM, "Redefinition of function %s.\n", found->key);
				}

				set_id_defined(found);
			}

			expected_token(token_buff, LEFT_PARANTHESIS);

			struct htab_t *new_symtable = htab_init(symtable->arr_size);
			if(!found)
			{
				found = create_fun_record(symtable, actual_token->attr.string_value);
				set_id_defined(found);
				neterm_fundecparams(token_buff, new_symtable,found,primal_code);

				expected_token(token_buff, RIGHT_PARANTHESIS);
				expected_token(token_buff, AS);
				found->data.type = neterm_type(token_buff, symtable,primal_code);
			}
			else
			{
				check_fun_declaration_params(token_buff, symtable, new_symtable, found, primal_code);
				expected_token(token_buff, RIGHT_PARANTHESIS);
				expected_token(token_buff, AS);

				if(found->data.type != neterm_type(token_buff, symtable,primal_code))
				{
					error_msg(ERR_CODE_SEM, "Return type in function '%s' doesnt match its declaration.\n", found->key);
				}
			}

			fun_record_actual = found;
			global_symtable = symtable;
			expected_token(token_buff, NEW_LINE);
			append_str_to_str(primal_code, "LABEL ");
			append_str_to_str(primal_code, found->key);
			append_char_to_str(primal_code, '\n');
			append_str_to_str(primal_code, "DEFVAR LF@%returnval\n");

			neterm_statementblock(token_buff, new_symtable,primal_code);
			htab_free(new_symtable);
			expected_token(token_buff, END);
			expected_token(token_buff, FUNCTION);
			expected_token(token_buff, NEW_LINE);

			generate_implicit_value(primal_code, "%returnval", found->data.type);
			append_str_to_str(primal_code, "PUSHS LF@%returnval\n");
			append_str_to_str(primal_code, "RETURN\n");

			inside_fun = false;
			actual_token = token_buffer_peek_token(token_buff);
			funblock_rule_continue(token_buff, symtable, actual_token->type,primal_code);
			break;

		case NEW_LINE:
			actual_token = token_buffer_peek_token(token_buff);
			funblock_rule_continue(token_buff, symtable, actual_token->type,primal_code);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			break;
	}
}


/* Nonterminal function fundecparams*/
void neterm_fundecparams(token_buffer * token_buff, htab_t * symtable, htab_listitem *new_fun_record, String * primal_code)
{
	if(!first_par_made)
	{
		actual_param = &(new_fun_record->data.first_par);
		first_par_made = true;
	}

	token * actual_token = token_buffer_peek_token(token_buff);

	switch(actual_token->type){

		case IDENTIFIER:
			actual_token = token_buffer_get_token(token_buff);
			fundec_check_param_name(new_fun_record, global_symtable, actual_token->attr.string_value, first_par);
			add_func_par_count(new_fun_record);
			*actual_param = malloc(sizeof(struct fun_par));


			if(*actual_param == NULL)
			{
				error_msg(ERR_CODE_INTERN, "Param malloc error\n");
			}

			(*actual_param)->par_name = actual_token->attr.string_value;
			first_par = false;
			expected_token(token_buff, AS);
			(*actual_param)->par_type = neterm_type(token_buff, symtable,primal_code);

			if(inside_fun)
			{
				create_param(symtable, actual_token->attr.string_value, (*actual_param)->par_type);
			}

			if((token_buffer_get_token(token_buff))->type == COMA)
			{
				actual_token = token_buffer_peek_token(token_buff);
				if(actual_token->type != IDENTIFIER)
				{
					syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
				}
				actual_param = &((*actual_param)->par_next);

			}

			token_buffer_unget_token(token_buff);
			neterm_fundecparamsnext(token_buff, symtable, new_fun_record,primal_code);
			break;

		case RIGHT_PARANTHESIS:
			new_fun_record->data.first_par = NULL;
			first_par_made = false;
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 2, IDENTIFIER, RIGHT_PARANTHESIS);
			break;
	}
}

/* Nonterminal function fundecparamsnext*/
void neterm_fundecparamsnext(token_buffer * token_buff, htab_t * symtable, htab_listitem *fun_record, String * primal_code)
{

	token * actual_token = token_buffer_peek_token(token_buff);

	switch(actual_token->type){
		case COMA:
			token_buffer_get_token(token_buff);
			neterm_fundecparams(token_buff, symtable, fun_record,primal_code);
			break;

		case RIGHT_PARANTHESIS:
			(*actual_param)->par_next = NULL;
			first_par_made = false;
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 2, COMA, RIGHT_PARANTHESIS);
			break;
	}
}

/* Nonterminal function type*/
int neterm_type(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{

	token *actual_token = token_buffer_get_token(token_buff);

	switch (actual_token->type){
		case INTEGER:
			return INTEGER_TYPE;

		case DOUBLE:
			return DOUBLE_TYPE;

		case STRING:
			return STRING_TYPE;

		case BOOLEAN:
			return BOOLEAN_TYPE;
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 3, INTEGER, DOUBLE, STRING, BOOLEAN);
			return VOID_TYPE;
			break;
	}
}


static bool else_statement = false;

/* Nonterminal function statementblock*/
void neterm_statementblock(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{

	int variable_type;
	int expr_return_type;
	unsigned int order;

	token *actual_token = token_buffer_peek_token(token_buff);
	switch(actual_token->type){
		case IDENTIFIER:
			neterm_funcallORasign(token_buff, symtable,primal_code);
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		case DIM:
			if(inside_if || inside_while)
			{
				error_msg(ERR_CODE_SYNTAX, "Variable was declared in IF statement or While loop\n");
			}
			neterm_decORasign(token_buff, symtable,primal_code);
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		case IF:
			token_buffer_get_token(token_buff);
			inside_if++;

			variable_type = BOOLEAN_TYPE;
			expr_return_type = parse_expression(token_buff, symtable, primal_code, THEN);
			parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);

			append_str_to_str(primal_code, "PUSHS bool@true\n");
			append_str_to_str(primal_code, "JUMPIFNEQS ");
			order = generate_if_label_order();
			generate_if_label(primal_code, label_else, order);

			expected_token(token_buff, THEN);
			expected_token(token_buff, NEW_LINE);
			neterm_statementblock(token_buff, symtable,primal_code);
			neterm_elsestatement(token_buff, symtable,primal_code, order);

			if(!else_statement)
			{
				append_str_to_str(primal_code, "LABEL ");
            	generate_if_label(primal_code, label_end_if, order);
			}

			expected_token(token_buff, END);
			expected_token(token_buff, IF);
			expected_token(token_buff, NEW_LINE);
			inside_if--;
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		case PRINT:
			token_buffer_get_token(token_buff);

			parse_expression(token_buff, symtable, primal_code, SEMICOLON);
			append_str_to_str(primal_code, "POPS GF@%SWAP\n");
			append_str_to_str(primal_code, "WRITE GF@%SWAP\n");

			expected_token(token_buff, SEMICOLON);
			neterm_exprnext(token_buff, symtable,primal_code);
			expected_token(token_buff, NEW_LINE);
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		case INPUT:
			token_buffer_get_token(token_buff);
			actual_token = token_buffer_get_token(token_buff);
			if(actual_token->type != IDENTIFIER)
			{
				syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			}

			htab_listitem *found_var = htab_find(symtable, actual_token->attr.string_value);
			if(!found_var)
			{
				error_msg(ERR_CODE_SEM, "Variable '%s' wasnt defined.",actual_token->attr.string_value);
			}
			else if(is_function(found_var))
			{
				error_msg(ERR_CODE_SEM, "Cant assign value from input to function '%s'",actual_token->attr.string_value);
			}

			append_str_to_str(primal_code, "WRITE string@?\\032");
			append_str_to_str(primal_code, "\nREAD LF@");
			append_str_to_str(primal_code, actual_token->attr.string_value);

			switch (get_id_type(found_var))
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

			expected_token(token_buff, NEW_LINE);
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		case DO:
			token_buffer_get_token(token_buff);
			inside_while++;
			expected_token(token_buff, WHILE);

			order = generate_if_label_order();
			append_str_to_str(primal_code, "LABEL ");
			generate_if_label(primal_code, label_if, order);

			variable_type = BOOLEAN_TYPE;
			expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
			parse_semantic_expression(primal_code, NULL, variable_type, expr_return_type);

			append_str_to_str(primal_code, "PUSHS bool@true\n");
			append_str_to_str(primal_code, "JUMPIFNEQS ");
			generate_if_label(primal_code, label_else, order);

			expected_token(token_buff, NEW_LINE);
			neterm_statementblock(token_buff, symtable,primal_code);

			append_str_to_str(primal_code, "JUMP ");
			generate_if_label(primal_code, label_if, order);
			append_str_to_str(primal_code, "LABEL ");
			generate_if_label(primal_code, label_else, order);

			expected_token(token_buff, LOOP);
			inside_while--;
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		case RETURN:
			if(!inside_fun)
			{
				error_msg(ERR_CODE_SYNTAX, "Return was called in Scope block - SYNTAX ERROR\n");
			}
			token_buffer_get_token(token_buff);

			variable_type = get_id_type(fun_record_actual);
			expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
			parse_semantic_expression_modified(primal_code, "LF", "%returnval", variable_type, expr_return_type);

			append_str_to_str(primal_code, "PUSHS LF@%returnval\n");
			append_str_to_str(primal_code, "RETURN\n");

			expected_token(token_buff, NEW_LINE);
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		case END:
		case ELSE:
		case LOOP:
			break;

		case NEW_LINE:
			expected_token(token_buff, NEW_LINE);
			neterm_statementblock(token_buff, symtable,primal_code);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type,10,IDENTIFIER,IF,PRINT,INPUT,DO,RETURN,ELSE,LOOP,END,NEW_LINE);
			break;
	}
}

/* Nonterminal function elsestatement*/
void neterm_elsestatement(token_buffer * token_buff, htab_t * symtable, String * primal_code, int order)
{
	token *actual_token = token_buffer_peek_token(token_buff);
	switch(actual_token->type){

		case ELSE:
			else_statement = true;
			token_buffer_get_token(token_buff);
			expected_token(token_buff, NEW_LINE);
			append_str_to_str(primal_code, "JUMP ");
            generate_if_label(primal_code, label_end_if, order);
            append_str_to_str(primal_code, "LABEL ");
            generate_if_label(primal_code, label_else, order);

			neterm_statementblock(token_buff, symtable,primal_code);
			else_statement = false;
			break;

		case END:
			if(else_statement)
			{
				append_str_to_str(primal_code, "LABEL ");
            	generate_if_label(primal_code, label_end_if, order);
            }
            else
           	{
           		append_str_to_str(primal_code, "LABEL ");
            	generate_if_label(primal_code, label_else, order);
           	}
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 2,ELSE,END);
			break;
	}
}

/* Nonterminal function decORasign*/
void neterm_decORasign(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token *actual_token = token_buffer_get_token(token_buff);
	htab_listitem *found = NULL;

	switch(actual_token->type){

		case DIM:

			actual_token = token_buffer_get_token(token_buff);
			if(actual_token->type != IDENTIFIER)
			{
				syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			}

			if(!inside_scopeblock)
			{
				found = htab_find(global_symtable, actual_token->attr.string_value);
				if(found)
				{
					check_variable_name(found, actual_token->attr.string_value);
				}
			}

			found = htab_find(symtable, actual_token->attr.string_value);
			if(found)
			{
				check_variable_name(found, actual_token->attr.string_value);
			}

			if(!inside_scopeblock)
			{
				compare_with_params(symtable,fun_record_actual,actual_token->attr.string_value);
			}

			found = htab_make_item(actual_token->attr.string_value);
			htab_append(found, symtable);

			expected_token(token_buff, AS);
			set_id_type(found, neterm_type(token_buff, symtable,primal_code));
			set_id_defined(found);
			set_id_declared(found);

			append_str_to_str(primal_code, "DEFVAR LF@");
			append_str_to_str(primal_code, found->key);
			append_char_to_str(primal_code, '\n');

			neterm_decasign(token_buff, symtable, found,primal_code);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, DIM);
			break;

	}
}

/* Nonterminal function decasign*/
void neterm_decasign(token_buffer * token_buff, htab_t * symtable, htab_listitem *found, String * primal_code)
{
	int variable_type;
	int expr_return_type;

	token *actual_token = token_buffer_get_token(token_buff);
	switch(actual_token->type){
		case EQUALS:
			generate_implicit_value(primal_code, found->key, found->data.type);

			variable_type = get_id_type(found);
			expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
			parse_semantic_expression(primal_code, found, variable_type, expr_return_type);

			break;

		case NEW_LINE:
			generate_implicit_value(primal_code, found->key, found->data.type);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 2,EQUALS, NEW_LINE);
			break;
	}
}



/* Nonterminal function funcallORasign*/
void neterm_funcallORasign(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	htab_listitem *found = NULL;
	token *actual_token = token_buffer_peek_token(token_buff);

	switch(actual_token->type){

		case IDENTIFIER:
			actual_token = token_buffer_get_token(token_buff);
			found = htab_find(symtable, actual_token->attr.string_value);

			if(found)
			{
				if(is_function(found))
					error_msg(ERR_CODE_SEM, "Function identifier '%s' used for assigning a value\n",actual_token->attr.string_value);

				if(inside_fun)
				{
					funcall_compare_with_params(global_symtable,fun_record_actual,actual_token->attr.string_value);
				}
			}
			else
			{	
				if(inside_fun)
					funcall_compare_with_functions(global_symtable, actual_token->attr.string_value);

				error_msg(ERR_CODE_SEM, "Variable '%s' wasnt declared but it is used\n", actual_token->attr.string_value);
			}

			expected_token(token_buff, EQUALS);

			neterm_funcallORasign2(token_buff, symtable, found,primal_code);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1,IDENTIFIER);
			break;
	}
}




/* Nonterminal function funcallORasign2*/
void neterm_funcallORasign2(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_var, String * primal_code)
{

	htab_listitem *found_fun = NULL;
	token *actual_token = token_buffer_peek_token(token_buff);
	int variable_type;
	int expr_return_type;

	switch(actual_token->type){

		case IDENTIFIER:


			if(token_buffer_next_token(token_buff)->type != LEFT_PARANTHESIS)
			{
					variable_type = get_id_type(found_var);
					expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
					parse_semantic_expression(primal_code, found_var, variable_type, expr_return_type);
					expected_token(token_buff, NEW_LINE);
					break;
			}

			actual_token = token_buffer_get_token(token_buff);
			found_fun = htab_find(global_symtable, actual_token->attr.string_value);

			if(!found_fun)
			{
				error_msg(ERR_CODE_SEM, "Function '%s' wasnt declared but is used\n", actual_token->attr.string_value);
			}
			else
			{
				append_str_to_str(primal_code, "CREATEFRAME\n");	//prepare Temporary frame

				expected_token(token_buff, LEFT_PARANTHESIS);
				neterm_funcallparams(token_buff, symtable, found_fun,primal_code);
				expected_token(token_buff, RIGHT_PARANTHESIS);
				expected_token(token_buff, NEW_LINE);

				append_str_to_str(primal_code, "PUSHFRAME\n");
				append_str_to_str(primal_code, "CALL ");
				append_str_to_str(primal_code, found_fun->key);
				append_char_to_str(primal_code, '\n');
				append_str_to_str(primal_code, "POPFRAME\n");

				if(found_var->data.type != found_fun->data.type)
				{
					int fun_type = make_type_conversion(found_var->data.type, found_fun->data.type);
					if(!fun_type)
						error_msg(ERR_CODE_TYPE, "Variable '%s' type doesnt match with return type of function '%s'\n", found_var->key, found_fun->key);
					else if(fun_type == DOUBLE_TYPE)
					{
						append_str_to_str(primal_code, "INT2FLOATS\n");
					}
					else
						append_str_to_str(primal_code, "FLOAT2INTS\n");
				}

				append_str_to_str(primal_code, "POPS LF@");
				append_str_to_str(primal_code, found_var->key);
				append_char_to_str(primal_code, '\n');


			}
			break;

		case INT_VALUE:
		case DOUBLEE:
		case DOUBLE_WITH_EXP:
		case INT_WITH_EXP:
		case LEFT_PARANTHESIS:
		case STRING_VALUE:
		case TRUE:
		case FALSE:
			variable_type = get_id_type(found_var);
			expr_return_type = parse_expression(token_buff, symtable, primal_code, NEW_LINE);
			parse_semantic_expression(primal_code, found_var, variable_type, expr_return_type);
			expected_token(token_buff, NEW_LINE);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 2, INT_VALUE, IDENTIFIER);
			break;
	}
}


/* Generate funcall param order*/
unsigned int generate_funcall_param_order()
{
	static unsigned int counter_funcall = 1;
	return counter_funcall++;
}

/* Generates relevant instructions of if label */
void generate_funcall_label(String * primal_code, unsigned int order)
{
	char buffer[4] = "0000";
	snprintf(buffer, 4, "%d", order);
	append_str_to_str(primal_code, buffer);
}

/* Nonterminal function funcallparams*/
void neterm_funcallparams(token_buffer * token_buff, htab_t * symtable, htab_listitem *found,String *primal_code)
{
	token *actual_token = token_buffer_peek_token(token_buff);

	switch(actual_token->type){

		case RIGHT_PARANTHESIS:
			if(found->data.par_count)
			{
				error_msg(ERR_CODE_TYPE, "Function '%s' has %d paramater/s.\n", found->key, found->data.par_count);
			}
			break;

		case IDENTIFIER:
		case INT_VALUE:
		case DOUBLEE:
		case DOUBLE_WITH_EXP:
		case INT_WITH_EXP:
		case LEFT_PARANTHESIS:
		case STRING_VALUE:
		case TRUE:
		case FALSE:

			neterm_funcallparam(token_buff, symtable, found, primal_code);
			neterm_funcallparamsnext(token_buff, symtable,found,primal_code);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 3,IDENTIFIER, RIGHT_PARANTHESIS, INT_VALUE);
			break;
	}
}


/* Nonterminal function funcallparam*/
void neterm_funcallparam(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_fun, String * primal_code)
{
	if(!first_par_taken)
	{
		global_actual_param = found_fun->data.first_par;
		first_par_taken = true;
	}

	htab_listitem *found_var = NULL;
	token *actual_token = token_buffer_peek_token(token_buff);

	switch(actual_token->type){

		case IDENTIFIER:
			actual_token = token_buffer_get_token(token_buff);
			found_var = htab_find(symtable, actual_token->attr.string_value);
			if(!found_var)
			{
				if(inside_fun)
				{
					fun_call_param = true;
					funcall_compare_with_params(symtable, fun_record_actual, actual_token->attr.string_value);
					check_funcall_params(symtable, fun_record_actual, found_fun, actual_token->attr.string_value, primal_code);
					fun_call_param = false;
				}
				else
				{
					error_msg(ERR_CODE_SEM, "Variable '%s' wasnt declared but is used as parameter\n", actual_token->attr.string_value);
				}
			}
			else
			{
				if(is_function(found_var))
					error_msg(ERR_CODE_OTHERS, "Function '%s' is used as parameter of function \n", found_var->key);

				global_par_count++;

				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, global_actual_param->par_name);
				append_char_to_str(primal_code, '\n');

				append_str_to_str(primal_code, "PUSHS LF@");
				append_str_to_str(primal_code, found_var->key);
				append_char_to_str(primal_code, '\n');

				if(found_var->data.type != global_actual_param->par_type)
				{
					int fun_type = make_type_conversion(found_var->data.type, global_actual_param->par_type);
					if(!fun_type)
					{
						error_msg(ERR_CODE_TYPE, "Parameter type and variable '%s' type dont match\n", found_var->key);
					}
					else if(fun_type == INTEGER_TYPE)
					{
						append_str_to_str(primal_code, "INT2FLOATS\n");
					}
					else
						append_str_to_str(primal_code, "FLOAT2INTS\n");
				}

				append_str_to_str(primal_code, "POPS TF@");
				append_str_to_str(primal_code, global_actual_param->par_name);
				append_char_to_str(primal_code, '\n');

				global_actual_param = global_actual_param->par_next;
			}
			break;

		case INT_VALUE:
		case DOUBLEE:
		case DOUBLE_WITH_EXP:
		case INT_WITH_EXP:
		case STRING_VALUE:
		case TRUE:
		case FALSE:
			neterm_constvalue(token_buff, symtable, found_fun,primal_code);
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 3,IDENTIFIER, RIGHT_PARANTHESIS, INT_VALUE);
			break;
	}
}

/* Nonterminal function funcallparamsnext*/
void neterm_funcallparamsnext(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_fun, String * primal_code)
{
	token *actual_token = token_buffer_peek_token(token_buff);

	switch(actual_token->type){

		case COMA:
			token_buffer_get_token(token_buff);
			neterm_funcallparams(token_buff, symtable, found_fun,primal_code);
			break;

		case RIGHT_PARANTHESIS:
			if(global_par_count != found_fun->data.par_count)
				error_msg(ERR_CODE_TYPE, "Called function '%s' expects %d parameter/s\n", found_fun->key, found_fun->data.par_count);

			global_par_count = 0;
			global_actual_param = NULL;
			first_par_taken = false;
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 2,COMA, RIGHT_PARANTHESIS);
			break;
	}
}

/* Nonterminal function constvalue*/
void neterm_constvalue(token_buffer * token_buff, htab_t * symtable, htab_listitem *found_fun, String * primal_code)
{
	token *actual_token = token_buffer_get_token(token_buff);

	String str;
	init_string(&str);

	if(global_actual_param == NULL)
		error_msg(ERR_CODE_TYPE, "Wrong number of parameters in called function '%s'\n", found_fun->key);

	switch(actual_token->type){

		case INT_VALUE:
			if(global_actual_param->par_type != INTEGER_TYPE)
			{
				if(global_actual_param->par_type == DOUBLE_TYPE)
				{
					append_str_to_str(primal_code, "DEFVAR TF@");
					append_str_to_str(primal_code, global_actual_param->par_name);
					append_char_to_str(primal_code, '\n');
					append_str_to_str(primal_code, "PUSHS int@");
					sprintf((&str)->str, "%d", actual_token->attr.int_value);
					append_str_to_str(primal_code, (&str)->str);
					append_str_to_str(primal_code, "\nINT2FLOATS\n");
					append_str_to_str(primal_code, "POPS TF@");
					append_str_to_str(primal_code, global_actual_param->par_name);
					append_char_to_str(primal_code, '\n');

				}
				else
					error_msg(ERR_CODE_TYPE, "In function '%s' as %d. parameter expected %s \n", found_fun->key,++global_par_count, get_string_from_type(global_actual_param->par_type));
			}
			else
			{
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, global_actual_param->par_name);
				append_char_to_str(primal_code, '\n');
				append_str_to_str(primal_code, "PUSHS int@");
				sprintf((&str)->str, "%d",actual_token->attr.int_value);
				append_str_to_str(primal_code, (&str)->str);
				append_char_to_str(primal_code, '\n');
				append_str_to_str(primal_code, "POPS TF@");
				append_str_to_str(primal_code, global_actual_param->par_name);
				append_char_to_str(primal_code, '\n');

			}
			clear_string(&str);
			global_par_count++;
			global_actual_param = global_actual_param->par_next;
			break;

		case DOUBLEE:
		case DOUBLE_WITH_EXP:
		case INT_WITH_EXP:
			if(global_actual_param->par_type != DOUBLE_TYPE)
			{
				if(global_actual_param->par_type == INTEGER_TYPE)
				{
					append_str_to_str(primal_code, "DEFVAR TF@");
					append_str_to_str(primal_code, global_actual_param->par_name);
					append_char_to_str(primal_code, '\n');
					append_str_to_str(primal_code, "PUSHS float@");
					sprintf((&str)->str, "%g",actual_token->attr.double_value);
					append_str_to_str(primal_code, (&str)->str);
					append_str_to_str(primal_code, "\nFLOAT2INTS\n");
					append_str_to_str(primal_code, "POPS TF@");
					append_str_to_str(primal_code, global_actual_param->par_name);
					append_char_to_str(primal_code, '\n');

				}
				else
					error_msg(ERR_CODE_TYPE, "In function '%s' as %d. parameter expected %s \n", found_fun->key,++global_par_count, get_string_from_type(global_actual_param->par_type));
			}
			else
			{
				append_str_to_str(primal_code, "DEFVAR TF@");
				append_str_to_str(primal_code, global_actual_param->par_name);
				append_char_to_str(primal_code, '\n');
				append_str_to_str(primal_code, "PUSHS float@");
				sprintf((&str)->str, "%d",actual_token->attr.int_value);
				append_str_to_str(primal_code, (&str)->str);
				append_char_to_str(primal_code, '\n');
				append_str_to_str(primal_code, "POPS TF@");
				append_str_to_str(primal_code, global_actual_param->par_name);
				append_char_to_str(primal_code, '\n');
			}
			clear_string(&str);
			global_par_count++;
			global_actual_param = global_actual_param->par_next;
			break;

		case STRING_VALUE:
			if(global_actual_param->par_type != STRING_TYPE)
			{
					error_msg(ERR_CODE_TYPE, "In function '%s' as %d. parameter expected %s \n", found_fun->key,++global_par_count, get_string_from_type(global_actual_param->par_type));
			}
			append_str_to_str(primal_code, "DEFVAR TF@");
			append_str_to_str(primal_code, global_actual_param->par_name);
			append_char_to_str(primal_code, '\n');
			append_str_to_str(primal_code, "PUSHS string@");
			append_str_to_str(primal_code, actual_token->attr.string_value);
			append_char_to_str(primal_code, '\n');
			append_str_to_str(primal_code, "POPS TF@");
			append_str_to_str(primal_code, global_actual_param->par_name);
			append_char_to_str(primal_code, '\n');

			clear_string(&str);


			global_par_count++;
			global_actual_param = global_actual_param->par_next;
			break;

		case TRUE:
		case FALSE:
			if(global_actual_param->par_type != BOOLEAN_TYPE)
			{
					error_msg(ERR_CODE_TYPE, "In function '%s' as %d. parameter expected %s \n", found_fun->key,++global_par_count, get_string_from_type(global_actual_param->par_type));
			}
			append_str_to_str(primal_code, "DEFVAR TF@");
			append_str_to_str(primal_code, global_actual_param->par_name);
			append_char_to_str(primal_code, '\n');
			append_str_to_str(primal_code, "PUSHS bool@");
			append_str_to_str(primal_code, (actual_token->type) == TRUE ? "TRUE" : "FALSE");
			append_char_to_str(primal_code, '\n');
			append_str_to_str(primal_code, "POPS TF@");
			append_str_to_str(primal_code, global_actual_param->par_name);
			append_char_to_str(primal_code, '\n');
			clear_string(&str);

			global_par_count++;
			global_actual_param = global_actual_param->par_next;
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1,INT_VALUE);
			break;
	}
}

/* Nonterminal function exprnext*/
void neterm_exprnext(token_buffer * token_buff, htab_t * symtable, String * primal_code)
{
	token *actual_token = token_buffer_peek_token(token_buff);

	switch(actual_token->type){

		case IDENTIFIER:
		case INT_VALUE:
		case DOUBLEE:
		case DOUBLE_WITH_EXP:
		case INT_WITH_EXP:
		case LEFT_PARANTHESIS:
		case STRING_VALUE:
		case TRUE:
		case FALSE:
			parse_expression(token_buff, symtable, primal_code, SEMICOLON);
			append_str_to_str(primal_code, "POPS GF@%SWAP\n");
			append_str_to_str(primal_code, "WRITE GF@%SWAP\n");
			expected_token(token_buff, SEMICOLON);
			neterm_exprnext(token_buff, symtable,primal_code);
			break;

		case NEW_LINE:
			break;

		default:
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type,2, DOUBLEE,NEW_LINE);
			break;
	}
}

/*	Chooses next rule from neterm_funblock	*/
void funblock_rule_continue(token_buffer * token_buff, htab_t * symtable ,int token_type, String *primal_code)
{
	token * actual_token = token_buffer_peek_token(token_buff);

	switch (token_type){

		case SCOPE:
			append_str_to_str(primal_code, "LABEL %MAIN\n");
			append_str_to_str(primal_code, "DEFVAR GF@%SWAP\n");
			append_str_to_str(primal_code, "DEFVAR GF@%SWAP2\n");
			append_str_to_str(primal_code, "CREATEFRAME\n");
			append_str_to_str(primal_code, "PUSHFRAME\n");
			neterm_scopeblock(token_buff, symtable, primal_code);
			append_str_to_str(primal_code, "POPFRAME\n");
			break;

		case DECLARE:
		case FUNCTION:
		case NEW_LINE:
			neterm_funblock(token_buff, symtable, primal_code);
			break;

		case EOF:
			error_msg(ERR_CODE_SYNTAX, "Scope block wasnt found\n");
			break;

		default :
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 4, SCOPE, DECLARE, FUNCTION, NEW_LINE);
			break;
	}
}


/* Checks param name with functions and its others params*/
void fundec_check_param_name(htab_listitem *new_fun_record, htab_t *global_symtable, char *par_name, bool first_par)
{
	if(!strcmp(par_name, new_fun_record->key))
	{
		error_msg(ERR_CODE_SEM, "Parameter name '%s' is same as its function name '%s' \n", par_name, new_fun_record->key);
	}

	if(global_symtable)
	{
		size_t size = htab_bucket_count(global_symtable);
		for(unsigned long i = 0; i < size; i++)
		{
			struct htab_listitem *help = global_symtable->buckets[i];
			while(help)
			{
				if(!strcmp(par_name, help->key))
				{
					error_msg(ERR_CODE_SEM, "Parameter name '%s' has same name as function '%s' defined/declare before\n", par_name, help->key);
				}
				help = help->next;
			}
		}
	}

	if(first_par)
	{
		return;
	}

	struct fun_par *tmp_par = new_fun_record->data.first_par;
	int par_count = new_fun_record->data.par_count;

	while(par_count)
	{
		if(!strcmp(tmp_par->par_name, par_name))
		{
			error_msg(ERR_CODE_SEM, "Parameters have the same IDENTIFIERS\n");
		}

		tmp_par = tmp_par->par_next;
		par_count--;
	}
}

/* Compares found identifier with functions*/
void funcall_compare_with_functions(htab_t *global_symtable, char *var_name)
{
	if(global_symtable)
	{
		size_t size = htab_bucket_count(global_symtable);
		for(unsigned long i = 0; i < size; i++)
		{
			struct htab_listitem *help = global_symtable->buckets[i];
			while(help)
			{
				if(!strcmp(var_name, help->key))
				{
					error_msg(ERR_CODE_SEM, "Function identifier '%s' used for assigning a value\n",var_name);
				}
				help = help->next;
			}
		}
	}
}

/* Compares found identifier with params */
void funcall_compare_with_params(htab_t * global_symtable, htab_listitem *record, char * var_name)
{
	struct fun_par *tmp_par = record->data.first_par;
	int par_count = record->data.par_count;

	if(par_count)
	{
		while(par_count)
		{
			if(!strcmp(tmp_par->par_name, var_name))
			{
				if(!fun_call_param)
				{
					error_msg(ERR_CODE_SEM, "Parameter '%s' used for assigning a value.\n", var_name);
				}
				break;
			}
			tmp_par = tmp_par->par_next;
			par_count--;
		}
	}
}

/*	Eats next token and compares with the one we expect */
void expected_token(token_buffer * token_buff, int tok_type)
{
	token * actual_token = token_buffer_get_token(token_buff);
	if (actual_token->type != tok_type)
	{
		syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, tok_type);
	}
}

/* Checks funcall paramaters */
void check_funcall_params(htab_t * symtable, htab_listitem *actual_fun, htab_listitem *called_fun, char *var_name, String *primal_code)
{
	struct fun_par *actual_fun_par = actual_fun->data.first_par;
	int actual_par_type = actual_fun_par->par_type;
	int actual_fun_par_count = actual_fun->data.par_count;

	struct fun_par *called_fun_par = called_fun->data.first_par;
	unsigned int counter = called_fun->data.par_count;
	int types[counter];
	int begin = 0;

	if(counter)
		types[begin++] = called_fun_par->par_type;

	while(counter)
	{
		if(called_fun_par->par_next)
		{
			called_fun_par = called_fun_par->par_next;
			types[begin++] = called_fun_par->par_type;
		}
		counter--;
	}

	if(actual_fun_par_count)
	{
		while(actual_fun_par_count)
		{
			if(!strcmp(actual_fun_par->par_name, var_name))
				break;
			actual_fun_par = actual_fun_par->par_next;
			actual_par_type = actual_fun_par->par_type;
			actual_fun_par_count--;
		}

		if(!actual_fun_par_count)
			error_msg(ERR_CODE_SEM, "Variable '%s' wasnt declared\n", var_name);
		else
		{
			if(actual_par_type != types[global_par_count]) // before was here types[global_par_count++]
			{
				int fun_par_type = make_type_conversion(actual_par_type, types[global_par_count++]);
				if(!fun_par_type)
					error_msg(ERR_CODE_TYPE, "Parameter in called function has different type\n", var_name);
				else
				{
					append_str_to_str(primal_code, "DEFVAR TF@");
					append_str_to_str(primal_code, actual_fun_par->par_name);
					append_char_to_str(primal_code, '\n');

					if(fun_par_type == DOUBLE_TYPE)
					{
						append_str_to_str(primal_code, "INT2FLOATS\n");
					}
					else
						append_str_to_str(primal_code, "FLOAT2INTS\n");
				}
			}
		}

		global_actual_param = global_actual_param->par_next;
	}
	else
	{
		error_msg(ERR_CODE_SEM, "Variable '%s' wasnt declared\n", var_name);
	}
}

/* Checks record name and prints error message*/
void check_variable_name(htab_listitem *record, char * var_name)
{
	if(is_function(record) && is_defined(record))
		error_msg(ERR_CODE_SEM, "IDENTIFIER '%s' was defined as a function.\n", record->key);
	else if(is_function(record) && is_declared(record))
		error_msg(ERR_CODE_SEM, "IDENTIFIER '%s' was declared as a function.\n", record->key);
	else if(is_defined(record))
		error_msg(ERR_CODE_SEM, "IDENTIFIER '%s' was declared before.\n", record->key);
}

/* Compares variable name with parameters*/
void compare_with_params(htab_t * symtable, htab_listitem *record, char * var_name)
{
	struct fun_par *tmp_par = record->data.first_par;
	int par_count = record->data.par_count;

	while(par_count)
	{
		if(!strcmp(tmp_par->par_name, var_name))
		{
			error_msg(ERR_CODE_SEM, "New defined variable has the same name as parameter\n");
		}

		tmp_par = tmp_par->par_next;
		par_count--;
	}
}

/* Checks params names*/
void check_params_names(struct htab_listitem * item, token *token)
{
	struct fun_par *actual_param = item->data.first_par;

	while(actual_param)
	{
		if(!strcmp(actual_param->par_name, token->attr.string_value))
		{
			error_msg(ERR_CODE_SEM, "PARAM '%s' with same name was declared before\n", token->attr.string_value);
		}

		actual_param = actual_param->par_next;
	}
}

/*	Checks new lines put after scopeblock till EOF.	If somethhing unexpected appears, kills program with lex error*/
void check_new_lines_after_scopeblock(token_buffer * token_buff)
{
	token *actual_token = token_buffer_get_token(token_buff);
	while(actual_token->type != EOF)
	{
		if(actual_token->type != NEW_LINE)
		{
			if(actual_token->type != LEXICAL_ERROR)
				error_msg(ERR_CODE_SYNTAX, "Something after END SCOPE was found - SYNTAX ERROR\n");
			else
				error_msg(ERR_CODE_LEXICAL, "Something after END SCOPE was found - SYNTAX ERROR\n");
		}
		actual_token = token_buffer_get_token(token_buff);
	}
	token_buffer_unget_token(token_buff);
}

/* Checks if all declared functions were defined */
void check_all_functions_definitions(htab_t *symtable)
{
	if(symtable)
	{
		size_t size = htab_bucket_count(symtable);
		for(unsigned long i = 0; i < size; i++)
		{
			struct htab_listitem *help = symtable->buckets[i];
			while(help)
			{
				if(is_function(help) && is_declared(help) && !is_defined(help))
				{
					error_msg(ERR_CODE_SEM, "Function '%s' wasnt defined\n", help->key);
				}

				help = help->next;
			}
		}
	}
	else
	{
		printf("Symtable doesnt exist!\n");
	}
}

/* Checks function definiciton parameters*/
void check_fun_definiton_params(token_buffer * token_buff, htab_listitem *record, int par_counter)
{
	int counter = 0;
	token *actual_token = token_buffer_peek_token(token_buff);
	while(1)
	{
		if(actual_token->type == FUNCTION)
		{
			actual_token = token_buffer_get_next(token_buff, ++counter);

			if(actual_token->type == IDENTIFIER)
			{
				if(!strcmp(actual_token->attr.string_value, record->key))
					break;
			}
		}
		else if (actual_token->type == SCOPE)
				error_msg(ERR_CODE_SEM, "Function is declared but wasnt defined\n");
		actual_token = token_buffer_get_next(token_buff, ++counter);
	}


	int par_count = record->data.par_count;
	struct fun_par **pardec = &(record->data.first_par);

	while(actual_token->type != RIGHT_PARANTHESIS)
	{
		actual_token = token_buffer_get_next(token_buff, ++counter);
		if(actual_token->type == IDENTIFIER)
		{
			if(!par_count)
				error_msg(ERR_CODE_SEM, "Function declaration and function definition parameters count doesnt match\n");

			if(strcmp(actual_token->attr.string_value, (*pardec)->par_name))
			{
				(*pardec)->par_name = actual_token->attr.string_value;
			}

			par_count--;
			pardec = &((*pardec)->par_next);
		}
	}
}

/* Checks and compares parameters from declaration with new function definition*/
void check_fun_declaration_params(token_buffer * token_buff, htab_t * symtable, htab_t * new_symtable, struct htab_listitem *record, String *primal_code)
{
	int par_count = 0;
	token *actual_token;
	struct fun_par *param = record->data.first_par;
	struct fun_par **par_changed = &(record->data.first_par);

	if(param == NULL)
	{
		expected_token(token_buff, RIGHT_PARANTHESIS);
		token_buffer_unget_token(token_buff);
		if(par_count != record->data.par_count)
		{
			error_msg(ERR_CODE_SEM, "Parameters' count of fun declaration and definition dont match\n");
		}
		return;
	}

	while(1)
	{
		actual_token = token_buffer_get_token(token_buff);
		if(actual_token->type != IDENTIFIER)
		{
			error_msg(ERR_CODE_SYNTAX, "Syntax error expected parameter IDENTIFIER\n");
		}

		expected_token(token_buff, AS);

		if(neterm_type(token_buff, symtable,primal_code) != param->par_type)
		{
			error_msg(ERR_CODE_SEM, "Type params of fun declaration and definition dont match\n");
		}

		create_param(new_symtable, actual_token->attr.string_value, param->par_type);

		par_count++;
		actual_token = token_buffer_peek_token(token_buff);

		if(actual_token->type == RIGHT_PARANTHESIS)
		{
			if(par_count != record->data.par_count)
			{
				error_msg(ERR_CODE_SEM, "Param count of fun declaration and definition dont match\n");
			}
			break;
		}
		else if(actual_token->type == COMA)
		{
			token_buffer_get_token(token_buff);
			if(token_buffer_peek_token(token_buff)->type != IDENTIFIER)
			{
				syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 1, IDENTIFIER);
			}

			param = param->par_next;
			par_changed = &((*par_changed)->par_next);
		}
		else
		{
			syntax_error_unexpected(actual_token->line, actual_token->pos ,actual_token->type, 2, LEFT_PARANTHESIS, COMA);
		}
	}
}

/* Adds built in functions into instructon list*/
void add_build_in_functions(htab_t * symtable, String * primal_code)
{
	struct htab_listitem * record = NULL;
	struct fun_par ** actual_param = NULL;

	//Function length(s as string) as integer
	record = create_fun_record(symtable, "length");
	set_id_defined(record);
	set_func_par_count(record, 1);
	record->data.type=INTEGER_TYPE;

	actual_param = &(record->data.first_par);
	(*actual_param)=malloc(sizeof(struct fun_par));
	(*actual_param)->par_name="s";
	(*actual_param)->par_type=STRING_TYPE;

	(*actual_param)->par_next = NULL;
	generate_length(primal_code);

	//Function substr(s as string, i as integer, n as integer) as string
	record = create_fun_record(symtable, "substr");
	set_id_defined(record);
	set_func_par_count(record, 3);
	record->data.type=STRING_TYPE;

	actual_param = &(record->data.first_par);
	(*actual_param)=malloc(sizeof(struct fun_par));
	(*actual_param)->par_name="s";
	(*actual_param)->par_type=STRING_TYPE;

	actual_param = &((*actual_param)->par_next);
	(*actual_param)=malloc(sizeof(struct fun_par));
	(*actual_param)->par_name="i";
	(*actual_param)->par_type=INTEGER_TYPE;

	actual_param = &((*actual_param)->par_next);
	(*actual_param)=malloc(sizeof(struct fun_par));
	(*actual_param)->par_name="n";
	(*actual_param)->par_type=INTEGER_TYPE;

	(*actual_param)->par_next = NULL;	//last one
	generate_substr(primal_code);

	//Function asc(s as string, i as integer) as integer
	record = create_fun_record(symtable, "asc");
	set_id_defined(record);
	set_func_par_count(record, 2);
	record->data.type=INTEGER_TYPE;

	actual_param = &(record->data.first_par);
	(*actual_param)=malloc(sizeof(struct fun_par));
	(*actual_param)->par_name="s";
	(*actual_param)->par_type=STRING_TYPE;

	actual_param = &((*actual_param)->par_next);
	(*actual_param)=malloc(sizeof(struct fun_par));
	(*actual_param)->par_name="i";
	(*actual_param)->par_type=INTEGER_TYPE;

	(*actual_param)->par_next = NULL;
	generate_asc(primal_code);

	// Function chr(i as integer) as string
	record = create_fun_record(symtable, "chr");
	set_id_defined(record);
	record->data.type=STRING_TYPE;
	set_func_par_count(record, 1);

	actual_param = &(record->data.first_par);
	(*actual_param)=malloc(sizeof(struct fun_par));
	(*actual_param)->par_name="i";
	(*actual_param)->par_type=INTEGER_TYPE;

	(*actual_param)->par_next = NULL;
	generate_chr(primal_code);
}

/* Generates order of if label */
unsigned int generate_if_label_order()
{
	static unsigned int counter = 0;
		return counter++;
}


/* Generates relevant instructions of if label */
void generate_if_label(String * primal_code, enum_label_names prefix, unsigned int order)
{
	char buffer[8] = "00000000";
	if(order > 999999999)
		error_msg(ERR_CODE_INTERN, "Buffer overflow\n");

	switch (prefix)
	{
		case label_if:
			snprintf(buffer, 8, "%d", order);
			append_char_to_str(primal_code, '%');
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "IF\n");
			break;
		case label_else:
			snprintf(buffer, 8, "%d", order);
			append_char_to_str(primal_code, '%');
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ELSE\n");
			break;
		case label_end_if:
			snprintf(buffer, 8, "%d", order);
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
	char buffer[8] = "00000000";
	if(order > 999999999)
		error_msg(ERR_CODE_INTERN, "Buffer overflow\n");

	append_str_to_str(primal_code, "JUMP ");
	switch (prefix)
	{
		case label_if:
			snprintf(buffer, 8, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "IF\n");
			break;
		case label_else:
			snprintf(buffer, 8, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ELSE\n");
			break;
		case label_end_if:
			snprintf(buffer, 8, "%d", order);
			append_str_to_str(primal_code, buffer);
			append_str_to_str(primal_code, "ENDIF\n");
			break;
		default:
			break;
	}
}

/* Generates code of hard value */
void generate_implicit_value(String * primal_code, char * name, enum_type type)
{
	append_str_to_str(primal_code, "MOVE LF@");
	append_str_to_str(primal_code, name);
	switch (type)
	{
		case VOID_TYPE:
		return;
		break;

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
	else if (variable_type == BOOLEAN_TYPE && expr_return_type == INTEGER_TYPE){
		append_str_to_str(primal_code, "POPS GF@%SWAP\n");
		append_str_to_str(primal_code, "EQ GF@%SWAP GF@%SWAP int@0\n");
		append_str_to_str(primal_code, "PUSHS GF@%SWAP\nNOTS\n");
	}
	else if (variable_type == BOOLEAN_TYPE && expr_return_type == DOUBLE_TYPE){
		append_str_to_str(primal_code, "POPS GF@%SWAP\n");
		append_str_to_str(primal_code, "EQ GF@%SWAP GF@%SWAP float@0.0\n");
		append_str_to_str(primal_code, "PUSHS GF@%SWAP\nNOTS\n");
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

/* Parses semantic expression and generates relevant instructions*/
void parse_semantic_expression_modified(String * primal_code, char * frame, char * name, int variable_type, int expr_return_type)
{

	if (variable_type == DOUBLE_TYPE && expr_return_type == INTEGER_TYPE) {
		append_str_to_str(primal_code, "INT2FLOATS\nPOPS ");
		append_str_to_str(primal_code, frame);
		append_char_to_str(primal_code, '@');
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == DOUBLE_TYPE && expr_return_type == DOUBLE_TYPE){
		append_str_to_str(primal_code, "POPS ");
		append_str_to_str(primal_code, frame);
		append_char_to_str(primal_code, '@');
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == INTEGER_TYPE && expr_return_type == DOUBLE_TYPE){
		append_str_to_str(primal_code, "FLOAT2INTS\nPOPS ");
		append_str_to_str(primal_code, frame);
		append_char_to_str(primal_code, '@');
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}
	else if (variable_type == INTEGER_TYPE && expr_return_type == INTEGER_TYPE){
		append_str_to_str(primal_code, "POPS ");
		append_str_to_str(primal_code, frame);
		append_char_to_str(primal_code, '@');
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}

	else if (variable_type == BOOLEAN_TYPE && expr_return_type == BOOLEAN_TYPE){
		append_str_to_str(primal_code, "POPS ");
		append_str_to_str(primal_code, frame);
		append_char_to_str(primal_code, '@');
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}

	else if (variable_type == STRING_TYPE && expr_return_type == STRING_TYPE){
		append_str_to_str(primal_code, "POPS ");
		append_str_to_str(primal_code, frame);
		append_char_to_str(primal_code, '@');
		append_str_to_str(primal_code, name);
		append_char_to_str(primal_code, '\n');
	}

	else {
		error_msg(ERR_CODE_TYPE, "The expression value does not match the variable type\n");
	}
}

/* Makes implicit type conversion*/
int make_type_conversion(int type_1, int type_2)
{

	if(type_1 == DOUBLE_TYPE && type_2 == INTEGER_TYPE)
		return DOUBLE_TYPE;

	else if(type_1 == DOUBLE_TYPE && type_2 == STRING_TYPE)
		return 0;

	else if(type_1 == DOUBLE_TYPE && type_2 == BOOLEAN_TYPE)
		return 0;

	else if(type_1 == INTEGER_TYPE && type_2 == DOUBLE_TYPE)
		return INTEGER_TYPE;

	else if(type_1 == INTEGER_TYPE && type_2 == STRING_TYPE)
		return 0;

	else if(type_1 == INTEGER_TYPE && type_2 == BOOLEAN_TYPE)
		return 0;

	else if(type_1 == STRING_TYPE && type_2 == BOOLEAN_TYPE)
		return 0;
	else
		return 0;
}

/* Generates chr function */
void generate_chr(String *primal_code)
{
	append_str_to_str(primal_code,  "LABEL chr\n"
                                    "DEFVAR LF@%returnval\n");
    append_str_to_str(primal_code,  "INT2CHAR LF@%returnval LF@i\n"
    				    			"PUSHS LF@%returnval\n"
                                    "RETURN\n"
			 						"\n");
}

/* Generates length function */
void generate_length(String *primal_code)
{
	append_str_to_str(primal_code, 	"LABEL length\n"
									"DEFVAR LF@%returnval\n"
									"STRLEN LF@%returnval LF@s\n"
									"PUSHS LF@%returnval\n"
									"RETURN\n"
                                    "\n");
}

/* Generates substr function */
void generate_substr(String *primal_code)
{
	append_str_to_str(primal_code,	"LABEL substr\n");
	append_str_to_str(primal_code,	"DEFVAR LF@len\n");
	append_str_to_str(primal_code,	"DEFVAR LF@greater\n");
	append_str_to_str(primal_code,	"DEFVAR LF@lenminusi\n");
	append_str_to_str(primal_code,	"DEFVAR LF@char\n");
	append_str_to_str(primal_code,	"DEFVAR LF@%returnval\n");
	append_str_to_str(primal_code,	"MOVE LF@%returnval string@\n");
	append_str_to_str(primal_code,	"STRLEN LF@len LF@s\n");
	append_str_to_str(primal_code,  "JUMPIFEQ end1 LF@len int@0\n");
	append_str_to_str(primal_code,	"GT LF@greater LF@i int@0\n");
	append_str_to_str(primal_code,	"NOT LF@greater LF@greater\n");
	append_str_to_str(primal_code,	"JUMPIFEQ end1 LF@greater bool@true\n");
	append_str_to_str(primal_code,	"LT LF@greater LF@n int@0\n");
	append_str_to_str(primal_code,	"JUMPIFEQ end2 LF@greater bool@true\n");
	append_str_to_str(primal_code,	"SUB LF@lenminusi LF@len LF@i\n");
	append_str_to_str(primal_code,	"GT LF@greater LF@n LF@lenminusi\n");
	append_str_to_str(primal_code,	"JUMPIFEQ end2 LF@greater bool@true\n");
	append_str_to_str(primal_code,	"SUB LF@i LF@i int@1\n");
	append_str_to_str(primal_code,	"LABEL loop\n");
	append_str_to_str(primal_code,	"GETCHAR LF@char LF@s LF@i\n");
	append_str_to_str(primal_code,	"CONCAT LF@%returnval LF@%returnval LF@char\n");
	append_str_to_str(primal_code,	"ADD LF@i LF@i int@1\n");
	append_str_to_str(primal_code,	"SUB LF@n LF@n int@1\n");
	append_str_to_str(primal_code,	"JUMPIFNEQ loop LF@n int@0\n");
	append_str_to_str(primal_code,	"PUSHS LF@%returnval\n");
	append_str_to_str(primal_code,	"RETURN\n");
	append_str_to_str(primal_code,	"LABEL end1\n");
	append_str_to_str(primal_code,	"PUSHS string@\n");
	append_str_to_str(primal_code,	"RETURN\n");
	append_str_to_str(primal_code,	"LABEL end2\n");
	append_str_to_str(primal_code,	"SUB LF@i LF@i int@1\n");
	append_str_to_str(primal_code,	"SUB LF@n LF@len LF@i\n");
	append_str_to_str(primal_code,	"JUMP loop\n");
}

/* Generates asc function */
void generate_asc(String *primal_code)
{
	append_str_to_str(primal_code,  "LABEL asc\n"
									"DEFVAR LF@%returnval\n"
									"STRLEN LF@%returnval LF@s\n");
    append_str_to_str(primal_code,  "PUSHS LF@i\n"
									"PUSHS int@0\n"
									"GTS\nNOTS\n");
    append_str_to_str(primal_code,  "PUSHS bool@true\n"
									"JUMPIFEQS ATrue\n"
									"PUSHS LF@i\n");
    append_str_to_str(primal_code,  "PUSHS LF@%returnval\n"
									"GTS\n"
									"PUSHS bool@true\n");
    append_str_to_str(primal_code,  "JUMPIFEQS ATrue\n"
   									"SUB LF@i LF@i int@1\n"
									"STRI2INT LF@%returnval LF@s LF@i\n"
									"PUSHS LF@%returnval\n"
									"RETURN\n");
    append_str_to_str(primal_code, 	"LABEL ATrue\n"
									"MOVE LF@%returnval int@0\n"
									"PUSHS LF@%returnval\n"
									"RETURN\n"
                                    "\n");
}

/* Creates function record */
struct htab_listitem *create_fun_record(htab_t * symtable, char *name)
{
	struct htab_listitem *record = htab_make_item(name);
	set_id_declared(record);
	set_id_function(record);
	htab_append(record, symtable);
	record->data.first_par= NULL;
	set_func_par_count(record, 0);
	return record;
}

/* Creates new param as record */
void create_param(htab_t * symtable, char *name, int type)
{
	struct htab_listitem *record = htab_make_item(name);
	set_id_declared(record);
	set_id_defined(record);
	htab_append(record, symtable);
	set_id_type(record, type);
}

/* Prints function paramteres types */
void printf_fun_par_types(htab_listitem *fun)
{
	struct fun_par *par = fun->data.first_par;
	unsigned int counter = fun->data.par_count;
	int pole[counter];
	int begin = 0;

	if(counter)
	{
		pole[begin++] = par->par_type;
	}

	while(counter)
	{
		if(par->par_next)
		{

			par = par->par_next;
			pole[begin++] = par->par_type;
		}
		counter--;
	}

	for(int i = 0; i < fun->data.par_count; i++)
	{
		printf("Parameter[%d] has type %s\n", i+1, get_string_from_type(pole[i]));
	}

	printf("\n");
}


/* Prints that im here :D */
void here(){
	printf("IAM HEEEEEEEEEEEEREEEEEEEEEEEEEEE \n");
}

/* Converts type as integer into relevant string*/
char* get_string_from_type(int type){

	switch(type){
		case INTEGER_TYPE: return "INTEGER_TYPE";
		case DOUBLE_TYPE: return "DOUBLE_TYPE";
		case BOOLEAN_TYPE: return "BOOLEAN_TYPE";
		case STRING_TYPE: return "STRING_TYPE";
	}
	return NULL;
}

/* Prints record in symtable */
void print_fun_record(htab_t * symtable, const char *key, char *name){
	struct htab_listitem *record = htab_find(symtable, key);
	struct fun_par *tmp_par = record->data.first_par;
	unsigned int par_count = record->data.par_count;
	unsigned int par_order = 1;
	char *par_type;
	char *par_name;

	if(par_count)
	{
		par_type = get_string_from_type(tmp_par->par_type);
		par_name = tmp_par->par_name;
	}

	printf("\n| Function name: '%s'  |\n", name);
	printf(" - function = %s  \n", is_function(record) ? "true" : "false");
	printf(" - declared = %s  \n", is_declared(record) ? "true" : "false");
	printf(" - defined  = %s  \n", is_defined(record) ? "true" : "false");
	printf("| Number of params: %d |\n", par_count);

	while(par_count)
	{
		printf("| %d. param, name: '%s', type: %s|\n", par_order++, par_name, par_type);
		if(tmp_par->par_next)
		{
			tmp_par = tmp_par->par_next;
			par_type = get_string_from_type(tmp_par->par_type);
			par_name = tmp_par->par_name;
		}
		par_count--;
	}
	printf("\n");
}

/*	Skips expressions just for a case expressions havent been done yet */
void skip_expression(token_buffer * token_buff,  htab_t * symtable)
{
	while(((token_buffer_peek_token(token_buff))->type != NEW_LINE) &&
			((token_buffer_peek_token(token_buff))->type != THEN) &&
			((token_buffer_peek_token(token_buff))->type != SEMICOLON))
	{
		token_buffer_get_token(token_buff);
	}
}
