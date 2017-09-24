#ifndef STRING_H
#define STRING_H


typedef struct
{
	char *str;
	int alloc_size;
	int len;
} String;

void init_string(String *s);
void append_char_to_str(String *s, char c);
void clear_string(String *s);
void free_string(String *s);
void str_copy_str(String *s1, String *s2);

int str_cmp_str(String *s1, String *s2);
int str_cmp_conststr(String *s1, const char *s2);
int str_len(String *s);

void print_error();
void print_string_info(String *s);

#endif