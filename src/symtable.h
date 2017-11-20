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

/* Helpful types with new values */
enum type
{
	INTEGER_TYPE = 1,
	STRING_TYPE = 2,
	DOUBLE_TYPE = 4,
	BOOLEAN_TYPE = 8
}typedef enum_type;

/* Identifier data structure */
typedef struct identifier_data
{	
	char type;		
	char flags;		//3-X,2-defined,1-declared,0-used
	unsigned int par_count;
	union argconst
	{
		struct func_par * first_par;
		token * constant;
	}u_argconst;
	
}id_data;

/* Function parameter structure */
struct func_par
{
	char par_type;
	char *par_name;
	struct func_par *par_next;
};

/* Symbol table item structure */
struct htab_listitem
{
	char * key;
	id_data data;
	struct htab_listitem * next;
};

/* Symbol table structure */
typedef struct htab_t
{
	size_t arr_size;
	size_t n;
	struct htab_listitem * buckets[];
}htab_t;


/* Sets identifier type */
void set_id_type(struct htab_listitem * item, char new_type);

/* Returns identifier type */
char get_id_type(struct htab_listitem * item);

/* Sets identifier is used */
void set_id_used(struct htab_listitem * item);

/* Sets identifier is declared */
void set_id_declared(struct htab_listitem * item);

/* Sets identifier is defined */
void set_id_defined(struct htab_listitem * item);

/* Sets identifier is function */
void set_id_function(struct htab_listitem * item);

/* Sets identifier is constant */
void set_id_constant(struct htab_listitem * item, token * constant);

/* Sets function inline */
void set_func_inline(struct htab_listitem * item);

/* Sets initial parameters count to function record*/
void set_func_par_count(struct htab_listitem * item, unsigned int value);

/* Increments function paramaters count */
void add_func_par_count(struct htab_listitem * item);

/* Returns if identifier is function */
bool id_is_function(struct htab_listitem * item);

/* Returns if identifier is used */
bool id_is_used(struct htab_listitem * item);

/* Returns if identifier is declared */
bool id_is_declared(struct htab_listitem * item);

/* Returns if identifier is defined */
bool id_is_defined(struct htab_listitem * item);



/* Returns size of symbol table */
unsigned int hash_function(const char *str);

/* Returns size of symbol table */
size_t htab_bucket_count(struct htab_t *t);

/* Initializes symbol table */
struct htab_t * htab_init(size_t size);

/* Makes new function record and sets initial values*/
struct htab_listitem * make_item(const char * key);

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
