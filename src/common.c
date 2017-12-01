#include "common.h"

#include "common.h"

/* Initializes hash table before it has been used */
htab_t * initialization(token_buffer * token_buff)
{
	int err_line = 1, err_pos = 0;

	init_garbage_collector();

	int state = 0, prev_state = 0;
	while ( state != EOF) {
		token * t;
		t = get_token(stdin, &err_line, &err_pos);
		if (t->type == NEW_LINE) {
			err_line += 1;
			err_pos = 0;
		}
        else if (t->type == LEXICAL_ERROR)
        {
            error_msg(ERR_CODE_LEXICAL, "Lexical error detected\n");
        }
		prev_state = state;
		state = t->type;
		if (state != NEW_LINE || prev_state != NEW_LINE)
			add_token(token_buff, t);
	}

	return htab_init(((token_buff->len)/16)+8);		//wild guess (one sixteenth of all tokens)
}




/**********DEBUGGING FUNCTIONS AND STRUCUTRES*****************/
char *key_words3[] = { "as", "declare", "dim", "do", "double", "else", "end", "function",
					  "if", "input", "integer", "loop", "print", "return", "scope", "string",
					  "then", "while", "for" ,"next"};

char * tok_names3 [] = {"LEXICAL_ERROR", "IDENTIFIER" , "AS", "DECLARE", "DIM", "DO", "DOUBLE", "ELSE", "END", "FUNCTION", "IF", "INPUT",
		"INTEGER", "LOOP", "PRINT", "RETURN", "SCOPE", "STRING",  "THEN", "WHILE", "AND", "OR", "BOOLEAN",
		"CONTINUE", "ELSEIF", "EXIT", "FALSE", "FOR", "NEXT", "NOT",  "SHARED", "STATIC", "TRUE", "SEMICOLON", "ADD", "SUB", "MUL", "DIV", "DIV2",
		"LESS_THAN", "GREATER_THAN", "LESS_OR_EQUALS", "GREATER_OR_EQUALS", "EQUALS", "NOT_EQUALS", "LEFT_PARANTHESIS", "RIGHT_PARANTHESIS",
		"DOUBLEE", "INT_WITH_EXP", "DOUBLE_WITH_EXP", "INT_VALUE", "STRING_VALUE", "COMA", "KEY_WORD", "INT_2", "INT_8", "INT_16", "NEW_LINE",
		"ADD_EQUALS", "SUB_EQUALS", "MUL_EQUALS", "DIV_EQUALS", "DIV2_EQUALS"};


void print_token(int tk, token *t)
{
	switch (tk)
	{
		case ADD : 				printf("ADD               :+:\n"); break;
		case SUB : 				printf("SUB               :-:\n"); break;
		case DIV : 				printf("DIV               :\\:\n"); break;
		case DIV2 : 			printf("DIV2              :/:\n "); break;
		case MUL: 				printf("MUL               :*:\n "); break;
		case IDENTIFICATOR : 	printf("IDENTIFICATOR     :%s: \n", t->attr.string_value); break;
		default : 				printf("KEY_WORD          :%s: \n", tok_names3[tk]); break;
		case SEMICOLON : 		printf("SEMICOLON         :;:\n"); break;
		case LESS_THAN: 		printf("LESS_THAN         :<:\n"); break;
		case GREATER_THAN : 	printf("GREATER_THAN      :>:\n"); break;
		case LESS_OR_EQUALS : 	printf("LESS_OR_EQUALS    :<=:\n"); break;
		case GREATER_OR_EQUALS: printf("GREATER_OR_EQUALS :>=:\n"); break;
		case EQUALS: 			printf("EQUALS            :=:\n"); break;
		case NOT_EQUALS : 		printf("NOT_EQUALS        :<>:\n"); break;
		case LEFT_PARANTHESIS : printf("LEFT_PARANTHESIS  :(:\n"); break;
		case RIGHT_PARANTHESIS: printf("RIGHT_PARANTHESIS :):\n"); break;
		case DOUBLEE : 			printf("DOUBLE            :%f: \n", t->attr.double_value); break;
		case INT_WITH_EXP : 	printf("INT_WITH_EXP      :%f: \n", t->attr.double_value); break;
		case DOUBLE_WITH_EXP : 	printf("DOUBLE_WITH_EXP   :%f: \n", t->attr.double_value); break;
		case INT_VALUE : 		printf("INT_VALUE         :%d:\n", t->attr.int_value); break;
		case STRING_VALUE : 	printf("STRING_VALUE      :%s: \n", t->attr.string_value); break;
		case COMA : 			printf("COMA              :,:\n"); break;
		case NEW_LINE: 			printf("NEW_LINE          :EOL:\n"); break;
		case LEXICAL_ERROR: 	printf("LEXICAL_ERROR \n"); break;
		case EOF : 				printf("EOF \n"); break;
	}
}

