/*
_HasContact                      cseg     00004719 00000086
CmdIgnoreObjs                    cseg     0000479F 00000028
CmdObserveObjs                   cseg     000047C7 00000028
CmdDistance                      cseg     000047EF 000000C0
*/

#include <math.h>

#include "../agi.h"

#include "obj_base.h"
#include "obj_proximity.h"


// returns 1 if the given view's baseline has touched another view's baseline
// (ie, sharing same pixels)
// ignores unupdated view objects and objects that are set to ignore
u16 obj_chk_contact(VIEW *v)
{
	VIEW *c;	// current

	if ( (v->flags & O_OBJIGNORE) == 0)	// if ignore objects.. return 0
	{
		for (c=objtable ; c<objtable_tail ; c++)
		{
			if ((c->flags & (O_DRAWN|O_ANIMATE)) != (O_DRAWN|O_ANIMATE)) goto loc472e;	// 6 0
			if ((c->flags & O_OBJIGNORE) != 0) goto loc472e;	// 9
			if (v->num == c->num) goto loc472e;
			if ( (v->x + v->x_size) < c->x) goto loc472e;
			if ( (c->x + c->x_size) < v->x) goto loc472e;
		
			if (v->y == c->y)
				return 1;
			if (v->y > c->y)
				if ( v->y_prev < c->y_prev)
					return 1;
			if (v->y < c->y)
				if (v->y_prev > c->y_prev)
					return 1;
			
			loc472e:
		}
	}
	return 0;
}

u8 *cmd_ignore_objects(u8 *c)
{
	objtable[*(c++)].flags |= O_OBJIGNORE;		// 9
	return c;
}

u8 *cmd_observe_objects(u8 *c)
{
	objtable[*(c++)].flags &= ~O_OBJIGNORE;	// 9
	return c;
}

u8 *cmd_distance(u8 *c)
{
	VIEW *v1;
	VIEW *v2;
	u16 dis;
	
	v1 = &objtable[*(c++)];
	v2 = &objtable[*(c++)];
	
	if (  ((v1->flags & O_DRAWN) == 0) || ((v2->flags & O_DRAWN) == 0)  )
	{
		state.var[*(c++)] = 255;
	}
	else
	{
		dis = abs(v1->y - v2->y);
		dis += abs(v1->x + (v1->x_size/2) -  v2->x - (v2->x_size / 2) );
		if (dis > 0xFE)
			state.var[*(c++)] = 0xFE;	// so you dun get confused with 255 (error) I guess
		else
			state.var[*(c++)] = dis;
	}
	
	return c;
}