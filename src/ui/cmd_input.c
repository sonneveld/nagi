/*
_InputPoll                       cseg     0000357C 000000D6
_InputPutChar                    cseg     00003652 000000D4
CmdCancelLine                    cseg     00003726 0000002D
CmdEchoLine                      cseg     00003753 00000052
_InputEcho                       cseg     000037A5 00000052
_InputEditOff                    cseg     000037F7 00000037
_InputEditOn                     cseg     0000382E 00000035
_InputEditStat                   cseg     00003863 0000000C
CmdPreventInput                  cseg     0000386F 00000029
CmdAcceptInput                   cseg     00003898 0000001C
CmdSetCursorChr                  cseg     000038B4 00000023
_InputRedraw                     cseg     000038D7 00000062
CmdOpenDialogue                  cseg     00003939 00000012
CmdCloseDialogu                  cseg     0000394B 00000012
*/

#include "../agi.h"

#include "../ui/cmd_input.h"


// events
#include "../ui/events.h"


#include "../ui/menu.h"
// line clear, goto pos
#include "../ui/window.h"
// variable msgstate.dialogue_open
#include "../ui/msg.h"
// agi string
#include "../agi_string.h"
// parsing
#include "../ui/parse.h"
// text_bg
#include "../ui/agi_text.h"
// agi_printf
#include "../ui/printf.h"

// objtable
#include "../view/obj_base.h"


// controller
#include "../ui/controller.h"

#include "../ui/mouse.h"

#include "../sys/chargen.h"


u16 input_edit_disabled = 0;
u8 input[42];
u8 input_prev[42];
u16 input_cur = 0;



// EVENT poll.. not just input
void input_poll(void)
{
	AGI_EVENT *si;
	
	if ( menu_next_input != 0)
		menu_input();
	si = control_key_map(event_read());
	
	while ( (si != 0) && (!flag_test(F02_PLAYERCMD)) )
	{
		switch (si->type)
		{
			case 1:	// ascii
				// only if it's ascii
				state.var[V19_KEYPRESSED] = si->data;
				if ( state.input_state != 0) 
					input_put_char(si->data);
				break;
			case 2:	// direction
				if (si->data == objtable->direction)
					state.var[V06_DIRECTION] = 0;
				else
					state.var[V06_DIRECTION] = si->data;
				if (state.ego_control_state != 0)
					objtable->motion = MT_NORM;
				break;
			case 3:	// controller
				control_state[si->data] = 1;
				break;
			case 10:	// mouse
				mouse_event_handle(si);	
				break;
			default:	
		}
		si = control_key_map(  event_read()  );
	}
}

// add to input string?
void input_put_char(u16 key_char)
{
	u16 max;
	
	if ( msgstate.dialogue_open != 0)
		max = 0x24;
	else
		max = 0x28 - strlen(state.string[0]); // the first string is the ">" thing?
	if (state.cursor != 0)
		max--;
	if ( state.var[V24_INPUTLEN] < max)
		max = state.var[V24_INPUTLEN];
	
	input_edit_on();

	switch (key_char)
	{
		case 8:	// backspace
			if ( input_cur != 0) 
			{
				input_cur --;
				input[input_cur] = 0;
				window_put_char(key_char);
				ch_update();
			}
			break;
			
		case 10:
			break;
		
		case 13:	// enter
			if ( input_cur != 0) 
			{
				strcpy(input_prev, input);
				parse(input);
				input_cur = 0;
				input[0] = 0;
				input_redraw();
			}
			break;
			
		default:
			if (  (max > input_cur) && (key_char != 0)  )
			{
				input[input_cur] = key_char;
				input_cur++;
				input[input_cur] = 0;
				window_put_char(key_char);
				ch_update();
			}
	}

	input_edit_off();
}

u8 *cmd_cancel_line(u8 *c)
{
	while (input_cur != 0)
		input_put_char(0x08);
	return c;
}

u8 *cmd_echo_line(u8 *c)
{
	if (state.input_state != 0)
		input_echo();
	return c;
}

void input_echo()
{
	if ( input_cur < strlen(input_prev)) 
	{
		input_edit_on();
		input[input_cur] = input_prev[input_cur];
		
		while (input[input_cur] != 0)
		{
			window_put_char(input[input_cur]);
			input_cur ++;
			input[input_cur] = input_prev[input_cur];
		}
		ch_update();

		input[input_cur] = 0;
		input_edit_off();
	}
}

void input_edit_off()
{
	if (input_edit_disabled == 0) 
	{
		input_edit_disabled = 1;
		if ( state.cursor != 0) 
		{
			window_put_char(state.cursor);
			ch_update();
		}
	}
}

void input_edit_on()
{
	if (input_edit_disabled == 1)
	{
		input_edit_disabled = 0;
		if ( state.cursor != 0) 
		{
			window_put_char(0x8);
			ch_update();
		}
	}
}

u16 input_edit_status()
{
	return input_edit_disabled;
}

u8 *cmd_prevent_input(u8 *c)
{
	state.input_state = 0;
	input_edit_on();
	window_line_clear(state.input_pos, 0);
	ch_update();
	return c;
}

u8 *cmd_accept_input(u8 *c)
{
	state.input_state = 1;
	input_redraw();
	return c;
}

u8 *cmd_set_cursor_char(u8 *c)
{
	u8 *di;
	di = logic_msg(*(c++));
	state.cursor = *di;
	return c;
}

// used a fair bit
void input_redraw()
{
	u8 msg[400];
	
	if (state.input_state != 0)
	{
		input_edit_on();
		window_line_clear(state.input_pos, state.text_bg);
		goto_row_col(state.input_pos, 0);
		agi_printf(str_wordwrap(msg, state.string[0], 40) );
		agi_printf(input);
		input_edit_off();
		ch_update();
	}
}

u8 hgc_note = 0;
void hgc_show_note(void)
{
	if (hgc_note == 0)
	{
		printf("\nI'm checking to see when this command is called.\n");
		printf("I think it's related to the HGC support and is probably not needed usually.\n");
		printf("Whenever a game needs user text/number input.. this is called.\n");
		printf("I need some sort of HGC emulator :(\n");
		printf("\n");
		hgc_note = 1;
	}
}

u8 *cmd_open_dialogue(u8 *c)
{
	printf("cmd_open_dialogue() called.\n");
	hgc_show_note();
	msgstate.dialogue_open = 1;
	return c;
}

u8 *cmd_close_dialogue(u8 *c)
{
	printf("cmd_close_dialogue() called.\n");
	hgc_show_note();
	msgstate.dialogue_open = 0;
	return c;
}