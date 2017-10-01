#ifndef TOKENS_H
#define TOKENS_H

typedef union attribute {
	int int_value;
	double double_value;
	char *string_value;	
} attribute;

typedef struct {
	int type;
	attribute attr;
} token;

typedef struct token_element {
	token tok;
	struct token_element *next;
} token_element_p;

typedef struct {
	token_element_p *First;
	token_element_p *Last;
} token_buffer;


void init_token_buffer(token_buffer *t);
void add_token(token_buffer *t, token elem);
void free_tokens(token_buffer *t);
void print_token_buff(token_buffer *t);

#endif