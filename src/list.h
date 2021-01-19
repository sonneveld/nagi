#ifndef NAGI_LIST_H
#define NAGI_LIST_H

// sonneveld: Possibly for build time performance: the NODE, LIST and 
// STACK types are also defined in agi.h
#ifndef NAGI_LIST_H_STRUCT
#define NAGI_LIST_H_STRUCT

//~ RaDIaT1oN (2002-04-29):
//~ add const to search declaration

/* STRUCTURES	---	---	---	---	---	---	--- */
struct node_struct
{
	struct node_struct *next;
	struct node_struct *prev;
	unsigned char contents[1];
};
typedef struct node_struct NODE;

struct list_struct
{
	NODE *head;
	NODE *tail;
	int contents_size;
};
typedef struct list_struct LIST;
typedef struct list_struct STACK;

#endif /* NAGI_LIST_H_STRUCT */


/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

extern NODE *node_header(void *contents);
extern void *node_next(void *contents);
extern void *node_prev(void *contents);

extern LIST *list_new(int contents_size);
extern void list_free(LIST *list);
extern void list_clear(LIST *list);
extern void list_clear_past(LIST *list, void *contents);
extern void *list_add(LIST *list);
extern void list_remove(LIST *list, void *contents);
extern int list_length(LIST *list);
extern void *list_element_at(LIST *list, int elementIndex);
extern void *list_element_head(LIST *list);
extern void *list_element_tail(LIST *list);
extern void list_sort(LIST *list, int (*compare)(const void *, const void *));

extern STACK *stack_new(int contents_size);
extern void stack_free(STACK *stack);
extern void stack_clear(STACK *stack);
extern void *stack_push(STACK *stack);
extern void stack_pop(STACK *stack);
extern void *stack_top(STACK *stack);

#endif /* NAGI_LIST_H */
