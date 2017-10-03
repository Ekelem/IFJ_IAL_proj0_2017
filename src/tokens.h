#ifndef TOKENS_H
#define TOKENS_H

#define DEFAULT_TOKEN_MALLOC 256
#define MALLOC_ERROR 60


typedef union attribute {
	int int_value;
	double double_value;
	char *string_value;
} attribute;

typedef struct {
	int type;
	attribute attr;
} token;

typedef struct {
	unsigned len;
	unsigned size;
	token **arr;
} token_buffer;


void init_token_buffer(token_buffer *t);
void add_token(token_buffer *t, token *elem);
void free_tokens(token_buffer *t);

#endif