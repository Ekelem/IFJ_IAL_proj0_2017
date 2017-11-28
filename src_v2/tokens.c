#include "tokens.h"


token_buffer * init_token_buffer() {
	token_buffer *t = malloc(sizeof(token_buffer));
	if(t==NULL)
	{
		fprintf(stderr, "Not enough memory\n");
		exit(MALLOC_ERROR);

	}
	if ((t->arr = (token **)calloc(DEFAULT_TOKEN_MALLOC, sizeof(token*))) == NULL) {
		fprintf(stderr, "Not enough memory\n");
		exit(MALLOC_ERROR);
	}
	t->actual = 0;
	t->len = 0;
	t->size = DEFAULT_TOKEN_MALLOC;
	return t;
}

void add_token(token_buffer *t, token *elem) {
	if (t->arr != NULL && elem != NULL) {
		if (t->len+1  >= t->size) {
			if ((t->arr = realloc(t->arr, (t->size + DEFAULT_TOKEN_MALLOC) * sizeof(token*))) == NULL) { //problem
				fprintf(stderr, "Not enough memory\n");
				exit(MALLOC_ERROR);
			}
			t->size = t->size + DEFAULT_TOKEN_MALLOC;
		}
	}

	t->arr[t->len] = elem;
	t->len += 1;
}

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

token * token_buffer_get_token(token_buffer *t)
{
	if (t->actual < t->len)
		return t->arr[t->actual++];
	else
		return NULL;
}

token * token_buffer_unget_token(token_buffer *t)
{
	return t->arr[t->actual--];
}

token * token_buffer_peek_token(token_buffer *t)
{
	if ((t->actual) < t->len)
		return t->arr[(t->actual)];
	else
		return NULL;
}

token * token_buffer_get_next(token_buffer *t, int counter)
{
	if ((t->actual) < t->len)
		return t->arr[(t->actual) + counter];
	else
		return NULL;
}

token * token_buffer_get_prev(token_buffer *t, int counter)
{
	return t->arr[(t->actual) - counter];
}

token * token_buffer_next_token(token_buffer *t)
{
	if ((t->actual) < t->len)
		return t->arr[(t->actual)+1];
	else
		return NULL;
}


