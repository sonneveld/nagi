/*
_ClearKbWatch                    cseg     00004C23 0000001A
CmdSetKey                        cseg     00004C3D 00000066
*/

#include <string.h>

#include "../agi.h"

#include "../ui/events.h"
#include "../ui/controller.h"

#include "../sys/endian.h"


u8 control_state[CONTROL_MAX];

void control_state_clear()
{
	memset(control_state, 0, sizeof(control_state));
}

u8 *cmd_set_key(u8 *c)
{
	u16 cont_num;
	u16 i;
	u16 key;

	key = load_le_16(c);
	c += 2;
	cont_num = *(c++);	// controller number
	
	for (i=0; i<39; i++)
		if (state.control_map[i].key==0)
		{
			state.control_map[i].key = key;
			state.control_map[i].num = cont_num;
			break;
		}
			
	return c;
}


// if given ascii key is special (ie, in some table), set it to type 3
AGI_EVENT *control_key_map(AGI_EVENT *agi_event)
{
	CMAP *c;
	
	if ( agi_event != 0)
		if (agi_event->type == 1)	// ascii character.. we HOPE!!!
		{
			c = state.control_map;
			while (c->key != 0)
			{
				if (agi_event->data == c->key)
				{
					agi_event->type = 3;
					agi_event->data = c->num;
					break;
				}
				c++;
			}
		}
		/*
		else if (agi_event->type == JOYSTICK)
		{
		check stuff
		}
		*/
	return agi_event;
}

