#ifndef H_GARBAGE_COLLECTOR
#define H_GARBAGE_COLLECTOR 42

#include <stdint.h>
#include "stack.h"
#include "common.h"

extern struct dynamic_stack * GARBAGE_COLLECTOR;

void init_garbage_collector();
void init_freed_stack();
void * garbage_malloc(size_t size);
void garbage_collect();
void garbage_free(void * addr);
void * garbage_realloc(void * addr, size_t new_size);
struct dynamic_stack * get_stack_of_freed_addr();

void garbage_col_push(void * value);
void garbage_col_free_push(void * value);

#endif