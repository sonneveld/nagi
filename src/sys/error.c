/*
_MaybeLongBeep                   cseg     00003FBC 00000008
_BeepSpeaker                     cseg     00003FC5 00000023
_SetAGIError                     cseg     00003FE8 00000035
_PrintErrCode                    cseg     0000401D 00000095
*/

//~ RaDIaT1oN (2002-04-29):
//~ snprintf wrap

#include "../agi.h"
// set jump
#include <setjmp.h>
#include "error.h"

// for room_init();
#include "../initialise.h"
// clear events
#include "../ui/events.h"
// stop sound
#include "../sound/sound_base.h"
// print
#include "../ui/msg.h"
// input_edit_on
#include "../ui/cmd_input.h"

// sprintf
#include <stdio.h>
// string func
#include <string.h>
// errors
#include <errno.h>


jmp_buf agi_err_state;

// TODO: beep_speaker() incomplete
void long_beep()
{
	printf("BEEEEEEP! (long beep)\n");
}

void beep_speaker()
{
	printf("BEEP!\n");
}


AGI_NO_RETURN void set_agi_error(u16 err_type, u16 err_data)
{
	// TODO: set_agi_error() incomplete
	sound_stop();
	// clear_memory();
	events_clear();
	room_init();
	beep_speaker();
	beep_speaker();
	
	state.var[V17_ERROR] = err_type;
	state.var[V18_ERROR2] = err_data;
	
	longjmp(agi_err_state, 1);
}

/*
u16 agi_print_err_code()
{
	u8 str[100];
	input_edit_on();	// remove the cursor
	beep_speaker();
	beep_speaker();
	
	switch (error_occured)
	{
		case 0x000:
			strcpy(str, "Disk error.\n");
			break;
		case 0x002:
			strcpy(str, "The disk drive is not ready.\n");
			break;
		case 0x100:
			strcpy(str, "The disk is write protected.\n");
			break;
		default:
			sprintf(str, "Disk error:\n%s\n", err_msg[error_occured] );
	}
	strcat(str, "\nPress ENTER to try again.");
	strcat(str, "\nPress ESC to quit.");
	return message_box(str);
}
*/

u16 print_err_code()
{
	char str[100];
	input_edit_on();	// remove the cursor
	beep_speaker();
	beep_speaker();
	
	switch (errno)
	{
		default:
			snprintf(str, sizeof(str), "Disk error:\n%s\n", strerror(errno) );
	}
	strncat(str, "\nPress ENTER to try again.", sizeof(str)-strlen(str)-1);
	strncat(str, "\nPress ESC to quit.", sizeof(str)-strlen(str)-1);
	return message_box(str);
}

/*
#message 1 "discard.view(%v18):%m30"
#message 2 "set.view(%v18,_):%m26"
#message 3 "set.view(_,%v18):%m30"
#message 4 "set.loop(%v18,_):%m26"
#message 5 "set.loop(%v18,_):\nBad loop #.%m25"
#message 6 "set.loop(%v18,_):%m31"
#message 7 "set.cel(%v18,_):%m26"
#message 8 "set.cel(%v18,_):\nBad cel #.%m25"
#message 9 "sound(%v18):\nSound not loaded.%m25"
#message 10 "set.cel(%v18,_):%m31"
#message 11 "Script buffer overflow.\nMaximum size = %v18%m25"
#message 12 "erase(%v18):%m26"
#message 13 "animate.obj(%v18):%m26"
#message 14 "stop.update(%v18):%m28"
#message 15 "Bad test: %v18%m25"
#message 16 "Bad action: %v18%m25"
#message 17 "start.update(%v18):%m28"
#message 18 "draw.pic(%v18):%m32"
#message 19 "draw(%v18):%m26"
#message 20 "draw(%v18):%m31"
#message 21 "discard.pic(%v18):%m32"
#message 22 ""
#message 23 "get(%v18) or put(%v18):%m26"
#message 25 "\nPress ESC to quit."
#message 26 "%m27.%m25"
#message 27 "\nBad object number"
#message 28 "%m27 or object not drawn.%m25"
#message 29 "\nView not "
#message 30 "%m29loaded.%m25"
#message 31 "%m29set.%m25"
#message 32 "Picture not loaded.%m25"
*/
