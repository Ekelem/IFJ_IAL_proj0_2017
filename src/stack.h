#ifndef STACK_H
#define STACK_H 42

#include "error.h"
#include "garbage_collector.h"
#include <stdint.h>

#include <stdbool.h>

#define STACK_ALLOC_STEP 1024

struct dynamic_stack
{
	void ** start;
	uint32_t actual;
	size_t size;
};

struct dynamic_stack * dynamic_stack_init();
void dynamic_stack_pop(struct dynamic_stack * stack);
void * dynamic_stack_top(struct dynamic_stack * stack);
void dynamic_stack_push(struct dynamic_stack * stack, void * value);
void dynamic_stack_realloc(struct dynamic_stack * stack);
void ** dynamic_stack_search(struct dynamic_stack * stack, void * value);

bool dynamic_stack_empty(struct dynamic_stack * stack);

bool dynamic_stack_full(struct dynamic_stack * stack);

#endif