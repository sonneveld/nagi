/*
CmdSetMenu                       cseg     00009056 000000B2
CmdSetMenuItem                   cseg     00009108 000000EB
CmdSubmitMenu                    cseg     000091F3 00000034
CmdEnableItem                    cseg     00009227 00000020
_MenuEnableAll                   cseg     00009247 00000032
CmdDisableItem                   cseg     00009279 0000001F
_MenuItemSet                     cseg     00009298 00000052

CmdMenuInput                     cseg     000092EA 00000020
_MenuInput                       cseg     0000930A 00000186
_MenuDraw                        cseg     00009490 00000052
_MenuClear                       cseg     000094E2 00000027
_MItemSelected                   cseg     00009509 00000055
_MenuNameDraw                    cseg     0000955E 00000066
_MenuCalcSize                    cseg     000095C4 00000073
*/

#include <stdlib.h>
#include <string.h>

#include "../agi.h"
#include "../sys/drv_video.h"
#include "../sys/gfx.h"
#include "../ui/menu.h"
#include "../ui/msg.h"
#include "../flags.h"
#include "../ui/window.h"
#include "../ui/agi_text.h"
#include "../ui/status.h"
#include "../ui/events.h"
#include "../ui/printf.h"
#include "../sys/chargen.h"
#include "../sys/vid_render.h"

u16 menu_next_input = 0;

u16 menu_col = 0;
u16 menu_item_row = 0;
u16 menu_item_col = 0;
u16 menu_submit = 0;			// 1 = submited.. no later changes possible

MENU *menu_head = 0;
MENU *menu_cur = 0;
MENU_ITEM *menu_item_cur = 0;	// menuE ptr cur
//u16 menu_size = 0;			// menu size  ah = width  al = height
//u16 menu_pos = 0;			// menu position ah = col  al = row

u8 menu_size_width = 0;
u8 menu_size_height = 0;
u8 menu_pos_x;
u8 menu_pos_y;

// mouse stuff?
u16 menu_drawn = 0;


u8 *cmd_set_menu(u8 *c)
{
	u8 *m_name;
	MENU *m;
	
	m_name = strdup(logic_msg(*(c++)));
	if (menu_submit != 1)
	{
		m = malloc(sizeof(MENU));
		if ( menu_head == 0)
		{
			menu_head = m;
			menu_col = 1;
		}
		else
		{
			if ( menu_cur->head == 0)
				menu_cur->status = 0;
			menu_cur->next = m;
			m->prev = menu_cur;
		}
		m->next = menu_head;
		menu_head->prev = m;
		menu_cur = m;
		m->row =  0;
		m->size = 0;
		m->name = m_name;
		m->col = menu_col;
		m->status = 1;
		m->head = 0;
		menu_col += strlen(m_name) + 1;
		menu_item_cur = 0;
		menu_item_row = 1;
	}
	return c;
}


u8 *cmd_set_menu_item(u8 *c)
{
	MENU_ITEM *mi_temp;
	MENU_ITEM *mi;
	u8 cont;
	u8 *name;
	
	name = strdup(logic_msg(*(c++)));
	cont = *(c++);
	
	if ( menu_submit != 1)
	{
		mi = malloc(sizeof(MENU_ITEM));
		if ( menu_item_cur == 0)
		{
			menu_cur->head = mi;
			menu_cur->cur = mi;
			mi->prev = mi;
		}
		else
		{
			menu_item_cur->next = mi;
			mi->prev = menu_item_cur;
		}
		mi->next = menu_cur->head;
		mi_temp = menu_cur->head;
		mi_temp->prev = mi;
		menu_item_cur = mi;
		mi->name = name;
		if ( menu_item_row == 1)
		{
			if ((strlen(name) + menu_cur->col) < 0x27)
				menu_item_col = menu_cur->col;
			else
				menu_item_col = 0x27 - strlen(name);
		}
		menu_item_row++;
		mi->row = menu_item_row;
		mi->col  = menu_item_col;
		mi->status = 1;
		mi->control = cont;
		menu_cur->size ++;
	}
	return c;
}


u8 *cmd_submit_menu(u8 *c)
{
	if (menu_cur->head == 0)
		menu_cur->status = 0;
	//set_mem_rm0();
	menu_cur = menu_head;
	menu_item_cur = menu_cur->head;
	menu_submit = 1;
	return c;
}


u8 *cmd_enable_item(u8 *c)
{
	menu_item_set(*(c++) , 1);
	return c;
}

// enable all items?  called by cmdrestart and restore
void menu_enable_all(void)
{
	MENU *si;
	MENU_ITEM *di;
	
	si = menu_head;
	if ( si != 0)
	{
		do
		{
			if ( si->status != 0)
			{
				di = si->head;
				do
				{
					di->status= 1;
					di = di->next;
				}
				while ( si->head != di);
			}
			si = si->next;
		} while ( menu_head != si);
	}
}

u8 *cmd_disable_item(u8 *c)
{
	menu_item_set(*(c++), 0);
	return c;
}

// change menu item stat
void menu_item_set(u16 cont, u16 new_state)
{
	MENU_ITEM *temp2;
	MENU *di;
	di = menu_head;
	do
	{
		if ( di ->status != 0)
		{
			temp2 =  di->head;
			do
			{
				if ( cont == temp2->control)
				{
					if ( new_state == 1)
						temp2->status = 1;
					else
						temp2->status = 0;
				}
				temp2 = temp2->next;
			}
			while (temp2 != di->head);
		}
		di = di->next;
	}
	while (menu_head != di);
}


u8 *cmd_menu_input(u8 *c)
{
	if ( flag_test(0xE) != 0)
		menu_next_input = 1;
	return c;
}

u8 *cmd_unknown_177(u8 *c)
{
	state.menu_state = *(c++);
	return c;
}


void menu_input()
{
	AGI_EVENT *temp2;
	MENU *si;
	MENU_ITEM *di;
	
	if (state.menu_state == 0)
		return;
	
	push_row_col();
	text_attrib_push();
	window_line_clear(0, calc_text_bg(0xF));
	ch_update();
	si = menu_head;
	do
	{
		menu_name(si);
		si = si->next;
	}
	while (menu_head != si);
	si = menu_cur;
	di = menu_item_cur;
	menu_draw(si);
	menu_drawn = 1;

	
menu_loop:
	temp2 = event_wait();
	joy_button_map(temp2);
	//useless_tandy(temp2);
	switch (temp2->type)
	{
		case 1:
			switch(temp2->data)
			{
				case 0xD:	// enter
					if ( di->status == 0)
						break;
					event_write(3, di->control);
					// continue on to quit.. ie.. no break here
				case 0x1B:	// esc
					menu_clear(si, di);
					text_attrib_pop();
					pop_row_col();
					if ( state.status_state != 0)
						status_line_write();
					else
					{
						window_line_clear(0,0);
						ch_update();
					}
					menu_next_input = 0;
					menu_drawn = 0;
					return;
			}
			break;
			
		case 2:
			switch (temp2->data)
			{
				case 1:	// up
					menu_item_name(di);
					di = di->prev;
					menu_item_name_invert(di);
					break;
				
				case 2:	// pgup // top of menu
					menu_item_name(di);
					di = si->head;
					menu_item_name_invert(di);
					break;
				
				case 3:	// right
					menu_clear(si, di);
					do
					{
						si = si->next;
					}
					while (si->status == 0);
					di = si->cur;
					menu_draw(si);
					break;
					
				case 4:	// pgdn // bottom of menu
					menu_item_name(di);
					di = (si->head)->prev;
					menu_item_name_invert(di);
					break;
				
				case 5:	// down
					menu_item_name(di);
					di = di->next;
					menu_item_name_invert(di);
					break;
				
				case 6:	// home - last  menu
					menu_clear(si, di);
					si = menu_head->prev;
					di = si->cur;
					menu_draw(si);
					break;
				
				case 7:	// left
					menu_clear(si, di);
					do
					{
						si = si->prev;
					}
					while (si->status == 0);
					di = si->cur;
					menu_draw(si);
					break;
				
				case 8:	// end - last menu	
					menu_clear(si, di);
					si = menu_head;
					di = si->cur;
					menu_draw(si);
					break;
				
				default:	// NONE?
			}
			break;
	}
	
	menu_cur = si;
	menu_item_cur = di;
	goto menu_loop;
}


void menu_draw(MENU *m)
{
	MENU_ITEM *di;
	
	menu_name_invert(m);
	menu_calc_size(m);
	gfx_msgbox(menu_pos_x, menu_pos_y, menu_size_width, menu_size_height, 0x0F , 0);
	di = m->head;
	if ( di != 0 )
	{
		do
		{
			if ( m->cur == di)
				menu_item_name_invert(di);	// one colour?
			else
				menu_item_name(di);	// other colour?
		
			di = di->next;
		}
		while ( m->head != di);
	}
}


void menu_clear(MENU *m, MENU_ITEM *mi)
{
	m->cur = mi;
	menu_name(m);
	render_update(menu_pos_x, menu_pos_y, menu_size_width, menu_size_height);
}


// and yes..i  know I can muck around with unions

// selected menu item
void menu_item_name_invert(MENU_ITEM *mi)
{
	text_colour(do_nothing(0x0F), calc_text_bg(0));
	goto_row_col(mi->row, mi->col);
	if ( mi->status == 0)
		text_shade = 1;
	agi_printf(mi->name);
	text_shade = 0;
}

// selected name of menu
void menu_name_invert(MENU *m)
{
	text_colour(do_nothing(0x0F), calc_text_bg(0));
	goto_row_col(m->row, m->col);
	if ( m->status == 0)
		text_shade = 1;
	agi_printf(m->name);
	text_shade = 0;
}



// normal unselected menu item
void menu_item_name(MENU_ITEM *mi)
{
	text_colour(do_nothing(0), calc_text_bg(0x0F));
	goto_row_col(mi->row, mi->col);
	if ( mi->status == 0)
		text_shade = 1;
	agi_printf(mi->name);
	text_shade = 0;
}

// normal name of menu
void menu_name(MENU *m)
{
	text_colour(do_nothing(0), calc_text_bg(0x0F));
	goto_row_col(m->row, m->col);
	if ( m->status == 0)
		text_shade = 1;
	agi_printf(m->name);
	text_shade = 0;
}

// calculate the menu size??
void menu_calc_size(MENU *m)
{
	menu_size_height = LINE_SIZE * (m->size + 2);
	menu_size_width = (strlen((m->head)->name) << 2) + 8;
	
	menu_pos_y = (m->size + 3 - state.window_row_min) * LINE_SIZE - 1;
	menu_pos_x = ((m->head)->col - 1)<<2;
}

