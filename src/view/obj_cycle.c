/*
CmdNormalCycle                   cseg     00006ABB 0000002C
CmdEndOfLoop                     cseg     00006AE7 0000003D
CmdReverseCycle                  cseg     00006B24 0000002C
CmdReverseLoop                   cseg     00006B50 0000003D
CmdCycleTime                     cseg     00006B8D 00000043
CmdStopCycling                   cseg     00006BD0 00000025
CmdStartCycling                  cseg     00006BF5 00000025
*/


#include "../agi.h"

#include "../view/obj_base.h"
#include "../view/obj_cycle.h"

#include "../flags.h"

u8 *cmd_normal_cycle(u8 *c)
{
	VIEW *v;
	
	v = &objtable[*(c++)];
	v->cycle = CY_NORM;
	v->flags |= O_CYCLE;
	return c;
}

u8 *cmd_end_of_loop(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->cycle = CY_END;
	v->flags |= (O_UPDATE|O_CYCLE|O_SKIPUPDATE);
	v->loop_flag = *(c++);
	flag_reset(v->loop_flag);
	return c;
}

u8 *cmd_reverse_cycle(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->cycle = CY_REV;
	v->flags |= O_CYCLE;
	return c;
}

u8 *cmd_reverse_loop(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->cycle = CY_REVEND;
	v->flags |= (O_UPDATE|O_CYCLE|O_SKIPUPDATE);
	v->loop_flag = *(c++);
	flag_reset(v->loop_flag);
	return c;
}

u8 *cmd_cycle_time(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->cycle_time = state.var[*(c++)];
	v->cycle_count = v->cycle_time;
	return c;
}

u8 *cmd_stop_cycling(u8 *c)
{
	objtable[*(c++)].flags &= ~O_CYCLE;
	return c;
}

u8 *cmd_start_cycling(u8 *c)
{
	objtable[*(c++)].flags |= O_CYCLE;
	return c;
}

