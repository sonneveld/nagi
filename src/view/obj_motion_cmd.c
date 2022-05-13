/*
CmdMoveObj                       cseg     00006C1D 0000007D
CmdMoveObjV                      cseg     00006C9A 000000A1
CmdFollowEgo                     cseg     00006D3B 00000066
CmdWander                        cseg     00006DA1 00000039
CmdNormalMotion                  cseg     00006DDA 00000027
CmdStopMotion                    cseg     00006E01 0000003D
CmdStartMotion                   cseg     00006E3E 00000039
CmdStepSize                      cseg     00006E77 0000003E
CmdStepTime                      cseg     00006EB5 00000042
CmdSetDir                        cseg     00006EF7 0000003E
CmdGetDir                        cseg     00006F35 00000033
CmdProgramContr                  cseg     00006F68 00000012
CmdPlayerContro                  cseg     00006F7A 0000001A 
*/

#include "../agi.h"

#include "../view/obj_base.h"
#include "../view/obj_motion_cmd.h"

// reset flag
#include "../flags.h"
// obj_move_update()
#include "../view/obj_motion.h"



u8 *cmd_move_obj(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	v->motion = MT_MOVE;
	v->move.x = *(c++);
	v->move.y = *(c++);
	v->move.step_size= v->step_size;
	
	if (*c != 0)
		v->step_size = *c;
	c++;
	v->move.flag = *(c++);
	flag_reset(v->move.flag);
	v->flags |= O_UPDATE;
	if ( v == objtable )
		state.ego_control_state = 0;
	
	obj_move_update(v);
	return c;
}


u8 *cmd_move_obj_v(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	v->motion = MT_MOVE;
	v->move.x = state.var[*(c++)];	// x
	v->move.y = state.var[*(c++)];	// y
	v->move.step_size= v->step_size;		// old step_size
	
	if (state.var[*c] != 0)
		v->step_size = state.var[*c];
	c++;
	v->move.flag = *(c++);			// flag
	flag_reset(v->move.flag);
	v->flags |= O_UPDATE;
	if ( v == objtable )
		state.ego_control_state = 0;
	obj_move_update(v);
	return c;
}

u8 *cmd_follow_ego(u8 *c)
{
	VIEW *v;

	v = &objtable[*(c++)];
	
	v->motion = MT_FOLLOW;
	if (*c <= v->step_size)
		v->follow.step_size = v->step_size;
	else
		v->follow.step_size = *c;
	c++;
	v->follow.flag = *(c++);
	flag_reset(v->follow.flag);
	v->follow.count = 0xFF;
	v->flags |= O_UPDATE;
	return c;
}


u8 *cmd_wander(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	if ( v == objtable)
		state.ego_control_state = 0;
	v->motion = MT_WANDER;
	v->flags |= O_UPDATE;
	return c;
}


u8 *cmd_normal_motion(u8 *c)
{
	objtable[*(c++)].motion = MT_NORM;
	return c;
}


u8 *cmd_stop_motion(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->direction = 0;
	v->motion = MT_NORM;
	if ( v == objtable)
	{
		state.var[V06_DIRECTION] = 0;	// ego direction
		state.ego_control_state = 0;
	}
	return c;
}


u8 *cmd_start_motion(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->motion = MT_NORM;
	if ( v == objtable)
	{
		state.var[V06_DIRECTION] = 0;	// ego direction
		state.ego_control_state = 1;
	}
	return c;
}

u8 *cmd_step_size(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	temp->step_size = state.var[*(c++)];
	return c;
}

u8 *cmd_step_time(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->step_count = state.var[*(c++)];
	v->step_time = v->step_count;
	return c;
}


u8 *cmd_set_dir(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	temp->direction = state.var[*(c++)];
	return c;
}


u8 *cmd_get_dir(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	state.var[*(c++)] = temp->direction;
	return c;
}


u8 *cmd_program_control(u8 *c)
{
	state.ego_control_state = 0;
	return c;
}


u8 *cmd_player_control(u8 *c)
{
	state.ego_control_state = 1;
	if (objtable->motion != MT_EGO)
		objtable->motion = MT_NORM;
	return c;
}

