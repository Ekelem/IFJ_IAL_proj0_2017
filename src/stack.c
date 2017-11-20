#include "stack.h"

#define malloc(size) garbage_malloc(size)
#define free(addr) garbage_free(addr)
#define realloc(addr, size) garbage_realloc(addr, size)

/* Initializes dynamic stack */
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

/* Pops value from dynamic stack */
void dynamic_stack_pop(struct dynamic_stack * stack)
{
	if (!dynamic_stack_empty(stack))
		stack->actual--;
}

/* Pushes value on dynamic stack */
void dynamic_stack_push(struct dynamic_stack * stack, void * value)
{
	if (dynamic_stack_full(stack))
		dynamic_stack_realloc(stack);

	stack->start[++(stack->actual)]=value;
}

/* Searches for value in dynamic stack */
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

/* Returns top of dynamic stack */
void * dynamic_stack_top(struct dynamic_stack * stack)
{
	return stack->start[stack->actual];
}

/* Reallocates memory for dynamic stack */
void dynamic_stack_realloc(struct dynamic_stack * stack)
{
	if (realloc(stack->start, stack->size + STACK_ALLOC_STEP)==NULL)
		error_msg(ERR_CODE_INTERN, "Could not realloc on address %d (%d bytes)", stack->start, stack->size + STACK_ALLOC_STEP);

	stack->size+=STACK_ALLOC_STEP;
}

/* Checks if dynamic stack is empty */
bool dynamic_stack_empty(struct dynamic_stack * stack)
{
	return (stack->actual == -1);
}

/* Checks if dynamic stack is empty */
bool dynamic_stack_full(struct dynamic_stack * stack)
{
	return (stack->actual == stack->size);
}


/********************************************/
/****************EXPRESSSION*****************/
/********************************************/

/* Initializes stack for expressions */
void stack_init (TStack *s) {
	s->First = NULL;
	s->Last = NULL;
}

/* Pushes token on stack */
void push_expr_token (TStack *s, token *t) {
	TSElem *new = malloc(sizeof(TSElem));
	if (new == NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", sizeof(TSElem));
	new->t_elem = t;
	new->is_valid = true;
	if (t->type == DOUBLEE || t->type == DOUBLE_WITH_EXP || t->type == INT_WITH_EXP)
		new->conv_double = true;
	else
		new->conv_double = false;
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

/* Pops last element from stack */
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

/* Pops first element from stack */
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

/* Returns last element on stack */
token *peek_last_expr(TStack *s) {
	if (!(SEmpty(s))) {
		return s->Last->t_elem;
	}
	return NULL;
}

/* Returns first element on stack */
token *peek_first_expr(TStack *s) {
	if (!(SEmpty(s))) {
		return s->First->t_elem;
	}
	return NULL;
}

/* Checks if stack is empty */
bool SEmpty (TStack *s) {
	return (s->First == NULL);
}

/* Prints the whole stack */
void print_stack(TStack *s) {
	if (s->First == NULL)
		printf("NULL\n");

	TSElem *tmp = s->First;
	while (tmp != NULL) {
		printf("%d\n", tmp->t_elem->type);
		tmp = tmp->next;
	}
}

/* Deletes current expression */
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

/* Counts number of stack elements */
int stack_counter(TStack *s) {
	int counter = 0;
	TSElem *tmp = s->First;
	while (tmp != NULL) {
		counter++;
		tmp = tmp->next;
	}
	return counter;
}

/* Frees memory of whole stack */
void dealloc_tstack(TStack *s) {
	TSElem *tmp = s->First;
	while (tmp != NULL) {
		s->First = s->First->next;
		free(tmp);
		tmp = s->First;
	}
}

/* Counts number of stack elements. Checks validity of counter*/
int stack_valid_counter(TStack *s) {
	int counter = 0;
	TSElem *tmp = s->First;
	while (tmp != NULL) {
		if (tmp->is_valid)
			counter++;
		tmp = tmp->next;
	}
	return counter;
}

/* Copies whole stack into another stack */
TStack stack_copy(TStack *s) {
	TSElem *tmp = s->First;
	TStack new;
	stack_init(&new);

	while (tmp != NULL) {
		push_expr_token(&new, tmp->t_elem);
		tmp = tmp->next;
	}

	return new;
}

/* Initializes BStack */
void BInit (BStack *s) {
	s->First = NULL;
}

/* Pushes boolean value on stack */
void BPush (BStack *s, bool is_bool_value) {
	BSElem *new = malloc(sizeof(BSElem));
	new->is_bool_value = is_bool_value;
	new->next = s->First;
	s->First = new;
}

/* Pops boolean value on stack */
void BPop (BStack *s) {
	if (s->First != NULL) {
		BSElem *tmp = s->First;
		s->First = s->First->next;
		free(tmp);
	}
}

/* Returns boolean value on top of the BStack */
bool BTop (BStack *s) {
	if (s->First == NULL)
		exit(42);
	return s->First->is_bool_value;
}

/* Checks if BStack is empty */
bool BEmpty (BStack *s) {
	return (s->First == NULL);
}

/* Checks two first in row values if they are boolean*/
bool BTop_equals(BStack *s) {
	if (s->First != NULL && s->First->next != NULL) {
		if (s->First->is_bool_value && s->First->next->is_bool_value){
			BSElem *tmp = s->First;
			s->First = s->First->next;
			free(tmp);
			return true;
		}
	}

	return false;
}

/* Frees whole BStack*/
void dealloc_BStack(BStack *s) {
	BSElem *tmp = s->First;
	while (tmp != NULL) {
		s->First = s->First->next;
		free(tmp);
		tmp = s->First;
	}
	s->First = NULL;
}

/* Prints whole BStacl*/
void print_BStack(BStack *s) {
	BSElem *tmp = s->First;
	while (tmp != NULL) {
		printf("%d\n", tmp->is_bool_value);
		tmp = tmp->next;
	}
}