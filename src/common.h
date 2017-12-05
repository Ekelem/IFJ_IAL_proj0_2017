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

#ifndef H_COMMON
#define H_COMMON 42

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "tokens.h"
#include "symtable.h"
#include "error.h"
#include "garbage_collector.h"

/*Dynamic structure Garbage collector*/
extern struct dynamic_stack * GARBAGE_COLLECTOR;

/* Initializes hash table before it has been used */
htab_t * initialization(token_buffer * token_buff);

/* Debug function, prints given token*/
void print_token(int tk, token *t);
#endif