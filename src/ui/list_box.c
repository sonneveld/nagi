/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>


/* OTHER headers	---	---	---	---	---	---	--- */
#include "list_box.h"
#include "msg.h"
#include "events.h"
#include "window.h"
#include "agi_text.h"
#include "printf.h"
#include "../sys/chargen.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
int list_box (u8 **list, int size, int init);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


#define LISTBOX_H 19
#define LISTBOX_W 0x22



// i would like to apologise for this crappy crappy code.. but at least it checks for buffer overflows
void list_print(u8 **list, TPOS *pos, SIZE *size, int up, int down)
{
	int i;
	
	u8 str[500];
	
	window_clear(pos->row, pos->col, pos->row+size->h-1, 
			pos->col+size->w-1, 0xFF);
	
	for (i=0; i<size->h; i++)
	{
		goto_row_col(pos->row+i, pos->col);
		
		if ((strlen(list[i]) > 550) || (size->w >= 500))
			agi_printf(" - *TOO LONG*");
		else
		{
			sprintf(str, " - %s", list[i]);
			str[size->w] = 0;
			agi_printf("%s", str);
		}
	}
	
	goto_row_col(pos->row, pos->col+size->w+1);
	if (up)
		agi_printf("U");
	else
		agi_printf(" ");
	
	goto_row_col(pos->row+size->h-1, pos->col+size->w+1);
	if (down)
		agi_printf("D");
	else
		agi_printf(" ");
}

void list_box_draw_arrow(u16 var8)
{
	goto_row_col(var8, msgstate.tpos.col);
	window_put_char(0x1A);
}

void list_box_draw_blank(u16 var8)
{
	goto_row_col(var8, msgstate.tpos.col);
	window_put_char(' ');
}


// return 0-(size-1)
// return -1 if canceled.
// list comprises of pointers to strings.. they will be cut to fit into screen
// only scrolls up and down
// first pointer = heading for listbox.. 
// size refers to the number of strings OTHER than the header
// init is the first item to point at.
int list_box(u8 **list, int size, int init)
{
	SIZE list_size;
	TPOS list_pos;
	
	int item_cur;
	int item_top;
	
	AGI_EVENT *state_event;
	
	// what can i do?
	if (list == 0)
		return -1;
	
	text_attrib_push();
	push_row_col();
	text_colour(0, 0x0F);
	
	item_cur = init;
	if (item_cur >= size)
		item_cur = size-1;
	if (item_cur < 0)
		item_cur = 0;
	
	// draw message box
	message_box_draw(list[0], LISTBOX_H, LISTBOX_W, 1);
	
	// get size of remaining msgbox
	list_size.h = msgstate.tpos_edge.row - msgstate.tpos.row - msgstate.printed_height;
	list_size.w = msgstate.tsize.w;
	if (size > list_size.h)
		list_size.w -= 2;
	list_pos.row = msgstate.tpos.row + msgstate.printed_height + 1;
	list_pos.col = msgstate.tpos.col;
	
	if (size <= list_size.h)
		item_top = 0;
	else
	{
		item_top = item_cur - ((list_size.h-1) / 2);
		if (item_top < 0)
			item_top = 0;
		if (item_top > (size-list_size.h)) 
			item_top = size-list_size.h;	
	}
	
	
	list_print(list+1+item_top, &list_pos, &list_size, 
			(item_top > 0), ((item_top + list_size.h) < size));
	
	list_box_draw_arrow(list_pos.row + item_cur - item_top);

	// later on:
	// letters.. sort  / left/right/pgup/pgdn/home/end/mouse/
	
	for(;;)
	{
		ch_update();
		state_event = event_wait();
		//joy_butt_map(state_event);
		switch (state_event->type)
		{
			case 1:
				switch (state_event->data)
				{
					case 0xD:
						return item_cur;
						
					case 0x1B:
						return -1;
				}
				break;
				
			case 2:
				list_box_draw_blank(list_pos.row + item_cur - item_top);
				switch(state_event->data)
				{
					case 1:
						if (item_cur > 0)
						{
							item_cur--;
							if (item_cur < item_top)
							{
								int scroll;
								scroll = (list_size.h * 3 )/ 4;
								if ((item_top - scroll) < 0)
									scroll = item_top;
								item_top -= scroll;
								list_print(list+1+item_top, &list_pos, &list_size, 
										(item_top > 0), ((item_top + list_size.h) < size));
							}
						}
					
						break;
						
					case 5:
						if (item_cur < (size -1))
						{
							item_cur++;
							if (item_cur > (item_top+list_size.h-1))
							{
								int scroll;
								scroll = (list_size.h * 3 )/ 4;
								if ((item_top+list_size.h-1+ scroll) > size)
									scroll = size - (item_top+list_size.h);
								item_top += scroll;
								list_print(list+1+item_top, &list_pos, &list_size, 
										(item_top > 0), ((item_top + list_size.h) < size));
							}
						}
						break;
						
					default:
				}
				list_box_draw_arrow(list_pos.row + item_cur - item_top);

				break;
		}
	}
	
	pop_row_col();
	text_attrib_pop();
	return -1;
}

