/*
CmdPrint                         cseg     00001C06 00000023
CmdPrintV                        cseg     00001C29 0000002B
CmdPrintAt                       cseg     00001C54 0000001D
CmdPrintAtV                      cseg     00001C71 00000025
_PrintAt                         cseg     00001C96 00000052
_Print                           cseg     00001CE8 000000AE
_MessageBox                      cseg     00001D96 00000195
CmdCloseWindow                   cseg     00001F2B 00000029
R_Display1F54                    cseg     00001F54 0000003F
R_Display1F93                    cseg     00001F93 0000025D
_LogicMsg                        cseg     000021F0 00000060
CmdDisplay                       cseg     00002250 00000057
CmdDisplayV                      cseg     000022A7 0000006F
_StrToIntPtr                     cseg     00002316 00000038
_DispNewLine                     cseg     0000234E 0000001F
*/

#include "../agi.h"
#include "../ui/msg.h"

// LOGIC
#include "../logic/logic_base.h"
// printf/sprintf
#include "../ui/printf.h"
// text_att_push
#include "../ui/agi_text.h"
// push pos
#include "../ui/window.h"
// parsing variables
#include "../ui/parse.h"
// object
#include "../objects.h"
// flags
#include "../flags.h"
// string zero pad
#include "../ui/string.h"

//u16 word_dseg_D09 = 20;	// row related ..   the MAX WIDTH???
#define HEIGHT_MAX 20
// size of a line in vertical pixels
//this is related to the pic buff size.. not the screen
#define LINE_SIZE 8

MSGSTATE msgstate = { 0xFFFF, {0xFFFF, 0xFFFF},
				0, '\\', 0, 
				{0,0}, {0,0}, {0,0},
				{0,0}, {0,0}
				};

u8 *cmd_print(u8 *c)
{
	message_box(logic_msg( *(c++) ) );
	return c;
}

u8 *cmd_print_v(u8 *c)
{
	message_box(logic_msg( state.var[*(c++)] ) );
	return c;
}

u8 *cmd_print_at(u8 *c)
{
	u16 msg_num;
	msg_num = *(c++);
	return print_at(msg_num, c);
}

u8 *cmd_print_at_v(u8 *c)
{
	u16 msg_num;
	msg_num = state.var[*(c++)];
	return print_at(msg_num, c);
}

u8 *print_at(u16 msg_num, u8 *c)
{
	msgstate.wanted_pos.row = *(c++);
	msgstate.wanted_pos.col = *(c++);
	msgstate.wanted_width = *(c++);
	
	if (msgstate.wanted_width == 0)
		msgstate.wanted_width = 30;
	message_box( logic_msg(msg_num) );
	
	msgstate.wanted_pos.col = 0xFFFF;
	msgstate.wanted_pos.row = 0xFFFF;
	msgstate.wanted_width = 0xFFFF;
	
	return c;
}


u8 *cmd_close_window(u8 *c)
{
	if ( msgstate.active != 0)
		render_update(msgstate.bgpos.x, msgstate.bgpos.y, msgstate.bgsize.w, msgstate.bgsize.h);
	msgstate.dialogue_open = 0;
	msgstate.active = 0;
	return c;
}

u8 *cmd_display(u8 *c)
{
	u16 row, col;
	u8 msg[1000];

	push_row_col();
	row = *(c++);
	col = *(c++);
	goto_row_col(row, col);
	agi_printf(str_wordwrap(msg, logic_msg(*(c++)), 40));
	pop_row_col();

	return c;
}

u8 *cmd_display_v(u8 *c)
{
	u16 row, col;
	u8 msg[1000];

	push_row_col();
	row = state.var[*(c++)];
	col = state.var[*(c++)];
	goto_row_col(row, col);
	agi_printf(str_wordwrap(msg, logic_msg(state.var[*(c++)]), 40));
	pop_row_col();
	
	return c;
}





u16 message_box(u8 *var8)
{
	u32 temp;
	u16 ret;
	
	message_box_draw(var8, 0, 0, 0);
	
	if ( flag_test(F15_PRINTMODE) ) 
	{
		flag_reset(F15_PRINTMODE);
		return 1;
	}
	else
	{
		if ( state.var[V21_WINDOWTIMER] == 0) 
		{
			ret = (user_bolean_poll() == 1);
			// 1==enter 0==esc
		}
		else
		{
			temp = calc_agi_tick() + state.var[V21_WINDOWTIMER] * 10;
			while (  (calc_agi_tick() < temp) && (has_user_reply() == 0xFFFF) )
				SDL_Delay(50);
			ret = 1;
			state.var[V21_WINDOWTIMER] = 0;
		}
		
		cmd_close_window(0);
		return ret;
	}
}

// var8 is the string
// vara is starting row?
// varc is a width 
// vare is a toggle... if == 1 then force width and height
void message_box_draw(u8 *str, u16 row, u16 w, u16 toggle)
{
	u8 char_orig;
	u8 msg_err[100];	// 2bc
	u8 msg[600];	// 258
	u16 ax;
	
	if ( msgstate.active != 0)
		cmd_close_window(0);
	text_attrib_push();
	push_row_col();
	text_colour(0, 0x0F);
	
	if (  (msgstate.wanted_width == 0xFFFF) && (w == 0)  )
		w = 30;
	else if ( msgstate.wanted_width != 0xFFFF) 
		w = msgstate.wanted_width;
	
	for(;;)
	{
		str_wordwrap(msg, str, w);	//split it up to fit in a msg box???
		if (toggle != 0)
		{
			msgstate.tsize.w = w;
			if ( row != 0)
				msgstate.tsize.h = row;
		}
	
		// height_max is dependant on whether your have a hgc or not..
		// version 3 just ditches this variable.. makes it a constant
		if ( msgstate.tsize.h <= (HEIGHT_MAX - 1))
			break;
		
		char_orig = str[20];
		str[20] = 0;
		sprintf(msg_err, "Message too verbose:\n\n\"%s...\"\n\nPress ESC to continue.", str);
		str[20] = char_orig;
		str = msg_err;
	}
	
	if (msgstate.wanted_pos.row == 0xFFFF)
		ax = ((HEIGHT_MAX - msgstate.tsize.h - 1) >> 1) + 1; 
	else
		ax = msgstate.wanted_pos.row;

	msgstate.tpos.row = ax + state.window_row_min;
	msgstate.tpos_edge.row = msgstate.tsize.h + msgstate.tpos.row - 1;
	
	if (msgstate.wanted_pos.col == 0xFFFF)
		msgstate.tpos.col = (40 - msgstate.tsize.w) / 2;
	else
		msgstate.tpos.col = msgstate.wanted_pos.col;

	window_col = msgstate.tpos.col;
	window_row = 0;
	msgstate.tpos_edge.col = msgstate.tpos.col + msgstate.tsize.w;
	
	goto_row_col(msgstate.tpos.row, msgstate.tpos.col);
	
	msgstate.bgsize.w = (msgstate.tsize.w * 4) + 10;
	msgstate.bgsize.h = msgstate.tsize.h * LINE_SIZE + 10;
	msgstate.bgpos.y = (msgstate.tpos_edge.row - state.window_row_min + 1) * LINE_SIZE + 4; 
	msgstate.bgpos.x = (msgstate.tpos.col * 4) - 5;
	
	gfx_msgbox(msgstate.bgpos.x, msgstate.bgpos.y, msgstate.bgsize.w, msgstate.bgsize.h, 0x0F, 0x04);
	
	msgstate.active = 1;
	agi_printf(msg);
	window_col = 0;
	
	pop_row_col();
	text_attrib_pop();
	
	msgstate.dialogue_open = 1;
}



// used by r_display 'n stuff
u16 disp_char_cur = 0;		// character count for the count line
u16 disp_width_max = 0;	// desired maximum width
u8 *disp_last_word = 0;	// ptr to the last word.. so you can wrap lines

// var8 = msg,  vara = str  varc = w
// I think it arranges the text.. inserts \n and stuff man
u8 *str_wordwrap(u8 *msg, u8 *str, u16 w)
{
	disp_char_cur = 0;
	disp_width_max = w;
	disp_last_word = 0;
	msgstate.tsize.w = 0;
	msgstate.tsize.h = 0;
	
	if (str != 0) 
	{
		str = r_display1f93(str, msg);
		*str = 0;
		display_new_line();
	}
	return msg;
}

//u16 stuff[] = {0, 0x0A, 0x20, 0x25};

// add str to msg
u8 *r_display1f93(u8 *given_source, u8 *given_msg)
{
	LOGIC *log_orig;	// orig log0
	u16 var_pad;	// number of zeroes to pad variable with
	u16 my_num;	// msg number
	u8 *my_str;
	u8 *log_msg;	// logic msg data
	u8 *source;
	u8 *msg;
	u8 *bx;
	
	source = given_source;
	msg = given_msg;

	while (   (*source != 0) && (msgstate.tsize.h <= (HEIGHT_MAX - 1) ) )
	{
		while ( disp_char_cur < disp_width_max)
		{
			if ( *source == msgstate.newline_char) 
			{
				source++;
				*(msg++) = *(source++);
				disp_char_cur++;
			}
			else switch(*source)
			{
				case 0x00:			// null
					return msg;
					//goto loc21e8;
				
				
				case 0x0A:			// linefeed
					*(msg++) = *(source++);
					display_new_line();
					break;
				
				
				case 0x20:			// space
					disp_last_word = msg;
					*(msg++) = *(source++);
					disp_char_cur++;
					break;
				
				
				case 0x25:			// % control 
					source++;
					switch (*(source++))
					{
						case 'g':	// global log msg
							source = str_to_int_ptr(source, &my_num);
							log_orig = logic_cur;
							logic_cur = logic_list_find(0);
							log_msg = logic_msg(my_num);
							if ( log_msg != 0)
								msg = r_display1f93(log_msg, msg);
							logic_cur = log_orig;
							break;
							
						case 'm':	// log msg
							source = str_to_int_ptr(source, &my_num);
							log_msg = logic_msg(my_num);
							if ( log_msg != 0)
								msg = r_display1f93(log_msg, msg);
							break;
							
						case 'o':	// object name
							source = str_to_int_ptr(source, &my_num);
							bx = object + state.var[my_num] * 3;
							my_str = object;
							my_str += load_le_16(bx);
							msg = r_display1f93(my_str, msg);
							break;
						
						case 's':	// string	
							source = str_to_int_ptr( source, &my_num);
							msg = r_display1f93(state.string[my_num], msg);
							break;
						
						case 'v':	// variable
							var_pad = 0;
							source = str_to_int_ptr(source, &my_num);
							my_str = int_to_string(state.var[my_num]);
							if (*source == '|')
							{
								source++;
								source = str_to_int_ptr(source, &var_pad);
								my_str = string_zero_pad(my_str, var_pad);
							}
							msg = r_display1f93(my_str, msg);
							break;
						
						case 'w':	// word
							source = str_to_int_ptr(source, &my_num);
							my_num--;
							if ( my_num < word_total)
								msg = r_display1f93(word_string[my_num], msg);
							break;
						
						default:
					}
					break;
					
					
				default:			// normal character
					*(msg++) = *(source++);
					disp_char_cur++;
			}
		}
		
		if ( disp_last_word == 0)
		{
			*(msg++) = 0x0A;	// new line
			display_new_line();
		}
		else
		{
			*msg = 0;
			disp_char_cur = disp_char_cur - (msg - disp_last_word);
			display_new_line();
			msg = disp_last_word;
			*msg = 0x0A;	// new line
			
			while ( *(msg++) == ' ');
				
			msg = strcpy(disp_last_word+1, msg);
			
			disp_last_word = 0;
			while ( *msg != 0)
			{
				msg++;
				disp_char_cur ++;
			}
		}
	}
//loc21e8:
	return msg;
}




u8 *logic_msg(u16 msg_num)
{
	u16 temp_off;
	u8 *msg_item;

	if (msg_num > logic_cur->msg_total)	// num messages
		return 0;
	else
	{
		msg_item = (logic_cur->msg) + (msg_num<<1);
		temp_off = load_le_16(msg_item);

		if (temp_off == 0)
			set_agi_error(0xE, msg_num);
		return(logic_cur->msg + temp_off);
	}
}


// converts a string to an int
u8 *str_to_int_ptr(u8 *s, u16 *num)
{
	u16 di;
	di = 0;
	while ( (*s>='0') && (*s<='9') )
		di = di * 10 + *(s++) - '0';
	*num = di;
	return s;
}

// new line?
void display_new_line()
{
	msgstate.tsize.h++;
	if ( disp_char_cur > msgstate.tsize.w) 
		msgstate.tsize.w = disp_char_cur;
	disp_char_cur = 0;
}