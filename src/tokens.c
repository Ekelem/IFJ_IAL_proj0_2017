#include "tokens.h"

/* Initializes token buffer */
token_buffer * init_token_buffer() {
	token_buffer *t = malloc(sizeof(token_buffer));
	if ((t->arr = (token **)calloc(DEFAULT_TOKEN_MALLOC, sizeof(token))) == NULL) {
		fprintf(stderr, "Not enough memory\n");
		exit(MALLOC_ERROR);
	}
	t->actual = 0;
	t->len = 0;
	t->size = DEFAULT_TOKEN_MALLOC;
	return t;
}

/* Adds token to token buffer */
void add_token(token_buffer *t, token *elem) {
	if (t->arr != NULL && elem != NULL) {
		if (t->len+1  >= t->size) {
			if ((t->arr = realloc(t->arr, t->size + DEFAULT_TOKEN_MALLOC * sizeof(token))) == NULL) {
				fprintf(stderr, "Not enough memory\n");
				exit(MALLOC_ERROR);
			}
			t->size = t->size + DEFAULT_TOKEN_MALLOC;
		}
	}

	t->arr[t->len] = elem;
	t->len += 1;
}

/* Frees tokens from token buffer */
void free_tokens(token_buffer *t) {
	if (t->arr) {
		for(unsigned i = 0; i < t->size; i++) {
			if (t->arr[i] != NULL)
			{
				if (t->arr[i]->type == STRING_VALUE || t->arr[i]->type == IDENTIFIER)
					free(t->arr[i]->attr.string_value);

			free(t->arr[i]);
			}
		}
		free(t->arr);
		free(t);
	}
}

/* Returns next token from token buffer and pops it*/
token * token_buffer_get_token(token_buffer *t)
{
	if (t->actual < t->len)
		return t->arr[t->actual++];
	else
		return NULL;
}

/*Returns next token from token buffer */
token * token_buffer_peek_token(token_buffer *t)
{
	if ((t->actual) < t->len)
		return t->arr[(t->actual)];
	else
		return NULL;
}
