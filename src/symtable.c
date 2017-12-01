/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#include "symtable.h"
#include <stdbool.h>

unsigned int hash_function(const char *str)
{
	unsigned int h=0;
	const unsigned char *p;
	for(p=(const unsigned char*)str; *p!='\0'; p++)
		h = 65599*h + *p;
	return h;
}


void set_id_type(struct htab_listitem * item, int set_type){
	item->data.type= set_type;}

int get_id_type(struct htab_listitem * item){
	return item->data.type;}

void set_id_declared(struct htab_listitem * item){
	item->data.flags|= 2;}

void set_id_defined(struct htab_listitem * item){
	item->data.flags|= 4;}

void set_id_function(struct htab_listitem * item){
	item->data.flags|= 8;}

bool is_declared(struct htab_listitem * item){
	return (item->data.flags & 2);}

bool is_defined(struct htab_listitem * item){
	return (item->data.flags & 4);}

bool is_function(struct htab_listitem * item){
	return (item->data.flags & 8);}

void set_func_par_count(struct htab_listitem * item, unsigned int count){
	item->data.par_count= count;}

void add_func_par_count(struct htab_listitem * item){
	item->data.par_count+= 1;}


/*void set_id_type(struct htab_listitem * item, int set_type){
	item->data.type= set_type;}

int get_id_type(struct htab_listitem * item){
	return item->data.type;}

void set_id_function(struct htab_listitem * item){
	item->data.is_function = true;}

void set_id_declared(struct htab_listitem * item){
	item->data.is_declared = true;}

void set_id_defined(struct htab_listitem * item){
	item->data.is_defined = true;}

void set_func_par_count(struct htab_listitem * item, unsigned int count){
	item->data.par_count= count;}

void add_func_par_count(struct htab_listitem * item){
	item->data.par_count+= 1;}

bool is_function(struct htab_listitem * item){
	return (item->data.is_function);}

bool is_declared(struct htab_listitem * item){
	return (item->data.is_declared);}

bool is_defined(struct htab_listitem * item){
	return (item->data.is_defined);}
*/

/* Hash table functions */

size_t htab_bucket_count(struct htab_t *t)
{
	return t->arr_size;
}

struct htab_t * htab_init(size_t size)
{
	const size_t allocmem = (sizeof(struct htab_t)+(size*(sizeof(struct htab_listitem*))));
	struct htab_t * result= malloc(allocmem);
	if (result == NULL)
		return NULL;

	result->arr_size=size;
	result->n=0;
	size_t bucket_count = htab_bucket_count(result);
	for (unsigned int i=0; i<bucket_count; i++)
	{
		result->buckets[i]=NULL;
	}
	return result;
}

struct htab_listitem * htab_make_item(const char * key)
{
	struct htab_listitem * item = malloc(sizeof(struct htab_listitem));
	if (item==NULL)
		return NULL;
	item->key=malloc(strlen(key)+1);
	if (item->key == NULL)
	{
		free(item);
		return NULL;
	}
	strcpy(item->key, key);
	item->data.type = VOID_TYPE;
	/*
	item->data.is_function = false;
	item->data.is_declared = false;
	item->data.is_defined = false;*/
	item->data.flags = 0;
	item->data.first_par = NULL;
	item->next=NULL;
	return item;
}

void htab_append(struct htab_listitem *item, struct htab_t *t)
{
	unsigned int index = (hash_function(item->key) % (htab_bucket_count(t)));
	struct htab_listitem *end = t->buckets[index];
	if (end == NULL)
	{
		t->buckets[index]=item;
		t->buckets[index]->next=NULL;
		return;
	}
	while (end->next!=NULL)
		end=end->next;

	end->next=item;
	end->next->next=NULL;
}

struct htab_listitem * htab_last(struct htab_t *t, const char * key)
{
	unsigned int index = (hash_function(key) % htab_bucket_count(t));
	struct htab_listitem * item=t->buckets[index];
	if (item==NULL)
		return NULL;
	while((item->next) != NULL)
	{
		item=item->next;
	}
	return item;
}

struct htab_listitem * htab_find(struct htab_t *t, const char * key)
{
	unsigned int index = (hash_function(key) % htab_bucket_count(t));
		struct htab_listitem * item = t->buckets[index];
		while (item!=NULL)
		{
			if (strcmp((item->key),key)==0)
			{
				return item;
			}
			item=item->next;
		}
		return NULL;
}

void htab_clear (struct htab_t * t)
{
	struct htab_listitem * item;
	struct htab_listitem * pred_item;
	for (unsigned int i=0; i<(htab_bucket_count(t)); i++)
	{
		item =t->buckets[i];
		while (item!=NULL)
		{
			pred_item=item;
			item=item->next;
			free(pred_item->key);
			free(pred_item);
		}
		t->buckets[i]=NULL;
	}
	t->n=0;
}

void htab_free(struct htab_t * t)
{
	htab_clear(t);
	free(t);
}

struct htab_listitem * htab_lookup_add(struct htab_t *t, const char * key)
{
	struct htab_listitem * item=htab_find(t, key); 
	if (item==NULL)
	{
		item=htab_make_item(key);
		if (item==NULL)
			return NULL;

		(t->n)++;

		unsigned int index = (hash_function(key) % htab_bucket_count(t));
		if (t->buckets[index] == NULL)
		{
			t->buckets[index]=item;
		}
		else
		{
			struct htab_listitem * itemlast = htab_last(t, key);
			itemlast->next=item;
		}
	}
		return item;
}

struct htab_t *htab_move(long newsize, struct htab_t *t2) {
	if (newsize < 1) {
		fprintf(stderr, "Invalid parameter, newsize < 1\n");
		return NULL;
	}
	if (t2 == NULL) {
		fprintf(stderr, "Invalid parameter, t2\n");
		return NULL;
	}
	struct htab_t *tmp = htab_init(newsize);
	if (tmp == NULL)
		return NULL;

	size_t table_size = htab_bucket_count(t2);
	for (unsigned long i = 0; i < table_size; i++) {
		struct htab_listitem *help = t2->buckets[i];
		while(help != NULL) {
			struct htab_listitem *add = htab_lookup_add(tmp, help->key);
			add->data = help->data;
			help = help->next;
		}
	}

	tmp->arr_size = newsize;
	tmp->n = t2->n;

	return tmp;
}

void htab_print(struct htab_t *symtable)
{
	if(symtable)
	{
		printf("Symtable:   ");
		size_t size = htab_bucket_count(symtable);
		for(unsigned long i = 0; i < size; i++)
		{
			struct htab_listitem *help = symtable->buckets[i];
			while(help)
			{
				printf("|ID:'%s'| ", help->key);
				help = help->next;
			}	
		}
		printf("\n");
	}
	else
		printf("Symtable doesnt exist!\n");
}


void htab_foreach(htab_t* t, htab_t * other_symtable, String * primal_code, void(*function)(struct htab_listitem * item, htab_t * other_symtable, String * primal_code))
{
	for (unsigned int i=0; i<(htab_bucket_count(t)); i++)
	{
		struct htab_listitem * item =t->buckets[i];
		while (item!=NULL)
		{
			function(item, other_symtable, primal_code);
			item=item->next;
		}
	}
}