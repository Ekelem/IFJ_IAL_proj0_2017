#ifndef STACK_H
#define STACK_H 42

#include "error.h"
#include "garbage_collector.h"
#include "scanner.h"

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

/********************************************/
/****************EXPRESSSION*****************/
/********************************************/


typedef struct tselem {
	token *t_elem;
	bool is_valid;
	bool conv_double;
	struct tselem *next;
	struct tselem *prev;
}TSElem;


typedef struct tstack {
	TSElem *First;
	TSElem *Last;
} TStack;

void stack_init(TStack *s);

void push_expr_token(TStack *s, token *t);

void pop_last_expr(TStack *s);

void pop_first_expr(TStack *s);

token *peek_last_expr(TStack *s);

token *peek_first_expr(TStack *s);

bool SEmpty (TStack *s);

void print_stack(TStack *s);

void delete_current_expr(TSElem *s);

int stack_counter(TStack *s);

void dealloc_tstack(TStack *s);

int stack_valid_counter(TStack *s);

TStack stack_copy(TStack *s);


/********************************************/
/****************VALUE_STACK*****************/
/********************************************/

typedef struct bselem {
	bool is_bool_value;
	struct bselem *next;
} BSElem;


typedef struct bstack {
	BSElem *First;
} BStack;


void BInit (BStack *s);
void BPush (BStack *s, bool is_bool_value);
void BPop (BStack *s);
bool BTop (BStack *s);
bool BEmpty (BStack *s);
bool BTop_equals(BStack *s);
void dealloc_BStack(BStack *s);
void print_BStack(BStack *s);

#endif