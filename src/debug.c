#include <stdio.h>
#include "agi.h"

//~ RaDIaT1oN (2002-04-29):
//~ use %p instead of %X for pointers.

#include "view/obj_base.h"

void print_hex_array(u8 *a, int size)
{
	int i;
	//printf("\n");
	for (i = 0; i < size; i++)
	{
		printf("0x%02X ", a[i]);
	}
	//printf("\n"); 
}

void print_view(VIEW *v)
{
	printf("\n");
	printf("st=%d stc=%d ", v->step_time, v->step_count);
	printf("x=%d y=%d ", v->x, v->y);

	printf("v_cur=%d *v=0x%p ", v->view_cur, v->view_data);
	printf("l_cur=%d l_total=%d *l=0x%p ", v->loop_cur, v->loop_total, v->loop_data);
	printf("c_cur=%d c_total=%d *c=0x%p *cc=0x%X *B=0x%p ", v->cel_cur, v->cel_total, v->cel_data, 0 , v->blit);

	printf("xc=%d yc=%d xs=%d ys=%d ", v->x_prev, v->y_prev, v->x_size, v->y_size);
	printf("step=%d ct=%d ctcopy=%d dir=%d mot=%d cyc=%d pri=%d flags=0x%X ", 	v->step_size, v->cycle_time, v->cycle_count, v->direction, v->motion, v->cycle, v->priority, v->flags);
	//printf("u27=%d u28=%d u29=%d u2A=%d",  v->unknown27, v->unknown28, v->unknown29, v->unknown2A);
	printf("\n");
}