/*
CmdSetPriority                   cseg     000079B9 00000030
CmdRlesPriority                  cseg     000079E9 00000025
CmdGetPriority                   cseg     00007A0E 00000033
CmdSetPriorityV                  cseg     00007A41 00000043
*/

#include "../agi.h"

#include "../view/obj_base.h"
#include "../view/obj_priority.h"

u8 *cmd_set_priority(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->flags |= O_PRIFIXED;
	v->priority = *(c++);
	return c;
}

u8 *cmd_release_priority(u8 *c)
{
	objtable[*(c++)].flags &= ~O_PRIFIXED;
	return c;
}

u8 *cmd_get_priority(u8 *c)
{
	VIEW *temp;
	temp = &objtable[*(c++)];
	state.var[*(c++)] = temp->priority;
	return c;
}

u8 *cmd_set_priority_v(u8 *c)
{
	VIEW *v;
	v = &objtable[*(c++)];
	v->flags |= O_PRIFIXED;
	v->priority = state.var[*(c++)];
	return c;
}