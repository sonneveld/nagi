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
#include "sys/video.h"
#include "ui/cmd_input.h"
#include "logic/logic_base.h"
#include "sys/ini_config.h"
#include "sys/mem_wrap.h"

// extra
#include "version/gamelist.h"
#include "version/ver_init.h"


#include "ui/msg.h"


#include "ui/window.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
int main(int argc, char *argv[]);

/* VARIABLES	---	---	---	---	---	---	--- */
// external presumably to do with setjmp?
u16 old_score = 0;

/* CODE	---	---	---	---	---	---	---	--- */

u8 *window_title = 0;



void font_benchmark(void)
{
	fpos_t file_size;
	FILE *file_stream;
	u8 *buff;
	
	int ticks=0;
	int i = 0;
	POS my_pos;
	
	file_stream=fopen("petshop.txt", "rb");
	
	if (file_stream == 0)
	{
		printf("can't open\n");
		agi_exit();
	}
	
	fseek(file_stream, 0, SEEK_END);
	fgetpos(file_stream, &file_size);
	fseek(file_stream, 0, SEEK_SET);

	buff = (u8 *)a_malloc(file_size);
	
	if ( fread(buff, sizeof(u8), file_size, file_stream) != file_size)
		if (print_err_code == 0)
			agi_exit();
		
	fclose(file_stream);
	
	// basic
	// inverted
	// shaded
	// inverted/shaded
	
		text_shade = 1;
		
		ticks = SDL_GetTicks();
		
		while (i < 100000)
		{
			switch (*buff)
			{
				case 0x08:
				case 0x0D:
				case 0x0A:
				case ' ':
					break;
				default:
					i++;
					
					window_put_char(*(buff));
					sdlvid_pos_get(&my_pos);
					if (my_pos.row==24)
					{
						my_pos.row = 0;
						sdlvid_pos_set(&my_pos);
					}
			}
			buff++;
		}
		
		printf("time = %dms\n", SDL_GetTicks() - ticks);
		
	a_free(buff);
}

int main(int argc, char *argv[])
{
	u16 snd_flag;
	GAMEINFO *g_cur;

	(void) argc;	// SDL won't let me use these anyway
	(void) argv;	// i think
	
	nagi_init();		// initiailise NAGI
	
	//printf("starting font benchmark...bitch...\n");
	//font_benchmark();
	//agi_exit();
	
	ini_open("standard.ini");
	g_cur = standard_select();
	standard_init(g_cur);	// find the agi standard to emulate
	
	printf("%s\n", g_cur->name);
	// version
	printf("Game ID = \"%s\"\n", standard.game_id);
	printf("v%d.%03d\n", standard.ver_major, standard.ver_minor);
	printf("\n");
	
	if (g_cur->name != 0)
	{
		window_title = a_malloc(strlen(g_cur->name) + strlen("NAGI - ") + 10);
		sprintf(window_title, "%s - NAGI", g_cur->name);
	}
	else
		window_title = "NAGI";
	SDL_WM_SetCaption(window_title, 0);

	game_list_delete(g_cur);
	ini_close();
	
	agi_init();		// initiailise AGI with version
	delay_init();	// initiailise delay
	
	printf("\nEntering main AGI loop...\n");
	for (;;)
	{
		do_delay();

		control_state_clear();
		flag_reset(F02_PLAYERCMD);		// player has not issued command line
		flag_reset(F04_SAIDACCEPT);	// said command has not yet accepted the user input

		#warning need joy polling setup.
		//poll_joystick();	// poll the joystick
		input_poll();	// read the events and do something with them

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
		if (vstate.text_mode == 0)
			objtable_update();
	}
}
