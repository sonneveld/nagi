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
#include "version/gamelist.h"
#include "version/ver_init.h"

#include "sys/chargen.h"

#include "ui/msg.h"

#include "ui/window.h"

#include "version/standard.h"

#include "res/res.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
int main(int argc, char *argv[]);

/* VARIABLES	---	---	---	---	---	---	--- */
// external presumably to do with setjmp?
u16 old_score = 0;

/* CODE	---	---	---	---	---	---	---	--- */

//u8 *window_title = 0;

u8 *fb_buff;

void fbenchmark_init(void)
{
	fpos_t file_size;
	FILE *file_stream;

	file_stream=fopen("petshop.txt", "rb");
	
	if (file_stream == 0)
	{
		printf("can't open\n");
		agi_exit();
	}
	
	fseek(file_stream, 0, SEEK_END);
	fgetpos(file_stream, &file_size);
	fseek(file_stream, 0, SEEK_SET);

	fb_buff = (u8 *)a_malloc(file_size);
	
	if ( fread(fb_buff, sizeof(u8), file_size, file_stream) != file_size)
		if (print_err_code == 0)
			agi_exit();
		
	fclose(file_stream);
}


void font_benchmark(void)
{
	int ticks=0;
	int i = 0;
	POS my_pos;
	
	u8 *wbuff;
	
	// basic
	// inverted
	// shaded
	// inverted/shaded
	
wbuff = fb_buff;
		
		ticks = SDL_GetTicks();
		
		while (i < 100000)
		{
			switch (*wbuff)
			{
				case 0x08:
				case 0x0D:
				case 0x0A:
				case ' ':
					break;
				default:
					i++;
					
					window_put_char(*(wbuff));
					ch_pos_get(&my_pos);
					if (my_pos.row==24)
					{
						my_pos.row = 0;
						ch_pos_set(&my_pos);
						ch_update();
					}
			}
			wbuff++;
		}
		
		printf("time = %f per sec\n", (double) 100000000 / (double)(SDL_GetTicks() - ticks));
}

/*

	printf("starting font benchmark...bitch...\n");
	fbenchmark_init(); 
	
	printf("plain\n");
	text_colour(1, 0);
	font_benchmark();
	
	printf("invert\n");
	text_colour(1, 1);
	font_benchmark();	
	
	printf("shaded\n");
	text_shade = 1;
	font_benchmark();
	text_shade = 0;
	
	
	a_free(fb_buff);
	agi_exit();*/
	
int main(int argc, char *argv[])
{
	u16 snd_flag;
	GAMEINFO *g_cur;
	
	(void) argc;	// SDL won't let me use these anyway
	(void) argv;	// i think
	
	nagi_init();		// initialise NAGI
	standard_select_ng();
	agi_init();		// initialise AGI with version
	delay_init();	// initialise delay
	
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
		if (chgen_textmode == 0)
			objtable_update();
	}
}
