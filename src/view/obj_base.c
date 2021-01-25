/*
_EraseAll                        cseg     00000307 00000026
_FreeList                        cseg     0000032D 0000002B
_BuildBlitList                   cseg     00000358 000000D7
sub_cseg_42F                     cseg     0000042F 0000002F
_BlitAll                         cseg     0000045E 0000002A
_CheckMove                       cseg     00000488 00000051
CmdAnimateObj                    cseg     000004D9 0000001C
_AnimateObj                      cseg     000004F5 00000048
CmdUnanimateAll                  cseg     0000053D 00000026
_UpdateGraphics                  cseg     00000563 000000E1
_CalcDirection                   cseg     00000644 00000036
_UpdateMovement                  cseg     0000067A 0000005F
_CheckBlockStat                  cseg     000006D9 000000CF
*/

// update graphics : a few functions incomplete

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../agi.h"

// for put block

#include "obj_blit.h"
#include "obj_base.h"
#include "obj_update.h"
#include "view_base.h"
#include "obj_loop.h"
#include "obj_motion.h"

#include "../view/obj_picbuff.h"

// for pic_visible
#include "../picture/pic_res.h"

// for agi_error
#include <setjmp.h>
#include "../sys/error.h"

// for checkblock
#include "../view/obj_block.h"

// for rand
#include "../sys/rand.h"
// for flags
#include "../flags.h"
#include "../sys/mem_wrap.h"

#include <stdlib.h>
#include <assert.h>

VIEW *objtable = 0;
VIEW *objtable_tail;
u16 objtable_size;

static void blit_add(VIEW *v, BLIT *h);
static s16 gen_sort_pos(s16 var8);
static void obj_animate(u8 num);

void objtable_new(u16 max)
{
	int i;
	VIEW *v;
	
	if (objtable == 0)
	{
		objtable_size = max * sizeof(VIEW);
		objtable = (VIEW *)a_malloc(objtable_size);
	}

	memset(objtable, 0, objtable_size);
	objtable_tail = objtable + max;
	// something to do with add.to.pic ???
	//view_pic_add.0 = objtable_tail - 1;  

	for (i=0, v=objtable ; i<max ; i++, v++)
		v->num = i;
}

// pass a sprite list head
// erase list draws the screen back without the objects one by one.. like putting
// the peel of an onion back on or something..
u16 blitlist_erase(BLIT *b)
{
	BLIT *cur;

	for (cur=b->prev ; cur!=0 ; cur=cur->prev)
		blit_restore(cur);
	
	return blitlist_free(b);	
}

// set_memptr2() is the same as set_mem_ptr()
// you can think of it as a weird free() command.. it frees up everything 
// that had been allocated from that point.
// NOT good if something else had been allocated after the sprite list head
u16 blitlist_free(BLIT *b)
{
	BLIT *s, *d;
	
	for (s=b->prev ; s!=0 ; s = d)
	{
		d = s->prev;
		if (s->buffer != 0)
			a_free(s->buffer);
		a_free(s);	// set_memptr2(si);
		s = 0;
	}
	
	b->next = 0;	// prev
	b->prev = 0;	// next
	return 0;
}

// head of all blit objects that are updated on each cycle
BLIT blitlist_updated = {0,0,0,0,0,0,0,0};	// only used for these two addresses? next 'n prev
// head of all blit objects are are not updated on each cycle
BLIT blitlist_static = {0,0,0,0,0,0,0,0};

// var a = head .. var 8 = function
BLIT *blitlist_build( u16(*f)(VIEW *) , BLIT *head)	// function, spritelist
{
	s16 sort_order[256];		//408	its sort_order
	s16 cur_pos;		// 0x202	priority
	VIEW *view[256];		//206	position of vtable in mem
	
	u16 i;
	u16 num = 0;
	u16 d;	
	u16 next = 256;
	
	VIEW *s;
	
	for (s=objtable ; s<objtable_tail ; s++)
	{
		if (f(s) != 0)
		{	
			view[num] = s;
			if ( (s->flags&O_PRIFIXED) != 0)
				sort_order[num] = gen_sort_pos(s->priority);	// sub4CBB
			else
				sort_order[num] = s->y;
			num++;
		}
	}
	
	for (i=0 ; i<num ; i++)
	{
		cur_pos = 255;
		
		for (d=0; d<num; d++)
			if (sort_order[d] < cur_pos )
			{
				next = d;
				cur_pos = sort_order[d];
			}
		
		sort_order[next] = 255;
		assert(next != 256);
		blit_add( view[next], head );
	}
	return head;	// a passed parameter
}

static u16 priority_orig = 1;

static s16 gen_sort_pos(s16 var8)
{
	s16 temp2;
	
	if (priority_orig != 0)
		temp2 = (var8-5)*12 + 0x30;
	else
	{
		temp2 = 0xa7;
		while (pri_table[temp2] >= var8)
		{
			temp2--;
			//~ this is a workaround for a bug in the original agi
			//~ does not do this check.
			if (temp2 < 0)
				return -1;
		}
	}
	return temp2;
}

// 1 var
// changes the priority table
u8 *cmd_unknown_174(u8 *c)
{
	//s16 tempc;
	s16 tempa;
	s16 temp6;
	s16 pri_value;
	s16 pri_y;
	
	priority_orig = 0;
	tempa = *(c++);
	//tempc = ((0xA8-tempa)*0xA8)/0xA;
	
	for (pri_y=0; pri_y<0xA8; pri_y++)
	{
		temp6 = pri_y-tempa;
		if ( temp6 >= 0)		// signed
		{
			//pri_value = ((temp6*0xA8)/tempc) + 5;
			pri_value = (temp6*0xA)/(0xA8-tempa) + 5;
			if (pri_value > 0xF)
				pri_value = 0xF;
		}
		else
			pri_value = 4;
		pri_table[pri_y] = pri_value;
	}
	return c;
}

/*
u8 *cmd_unknown_174(u8 *c)
{
	s16 tempa;
	s16 temp6;
	s16 pri_value;
	s16 pri_y;
	
	u8 *sbuff;
	
	priority_orig = 0;
	
	for (tempa=0; tempa<0xa8; tempa++)
	{
		sbuff = screen_buffer;
		for (pri_y=0; pri_y<0xA8; pri_y++)
		{
			temp6 = pri_y-tempa;
			if ( temp6 >= 0)		// signed
			{
				pri_value = 5 + (temp6*0xA)/(0xA8-tempa);
				if (pri_value > 0xF)
					pri_value = 0xF;
			}
			else
				pri_value = 4;
			pri_table[pri_y] = pri_value;
			memset(sbuff, pri_value, 320);
			sbuff += 320;
		}
		sys_scale_update(0,0,0,0);
		event_wait();
	}
	agi_exit();
	return c;
}
*/

static void blit_add(VIEW *v, BLIT *h)  // var8 = v    vara = h
{
	BLIT *d, *c;
	
	d = blit_new(v);
	d->prev = h->prev;
	
	if (h->prev != 0)
	{
		c = d->prev;
		c->next = d;
	}
	
	h->prev = d;
	if (h->next == 0)
		h->next = d; 
	//  old question: wouldn't it be d->next = b ????
// it's done in this weird way so you can either start from the head and go up and down teh list...	

}

// not in this module.. actually at 8fd0.. called by show_obj() too
// create a BLIT thingy
// createa  blit struct
BLIT *blit_new(VIEW *v)
{
	BLIT *b;

	b = a_malloc(sizeof(BLIT));
	b->prev = 0;
	b->next = 0;
	b->v = v;
	b->x = v->x;
	b->y = (v->y) - (v->y_size) + 1;
	b->x_size = v->x_size;
	/*if (display_type == 2)	// HGC man
	{
		// do boring HGC stuff that I frankly don't care about.
		// probably to take into account the higher res
	}*/

	b->y_size = v->y_size;
	b->buffer = a_malloc( (v->x_size) * (v->y_size) );
	v->blit = b;
	return b;
}


void blitlist_draw(BLIT *h)
{
	BLIT *b;
	
	for ( b=h->next ; b !=0 ; b=b->next )
	{
		blit_save(b);
		obj_blit(b->v);
	}
}

// update the blit list
// redraws all the cels in order and then updates the pos variables
void blitlist_update(BLIT *h)
{
	VIEW *v;
	BLIT *b;
	
	for ( b=h->prev ; b !=0 ; b=b->prev )
	{
		v = b->v;	// view table
		obj_cel_update(v);
		if (v->step_count == v->step_time)
		{
			if ( (v->x == v->x_prev) && (v->y == v->y_prev) )
				v->flags |= O_MOTIONLESS;	// bit 14
			else
			{
				v->x_prev = v->x;
				v->y_prev = v->y;
				v->flags &= ~O_MOTIONLESS;
			}
		}
	}
}


u8 *cmd_animate_obj(u8 *c)
{
	obj_animate(*(c++));
	return c;	
}

static void obj_animate(u8 num)
{
	VIEW *v;
	
	v = &objtable[num];  // sizeof(view)
	
	if (v >= objtable_tail)
		set_agi_error(0x0D, num);
	if (  (v->flags & O_ANIMATE) == 0)
	{
		v->flags = O_UPDATE|O_CYCLE|O_ANIMATE;
		v->motion = MT_NORM;
		v->cycle = CY_NORM;
		v->direction = 0;
	}
}

u8 *cmd_unanimate_all(u8 *c)
{
	VIEW *v;
	
	blists_erase();
	
	for (v=objtable ; v<objtable_tail ; v++)
		v->flags &= ~(O_ANIMATE|O_DRAWN);
	
	return c;
}



// picks the loop depending on the direction
#define RIGHT 0
#define LEFT 1
#define DOWN 2
#define UP 3
#define IGNORE 4
static u8 loop_small[] = {IGNORE, IGNORE, RIGHT,RIGHT,RIGHT, IGNORE,LEFT,LEFT,LEFT,RIGHT};	//8d5
static u8 loop_large[] = {IGNORE,    UP,    RIGHT,RIGHT,RIGHT, DOWN, LEFT,LEFT,LEFT,RIGHT};	// 8df
// if loop == 4 then it doesn't change it (ie, moving characters only have 0-3 loops or something

void objtable_update()
{
	u8 new_loop;
	u16 touched;
	VIEW *v;
		
	touched = 0;

	for (v=objtable ; v<objtable_tail ; v++)
	{
		if ((v->flags & (O_DRAWN|O_ANIMATE|O_UPDATE)) == (O_DRAWN|O_ANIMATE|O_UPDATE))	// 0, 4, 6
		{
			touched++;	// good for 65535 touchings
			new_loop = IGNORE;
			
			// if loop released (ie, agi picks the loop depending on direction)
			if ( (v->flags & O_LOOPFIXED) == 0)	// flag 13
			{
				
				if ( (v->loop_total==2)||(v->loop_total==3) )
					new_loop =  loop_small[v->direction];
				else if (v->loop_total==4)
					new_loop = loop_large[v->direction];
				else if (c_game_loop_update!=L_FOUR)
				{
					if ((c_game_loop_update==L_ALL)&&(v->loop_total>4))
						new_loop = loop_large[v->direction];
					else if (c_game_loop_update==L_FLAG)
					{
						if ((flag_test(0x14)!=0)&&(v->loop_total>4))
							new_loop = loop_large[v->direction];
					}
				}
			}
			if (v->step_count == 1)
				if (new_loop != IGNORE)
					if (v->loop_cur != new_loop)
					{
						obj_loop_set(v, new_loop);
					}
					
			if ((v->flags & O_CYCLE) != 0)	// if cycling (flag 5)
				if (v->cycle_count != 0)
				{
					v->cycle_count--;
					if (v->cycle_count == 0)
					{
						obj_loop_update(v);
						v->cycle_count = v->cycle_time;
					}
				}
		}
	}

	if (touched != 0) 
	{
		blitlist_erase(&blitlist_updated);
		objs_step_update();
		blitlist_draw(build_updated_list());
		blitlist_update(&blitlist_updated);
		objtable->flags &= ~(O_LAND|O_WATER);
	}

	
}
#undef RIGHT
#undef LEFT
#undef DOWN
#undef UP
#undef IGNORE
