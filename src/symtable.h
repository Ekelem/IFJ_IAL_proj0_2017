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

#ifndef SYMTABLE_H
#define SYMTABLE_H 42

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "scanner.h"
#include "error.h"

/* Type enumeration */
enum type
{
	VOID_TYPE = 0,
	INTEGER_TYPE = 1,
	STRING_TYPE = 2,
	DOUBLE_TYPE = 4,
	BOOLEAN_TYPE = 8
}typedef enum_type;

/* Identifier data */
typedef struct identifier_data
{	
	int type;		
	char flags;
	unsigned int par_count;
	struct fun_par * first_par;
}id_data;

/* Function parameter */
struct fun_par
{
	int par_type;
	char *par_name;
	struct fun_par *par_next;
};

/* Symbol table item */
typedef struct htab_listitem
{
	char * key;
	id_data data;
	struct htab_listitem * next;
}htab_listitem;

/* Symbol table*/
typedef struct htab_t
{
	size_t arr_size;
	size_t n;
	struct htab_listitem * buckets[];
}htab_t;

/* Sets identifier type */
void set_id_type(struct htab_listitem * item, int set_type);

/* Returns identifier type */
int get_id_type(struct htab_listitem * item);

/* Sets identifier declared */
void set_id_declared(struct htab_listitem * item);

/* Sets identifier defined */
void set_id_defined(struct htab_listitem * item);

/* Sets identifier function */
void set_id_function(struct htab_listitem * item);

/* Returns if identifier is declared */
bool is_declared(struct htab_listitem * item);

/* Returns if identifier is defined */
bool is_defined(struct htab_listitem * item);

/* Returns if identifier is function */
bool is_function(struct htab_listitem * item);

/* Sets number of parameters of function*/
void set_func_par_count(struct htab_listitem * item, unsigned int count);

/* Increments number of parameters of function*/
void add_func_par_count(struct htab_listitem * item);

/* Prints the symtable*/
void htab_print(struct htab_t *symtable);

/* Returns size of symbol table */
unsigned int hash_function(const char *str);

/* Returns size of symbol table */
size_t htab_bucket_count(struct htab_t *t);

/* Initializes symbol table */
struct htab_t * htab_init(size_t size);

/* Makes new identifier record and sets initial values*/
struct htab_listitem * htab_make_item(const char * key);

/* Append identifier record to symbol table */
void htab_append(struct htab_listitem *item, struct htab_t *t);

/* Returns last identifier record in symbol table */
struct htab_listitem * htab_last(struct htab_t *t, const char * key);

/* Returns identifier record with relevant key */
struct htab_listitem * htab_find(struct htab_t *t, const char * key);

/* Clears symbol table. Deletes and frees all items */
void htab_clear (struct htab_t * t);

/* Deletes and frees symbol table */
void htab_free(struct htab_t * t);

/* Adds identifier record at relevant index in symbol table*/
struct htab_listitem * htab_lookup_add(struct htab_t *t, const char * key);

/* Moves items of symbol table */
struct htab_t *htab_move(long newsize, struct htab_t *t2);

#endif
