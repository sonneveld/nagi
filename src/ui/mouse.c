/*
THE DIFFERENT TYPES OF MICE ACCESS:
NONE
-------
Ignore any mouse movement

SIERRA
---------
Manipulate menus and inventory with mouse
double click acceses menu I think or looks
clicking on the screen moves the ego

BRIAN
--------
Manipulate menus and inventory with mouse
double click acceses menu I think or looks
clicking on screen entering position into a buffer
called cmd171 reads the buffer or returns the mouse position

NICK
------
set areas on the screen


*/

/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "../sys/script.h"
#include "menu.h"
#include "../list.h"
#include "../logic/cmd_table.h"
#include "events.h"
#include "../sys/drv_video.h"
#include "../sys/vid_render.h"
#include "../view/obj_motion.h"
#include "../sys/chargen.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

struct mouse_ext_struct
{
	u8 button;
	u16 x;
	u16 y;
};
typedef struct mouse_ext_struct M_EXT;

#define M_BUFF_SIZE 50
M_EXT mstack[M_BUFF_SIZE];
u8 mstack_index = 0;
// debug stuff
int mstack_max = 0;

void mstack_push(int button, int x, int y)
{
	if (mstack_index < M_BUFF_SIZE)
	{
		mstack[mstack_index].button = button;
		mstack[mstack_index].x = x;
		mstack[mstack_index].y = y;
		mstack_index++;
		if (mstack_index > mstack_max)
		{
			mstack_max = mstack_index;
			printf("mstack max = %d\n", mstack_max);
		}
	}
}

// 0 if succesful
// -1 if not
int mstack_pop(int *button, int *x, int *y)
{
	if (mstack_index > 0)
	{
		mstack_index--;
		*button = mstack[mstack_index].button;
		*x = mstack[mstack_index].x;
		*y = mstack[mstack_index].y;
		return 0;
	}
	else
		return -1;
}



struct mouse_area_struct
{
	int id;
	POS pos;
	SIZE size;
};
typedef struct mouse_area_struct M_AREA;

STACK *m_area_stack = 0;

void mouse_area_init(void)
{
	if (m_area_stack == 0)
		m_area_stack = stack_new(sizeof(M_AREA));
}

void mouse_area_shutdown(void)
{
	stack_free(m_area_stack);
	m_area_stack = 0;
}

void mouse_area_push(int id, int x, int y, int w, int h)
{
	NODE *n;
	n = stack_push(m_area_stack);
	((M_AREA *)n->contents)->id = id;
	((M_AREA *)n->contents)->pos.x = x;
	((M_AREA *)n->contents)->pos.y = y;
	((M_AREA *)n->contents)->size.w = w;
	((M_AREA *)n->contents)->size.h = h;
}

void mouse_area_pop(void)
{
	stack_pop(m_area_stack);
}

void mouse_area_clear(void)
{
	stack_clear(m_area_stack);
}

// -1 on error or if it doesn't register
// changes pos to relative pos
int mouse_area_check(POS *pos)
{
	int id = -1;
	
	if (m_area_stack != 0)
	{
		NODE *n;
		n = m_area_stack->tail;
		
		while ((n != 0) && (id == -1) )
		{
			//printf("id=%d x=%d y=%d w=%d h=%d\n", ((M_AREA *)n->contents)->id,((M_AREA *)n->contents)->pos.x,
			//((M_AREA *)n->contents)->pos.y, ((M_AREA *)n->contents)->size.h,
			//((M_AREA *)n->contents)->size.w);
			
			if ( (pos->x >= ((M_AREA *)n->contents)->pos.x)
				&& (pos->x < (((M_AREA *)n->contents)->pos.x + ((M_AREA *)n->contents)->size.w) )
				&& (pos->y >= (((M_AREA *)n->contents)->pos.y) )
				&& (pos->y < (((M_AREA *)n->contents)->pos.y + ((M_AREA *)n->contents)->size.h) )
				)
			{
				id = ((M_AREA *)n->contents)->id;
				pos->x -= ((M_AREA *)n->contents)->pos.x;
				pos->y -= ((M_AREA *)n->contents)->pos.y;
			}
			else
				n = n->prev;
		}
	}
	
	return id;
}


// MOUSE EXTENSION.. THE BUFF BRI WAY

// return the stack of button presses when cmd171 is pressed
// else just the position of the mouse

u8 *cmd_brian_poll_mouse(u8 *c)
{
	int get_butt, get_x, get_y;
	
	if (mstack_pop(&get_butt, &get_x, &get_y) != 0)
	{
		// if a button press isn't on stack then it's just a polled position
		get_butt=0;
		SDL_GetMouseState(&get_x, &get_y);
	}
	
	state.var[27] = get_butt;
	state.var[28] = get_x / c_vid_scale / rstate.drv->scale_x;

	state.var[29] = get_y / c_vid_scale / rstate.drv->scale_y;
	
	return c;
}

int status_prev;
int picbuff_prev;

void mouse_init(void)
{

	switch (c_game_mouse)
	{
		case M_BRIAN:
			cmd_table[171].func_name = "cmd.poll.mouse";
			cmd_table[171].func = cmd_brian_poll_mouse;
			cmd_table[171].param_total = 0;
			cmd_table[171].param_flag = 0;
			break;
		
		case M_SIERRA_V2:
			picbuff_prev = state.window_row_min;
			status_prev = state.status_line_row;
		
			mouse_area_init();
			// command entry
			// goes past bottom edge of screen though
			mouse_area_push(2, 
						0, (state.window_row_min + 21)* font_size.h, 
						40*font_size.w, 10*font_size.h);
			// game area
			mouse_area_push(1,
						0, state.window_row_min * font_size.h, 
						40*font_size.w, 21*font_size.h);
			
			// status bar
			mouse_area_push(0, 0, state.status_line_row*font_size.h, 40*font_size.w, 1*font_size.h);
		case M_SIERRA_V3:
		case M_NICK:
		default: 
			cmd_table[171].func_name = "cmd.push.script";
			cmd_table[171].func = cmd_unknown_171;
			cmd_table[171].param_total = 0;
			cmd_table[171].param_flag = 0;
			break;
	}

}


void mouse_event_handle(AGI_EVENT *event)
{
	POS pos;
	int id;
	
	switch (c_game_mouse)
	{
		case M_BRIAN:
			mstack_push(event->data, event->x, event->y);
			break;
		
		case M_SIERRA_V2:
			if ( (picbuff_prev != state.window_row_min) ||
				(status_prev != state.status_line_row) )
				mouse_init();
			pos.x = event->x;
			pos.y = event->y;
			id = mouse_area_check(&pos);
			//printf("Mouse Button! x=%d y=%d\n", pos.x, pos.y);
			pos.x = pos.x / c_vid_scale / rstate.drv->scale_x;
			pos.y = pos.y / c_vid_scale / rstate.drv->scale_y;
			switch (id)
			{
				case 0:	// status bar
					menu_input();
					break;
				case 1:	// picture buffer
					ego_move(pos.x, pos.y);
					break;
				case 2:	// lower area
					ego_move(pos.x, 169);	// 1 pixel past
					break;
			}
			break;
		
		case M_SIERRA_V3:
		case M_NICK:
		default:
			break;
	}
	
}




