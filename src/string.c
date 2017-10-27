#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"

#define ALLOC_ERROR 60
#define DEFAULT_STR_ALLOC 64

void init_string(String * s)
{
	if ((s->str = (char *) malloc(sizeof(char) * DEFAULT_STR_ALLOC)) == NULL)
		print_error();
	s->alloc_size = DEFAULT_STR_ALLOC;
	s->len = 0;
	s->str[0] = '\0';
}

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

void append_str_to_str(String *s, const char * append)
{
	if (s->len + strlen(append) >= s->alloc_size)
	{
		if ((s->str = (char *) realloc(s->str, (s->len + DEFAULT_STR_ALLOC) * sizeof(char))) == NULL)
			print_error();
		s->alloc_size += DEFAULT_STR_ALLOC;
	}
	s->len += strlen(append);
	strcat(s->str, append);
	//s->str[s->len] = '\0';
}

void clear_string(String *s)
{
	s->str[0] = '\0';
	s->len = 0;
}

void free_string(String *s)
{
	s->len = 0;
	s->alloc_size = 0;
	free(s->str);
}

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


int str_cmp_str(String *s1, String *s2)
{
	return strcmp(s1->str, s2->str);
}

int str_cmp_conststr(String *s1, const char *s2)
{
	return strcmp(s1->str, s2);
}

int str_len(String *s)
{
	return s->len;
}

void print_error()
{
	printf("Error while allocating memory\n");
	exit(ALLOC_ERROR);
}

void print_string_info(String *s)
{
	if( str_len(s) != 0) {
		printf("%s\n", s->str);
	}
	printf("Length: %d\nAllocated size: %d\n==========\n", s->len, s->alloc_size);
}