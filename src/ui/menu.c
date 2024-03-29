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


struct menu_item_struct
{
	struct menu_item_struct *next;	// 0-1	next
	struct menu_item_struct *prev; 	// 2-3 prev
	const char *name;				// 4-5
	u16 row; 				// 6-7
	u16 col;				// 8-9
	u16 status;				// a-b	// 1 = enable 0 = disable
	
	u16 control;			// c-d
};

typedef struct menu_item_struct MENU_ITEM;

struct menu_struct
{
	struct menu_struct *next;	// 0-1 next
	struct menu_struct *prev;	// 2-3 prev
	const char *name;				// 0x4-5
	u16 row;				// 6-7	// guesing it's row
	u16 col;				// 0x8-9
	u16 status;				// 0xA-b  -  0 = no items.. empty
	
	struct menu_item_struct *head;	// 0xC-0xD - menu_item head
	struct menu_item_struct *cur;	// 0xE-0xF - menu_item current
	u16 size;				// 0x10-0x11 - number of items
};

typedef struct menu_struct MENU;


static void menu_item_set(u16 cont, u16 new_state);
static void menu_leave(MENU *si, MENU_ITEM *di);
static void menu_draw(MENU *var8);
static void menu_clear(MENU *var8, MENU_ITEM *vara);
static void menu_item_name_invert(MENU_ITEM *mi);
static void menu_name_invert(MENU *m);
static void menu_item_name(MENU_ITEM *mi);
static void menu_name(MENU *m);
static void menu_calc_size(MENU *var8);


static int menu_next_input = 0;

static u16 menu_col = 0;
static u16 menu_item_row = 0;
static u16 menu_item_col = 0;
static u16 menu_submit = 0;			// 1 = submited.. no later changes possible

static MENU *menu_head = 0;
static MENU *menu_cur = 0;
static MENU_ITEM *menu_item_cur = 0;	// menuE ptr cur
//u16 menu_size = 0;			// menu size  ah = width  al = height
//u16 menu_pos = 0;			// menu position ah = col  al = row

static u8 menu_size_width = 0;
static u8 menu_size_height = 0;
static u8 menu_pos_x;
static u8 menu_pos_y;

// mouse stuff?
static u16 menu_drawn = 0;


int get_menu_requires_input_events(void)
{
	return menu_next_input;
}

u8 *cmd_set_menu(u8 *c)
{
	const char *m_name;
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
	const char *name;

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
	MENU *m;
	MENU_ITEM *mi;
	
	m = menu_head;
	if ( m != 0)
	{
		do
		{
			if ( m->status != 0)
			{
				mi = m->head;
				do
				{
					mi->status= 1;
					mi = mi->next;
				}
				while ( m->head != mi);
			}
			m = m->next;
		} while ( menu_head != m);
	}
}

u8 *cmd_disable_item(u8 *c)
{
	menu_item_set(*(c++), 0);
	return c;
}

// change menu item stat
static void menu_item_set(u16 cont, u16 new_state)
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
	if ( flag_test(F14_MENU) != 0)
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
	MENU *siTemp;
	MENU_ITEM *di;
	MENU_ITEM *diTemp;
	int hitMenu = 0;
	int hitMenuNext = 0;
	
	if (state.menu_state == 0)
		return;
	if (menu_head == 0)
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
					menu_leave(si,di);
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
					;
			}
			break;
		case 10 :			// Mouse
			hitMenu = font_size.w;
			for( siTemp = menu_head; ;
				siTemp = siTemp->next ) {
				hitMenuNext = hitMenu + ( ( strlen( siTemp->name ) + 1 ) * font_size.w );
				//printf( "%s %d %d %d\n", siTemp->name, hitMenu, hitMenuNext, temp2->x );
				if( temp2->x >= hitMenu &&
					temp2->x < hitMenuNext && temp2->y < font_size.w ) {
					menu_clear(si, di);
					si = siTemp;
					di = si->cur;
					break;
				}
				hitMenu = hitMenuNext;
					
				if( siTemp->next == menu_head ) {
					hitMenu = 0;
					break;
				}
			}
			if( hitMenu != 0 ) {
				menu_draw(si);
			} else {
				menu_calc_size(si);
				//printf( "%d %d %d\n", temp2->x, menu_pos_x << 2, ( menu_pos_x + menu_size_width ) << 2 );
				if( temp2->x >= menu_pos_x << 2 &&
					temp2->x < ( menu_pos_x + menu_size_width ) << 2 ) {
					menu_item_name(di);
					hitMenu = temp2->y / font_size.h;
					for( diTemp = si->head; ; diTemp = diTemp->next ) {
						if( diTemp->status && diTemp->row == hitMenu ) {
							di = diTemp;
							menu_item_name_invert( di );
							event_write(3, di->control);
							menu_leave(si, di);
							return;
						}
						if( diTemp->next == si->head ) {
							menu_leave(si,di);
							return;
						}
					}
				} else {
					menu_leave(si,di);
					return;
				}
			}
			break;
	}
	
	menu_cur = si;
	menu_item_cur = di;
	goto menu_loop;
}

static void menu_leave( MENU *si, MENU_ITEM *di )
{
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
}

static void menu_draw(MENU *m)
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


static void menu_clear(MENU *m, MENU_ITEM *mi)
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
static void menu_name_invert(MENU *m)
{
	text_colour(do_nothing(0x0F), calc_text_bg(0));
	goto_row_col(m->row, m->col);
	if ( m->status == 0)
		text_shade = 1;
	agi_printf(m->name);
	text_shade = 0;
}



// normal unselected menu item
static void menu_item_name(MENU_ITEM *mi)
{
	text_colour(do_nothing(0), calc_text_bg(0x0F));
	goto_row_col(mi->row, mi->col);
	if ( mi->status == 0)
		text_shade = 1;
	agi_printf(mi->name);
	text_shade = 0;
}

// normal name of menu
static void menu_name(MENU *m)
{
	text_colour(do_nothing(0), calc_text_bg(0x0F));
	goto_row_col(m->row, m->col);
	if ( m->status == 0)
		text_shade = 1;
	agi_printf(m->name);
	text_shade = 0;
}

// calculate the menu size??
static void menu_calc_size(MENU *m)
{
	menu_size_height = LINE_SIZE * (m->size + 2);
	menu_size_width = (strlen((m->head)->name) << 2) + 8;
	
	menu_pos_y = (m->size + 3 - state.window_row_min) * LINE_SIZE - 1;
	menu_pos_x = ((m->head)->col - 1)<<2;
}

