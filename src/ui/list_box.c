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

/* PROTOTYPES	---	---	---	---	---	---	--- */
int list_box (u8 **list, int size, int init);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


#define LISTBOX_H 19
#define LISTBOX_W 0x22

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
	
	// what can i do?
	if (list == 0)
		return -1;
	
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
	list_pos.row = msgstate.tpos.row + msgstate.printed_height + 1;
	list_pos.col = msgstate.tpos.col;
	
	goto_row_col(list_pos.row, list_pos.col);
	agi_printf("heya there");

	// display:
	// arrow pointer
	// list
	// arrow on right displaying if there's more on the top or bottom
	
	// if size <= list_size
	// width = text_width
	// else width = text_width - 2 (for arrows pointing
	
	
	// print list at current position
	// crop width
	
	// high light first option with arrow
	
	// while not exiting
	// get key info
	// if enter/esc return num or -1
	// if arrow.. 
	
	// keys to support/. ent/esc/up/down
	
	
	// later on:
	// letters.. sort  / left/right/pgup/pgdn/home/end/mouse/
	
	
	ch_update();
	return -1;
}