/*
_InitViewList                    cseg     0000395E 0000000F
_FreeViewList                    cseg     0000396D 0000000C
_FindView                        cseg     00003979 00000038
CmdLoadView                      cseg     000039B1 0000001F
CmdLoadViewV                     cseg     000039D0 00000027
_LoadView                        cseg     000039F7 00000080

CmdSetView                       cseg     00003A77 00000034
CmdSetViewV                      cseg     00003AAB 0000003C
_SetView                         cseg     00003AE7 00000060

// * NOT VIEW.. OBJ RELATED
CmdSetLoop                       cseg     00003B47 00000034
CmdSetLoopV                      cseg     00003B7B 0000003C
_SetLoop                         cseg     00003BB7 00000064
_SetLoop2                        cseg     00003C1B 0000003A

CmdSetCel                        cseg     00003C55 00000037
CmdSetCelV                       cseg     00003C8C 0000003F
_CheckCel                        cseg     00003CCB 0000009F
_SetCel                          cseg     00003D6A 00000035

CmdLastCel                       cseg     00003D9F 0000004E
CmdCurrentCel                    cseg     00003DED 00000038
CmdCurrentLoop                   cseg     00003E25 00000038
CmdCurrentView                   cseg     00003E5D 00000038
CmdNumOfLoops                    cseg     00003E95 00000038
// * END OF OBJ RELATED


CmdDiscardView                   cseg     00003ECD 0000001C
CmdDiscardViewV                  cseg     00003EE9 00000024
_DiscardView                     cseg     00003F0D 0000004A
*/

#include <stdlib.h>

#include "../agi.h"
#include <setjmp.h>
#include "../sys/error.h"
#include "../sys/memory.h"
#include "../sys/script.h"

#include "../res/res.h"

#include "../view/obj_update.h"
#include "../view/obj_base.h"

#include "../view/view_base.h"


// byte-order support
#include "../sys/endian.h"

#include <assert.h>

#include "../sys/mem_wrap.h"

VIEW_NODE view_list_head = {0,0,0};
VIEW_NODE *prev_view = 0;

void obj_loop_data(VIEW *v, u16 loop_num);
void obj_cel_data(VIEW *v, u16 loop_num);

void view_list_init()
{
	VIEW_NODE *cur, *next;
	
	cur = view_list_head.next;
	
	while (cur != 0)
	{
		next = cur->next;
		if (cur->data != 0)
			a_free(cur->data);
		a_free(cur);
		cur = next;
	}
	
	view_list_head.next = 0;
}

void view_list_new_room()
{
	view_list_init();
}

VIEW_NODE *view_find(u16 num)
{
	VIEW_NODE *prev, *cur;
	
	cur = view_list_head.next;
	prev = &view_list_head;

	while (  (cur!=0) && ( num!=(cur->num))  )
	{
		prev = cur;
		cur = cur->next;
	}

	prev_view = prev;	// find, load and discard
	return cur;
}
	
	
u8 *cmd_load_view(u8 *c)
{
	view_load(*(c++), 0);
	return c;
}

u8 *cmd_load_view_v(u8 *c)
{
	view_load(state.var[*(c++)], 0);
	return c;
}


VIEW_NODE *view_load(u16 num, u16 force_load)
{
	VIEW_NODE *v;
	v = view_find(num);
	
	if ( (v != 0) && (force_load == 0) ) return v;

	blists_erase();

	if (v == 0)
	{
		script_write(1, num);
		v = a_malloc(sizeof(VIEW_NODE));
		prev_view->next = v;
		v->next = 0;
		v->num = num;
		v->data = 0;
	}

	v->data = vol_res_load(dir_view(v->num), v->data);

	if (v->data == 0) return 0;

	render_view_dither(v->data);
	//sub5857(v->data);	cga_thing
	
	blists_draw();

	return v;
}

u8 *cmd_set_view(u8 *c)
{
	u8 view, num;

	view = *(c++);
	num = *(c++);
	
	obj_view_set(&objtable[view], num);	// * sizeof(VIEW)
	return c;
}


u8 *cmd_set_view_v(u8 *c)
{
	u8 view, num;

	view = *(c++);
	num = state.var[*(c++)];
	
	obj_view_set(&objtable[view], num);  //  * sizeof(VIEW)
	return c;
}

// need set loop, set cel
void obj_view_set(VIEW *v, u16 num)
{
	VIEW_NODE *vn;

	vn = view_find(num);
	if (vn == 0)
		set_agi_error(0x3, num);
	
	v->view_data = vn->data;
	v->view_cur = num;
	v->loop_total = (v->view_data)[2];
	
	if ( (v->loop_cur) >= (v->loop_total)  )
		obj_loop_set(v, 0);
	else
		obj_loop_set(v, v->loop_cur);
}





u8 *cmd_set_loop(u8 *c)
{
	u8 view, num;

	view = *(c++);
	num = *(c++);
	obj_loop_set(&objtable[view], num); //  * sizeof(VIEW)
	return c;
}


u8 *cmd_set_loop_v(u8 *c)
{
	u8 view, num;

	view = *(c++);
	num = state.var[*(c++)];
	obj_loop_set(&objtable[view], num);  // * sizeof(VIEW) 
	return c;
}



void obj_loop_set(VIEW *v, u16 loop_num)
{
	u16 view_num;
	
	view_num = (v-objtable);
	
	if (v->view_data == 0)
		set_agi_error(6, view_num);
	#warning error in the agi ???
	if (loop_num >= v->loop_total)
		set_agi_error(5, view_num);

	obj_loop_data(v, loop_num);
	obj_cel_set(v, v->cel_cur);
}


void obj_loop_data(VIEW *v, u16 loop_num)
{
	u16 loop_pos;
	u8 *data;
	
	v->loop_cur = loop_num;
	
	data = v->view_data + 5 + (loop_num << 1);
	loop_pos = load_le_16(data);
	
	v->loop_data = v->view_data + loop_pos;  // pos of first loop
	v->cel_total = (v->loop_data)[0];

	if ( v->cel_cur >= v->cel_total)
		v->cel_cur = 0;
}
	

u8 *cmd_set_cel(u8 *c)
{
	u8 num;
	VIEW *v;
	
	v =&objtable[*(c++)]; //* sizeof(VIEW)
	num = *(c++);
	
	obj_cel_set(v, num);
	v->flags &= ~O_SKIPUPDATE;
	return c;
}

u8 *cmd_set_cel_v(u8 *c)
{
	u8 num;
	VIEW *v;
	
	v = &objtable[*(c++)]; // * sizeof(VIEW)
	num = state.var[*(c++)];
	obj_cel_set(v, num);
	v->flags &= ~O_SKIPUPDATE;
	return c;
}


void obj_cel_set(VIEW *v, u16 loop_num)
{
	u16 view_num;
	
	view_num = (v - objtable) ; // / sizeof(VIEW)
	if (v->view_data == 0)
		set_agi_error(0xA, view_num);

	if (v->cel_total <= loop_num)
		set_agi_error(8, view_num);

	obj_cel_data(v, loop_num);

	if ((v->x + v->x_size) > 160 )
	{
		v->flags |= O_REPOS;
		v->x = 160 - v->x_size;
	}
	
	if ((v->y - v->y_size) < -1)
	{
		v->flags |= O_REPOS;
		v->y = (v->y_size) - 1;
		if (   (v->y <= state.horizon) && ((v->flags & O_HORIZONIGNORE) == 0)  )
			v->y = state.horizon + 1;
	}

}

void obj_cel_data(VIEW *v, u16 cel_num)
{
	v->cel_cur = cel_num;
	v->cel_data = v->loop_data + load_le_16(v->loop_data + (cel_num<<1) + 1);
	v->x_size =  (v->cel_data)[0];
	v->y_size = (v->cel_data)[1];
}


u8 *cmd_last_cel(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	state.var[*(c++)] = (temp->loop_data)[0] - 1;
	return c;
}

u8 *cmd_current_cel(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	state.var[*(c++)] = temp->cel_cur;
	return c;
}

u8 *cmd_current_loop(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	state.var[*(c++)] = temp->loop_cur;
	return c;
}

u8 *cmd_current_view(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	state.var[*(c++)] = temp->view_cur;
	return c;
}

u8 *cmd_number_of_loops(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	state.var[*(c++)] = temp->loop_total;
	return c;
}


u8 *cmd_discard_view(u8 *c)
{
	view_discard(*(c++) );
	return c;
}

u8 *cmd_discard_view_v(u8 *c)
{
	view_discard(state.var[*(c++)]);
	return c;
}

void view_discard(u16 num)
{
	VIEW_NODE *vn;
	
	vn = view_find(num);
	if (vn == 0)
		set_agi_error(1, num);
	script_write(7, num);
	prev_view->next = 0;
	blists_erase();
	if (vn->data != 0)
		a_free(vn->data);
	a_free(vn);
	//set_mem_ptr(si);
	blists_draw();
	update_var8();
}

#warning check the malloc() and free() stuff. .
#warning what if you run discard on a view object that points to another??