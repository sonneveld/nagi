/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "agi.h"
//#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "list.h"
#include "sys/mem_wrap.h"
/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

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
void list_clear_past(LIST *list, NODE *node)
{
	NODE *c, *n;
	
	assert((list != 0) && (node != 0));
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

// node *list_add (list *)
// return a new node at the end of the list.
NODE *list_add(LIST *list)
{
	NODE *n;
	
	assert(list != 0);
	
	n = a_malloc(sizeof(NODE) + list->contents_size);
	
	if (list->tail != 0)
		list->tail->next = n;
	n->next = 0;
	n->prev = list->tail;
	
	list->tail = n;
	if (list->head == 0)
		list->head = n;
	
	return n;
}


// void list_remove (list *, node *)
// hopefully node is in the list or strange things will happen
void list_remove(LIST *list, NODE *node)
{
	assert( (list != 0) && (node != 0) );
	
	if (list->tail == node)
		list->tail = node->prev;
	if (list->head == node)
		list->head = node->next;
	
	if (node->prev != 0)
		(node->prev)->next = node->next;
	if (node->next != 0)
		(node->next)->prev = node->prev;
	
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

NODE *list_element_at(LIST *list, int index)
{
	NODE *c;
	
	assert((list != 0));
	
	c = list->head;
	
	while ( (index) && (c != 0) )
	{
		index--;
		c = c->next;
	}
	
	if (index == 0)
		return c;
	else
		return 0;
}




STACK *stack_new(int contents_size)
{
	return list_new(contents_size);
}

void stack_free(STACK *stack)
{
	list_free(stack);
}

void stack_clear(STACK *stack)
{
	list_clear(stack);
}

NODE *stack_push(STACK *stack)
{
	return list_add(stack);
}

void stack_pop(STACK *stack)
{
	list_remove(stack, stack->tail);
}

NODE *stack_top(STACK *stack)
{
	return stack->tail;
}



