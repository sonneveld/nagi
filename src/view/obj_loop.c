/*
_UpdateView                      cseg     000048B3 000000C8
CmdFixLoop                       cseg     0000497B 00000028
CmdReleaseLoop                   cseg     000049A3 00000028
*/

#include "../agi.h"

#include "obj_base.h"
#include "view_base.h"

#include "../flags.h"

// updates the view depending on it's cycle 
void obj_loop_update(VIEW *v)
{
	u8 c;	// bp-1 
	u8 max; // bp-2

	if ( (v->flags & O_SKIPUPDATE) != 0 )
		v->flags &= ~O_SKIPUPDATE;
	else
	{
		c = v->cel_cur;
		max = v->cel_total - 1;
		switch (v->cycle)
		{
			case CY_NORM:	// normal
				c++;
				if ( c > max)
					c = 0;
				break;
				
			case CY_END:	// end.of.loop
				if ( c < max)
				{
					c++;
					if (c != max)
						break;
				}
				flag_set(v->loop_flag);
				v->flags &= ~O_CYCLE;
				v->direction = 0;
				v->cycle = 0;
				break;
				
			case CY_REVEND:	// reverse.loop
				if (c != 0)
				{
					c--;
					if (c != 0) break;
				}
				flag_set(v->loop_flag);
				v->flags &= ~O_CYCLE;
				v->direction = 0;
				v->cycle = 0;
				break;
				
			case CY_REV:	// reverse.cycle
				if (c != 0)
					c--;
				else
					c = max;
				break;
			default:
		}
		
		obj_cel_set(v, c);
	}
}

u8 *cmd_fix_loop(u8 *c)
{
	objtable[*(c++)].flags |= O_LOOPFIXED;	// 13	loop fixed
	return c;
}

u8 *cmd_release_loop(u8 *c)
{
	objtable[*(c++)].flags &= ~O_LOOPFIXED;	// 13	released
	return c;
}