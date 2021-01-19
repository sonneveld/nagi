/*
CmdPosition                      cseg     00007B53 0000003D
CmdPositionV                     cseg     00007B90 0000004D
CmdGetPosn                       cseg     00007BDD 00000043
CmdReposition                    cseg     00007C20 00000090
CmdRepositionTo                  cseg     00007CB0 00000043
CmdRepositonToV                  cseg     00007CF3 00000053
CmdObjOnWater                    cseg     00007D46 00000025
CmdObjOnLand                     cseg     00007D6B 00000025
CmdObjOnAnythin                  cseg     00007D90 00000025
CmdSetHorizon                    cseg     00007DB5 00000018
CmdIgnoreHorizo                  cseg     00007DCD 00000025
CmdObserveHoriz                  cseg     00007DF2 00000025
*/

#include "../agi.h"

#include "../view/obj_update.h"
#include "../view/obj_base.h"
#include "../view/obj_picbuff.h"
// for obj_pos_shuffle



u8 *cmd_position(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	v->x_prev = *(c++);
	v->x = v->x_prev;
	
	v->y_prev = *(c++);
	v->y = v->y_prev;
	
	return c;
}

u8 *cmd_position_v(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	v->x_prev = state.var[*(c++)];
	v->x = v->x_prev;
	
	v->y_prev = state.var[*(c++)];
	v->y = v->y_prev;
	
	return c;
}

u8 *cmd_get_position(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	state.var[*(c++)] = v->x;
	state.var[*(c++)] = v->y;	
	return c;
}

u8 *cmd_reposition(u8 *c)
{
	s16 offset;
	VIEW *v;
		
	v = &objtable[*(c++)];
	
	v->flags |= O_REPOS;
	
	offset = (s8)state.var[*(c++)];	// SIGNED
	
	if ( (offset<0) && (v->x<(-offset)) )
		v->x = 0;
	else
		v->x += offset;
	
	offset = (s8)state.var[*(c++)];	// SIGNED
	
	if ( (offset<0) && (v->y<(-offset)) )
		v->y = 0;
	else
		v->y += offset;
	
	obj_pos_shuffle(v);		// make sure it's not on a control line ro something
	return c; 
}

u8 *cmd_reposition_to(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	v->x = *(c++);
	v->y = *(c++);
	v->flags |= O_REPOS;
	obj_pos_shuffle(v);	
	return c;
}

u8 *cmd_reposition_to_v(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	v->x = state.var[*(c++)];
	v->y = state.var[*(c++)];
	v->flags |= O_REPOS;
	obj_pos_shuffle(v);	
	return c;
}

u8 *cmd_obj_on_water(u8 *c)
{
	objtable[*(c++)].flags |= O_WATER;	//8
	return c;
}

u8 *cmd_obj_on_land(u8 *c)
{
	objtable[*(c++)].flags |= O_LAND;	//11
	return c;
}

u8 *cmd_obj_on_anything(u8 *c)
{
	objtable[*(c++)].flags &= ~(O_LAND|O_WATER);	// turn off 11, 8
	return c;
}

u8 *cmd_set_horizon(u8 *c)
{
	state.horizon = *(c++);
	return c;
}

u8 *cmd_ignore_horizon(u8 *c)
{
	objtable[*(c++)].flags |= O_HORIZONIGNORE;	// turn on bit 3
	return c;
}

u8 *cmd_observe_horizon(u8 *c)
{
	objtable[*(c++)].flags &= ~O_HORIZONIGNORE;	// turn of bit 3
	return c;
}

