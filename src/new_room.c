/*
CmdNewRoom                       cseg     0000175C 00000017
CmdNewRoomV                      cseg     00001773 0000001F
_NewRoom                         cseg     00001792 0000011A
*/

#include "agi.h"

#include "sound/sound_base.h"
// input clear
#include "ui/events.h"
// controller
#include "ui/controller.h"
// stat
#include "ui/status.h"

// flags
#include "flags.h"
// memory
// script
#include "sys/script.h"
// lists new room (init.h)
#include "initialise.h"
// vt start
#include "view/obj_base.h"



#include "new_room.h"


#include "res/res.h"
#include "ui/cmd_input.h"
#include "logic/logic_base.h"
#include "trace.h"

// VERY IMPORTANT.. returns 0.. not the code pointer.

u8 *cmd_new_room(u8 *c)
{
	return (new_room( *(c++) )  );
}

u8 *cmd_new_room_v(u8 *c)
{
	return (new_room( state.var[*(c++)] ) );
}

u8 *new_room(u16 room_num)
{
	VIEW *si;
	
	sound_stop();
	//clear_memory();	// should be free'd in other functions
	events_clear();
	script_new();
	script_allow();

	for (si=objtable ; si<objtable_tail; si++)
	{
		si->flags &= ~(O_ANIMATE|O_DRAWN);
		si->flags |= O_UPDATE;
		si->cel_data = 0;
		si->view_data = 0;
		si->blit = 0;
		si->step_time = 1;
		si->step_count = 1;
		si->cycle_count = 1;
		si->cycle_time = 1;
		si->step_size = 1;
	}

	// VERSION THREE
	if (objtable->motion == MT_EGO)
	{
		objtable->motion = MT_NORM;
		state.var[V06_DIRECTION] = 0;
	}
	// 
	
	room_init();
	state.ego_control_state = 1;
	state.block_state = 0;
	state.horizon = 0x24;

	state.var[V01_OLDROOM] = state.var[V00_ROOM0];
	state.var[V00_ROOM0] = room_num;
	state.var[V05_OBJBORDER] = 0;
	state.var[V04_OBJECT] = 0;
	state.var[V16_EGOVIEWRES] = objtable->view_cur;

	state.var[V08_FREEMEM] = 10;
	
	// not in v2.936 and later
	//volumes_close();

	logic_load(room_num);
	if (trace_logic != 0)
	{
		logic_load_2(trace_logic);
	}

	switch(state.var[V02_BORDER])
	{
		case 1:	// move to bottom
			objtable->y = 0xA7;
			break;
		case 2:	// move to left
			objtable->x = 0;
			break;
		case 3:	// move to top
			objtable->y = 0x25;
			break;
		case 4:	// move to right
			objtable->x = 0xA0 - objtable->x_size;
			break;
	}
	
	state.var[V02_BORDER] = 0;
	flag_set(F05_NEWROOM);

	control_state_clear();
	status_line_write();
	input_redraw();

	return 0;
}
