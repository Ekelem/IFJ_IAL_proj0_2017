/*
 * IFJ17 Compiler Project, FIT VUT Brno 2017
 *
 * Authors:
 * Erik Kelemen    - xkelem01
 * Attila Lakatos  - xlakat01
 * Patrik Sober    - xsober00
 * Tomas Zubrik    - xzubri00
 *
 */

#ifndef H_GARBAGE_COLLECTOR
#define H_GARBAGE_COLLECTOR 42

#include <stdint.h>
#include "stack.h"
#include "common.h"

extern struct dynamic_stack * GARBAGE_COLLECTOR;

/* Initializes garbage collector*/
void init_garbage_collector();

/* Initializes dynamic stack */
void init_freed_stack();

/* Garbage allocates memory */
void * garbage_malloc(size_t size);

/* Frees unfreed memory in dynamic stack*/
void garbage_collect();

/* Garbage frees memory */
void garbage_free(void * addr);

/* Garbage reallocates memory */
void * garbage_realloc(void * addr, size_t new_size);

/* Assigns garbage collector as dynamic stack */
struct dynamic_stack * get_stack_of_freed_addr();

/* Reallocates memory of garbage collector if dynamic stack is full.*/
void garbage_col_push(void * value);

/* Reallocates memory of garbage collector if dynamic stack of freed adresses is full.*/
void garbage_col_free_push(void * value);

#endif