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


// MOUSE EXTENSION.. THE BUFF BRI WAY

// return the stack of button presses when cmd171 is pressed
// else just the position of the mouse

u8 *cmd_brian_poll_mouse(u8 *c)
{
	/*
	int get_butt, get_x, get_y;
	
	if (mstack_pop(&get_butt, &get_x, &get_y) != 0)
	{
		// if a button press isn't on stack then it's just a polled position
		get_butt=0;
		SDL_GetMouseState(&get_x, &get_y);
	}
	
	state.var[27] = get_butt;
	state.var[28] = get_x/vstate.scale/2;
	state.var[29] = get_y/vstate.scale;
	return c;*/
}
