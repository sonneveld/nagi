/* FUNCTION list 	---	---	---	---	---	---	---

*/
//~ RaDIaT1oN (2002-04-29):
//~ add const to search declaration

/* BASE headers	---	---	---	---	---	---	--- */
#include "agi.h"
//#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>
#include <stddef.h>


/* OTHER headers	---	---	---	---	---	---	--- */

#include "list.h"
#include "sys/mem_wrap.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */

/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

// NODE ------------------------------------------------

// returns a node pointer from a contents pointer
NODE *node_header(void *contents)
{
	NODE *node;
	
	assert(contents);
	
	node = (NODE *)((u8 *)contents - offsetof(NODE, contents));
	return node;
}

void *node_next(void *contents)
{
	NODE *n;
	
	assert(contents);
	
	n = node_header(contents);
	n = n->next;
	return (n ? (void *)(n->contents) : 0);}

void *node_prev(void *contents)
{
	NODE *n;
	
	assert(contents);
	
	n = node_header(contents);
	n = n->prev;
	return (n ? (void *)(n->contents) : 0);}

// LIST ------------------------------------------------

// list *list_new(node_size)
LIST *list_new(int contents_size)
{
	LIST *list;
	
	list = a_malloc(sizeof(LIST));
	list->head = 0;
	list->tail = 0;
	list->contents_size = contents_size;
	
	return list;
}

void list_free(LIST *list)
{
	assert(list != 0);
	list_clear(list);
	a_free(list);
}

// void list_clear(list *)
void list_clear(LIST *list)
{
	NODE *c, *n;
	
	assert (list != 0);
	
	c = list->head;
	while (c)
	{
		n = c->next;
		a_free(c);
		c = n;
	}
	list->head = 0;
	list->tail = 0;
}

// free all after a certain node
void list_clear_past(LIST *list, void *contents)
{
	NODE *node;
	NODE *c, *n;
	
	assert((list != 0) && (contents != 0));
	
	node = node_header(contents);
	c = node->next;
	while (c)
	{
		n = c->next;
		a_free(c);
		c = n;
	}
	
	list->tail = node;
	node->next = 0;
}

// void *list_add (list *)
// return a new node at the end of the list.
void *list_add(LIST *list)
{
	NODE *n;
	
	assert(list != 0);
	
	n = a_malloc(sizeof(NODE) + list->contents_size - 1);
	
	if (list->tail != 0)
		(list->tail)->next = n;
	n->next = 0;
	n->prev = list->tail;
	
	list->tail = n;
	
	if (list->head == 0)
		list->head = n;
	
	return (void *)(n->contents);
}


// void list_remove (list *, void *)
// hopefully node is in the list or strange things will happen
void list_remove(LIST *list, void *contents)
{
	NODE *node;
	assert( (list != 0) && (contents != 0) );
	
	node = node_header(contents);
	
	if ((node == list->tail) && (node == list->head) )	// 1 item
	{
		list->tail = 0;
		list->head = 0;
	}
	else if (node == list->tail)	// end
	{
		(node->prev)->next = 0;
		list->tail = node->prev;
	}
	else if (node == list->head)	// start
	{
		(node->next)->prev = 0;
		list->head = node->next;
	}
	else					// middle
	{
		(node->prev)->next = node->next;
		(node->next)->prev = node->prev;
	}
	
	a_free(node);		
}

int list_length(LIST *list)
{
	NODE *c;
	int length = 0;
	
	assert (list != 0);
	c = list->head;
	
	while (c != 0)
	{
		c = c->next;
		length++;
	}
	
	return length;
}

void *list_element_at(LIST *list, int ind)
{
	NODE *c;
	
	assert((list != 0));
	
	c = list->head;
	
	while ( (ind) && (c != 0) )
	{
		ind--;
		c = c->next;
	}
	
	return (ind ? 0: (void *)(c->contents));
}

void *list_element_head(LIST *list)
{
	NODE *n;
	assert(list);
	n = list->head;
	return (n ? (void *)(n->contents) : 0);
}

void *list_element_tail(LIST *list)
{
	NODE *n;
	assert(list);
	n = list->tail;
	return (n ? (void *)(n->contents) : 0);
}

void list_sort(LIST *list, int (*compare)(const void *, const void *))
{
	void **node_list, **node_list_ptr;
	NODE *cur, *prev;
	int len;
	
	len = list_length(list);
	
	if (len <= 1)
		return;
	
	// create list of pointers
	node_list = alloca( (len + 1) * sizeof(void *) );
	
	node_list_ptr = node_list;
	cur = list->head;
	
	while (cur != 0)
	{
		*node_list_ptr = cur->contents;
		node_list_ptr++;
		cur = cur->next;
	}
	
	node_list[len] = 0;

	// sort
	qsort(node_list, len, sizeof(void *), compare);
	
	// put ptrs back into list
	node_list_ptr = node_list;

	cur = node_header(*(node_list_ptr++));
	prev = 0;
	list->head = cur;
	
	do
	{
		cur->prev = prev;
		if (prev)
			prev->next = cur;
		prev = cur;
		if (*node_list_ptr)
			cur = node_header(*(node_list_ptr++));
		else
			cur = 0;
	} while (cur);
	
	if (prev)
		list->tail = prev;
	else
		list->tail = list->head;
	
	list->tail->next = 0;
}

// STACK ------------------------------------------------

STACK *stack_new(int contents_size)
{
	return list_new(contents_size);
}

void stack_free(STACK *stack)
{
	assert(stack);
	list_free(stack);
}

void stack_clear(STACK *stack)
{
	assert(stack);
	list_clear(stack);
}

void *stack_top(STACK *stack)
{
	assert(stack);
	return list_element_tail(stack);
}

void *stack_push(STACK *stack)
{
	assert(stack);
	return list_add(stack);
}

void stack_pop(STACK *stack)
{
	assert(stack);
	list_remove(stack, stack_top(stack));
}




