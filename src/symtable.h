/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#ifndef SYMTABLE_H
#define SYMTABLE_H 42

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "scanner.h"
#include "error.h"

enum type
{
	VOID_TYPE = 0,
	INTEGER_TYPE = 1,
	STRING_TYPE = 2,
	DOUBLE_TYPE = 4,
	BOOLEAN_TYPE = 8
}typedef enum_type;

typedef struct identifier_data
{	
	int type;		
	/*bool is_function;		
	bool is_declared;
	bool is_defined;*/
	char flags;
	unsigned int par_count;
	struct fun_par * first_par;
}id_data;

struct fun_par
{
	int par_type;
	char *par_name;
	struct fun_par *par_next;
};

typedef struct htab_listitem
{
	char * key;
	id_data data;
	struct htab_listitem * next;
}htab_listitem;

typedef struct htab_t
{
	size_t arr_size;
	size_t n;
	struct htab_listitem * buckets[];
}htab_t;


/*void set_id_type(struct htab_listitem * item, int set_type);
int get_id_type(struct htab_listitem * item);
void set_id_function(struct htab_listitem * item);
void set_id_declared(struct htab_listitem * item);
void set_id_defined(struct htab_listitem * item);
void set_id_used(struct htab_listitem * item);
void set_id_constant(struct htab_listitem * item, token * constant);
void set_func_par_count(struct htab_listitem * item, unsigned int count);
void add_func_par_count(struct htab_listitem * item);
bool is_function(struct htab_listitem * item);
bool is_used(struct htab_listitem * item);
bool is_declared(struct htab_listitem * item);
bool is_defined(struct htab_listitem * item);*/

void set_id_type(struct htab_listitem * item, int set_type);
int get_id_type(struct htab_listitem * item);
void set_id_declared(struct htab_listitem * item);
void set_id_defined(struct htab_listitem * item);
void set_id_function(struct htab_listitem * item);
bool is_declared(struct htab_listitem * item);
bool is_defined(struct htab_listitem * item);
bool is_function(struct htab_listitem * item);
void set_func_par_count(struct htab_listitem * item, unsigned int count);
void add_func_par_count(struct htab_listitem * item);

/* Prints the symtable*/
void htab_print(struct htab_t *symtable);

/* Returns size of symbol table */
unsigned int hash_function(const char *str);

/* Returns size of symbol table */
size_t htab_bucket_count(struct htab_t *t);

/* Initializes symbol table */
struct htab_t * htab_init(size_t size);

/* Makes new function record and sets initial values*/
struct htab_listitem * htab_make_item(const char * key);

/* Append function record to symbol table */
void htab_append(struct htab_listitem *item, struct htab_t *t);

/* Returns last function record in symbol table */
struct htab_listitem * htab_last(struct htab_t *t, const char * key);

/* Returns function record with relevant key */
struct htab_listitem * htab_find(struct htab_t *t, const char * key);

/* Clears symbol table. Deletes and frees all items */
void htab_clear (struct htab_t * t);

/* Deletes and frees symbol table */
void htab_free(struct htab_t * t);

/* Adds function record at relevant index in symbol table*/
struct htab_listitem * htab_lookup_add(struct htab_t *t, const char * key);

/* Moves items of symbol table */
struct htab_t *htab_move(long newsize, struct htab_t *t2);

/* Copies all function record from one symbol table to another*/
void htab_foreach(htab_t* t, htab_t * other_symtable, String * primal_code, void(*function)(struct htab_listitem * item, htab_t * other_symtable, String * primal_code));

#endif
