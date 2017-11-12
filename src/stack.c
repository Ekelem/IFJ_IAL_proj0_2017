#include "stack.h"

#define malloc(size) garbage_malloc(size)
#define free(addr) garbage_free(addr)
#define realloc(addr, size) garbage_realloc(addr, size)

struct dynamic_stack * dynamic_stack_init()
{
	struct dynamic_stack * result = malloc(sizeof(struct dynamic_stack));
	if (result==NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", sizeof(struct dynamic_stack));
	
	result->start=malloc(STACK_ALLOC_STEP);
	result->actual=-1;
	result->size=STACK_ALLOC_STEP;
	return result;
}
void dynamic_stack_pop(struct dynamic_stack * stack)
{
	if (!dynamic_stack_empty(stack))
		stack->actual--;
}

void dynamic_stack_push(struct dynamic_stack * stack, void * value)
{
	if (dynamic_stack_full(stack))
		dynamic_stack_realloc(stack);

	stack->start[++(stack->actual)]=value;
}

void ** dynamic_stack_search(struct dynamic_stack * stack, void * value)
{
	int temp_search;
	for (temp_search=stack->actual;temp_search>=0;--temp_search)
	{
		if (stack->start[temp_search]==value)
		{
			return &stack->start[temp_search];
		}
	}
	return NULL;
}

void * dynamic_stack_top(struct dynamic_stack * stack)
{
	return stack->start[stack->actual];
}

void dynamic_stack_realloc(struct dynamic_stack * stack)
{
	if (realloc(stack->start, stack->size + STACK_ALLOC_STEP)==NULL)
		error_msg(ERR_CODE_INTERN, "Could not realloc on address %d (%d bytes)", stack->start, stack->size + STACK_ALLOC_STEP);

	stack->size+=STACK_ALLOC_STEP;
}

bool dynamic_stack_empty(struct dynamic_stack * stack)
{
	return (stack->actual == -1);
}

bool dynamic_stack_full(struct dynamic_stack * stack)
{
	return (stack->actual == stack->size);
}


/********************************************/
/****************EXPRESSSION*****************/
/********************************************/

void stack_init (TStack *s) {
	s->First = NULL;
	s->Last = NULL;
}

void push_expr_token (TStack *s, token *t) {
	TSElem *new = malloc(sizeof(TSElem));
	if (new == NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", sizeof(TSElem));
	new->t_elem = t;
	new->is_valid = TRUE;
	new->next = NULL;
	new->prev = s->Last;
	if (s->First == NULL) {
		s->First = new;
	}
	else {
		s->Last->next = new;
	}
	s->Last = new;
}

void pop_last_expr(TStack *s) {
	if (s->Last != NULL) {
		TSElem *tmp = s->Last;

		if (s->First == s->Last) {
			s->First = NULL;
			s->Last = NULL;
			free(tmp);
		}
		else {
			s->Last->prev->next = NULL;
			s->Last = s->Last->prev;
			free(tmp);
		}
	}
}

void pop_first_expr(TStack *s) {
	if (!SEmpty(s)) {
		TSElem *tmp = s->First;


		if (s->First == s->Last) {
			s->First = NULL;
			s->Last = NULL;
			free(tmp);
		}
		else {
			s->First->next->prev = NULL;
			s->First = s->First->next;
			free(tmp);
		}
	}
}

token *peek_last_expr(TStack *s) {
	if (!(SEmpty(s))) {
		return s->Last->t_elem;
	}
	return NULL;
}

token *peek_first_expr(TStack *s) {
	if (!(SEmpty(s))) {
		return s->First->t_elem;
	}
	return NULL;
}


bool SEmpty (TStack *s) {
	return (s->First == NULL);
}

void print_stack(TStack *s) {
	if (s->First == NULL)
		printf("NULL\n");

	TSElem *tmp = s->First;
	while (tmp != NULL) {
		printf("%d\n", tmp->t_elem->type);
		tmp = tmp->next;
	}
}

void delete_current_expr(TSElem *s) {
	if (s != NULL) {
		TSElem *tmp = s;
		if (tmp->prev != NULL) {
			tmp->prev->next = tmp->next;
		}
		if (tmp->next != NULL) {
			tmp->next->prev = tmp->prev;
		}
		free(tmp);
	}
}

int stack_counter(TStack *s) {
	int counter = 0;
	TSElem *tmp = s->First;
	while (tmp != NULL) {
		counter++;
		tmp = tmp->next;
	}
	return counter;
}

void dealloc_tstack(TStack *s) {
	TSElem *tmp = s->First;
	while (tmp != NULL) {
		s->First = s->First->next;
		free(tmp);
		tmp = s->First;
	}
}