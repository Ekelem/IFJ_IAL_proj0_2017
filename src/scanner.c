/*
 *	IFJ/IAL project 2017
 *	author/s: Attila Lakatos
*/

//LEXICAL ANALYSIS

//Read tokens from input file (code)

#include "scanner.h"

char *key_words[] = { "as", "asc", "declare", "dim", "do", "double", "else", "end", "chr", "function",
					  "if", "input", "integer", "length", "loop", "print", "return", "scope", "string",
					  "substr", "then", "while"};


int base_to_int(char *str, int base) {
	int sum = 0, tmp, power = 1;

	for (int i = (int)strlen(str)-1; i >= 0; i--) {
		tmp = (str[i] >= '0' && str[i] <= '9') ? (str[i] - '0') * power : (str[i] - 'A' + 10)*power ;
		power *= base;
		sum += tmp;
	}
	return sum;
}

bool is_keyword(char *str, token *t)
{
	if (strlen(str) > 8)
		return false;
	char str_low[9] = "";
	for (unsigned i = 0; i < strlen(str); i++)
		str_low[i] = tolower(str[i]);
	str_low[strlen(str)+1] = '\0';
	for (int i = 0; i < 22; i++)
	{
		if(strcmp(key_words[i], str_low) == 0) {
			t->type = i + 2;
			return true;
		}
	}
	return false;
}

bool is_validID(char *str)
{
	if (str != NULL && strlen(str) != 0)
	{
		if (isalpha(str[0]) || str[0] == '_' )
		{
			for (unsigned i = 1; i < strlen(str); i++)
			{
				if ( !isalpha(str[i]) && !isdigit(str[i]) && str[i] != '_')
					return false;
			}
			return true;
		}
	}

	return false;
}

token * save_token(token *t, String *str, int type)
{
	if (type != KEY_WORD){
		t->type = type;
	}
	if (str == NULL) {
		return t;
	}
	else {
		switch (type)
		{
			case INT_VALUE:
				t->attr.int_value = atoi(str->str);
				free_string(str);
				break;
			case INT_2:
				t->attr.int_value = base_to_int(str->str, 2);
				t->type = INT_VALUE;
				type = INT_VALUE;
				free_string(str);
				break;
			case INT_8:
				t->attr.int_value = base_to_int(str->str, 8);
				t->type = INT_VALUE;
				type = INT_VALUE;
				free_string(str);
				break;
			case INT_16:
				t->attr.int_value = base_to_int(str->str, 16);
				t->type = INT_VALUE;
				type = INT_VALUE;
				free_string(str);
				break;

			case DOUBLEE:
			case DOUBLE_WITH_EXP:
			case INT_WITH_EXP:
				t->attr.double_value = atof(str->str);
				free_string(str);
				break;

			case IDENTIFIER:
			case STRING_VALUE:
				t->attr.string_value = str->str;
				break;
		}
	}
	return t;
}

token * get_token(FILE *f)
{
	int state = WHITE_SPACE;
	int c;
	token *t= malloc(sizeof(token));

	bool exponent = false;

	String ascii_seq;
	String s;

	do
	{
		c = fgetc(f);
		switch (state)
		{
			case WHITE_SPACE:
				if(!isspace(c))
				{
					if(isalpha(c) || c == '_') {
						init_string(&s);
						append_char_to_str(&s, c);
						state = IDENTIFICATOR;
					}
					else if (isdigit(c)) {
						init_string(&s);
						append_char_to_str(&s, c);
						state = NUMBER;
					}
					else
					{
						switch (c)
						{
							case '/':  state = DIV_OR_COMMENT;   break;
							case '<':  state = COMPARE_LESS;     break;
							case '>':  state = COMPARE_GREATER;  break;
							case '!':  state = EXCLAMATION_MARK; break;
							case '.':  state = DOUBLE_1; init_string(&s);        break;
							case '\'': state = LINE_COMMENT;     break;
							case '&':  state = BASE;	break;
							case '+':  state = ADD_EQ; 	break;
							case '-':  state = SUB_EQ;	break;
							case '*':  state = MUL_EQ;	break;
							case '\\': state = DIV2_EQ; break;
							case ',':  return save_token(t, NULL, COMA);
							case '=':  return save_token(t, NULL, EQUALS);
							case ';':  return save_token(t, NULL, SEMICOLON);
							case '(':  return save_token(t, NULL, LEFT_PARANTHESIS);
							case ')':  return save_token(t, NULL, RIGHT_PARANTHESIS);
							case '\"': return save_token(t, NULL, LEXICAL_ERROR);
						}
					}
				}
				else if (c == '\n') {
					return save_token(t, NULL, NEW_LINE);
				}
				if (state == WHITE_SPACE && !isspace(c) && c != EOF)
					return save_token(t, NULL, LEXICAL_ERROR);
				break;
			case NUMBER:
				if ( isdigit(c))
					append_char_to_str(&s, c);
				else if ( c == '.') {
					state = DOUBLE_1;
					append_char_to_str(&s, c);
				}
				else if (c == 'e' || c == 'E') {
					state = DOUBLE_2;
					append_char_to_str(&s, c);
				}
				else {
					ungetc(c, f);
					return save_token(t, &s, INT_VALUE);
				}
				break;

			case DOUBLE_1:
				if (isdigit(c))
					append_char_to_str(&s, c);
				else if (c == 'e' || c == 'E') {
					state = DOUBLE_3;
					append_char_to_str(&s, c);
				}
				else {
					ungetc(c, f);
					return save_token(t, &s, DOUBLEE);
				}
				break;

			case DOUBLE_2:
				if (isdigit(c)) {
					append_char_to_str(&s, c);
				}
				else if (isalpha(c)){
					return save_token(t, NULL, LEXICAL_ERROR);
				}
				else if (c == '+' || c == '-'){
					if (exponent)
						return save_token(t, NULL, LEXICAL_ERROR);
					exponent = true;
					append_char_to_str(&s, c);
				}
				else {
					ungetc(c, f);
					return save_token(t, &s, INT_WITH_EXP);
				}
				break;

			case DOUBLE_3:
				if (isdigit(c)) {
					append_char_to_str(&s, c);
				}
				else if (isalpha(c)){
					return save_token(t, NULL, LEXICAL_ERROR);
				}
				else if (c == '+' || c == '-'){
					if (exponent)
						return save_token(t, NULL, LEXICAL_ERROR);
					exponent = true;
					append_char_to_str(&s, c);
				}
				else {
					ungetc(c, f);
					return save_token(t, &s, DOUBLE_WITH_EXP);
				}
				break;

			case IDENTIFICATOR:
				if (isalpha(c) || isdigit(c) || c == '_') {
					append_char_to_str(&s, c);
				}
				else {
					if (is_keyword(s.str, t)) {
						ungetc(c, f);
						free_string(&s);
						return save_token(t, NULL, KEY_WORD);
					}
					else if ( is_validID(s.str)) {
						ungetc(c, f);
						return save_token(t, &s, IDENTIFIER);
					}
					else {
						return save_token(t, NULL, LEXICAL_ERROR);
					}
				}
				break;

			case COMPARE_LESS:
				if ( c == '=')
					return save_token(t, NULL, LESS_OR_EQUALS);
				else if (c == '>')
					return save_token(t, NULL, NOT_EQUALS);
				else {
					ungetc(c,f);
					return save_token(t, NULL, LESS_THAN);
				}

			case COMPARE_GREATER:
				if ( c == '=')
					return save_token(t, NULL, GREATER_OR_EQUALS);
				else {
					ungetc(c, f);
					return save_token(t, NULL, GREATER_THAN);
				}

			case EXCLAMATION_MARK:
				if (c == '"') {
					state = STRING_LITERAL_BEGINS;
					init_string(&s);
				}
				else {
					return save_token(t, NULL, LEXICAL_ERROR);
				}
				break;

			case STRING_LITERAL_BEGINS:
				if (c == '"') {
					return save_token(t, &s, STRING_VALUE);
				}
				else if (c == EOF || c == '\n') {
					return save_token(t, &s, LEXICAL_ERROR);
				}
				else if (c == '\\') {
					state = UNUSUAL_CHAR;
				}
				else {
					append_char_to_str(&s, c);
				}
				break;

			case UNUSUAL_CHAR:
				if (c == '"') {
					state = STRING_LITERAL_BEGINS;
					append_char_to_str(&s, '\"');
				}
				else if (c == 'n') {
					state = STRING_LITERAL_BEGINS;
					append_char_to_str(&s, '\n');
				}
				else if (c == 't') {
					state = STRING_LITERAL_BEGINS;
					append_char_to_str(&s, '\t');
				}
				else if (c == '\\') {
					state = STRING_LITERAL_BEGINS;
					append_char_to_str(&s, '\\');
				}
				else if (isdigit(c)){ //ASCII sequence
					ungetc(c, f);
					init_string(&ascii_seq);
					state = UNUSUAL_CHAR_2;
				}
				else {
					return save_token(t, NULL, LEXICAL_ERROR);
				}
				break;

			case UNUSUAL_CHAR_2:
				if ( ascii_seq.len == 3)
				{
					ungetc(c,f);
					append_char_to_str(&s, atoi(ascii_seq.str));
					free_string(&ascii_seq);
					state = STRING_LITERAL_BEGINS;
				}
				else if (isdigit(c)){
					append_char_to_str(&ascii_seq, c);
					state = UNUSUAL_CHAR_2;
				}
				else if ( isalpha(c) || c == ' ' || c == '"' || c == '\\')
				{
					if (ascii_seq.len < 3)
                        			return save_token(t, NULL, LEXICAL_ERROR);
                    			ungetc(c, f);
					append_char_to_str(&s, atoi(ascii_seq.str));
					free_string(&ascii_seq);
					state = STRING_LITERAL_BEGINS;
				}
				else {
					return save_token(t, NULL, LEXICAL_ERROR);
				}
				break;

			case LINE_COMMENT:
				if (c == '\n') {
					return save_token(t, NULL, NEW_LINE);
				}
				break;

			case DIV_OR_COMMENT:
				if (c == '\'') {
					state = BLOCK_COMMENT;
				}
				else {
					if (c == '='){
						return save_token(t, NULL, DIV_EQUALS);
					}
					else {
						ungetc(c, f);
						return save_token(t, NULL, DIV);
					}
				}
				break;

			case BLOCK_COMMENT:
				if (c == '\'') {
					state = BLOCK_COMMENT_END;
				}
				break;

			case BLOCK_COMMENT_END:
				if (c == '/') {
					state = WHITE_SPACE;
				}
				else {
					state = BLOCK_COMMENT;
				}
				break;

			case BASE:
				if (toupper(c) == 'B') {
					state = BASE_2;
					init_string(&s);
				}
				else if (toupper(c) == 'O') {
					init_string(&s);
					state = BASE_8;
				}
				else if (toupper(c) == 'H') {
					init_string(&s);
					state = BASE_16;
				}
				else
					return save_token(t, NULL, LEXICAL_ERROR);
				break;

			case BASE_2:
				if (c >= '0' &&  c <= '1'){
					append_char_to_str(&s, c);
				}
				else if ((c >= '2' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') ) {
					return save_token(t, NULL, LEXICAL_ERROR);
				}
				else {
					ungetc(c, f);
					return save_token (t, &s, INT_2);
				}
				break;

			case BASE_8:
				if (c >= '0' &&  c <= '7')
					append_char_to_str(&s, c);
				else if ((c >= '8' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
					return save_token(t, NULL, LEXICAL_ERROR);
				}
				else {
					ungetc(c, f);
					return save_token (t, &s, INT_8);
				}
				break;

			case BASE_16:
				if ((c >= '0' &&  c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
					append_char_to_str(&s, toupper(c));
				else {
					ungetc(c, f);
					return save_token (t, &s, INT_16);
				}
				break;

			case ADD_EQ:
				if (c == '=') {
					return save_token(t, NULL, ADD_EQUALS);
				}
				else {
					ungetc(c, f);
					return save_token(t, NULL, ADD);
				}
				break;

			case SUB_EQ:
				if (c == '=') {
					return save_token(t, NULL, SUB_EQUALS);
				}
				else {
					ungetc(c, f);
					return save_token(t, NULL, SUB);
				}
				break;

			case MUL_EQ:
				if (c == '=') {
					return save_token(t, NULL, MUL_EQUALS);
				}
				else {
					ungetc(c, f);
					return save_token(t, NULL, MUL);
				}
				break;

			case DIV2_EQ:
				if (c == '=') {
					return save_token(t, NULL, DIV2_EQUALS);
				}
				else {
					ungetc(c, f);
					return save_token(t, NULL, DIV2);
				}
				break;

		}
		if (c == EOF)
			break;

	}while(c != EOF);

	return save_token(t, NULL, EOF);
}
