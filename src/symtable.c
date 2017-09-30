/*
 *	IFJ/IAL project 2017
 *	author/s: Erik Kelemen
*/

#include "symtable.h"

unsigned int hash_function(const char *str)
{
	unsigned int h=0;
	const unsigned char *p;
	for(p=(const unsigned char*)str; *p!='\0'; p++)
		h = 65599*h + *p;
	return h;
}

void set_id_type(id_data* data, unsigned int new_type)
{
	data->type= new_type;
}

void set_id_used(id_data* data)
{
	data->flags|= 1;
}

void set_id_declared(id_data* data)
{
	data->flags|= 2;
}

void set_id_defined(id_data* data)
{
	data->flags|= 4;
}

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

struct htab_listitem * make_item(const char * key)
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
	item->data.type = 0;
	item->data.flags = 0;
	strcpy(item->key, key);
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
		item=make_item(key);
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