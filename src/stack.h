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

#ifndef STACK_H
#define STACK_H 42

#include "error.h"
#include "garbage_collector.h"
#include "scanner.h"

#include <stdint.h>
#include <stdbool.h>

#define STACK_ALLOC_STEP 1024

/* Dynamic stack structure */
struct dynamic_stack
{
	void ** start;
	uint32_t actual;
	size_t size;
};

/* Initializes dynamic stack */
struct dynamic_stack * dynamic_stack_init();

/* Pops value from dynamic stack */
void dynamic_stack_pop(struct dynamic_stack * stack);

/* Returns top of dynamic stack */
void * dynamic_stack_top(struct dynamic_stack * stack);

/* Pushes value on dynamic stack */
void dynamic_stack_push(struct dynamic_stack * stack, void * value);

/* Reallocates memory for dynamic stack */
void dynamic_stack_realloc(struct dynamic_stack * stack);

/* Searches for value in dynamic stack */
void ** dynamic_stack_search(struct dynamic_stack * stack, void * value);

/* Checks if dynamic stack is empty */
bool dynamic_stack_empty(struct dynamic_stack * stack);

/* Checks if dynamic stack is empty */
bool dynamic_stack_full(struct dynamic_stack * stack);


/****************Expression stack structures and functions*****************/

/* Element of expression stack structure*/
typedef struct tselem {
	token *t_elem;
	bool is_valid;
	bool conv_double;
	struct tselem *next;
	struct tselem *prev;
}TSElem;

/* Expression stack structure */
typedef struct tstack {
	TSElem *First;
	TSElem *Last;
} TStack;

/* Initializes stack for expressions */
void stack_init(TStack *s);

/* Pushes token on stack */
void push_expr_token(TStack *s, token *t);

/* Pops last element from stack */
void pop_last_expr(TStack *s);

/* Pops first element from stack */
void pop_first_expr(TStack *s);

/* Returns last element on stack */
token *peek_last_expr(TStack *s);

/* Returns first element on stack */
token *peek_first_expr(TStack *s);

/* Checks if stack is empty */
bool SEmpty (TStack *s);

/* Prints the whole stack */
void print_stack(TStack *s);

/* Deletes current expression */
void delete_current_expr(TSElem *s);

/* Counts number of stack elements */
int stack_counter(TStack *s);

/* Frees memory of whole stack */
void dealloc_tstack(TStack *s);

/* Counts number of stack elements. Checks validity of counter*/
int stack_valid_counter(TStack *s);

/* Copies whole stack into another stack */
TStack stack_copy(TStack *s);


/****************Boolean value stack*****************/

/* Boolean element in boolean structure*/
typedef struct bselem {
	bool is_bool_value;
	struct bselem *next;
} BSElem;

/* Boolean stack structure */
typedef struct bstack {
	BSElem *First;
} BStack;

/* Initializes BStack */
void BInit (BStack *s);

/* Pushes boolean value on stack */
void BPush (BStack *s, bool is_bool_value);

/* Pops boolean value on stack */
void BPop (BStack *s);

/* Returns boolean value on top of the BStack */
bool BTop (BStack *s);

/* Checks if BStack is empty */
bool BEmpty (BStack *s);

/* Checks two first in row values if they are boolean*/
bool BTop_equals(BStack *s);

/* Frees whole BStack*/
void dealloc_BStack(BStack *s);

/* Prints whole BStacl*/
void print_BStack(BStack *s);

/* Checks if value on the top is ok*/
bool is_top_ok(BStack *s);

#endif