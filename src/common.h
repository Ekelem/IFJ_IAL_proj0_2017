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

htab_t * initialization(const char * file_path, token_buffer * token_buff);
void output_primal_code(const char * file_path, String * primal_code);

#endif