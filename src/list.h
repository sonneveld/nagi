#ifndef list_h_struct
#define list_h_struct
/* STRUCTURES	---	---	---	---	---	---	--- */
struct node_struct
{
	struct node_struct *next;
	struct node_struct *prev;
	unsigned char contents[0];
};
typedef struct node_struct NODE;

struct list_struct
{
	// public to read
	NODE *head;
	NODE *tail;
	
	// private
	int contents_size;
};
typedef struct list_struct LIST;
typedef struct list_struct STACK;
#endif

#ifndef list_h_file
#define list_h_file
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
LIST *list_new(int contents_size);
void list_free(LIST *list);
void list_clear(LIST *list);
void list_clear_past(LIST *list, NODE *node);
NODE *list_add(LIST *list);
void list_remove(LIST *list, NODE *node);
int list_length(LIST *list);
NODE *list_element_at(LIST *list, int index);

STACK *stack_new(int contents_size);
void stack_free(STACK *stack);
void stack_clear(STACK *stack);
NODE *stack_push(STACK *stack);
void stack_pop(STACK *stack);
NODE *stack_top(STACK *stack);


#endif