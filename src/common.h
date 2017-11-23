#ifndef H_COMMON
#define H_COMMON 42

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "tokens.h"
#include "symtable.h"
#include "error.h"
#include "garbage_collector.h"


extern struct dynamic_stack * GARBAGE_COLLECTOR;
/* Initializes hash table before it has been used */
htab_t * initialization(token_buffer * token_buff);

#endif