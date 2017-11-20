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
htab_t * initialization(const char * file_path, token_buffer * token_buff);
/* Initializes output file and puts generated assembler code to it*/
void output_primal_code(const char * file_path, String * primal_code);

#endif