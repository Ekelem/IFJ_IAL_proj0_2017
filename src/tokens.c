#include "tokens.h"
#include <malloc.h>
#include <stdlib.h>


void init_token_buffer(token_buffer *t) {
	if ((t->arr = (token **)calloc(DEFAULT_TOKEN_MALLOC, sizeof(token))) == NULL) {
		fprintf(stderr, "Not enough memory\n");
		exit(MALLOC_ERROR);
	}

	t->len = 0;
	t->size = DEFAULT_TOKEN_MALLOC;
}

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

void free_tokens(token_buffer *t) {
	if (t->arr) {
		for(unsigned i = 0; i < t->size; i++) {
			free(t->arr[i]);
		}
		free(t->arr);
	}
}
