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

enum type
{
	INTEGER_TYPE = 1,
	STRING_TYPE = 2,
	DOUBLE_TYPE = 4,
	BOOLEAN_TYPE = 8
}typedef enum_type;

typedef struct identifier_data
{	
	char type;		//
	char flags;		//3-X,2-defined,1-declared,0-used)
	unsigned int par_count;
	union argconst
	{
		struct func_par * first_par;
		token * constant;
	}u_argconst;
	
}id_data;

struct func_par
{
	char par_type;
	char *par_name;
	struct func_par *par_next;
};

struct htab_listitem
{
	char * key;
	id_data data;
	struct htab_listitem * next;
};



typedef struct htab_t
{
	size_t arr_size;
	size_t n;
	struct htab_listitem * buckets[];
}htab_t;


void set_id_type(struct htab_listitem * item, unsigned int new_type);

void set_id_used(struct htab_listitem * item);

void set_id_declared(struct htab_listitem * item);

void set_id_defined(struct htab_listitem * item);

void set_id_function(struct htab_listitem * item);

void set_id_constant(struct htab_listitem * item, token * constant);

void set_func_inline(struct htab_listitem * item);

void set_func_par_count(struct htab_listitem * item, unsigned int value);

void add_func_par_count(struct htab_listitem * item);

bool id_is_function(struct htab_listitem * item);

bool id_is_used(struct htab_listitem * item);

bool id_is_declared(struct htab_listitem * item);

bool id_is_defined(struct htab_listitem * item);



unsigned int hash_function(const char *str);

size_t htab_bucket_count(struct htab_t *t);

struct htab_t * htab_init(size_t size);

struct htab_listitem * make_item(const char * key);

void htab_append(struct htab_listitem *item, struct htab_t *t);

struct htab_listitem * htab_last(struct htab_t *t, const char * key);

struct htab_listitem * htab_find(struct htab_t *t, const char * key);

void htab_clear (struct htab_t * t);

void htab_free(struct htab_t * t);

struct htab_listitem * htab_lookup_add(struct htab_t *t, const char * key);

struct htab_t *htab_move(long newsize, struct htab_t *t2);

#endif
