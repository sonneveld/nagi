 /* FUNCTION list        ---     ---     ---     ---     ---     ---     ---
_Start                           cseg     000066B0 0000004C
_LoadOVL                         cseg     000066FC 00000057

_RollOn                          cseg     00000078 00000036
_Exit                            cseg     000000AE 00000016
_ReadParameters                  cseg     000000C4 0000008C
_RunGame                         cseg     00000150 00000107
CmdPause                         cseg     00000257 00000028
CmdQuit                          cseg     0000027F 0000002F
_Finish                          cseg     000002AE 00000015
*/

// NAGIC
// Nick's Adventure Game Interpreter Clone ??
// New Adventure Game Interpreter Clone
// Nifty ...
// naughty


/* BASE headers	---	---	---	---	---	---	--- */
#include "agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
//#include <stdlib.h>
//#include <stdio.h>
#include <setjmp.h>

/* OTHER headers	---	---	---	---	---	---	--- */
#include "initialise.h"
#include "flags.h"
#include "sys/delay.h"
#include "sys/error.h"
#include "view/obj_base.h"
#include "view/obj_motion.h"
#include "ui/events.h"
#include "ui/controller.h"
#include "ui/status.h"
#include "ui/cmd_input.h"
#include "logic/logic_base.h"
#include "sys/ini_config.h"
#include "sys/mem_wrap.h"

// extra


#include "sys/chargen.h"
#include "ui/msg.h"
#include "ui/window.h"
#include "version/standard.h"
#include "res/res.h"

#include "sys/sys_dir.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
int main(int argc, char *argv[]);

/* VARIABLES	---	---	---	---	---	---	--- */
// external presumably to do with setjmp?
u16 old_score = 0;

/* CODE	---	---	---	---	---	---	---	--- */

int main(int argc, char *argv[])
{
	u16 snd_flag;
	
	(void) argc;	// SDL won't let me use these anyway
	(void) argv;	// i think

	dir_init(argv[0]);
	nagi_init();		// initialise NAGI
	
	standard_select_ng();
	agi_init();		// initialise AGI with version
	delay_init();	// initialise delay
	
	printf("\nEntering main AGI loop...\n");
	for (;;)
	{
		// reset all input vars
		control_state_clear();
		flag_reset(F02_PLAYERCMD);		// player has not issued command line
		flag_reset(F04_SAIDACCEPT);	// said command has not yet accepted the user input
		state.var[V19_KEYPRESSED] = 0;
		state.var[V09_BADWORD] = 0;
		
		//#warning need joy polling setup.
		//poll_joystick();	// poll the joystick
		//input_poll();	// read the events and do something with them
		
		// do_delay now calls input_poll during delay to decrease key lag
		do_delay();
		
		if (state.ego_control_state == 0)
			state.var[V06_DIRECTION] = objtable->direction;	// program control
		else
			objtable->direction = state.var[V06_DIRECTION];	// player control
		objs_dir_calc();
		
		old_score = state.var[V03_SCORE];
		snd_flag = flag_test(F09_SOUND);	// is sound on?

		// someone set us up the jump!
		setjmp(agi_err_state);

		while (logic_call(0) == 0)	// logic 0
		{
			// comes here if we need to restart logic0
			// newroom, restart, restore
			state.var[V09_BADWORD] = 0;
			state.var[V05_OBJBORDER] = 0;
			state.var[V04_OBJECT] = 0;
			flag_reset(2);	// player has not issued command line
                        old_score = state.var[V03_SCORE];
		} 

                objtable->direction = state.var[V06_DIRECTION];

		if ( (old_score!=state.var[V03_SCORE]) || (flag_test(F09_SOUND)!=snd_flag) )
			status_line_write();

		state.var[V05_OBJBORDER] = 0;
		state.var[V04_OBJECT] = 0;
		flag_reset(F05_NEWROOM);	// not in new room
		flag_reset(F06_RESTART);	// restart game not executed
		flag_reset(F12_RESTORE);	// restore game not executed

		// update graphics if not in textmode		
		if (chgen_textmode == 0)
			objtable_update();
	}
}
