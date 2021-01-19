/*
void objs_dir_calc()
void obj_motion_update(VIEW *v)
void obj_chk_block(VIEW *v)
void objs_step_update()

void obj_move_update(VIEW *v)
void obj_move_stop(VIEW *v)
u16 move16ed(s16 x, s16 y, s16 new_x, s16 new_y, u16 step_size)
u16 move172d(s16 disp, s16 step)
void ego_move(u16 ego_x, u16 ego_y)

u16 get_rand_dir()
void obj_wander_update(VIEW *v)

void obj_follow_update(VIEW *v)
*/

#include <stdlib.h>

#include "../agi.h"
#include "../flags.h"
#include "../sys/rand.h"

#include "obj_base.h"
#include "../view/obj_proximity.h"
#include "../view/obj_motion.h"
#include "../view/obj_block.h"
#include "../view/obj_picbuff.h"

void obj_motion_update(VIEW *v);
void obj_chk_block(VIEW *v);
void obj_move_stop(VIEW *v);
u16 move16ed(s16 x, s16 y, s16 new_x, s16 new_y, u16 step_size);
u16 move172d(s16 disp, s16 step);
u16 get_rand_dir(void);
void obj_wander_update(VIEW *v);
void obj_follow_update(VIEW *v);

s16 x_dir_mult[] = {0,0,1,1,1,0,-1,-1,-1};
s16 y_dir_mult[] = {0,-1,-1,0,1,1,1,0,-1};

// called from the game loop to update the direction all all animated sprites
void objs_dir_calc()
{
	VIEW *v;
	
	for (v=objtable; v<objtable_tail; v++)
		if ((v->flags & (O_DRAWN|O_ANIMATE|O_UPDATE))==(O_DRAWN|O_ANIMATE|O_UPDATE))
			if (v->step_count == 1)
				obj_motion_update(v);
} 

// update the animated objects with motion
void obj_motion_update(VIEW *v)
{
	switch(v->motion)
	{
		case MT_WANDER:
				obj_wander_update(v);
				break;
		case MT_FOLLOW:	
				obj_follow_update(v);
				break;
			case MT_EGO:
			case MT_MOVE:
				obj_move_update(v);
				break;
		default:
				;
	}

	if (state.block_state == 0)
		v->flags &= ~O_BLOCK;	// no block exists so why bother?
	else
		if ( ((v->flags&O_BLOCKIGNORE)==0) && (v->direction!=0) )
			obj_chk_block(v);
}


// for a change in the "block state"
// ie, you can stay in the block and not get out or stay out and not get in.
void obj_chk_block(VIEW *v)
{
	u16 stat;
	u16 x;
	u16 y;

	x = v->x;
	y = v->y;
	
	stat = block_chk_pos(x, y);
	
	x += v->step_size * x_dir_mult[v->direction];
	y += v->step_size * y_dir_mult[v->direction];
	
	if (stat == block_chk_pos(x, y))
		v->flags &= ~O_BLOCK;
	else
	{
		v->flags |= O_BLOCK;
		v->direction = 0;
		if (v == objtable)
			state.var[V06_DIRECTION] = 0;
	}
}

// updates all animated objs based on their step..  checks borders 'n stuff too
void objs_step_update()
{
	u16 border_code;
	s16 pos_y_orig;
	s16 pos_x_orig;
	VIEW *v;
	
	state.var[V05_OBJBORDER] = 0;
	state.var[V04_OBJECT] = 0;
	state.var[V02_BORDER] = 0;

	for (v=objtable; v<objtable_tail; v++)
		if ( (v->flags&(O_DRAWN|O_UPDATE|O_ANIMATE))==(O_DRAWN|O_UPDATE|O_ANIMATE))
		{
			if (v->step_count <= 1)
			{
				v->step_count = v->step_time;
				border_code = 0;			// touched nothing
				pos_x_orig = v->x;		// in case something HORRIBLE goes wrong
				pos_y_orig = v->y;
	
				if ((v->flags & O_REPOS) == 0) 	// flag 10
				{
					v->x += v->step_size * x_dir_mult[v->direction];
					v->y += v->step_size * y_dir_mult[v->direction];
				}
				
				if (v->x < 0)
				{
					v->x = 0;
					border_code = 4;		// left edge
				}
				else if ((v->x+v->x_size) > 0xa0) 
				{
					v->x = 160 - v->x_size;
					border_code = 2;		// right edge
				}
				
				if ((v->y-v->y_size) < -1)
				{
					v->y = v->y_size - 1;
					border_code = 1;		// top/horizon edge			
				}
				else if (v->y > 167) 
				{
					v->y = 167;
					border_code = 3;		// bottom edge
				}
				else if ( ((v->flags&O_HORIZONIGNORE)==0) && (state.horizon>=v->y) )
				{
					v->y = state.horizon + 1;
					border_code = 1;		// top/horizon edge
				}
				
				// check if the new position doesn't contact anything else
				if ( (obj_chk_contact(v)!=0) || (obj_chk_control(v)==0) )
				{
					v->x = pos_x_orig;
					v->y = pos_y_orig;
					border_code = 0;		// no touch
					obj_pos_shuffle(v);
				}
			
				if (border_code != 0) 
				{
					if (v->num == 0)
						state.var[V02_BORDER] = border_code;
					else
					{
						state.var[V04_OBJECT] = v->num;
						state.var[V05_OBJBORDER] = border_code;
					}
					
					if (v->motion == MT_MOVE)	// stop the moving object
						obj_move_stop(v);
				}
				
				v->flags &= ~O_REPOS; 	// it's been repositioned so now we can update it's movement
			}
			else
				v->step_count--;
		}
}


//  functions for the various motion types:

// MOVE MOTION

void obj_move_update(VIEW *v)
{
	v->direction = move16ed(v->x, v->y, v->move.x, v->move.y, v->step_size);
	if ( objtable == v)
		state.var[6] = v->direction;
	if (v->direction == 0)
		obj_move_stop(v);	// reached destination
}

// stop movement
void obj_move_stop(VIEW *v)
{
	v->step_size = v->move.step_size;
	// VERSION THREE
	if ( v->motion != MT_EGO)
		flag_set(v->move.flag);
	//
	v->motion = MT_NORM;
	if (v == objtable)
	{
		state.ego_control_state = 1;
		state.var[V06_DIRECTION] = 0;
	}
}

u16 worda7d[] = {8, 1, 2,    7, 0, 3,    6, 5, 4};

// find a new direction depending on the direction and size of the step
u16 move16ed(s16 x, s16 y, s16 new_x, s16 new_y, u16 step_size) // 8, a, c, e, 10
{ 
	u16 di;
	di = move172d(new_x-x, step_size);	// *2 for word
	di += move172d(new_y-y, step_size) * 0x3; // *2 for word
	return worda7d[di];
} 

u16 move172d(s16 disp, s16 step)
{
	if (-step >= disp)
		return 0;		// negative disp
	else if (step <= disp)
		return 2;		
	else
		return 1;
}

// v3 function that moves the ego.. presumably for mouse support
void ego_move(u16 ego_x, u16 ego_y)
{
	if (state.ego_control_state != 0)
	{
		objtable->motion = MT_EGO;
		if (ego_x < (objtable->x_size/2))
			objtable->move.x = -1;
		else
			objtable->move.x = ego_x - objtable->x_size/2;
		objtable->move.y = ego_y;
		objtable->move.step_size = objtable->step_size;
	}
}


// WANDER MOTION

u16 get_rand_dir()
{
	return (agi_rand() % 9);
}

void obj_wander_update(VIEW *v)
{
	s16 ax;
	ax = v->wander_count;
	v->wander_count--;	// count down until next direction change
	if ( (ax == 0) || ((v->flags & O_MOTIONLESS) != 0) )	// bit 14
	{
		v->direction = get_rand_dir();
		if (objtable == v)
			state.var[V06_DIRECTION] = v->direction;
		while (v->wander_count < 0x6)
			v->wander_count = agi_rand() % 0x33;			
	}
}
	
// FOLLOW MOTION

void obj_follow_update(VIEW *v)
{
	s16 temp8;
	s16 view_x;
	s16 ego_x;
	s16 dir_new;
	
	ego_x = objtable->x + objtable->x_size/2;
	view_x = v->x + v->x_size/2;
	dir_new = move16ed(view_x, v->y, ego_x, objtable->y, v->follow.step_size);
	
	if (dir_new == 0)
	{
		v->direction = 0;
		v->motion = MT_NORM;
		flag_set(v->follow.flag);
	}
	else
	{
		if ( (v->follow.count!=0xFF) && ((v->flags&O_MOTIONLESS) != 0) )
		{
			do
			{
				v->direction = get_rand_dir();
			}
			while (v->direction == 0);

			temp8 = ( (abs(v->y - objtable->y)+abs(view_x - ego_x)) >> 1) + 1;

			if ( temp8 <= v->step_size)
				v->follow.count = v->step_size;
			else
				do
				{
					v->follow.count = agi_rand() % 8; 
				}
				while ( v->follow.count < v->step_size);
		}
		else
		{
			if ( v->follow.count == 0xFF)
				v->follow.count = 0;		
			if (v->follow.count != 0)
			{
				if (v->follow.count > v->step_size)
					v->follow.count -= v->step_size;
				else
					v->follow.count = 0;
			}
			else
				v->direction = dir_new;
		}
	}	
}





