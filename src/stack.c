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