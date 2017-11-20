#ifndef TOKENS_H
#define TOKENS_H

#include <stdlib.h>
#include "scanner.h"

#define DEFAULT_TOKEN_MALLOC 256
#define MALLOC_ERROR 60

typedef struct {
	unsigned actual;
	unsigned len;
	unsigned size;
	token **arr;
} token_buffer;


/* Initializes token buffer */
token_buffer *  init_token_buffer();

/* Adds token to token buffer */
void add_token(token_buffer *t, token *elem);

/* Frees tokens from token buffer */
void free_tokens(token_buffer *t);

/* Returns next token from token buffer and pops it*/
token * token_buffer_get_token(token_buffer *t);

/*Returns next token from token buffer */
token * token_buffer_peek_token(token_buffer *t);

#endif