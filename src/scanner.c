/*
 *	IFJ/IAL project 2017
 *	author/s: Attila Lakatos
*/

//LEXICAL ANALYSIS

//Read tokens from input file (code)

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "scanner.h"
#include "string.h"

char *key_words[] = { "as", "asc", "declare", "dim", "do", "double", "else", "end", "chr", "function",
					  "if", "input", "integer", "length", "loop", "print", "return", "scope", "string",
					  "substr", "then", "while"};


bool is_keyword(char *str)
{
	if (strlen(str) > 8)
		return false;
	char str_low[9] = "";
	for (unsigned i = 0; i < strlen(str); i++)
		str_low[i] = tolower(str[i]);
	str_low[strlen(str)+1] = '\0';
	for (int i = 0; i < 22; i++)
	{
		if(strcmp(key_words[i], str_low) == 0)
			return true;
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

int save_token(token *t, String *str, int type)
{
	t->type = type;
	if (str == NULL) {
		return type;
	}
	else {
		switch (type)
		{
			case INT_VALUE:
				t->attr.int_value = atoi(str->str);
				free_string(str);
				break;

			case DOUBLEE:
			case DOUBLE_WITH_EXP:
			case INT_WITH_EXP:
				t->attr.double_value = atof(str->str);
				free_string(str);
				break;

			case IDENTIFIER:
			case KEY_WORD:
			case STRING_VALUE:
				t->attr.string_value = str->str;

				break;
		}
	}
	return type;
}

int get_token(FILE *f, token *t)
{
	int state = 0;
	int c;

	String ascii_seq;
	String s;

	while( (c = fgetc(f)) != EOF)
	{
		switch (state)
		{
			case WHITE_SPACE:
				if(!isspace(c))
				{
					if(isalpha(c)) {
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
							case '+':  return save_token(t, NULL, ADD);
							case '-':  return save_token(t, NULL, SUB);
							case '*':  return save_token(t, NULL, MUL);
							case '_':  state = IDENTIFICATOR; break;
							case '/':  state = DIV_OR_COMMENT; break;
							case '\\': return save_token(t, NULL, DIV2);
							case ';':  return save_token(t, NULL, SEMICOLON);
							case '<':  state = COMPARE_LESS; break;
							case '>':  state = COMPARE_GREATER; break;
							case '=':  return save_token(t, NULL, EQUALS);
							case '(':  return save_token(t, NULL, LEFT_PARANTHESIS);
							case ')':  return save_token(t, NULL, RIGHT_PARANTHESIS);
							case '!':  state = EXCLAMATION_MARK; break;
							case '.':  state = DOUBLE_1; break;
							case '\"': return save_token(t, NULL, LEXICAL_ERROR);
							case ',':  return save_token(t, NULL, COMA);
							case '\'': state = LINE_COMMENT; break;
							case '\n': state = WHITE_SPACE; break;
						}
					}
				}
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
				if (isdigit(c))
					append_char_to_str(&s, c);
				else {
					ungetc(c, f);
					return save_token(t, &s, INT_WITH_EXP);
				}
				break;

			case DOUBLE_3:
				if (isdigit(c))
					append_char_to_str(&s, c);
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
					if (is_keyword(s.str)) {
						ungetc(c, f);
						return save_token(t, &s, KEY_WORD);
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
				if (isdigit(c)){
					append_char_to_str(&ascii_seq, c);
					state = UNUSUAL_CHAR_2;
				}
				else if (str_len(&ascii_seq) == 3 || c == ' ' || c == '"' || c == '\\')
                {
                    if (c == ' ' || c == '"' || c == '\\')
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
					state = WHITE_SPACE;
				}
				break;

			case DIV_OR_COMMENT:
				if (c == '\'') {
					state = BLOCK_COMMENT;
				}
				else {
					ungetc(c, f);
					return save_token(t, NULL, DIV);
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

		}
	}

	return save_token(t, NULL, EOF);
}