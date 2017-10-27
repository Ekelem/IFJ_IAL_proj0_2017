#include "garbage_collector.h"

void init_garbage_collector()
{
	GARBAGE_COLLECTOR=malloc(sizeof(struct dynamic_stack));
	if (GARBAGE_COLLECTOR==NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", sizeof(struct dynamic_stack));

	GARBAGE_COLLECTOR->start=malloc(STACK_ALLOC_STEP);
	if (GARBAGE_COLLECTOR->start==NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", STACK_ALLOC_STEP);

	GARBAGE_COLLECTOR->actual=-1;
	GARBAGE_COLLECTOR->size=STACK_ALLOC_STEP;

	garbage_col_push(GARBAGE_COLLECTOR->start);		//index 0 -> GARBAGE_COLLECTOR->start[0]
	
	init_freed_stack();
}

void init_freed_stack()
{
	struct dynamic_stack * freed_stack=malloc(sizeof(struct dynamic_stack));
	if (freed_stack==NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", sizeof(struct dynamic_stack));

	garbage_col_push((void*)freed_stack);		//index 1 -> GARBAGE_COLLECTOR->start[1]

	freed_stack->start=malloc(STACK_ALLOC_STEP);
	if (freed_stack->start==NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", STACK_ALLOC_STEP);

	garbage_col_push((void*)freed_stack->start);	//index 2 -> GARBAGE_COLLECTOR->start[2]

	freed_stack->actual=-1;
	freed_stack->size=STACK_ALLOC_STEP;
}

void * garbage_malloc(size_t size)
{
	void* result=malloc(size);
	if (result==NULL)
		error_msg(ERR_CODE_INTERN, "Could not allocate %d bytes", size);

	if (dynamic_stack_empty(get_stack_of_freed_addr()))
	{
		garbage_col_push(result);
	}
	else
	{
		void ** freed_ptr = (void**)dynamic_stack_top(get_stack_of_freed_addr());
		*freed_ptr=result;
	}
	return result;
}

void garbage_free(void * addr)
{
	void ** found = dynamic_stack_search(GARBAGE_COLLECTOR, addr);
	if (found == NULL)
		;//something really really really bad
	else
	{
		*found = NULL;
		free(addr);
		garbage_col_free_push((void*)found);
	}
}

void * garbage_realloc(void * addr, size_t new_size)
{
	void ** found = dynamic_stack_search(GARBAGE_COLLECTOR, addr);
	void * new_addr;
	if (found == NULL)
	{
		new_addr = malloc(new_size);//something really really really bad
	}
	else
	{
		*found = NULL;
		new_addr = realloc(addr, new_size);
		garbage_col_free_push((void*)found);
	}

	garbage_col_push(new_addr);
	return new_addr;
}

void garbage_collect()
{
	while (GARBAGE_COLLECTOR->actual!=-1)
	{
		void * addr = dynamic_stack_top(GARBAGE_COLLECTOR);
		if (addr!=NULL)
			free(dynamic_stack_top(GARBAGE_COLLECTOR));

		dynamic_stack_pop(GARBAGE_COLLECTOR);
	}
	free(GARBAGE_COLLECTOR);
}

struct dynamic_stack * get_stack_of_freed_addr()
{
	return GARBAGE_COLLECTOR->start[1];
}

void garbage_col_push(void * value)
{
	if (dynamic_stack_full(GARBAGE_COLLECTOR))
	{
		if (realloc(GARBAGE_COLLECTOR->start, GARBAGE_COLLECTOR->size + STACK_ALLOC_STEP)==NULL)
			error_msg(ERR_CODE_INTERN, "Could not realloc on address %d (%d bytes)", GARBAGE_COLLECTOR->start, GARBAGE_COLLECTOR->size + STACK_ALLOC_STEP);

		GARBAGE_COLLECTOR->size+=STACK_ALLOC_STEP;
		GARBAGE_COLLECTOR->start[0]=GARBAGE_COLLECTOR->start;
	}

	GARBAGE_COLLECTOR->start[++GARBAGE_COLLECTOR->actual]=value;
}

void garbage_col_free_push(void * value)
{
	if (dynamic_stack_full(get_stack_of_freed_addr()))
	{
		if (realloc(get_stack_of_freed_addr()->start, get_stack_of_freed_addr()->size + STACK_ALLOC_STEP)==NULL)
			error_msg(ERR_CODE_INTERN, "Could not realloc on address %d (%d bytes)", get_stack_of_freed_addr()->start, get_stack_of_freed_addr()->size + STACK_ALLOC_STEP);

		get_stack_of_freed_addr()->size+=STACK_ALLOC_STEP;
		GARBAGE_COLLECTOR->start[2]=get_stack_of_freed_addr()->start;
	}

	get_stack_of_freed_addr()->start[++(get_stack_of_freed_addr()->actual)]=value;
}