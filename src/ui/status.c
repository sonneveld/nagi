/*
CmdStatus                        cseg     000031D8 0000002B
_Inventory                       cseg     00003203 00000143
_DisplayInvnt                    cseg     00003346 000000B6
_InventKey                       cseg     000033FC 00000068
_SwapInventColr                  cseg     00003464 00000059
_StatusWrite                     cseg     000034BD 0000008A
CmdStatusLineOn                  cseg     00003547 00000015
CmdStatusLneOff                  cseg     0000355C 0000001F
*/

#include <string.h>

#include "../agi.h"
#include "../ui/status.h"

#include "../flags.h"
#include "../objects.h"

#include "../ui/events.h"
#include "../ui/agi_text.h"
#include "../ui/window.h"
#include "../ui/cmd_input.h"
#include "../ui/printf.h"

#include "../sys/endian.h"
#include "../sys/chargen.h"
u16 invent_state = 0;

/*
u8 *cmd_status(u8 *c)
{
	u8 *obj;
	u8 *name;
	u8 msg[1000];

	strcpy(msg, "Inventory:\n---------------\n\n");
	
	obj = object;
	while (obj <= object_name)
	{
		if (obj[2] == 255)
		{
			strcat(msg, object +  load_le_16(obj));
			strcat(msg, "\n");
			//printf("%s\n", object +  load_le_16(obj));
		}
		obj += 3;
	}
	strcat(msg, "\n---------------");
	print(msg);
	return c;
}
*/



u8 *cmd_status(u8 *c)
{
	input_edit_on();		// turn any sound off
	text_attrib_push();
	text_colour(0, 0xF);	// text colour attributes
	cmd_text_screen(0);

	inventory();		// display/manipulate inventory
	invent_state = 0;
	text_attrib_pop();
	screen_redraw();
	return c;
}




void inventory(void)
{
	u16 col;
	u16 row;
	u16 item_count;
	INVENT *item_last;
	INVENT *item_cur;
	INVENT invent_item[50];
	int obj_cur;		// object pointer
	INVENT *si;
	AGI_EVENT *di;
	
	row = 2;
	col = 0;
	si = invent_item;
	item_cur = invent_item;
	obj_cur = 0;
	item_count = 0;

	while (obj_cur < inv_obj_table_size)
	{
		if (inv_obj_table[obj_cur].location == 0xFF)
		{
			if (item_count == state.var[V25_ITEM])
				item_cur = si;
		
			si->num = item_count;
			si->name = inv_obj_string + inv_obj_table[obj_cur].name;
			si->row = row;
		
			if ( (col & 1) == 0)
				si->col = 1;
			else
			{
				row++;
				si->col  = 39 - strlen(si->name);
			}
		
			col ++;
			si ++;
		}
		obj_cur ++;
		item_count ++;
	}

	if (col == 0)
	{
		si->num = 0;		// object number
		si->name = "nothing";	// name
		si->row = row;		// row (y)
		si->col = 16;		// col (x)
		si ++;
	}
	
	item_last = si-1;		
	si = item_cur;
	invent_display(invent_item, item_last, item_cur);
	if (flag_test(F13_STATSELECT) == 0)
		event_wait();
	else
	{
		invent_state = 1;
	key_loop:
		di = event_wait();
#warning need the joystick BUTT map..
		//joy_butt_map(di);
		
		switch(di->type)
		{
			case 1:			
				if (di->data == 0x0D)	// enter
				{
					state.var[V25_ITEM] = si->num;
					return;
				}
				if (di->data == 0x1B)	// esc
				{
					state.var[V25_ITEM] = 0xFF;
					return;
				}
				break;
			case 2:
				si = invent_key(invent_item, item_last, si, di->data);
				break;
		}
		
		goto key_loop;
	}

}

void invent_display(INVENT *invent, INVENT *last, INVENT *current)
{
	INVENT *di;
	
	goto_row_col(0, 11);
	agi_printf("You are carrying:");

	for (di=invent; di<=last; di++)
	{
		goto_row_col(di->row, di->col);
		if ( (current != di) || (flag_test(F13_STATSELECT) == 0) )
			text_colour(0, 15);
		else 
			text_colour(15, 0);	// selected
		agi_printf(di->name);
	}

	text_colour(0, 15);
	if (flag_test(F13_STATSELECT) != 0)
	{
		goto_row_col(24, 2);
		agi_printf("Press ENTER to select, ESC to cancel");
	}
	else
	{
		goto_row_col(24, 4);
		agi_printf("Press a key to return to the game");
	}
}

INVENT *invent_key(INVENT *invent, INVENT *last, INVENT *current, u16 direction)
{
	INVENT *si;
	
	si = current;

	switch (direction)
	{
		case 1:
			si -= 2;	// UP
			break;
		case 3:
			si ++;	// RIGHT
			break;
		case 5:
			si += 2;	// DOWN
			break;
		case 7:
			si --;		// LEFT
			break;
	}
	
	if ( (si<invent) || (si>last) )
		si = current;
	return( invent_swap_colour(current, si) );
}


INVENT *invent_swap_colour(INVENT *item_cur, INVENT *item_new)
{
	if (item_cur != item_new)
	{
		text_colour(15, 0);
		goto_row_col(item_new->row, item_new->col);
		agi_printf(item_new->name);

		text_colour(0, 15);
		goto_row_col(item_cur->row, item_cur->col);
		agi_printf(item_cur->name);
	}

	return(item_new);
}


void status_line_write()
{
	push_row_col();
	text_attrib_push();

	if (state.status_state != 0)
	{
		window_line_clear(state.status_line_row, 0xFF);	// clear the line at row word5db
		text_colour(0, 0x0F);

		goto_row_col(state.status_line_row, 1);
		agi_printf("Score:%d of %d", state.var[V03_SCORE], state.var[V07_MAXSCORE]);
		goto_row_col(state.status_line_row, 0x1E);

		if (flag_test(F09_SOUND) == 0)
			agi_printf("Sound:%s", "off");
		else
			agi_printf("Sound:%s", "on");
	}

	text_attrib_pop();
	pop_row_col();
	ch_update();
}

u8 *cmd_status_line_on(u8 *c)
{
	state.status_state = 1;
	status_line_write();
	return c;
}

u8 *cmd_status_line_off(u8 *c)
{
	state.status_state = 0;
	window_line_clear(state.status_line_row, 0);
	ch_update();
	return c;
}
