#include "../agi.h"

#include "../picture/pic_stack.h"

#include "../sys/mem_wrap.h"

#include <stdlib.h>

int pushed = 0;
int pushed_max = 0;

DATA *fill_push(DATA *data, u8 a, u8 b, u8 c, u8 d, u8 e, u8 f, u8 g)
{
	DATA *new;
	new = (DATA *)a_malloc(sizeof(DATA));
	
	new->a = a;
	new->b = b;
	new->c = c;
	new->d = d;
	new->e = e;
	new->f = f;
	new->g = g;
	new->next = data;

	pushed++;
	if (pushed > pushed_max)
	{
		pushed_max = pushed;
		if ( pushed_max == 1)
			printf("sizeof data = %d\n", sizeof(DATA));
		printf("pushed_max = %d\n", pushed_max);
	}
	return new;
}

DATA *fill_pop(DATA *data, u8 *a, u8 *b, u8 *c, u8 *d, u8 *e, u8 *f, u8 *g)
{
	DATA *under;
	
	*a = data->a;
	*b = data->b;
	*c = data->c;
	*d = data->d;
	*e = data->e;
	*f = data->f;
	*g = data->g;
	
	under = data->next;
	a_free(data);
	data = 0;
	pushed--;
	return under;
}

void fill_topab(DATA *data, u8 *a, u8 *b)
{
	*a = data->a;
	*b = data->b;
}