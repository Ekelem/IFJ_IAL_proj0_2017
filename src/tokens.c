#include "tokens.h"
#include <malloc.h>
#include <stdlib.h>

#define MALLOC_ERROR 60

void init_token_buffer(token_buffer *t) {
	t->First = NULL;
	t->Last = NULL;
}

void add_token(token_buffer *t, token elem) {
	if (t != NULL) {
		token_element_p *tmp;
		tmp = malloc(sizeof(struct token_element));
		if (tmp == NULL) {
			fprintf(stderr, "Not enough memory\n");
			exit(MALLOC_ERROR);
		}

		tmp->next = NULL;
		tmp->tok = elem;

		if (t->First == NULL) {
			t->First = tmp;
			t->Last = tmp;
		}
		else {
			t->Last->next = tmp;
			t->Last = tmp;
		}


	}
}

void free_tokens(token_buffer *t) {
	if (t) {
		token_element_p *tmp = t->First, *prev;
		while(tmp != NULL) {
			prev = tmp;
			tmp = tmp->next;
			free(prev);
		}
	}
}
