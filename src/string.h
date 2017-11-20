#ifndef STRING_H
#define STRING_H


typedef struct
{
	char *str;
	int alloc_size;
	int len;
} String;

/* Initializes string */
void init_string(String *s);

/* Appends character to string at the begining*/
void first_append_char_to_str(String *s, char c);

/* Appends character to string at the end*/
void append_char_to_str(String *s, char c);

/* Appends string to string */
void append_str_to_str(String *s, const char * append);

/* Clears string. Sets it value to zero character.*/
void clear_string(String *s);

/* Frees memory of string */
void free_string(String *s);

/* Copies value of first string to second string */
void str_copy_str(String *s1, String *s2);

/* Checks if string are the same */
int str_cmp_str(String *s1, String *s2);

/* Checks if string is same with set constant string value */
int str_cmp_conststr(String *s1, const char *s2);

/* Returns string length */
int str_len(String *s);

/* Prints error about allocating */
void print_error();

/* Prints string informations */
void print_string_info(String *s);

#endif