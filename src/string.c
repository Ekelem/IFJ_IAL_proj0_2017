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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"

#define ALLOC_ERROR 60
#define DEFAULT_STR_ALLOC 64

/* Initializes string */
void init_string(String * s)
{
	if ((s->str = (char *) malloc(sizeof(char) * DEFAULT_STR_ALLOC)) == NULL)
		print_error();
	s->alloc_size = DEFAULT_STR_ALLOC;
	s->len = 0;
	s->str[0] = '\0';
}

/* Appends character to string at the end*/
void append_char_to_str(String *s, char c)
{
	if (s->len + 1 >= s->alloc_size)
	{
		if ((s->str = (char *) realloc(s->str, (s->len + DEFAULT_STR_ALLOC) * sizeof(char))) == NULL)
			print_error();
		s->alloc_size += DEFAULT_STR_ALLOC;
	}
	s->str[s->len++] = c;
	s->str[s->len] = '\0';
}

/* Appends character to string at the begining*/
void first_append_char_to_str(String *s, char c)
{
	if (s->len + 1 >= s->alloc_size)
	{
		if ((s->str = (char *) realloc(s->str, (s->len + DEFAULT_STR_ALLOC) * sizeof(char))) == NULL)
			print_error();
		s->alloc_size += DEFAULT_STR_ALLOC;
	}
	for(int i = (int)s->len+1; i > 0; i--) {
		s->str[i] = s->str[i-1];
	}
	s->str[0] = c;
	s->len++;
	s->str[s->len] = '\0';
}

/* Appends string to string */
void append_str_to_str(String *s, const char * append)
{
	if ((s->len + strlen(append)) >= s->alloc_size)
	{
		if ((strlen(append)) > DEFAULT_STR_ALLOC)
		{
			if ((s->str = (char *) realloc(s->str, (s->alloc_size + (strlen(append)+1)) * sizeof(char))) == NULL)
			print_error();
			s->alloc_size += strlen(append)+1;
		}
		else
		{
			if ((s->str = (char *) realloc(s->str, (s->alloc_size + DEFAULT_STR_ALLOC) * sizeof(char))) == NULL)
			print_error();
			s->alloc_size += DEFAULT_STR_ALLOC;
		}
	}
	s->len += strlen(append);
	strcat(s->str, append);
	//s->str[s->len] = '\0';
}

/* Clears string. Sets it value to zero character.*/
void clear_string(String *s)
{
	s->str[0] = '\0';
	s->len = 0;
}

/* Frees memory of string */
void free_string(String *s)
{
	s->len = 0;
	s->alloc_size = 0;
	free(s->str);
}

/* Copies value of first string to second string */
void str_copy_str(String *s1, String *s2)
{
	if (s1->alloc_size < s2->len)
	{
		if ((s1->str = (char *) realloc(s1->str, s2->len)) == NULL)
			print_error();
		s1->alloc_size = s2->alloc_size;
	}
	strcpy(s1->str, s2->str);
	s1->len = s2->len;
}

/* Convert int value of char to escape sequence string */
void str_convert_ascii(String *s1, int c)
{
	if(c > 255 || c < 1)
		error_msg(ERR_CODE_LEXICAL, "ASCII value is above\n");


	if (c == '#' || c == '(' || c == ')' || c == '+' || c == ']' ||
	   (c >= 45 && c <= 63) || (c >= 65 && c <= 91) || (c >= 97 && c <= 125))
	{
		append_char_to_str(s1, c);
	}
	else
	{
		char ascii[4];
		sprintf(ascii, "%d", c);
		if (c < 10)
		{
			append_str_to_str(s1, "\\00");
			append_str_to_str(s1, ascii);
		}
		else if (c >= 10 && c < 100)
		{
			append_str_to_str(s1, "\\0");
			append_str_to_str(s1, ascii);
		}
		else
		{
			append_str_to_str(s1, "\\");
			append_str_to_str(s1, ascii);
		}
	}
}

/* Checks if string are the same */
int str_cmp_str(String *s1, String *s2)
{
	return strcmp(s1->str, s2->str);
}

/* Checks if string is same with set constant string value */
int str_cmp_conststr(String *s1, const char *s2)
{
	return strcmp(s1->str, s2);
}

/* Returns string length */
int str_len(String *s)
{
	return s->len;
}

/* Prints error about allocating */
void print_error()
{
	printf("Error while allocating memory\n");
	exit(ALLOC_ERROR);
}

/* Prints string informations */
void print_string_info(String *s)
{
	if( str_len(s) != 0) {
		printf("%s\n", s->str);
	}
	printf("Length: %d\nAllocated size: %d\n==========\n", s->len, s->alloc_size);
}