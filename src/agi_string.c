/*
CmdGetString                     cseg     00000C44 000000F3
CmdSetString                     cseg     00000D37 00000039
CmdWord2String                   cseg     00000D70 00000039
sub_cseg_DA9                     cseg     00000DA9 000000D5
CmdSetGameID                     cseg     00000E7E 0000002E
sub_cseg_EAC                     cseg     00000EAC 0000004C
sub_cseg_EF8                     cseg     00000EF8 00000054
*/

//~ RaDIaT1oN (2002-04-29):
//~ fix unit'd variable

/*
CmdGetNum                        cseg     00007126 000000C8
*/

#include "agi.h"

#include <string.h>
#include <ctype.h>

#include "agi_string.h"


// msg
#include "ui/msg.h"
// input_edit
#include "ui/cmd_input.h"
// agi_printf
#include "ui/printf.h"
// agi text
#include "ui/window.h"
// word_string
#include "ui/parse.h"
// finish
#include "base.h"
// char_wait
#include "ui/events.h"

// string_to_int
#include "ui/string.h"
#include "game_id.h"

#include "sys/chargen.h"


static void agi_string_clean(u16 str_agi, char *str_buff);


// the size is probably related to the text resolution






// get.string(sA,mB,Y,X,L);
u8 *cmd_get_string(u8 *c)
{
	char prompt[400];
	u16 input_stat_orig;	// original input status
	u16 prompt_num;	// msg number
	u16 len;	// length
	u16 col;	// x col
	u16 row;	// y  row]
	char *str_user;	// string

	input_stat_orig = input_edit_status();
	push_row_col();
	input_edit_on();
	
	str_user = state.string[*(c++)];
	prompt_num = *(c++);
	row = *(c++);
	col = *(c++);
	len = *(c++) + 1;
	if ( len > STRING_SIZE )
		len = STRING_SIZE;

	*str_user = 0;
	if ( row < 0x19)
		goto_row_col(row, col);
	
	// for ega
	agi_printf(str_wordwrap(prompt, logic_msg(prompt_num), 0x40));
	string_edit(str_user, STRING_SIZE, len);

	pop_row_col();
	if ( input_stat_orig != 0)
		input_edit_off();
	return c;
}

u8 *cmd_set_string(u8 *c)
{
	char *di;
	di = state.string[*(c++)];
	strncpy(di, logic_msg(*(c++)), STRING_SIZE);
	return c;
}

u8 *cmd_word_to_string(u8 *c)
{
	char *di;
	di = state.string[*(c++)];
	strncpy(di, word_string[*(c++)], STRING_SIZE);
	return c;
}

#if 0
// whatever is in str... it displays it and then allows you to edit it
// at least within the len limits
static u16 string_edit_old(u8 *str, u16 len)
{
	u8 buff[STRING_SIZE];
	u8 *str_end, *str_cur;
	u16 di = 0;
	int it_is_cold = 1;
	
	if (len > STRING_SIZE)
		len = STRING_SIZE;
	str_end = buff + len;
	
	strncpy(buff, str, len-1);
	agi_printf(buff);
	str_cur = buff + strlen(buff);
	
	while (it_is_cold == 1)
	{
		input_edit_off();
		ch_update();
		di = char_wait();
		input_edit_on();

		switch(di)
		{
			case 0x3:	// ctrl-c
			case 0x18:	// ctrl-x ??
				while ( buff < str_cur)
				{
					str_cur--;
					window_put_char(8);
				}
				break;
				
			case 0x8:	// bs
				if ( buff < str_cur)
				{
					str_cur--;
					window_put_char(di);
				}
				break;
				
			case 0xD:	// enter
				*str_cur = 0;
				strcpy(str, buff);
				ch_update();
				return di;	// LEAVE!
				break;
			
			case 0x1B:	// esc
				ch_update();
				return di;	// LEAVE!
				break;
			
			default:
				if ( str_end-1 > str_cur)
				{
					*(str_cur++) = di;
					window_put_char(di);
				}
		}
	}
	ch_update();
	return di;
}
#endif

u8 *cmd_set_game_id(u8 *c)
{
	strncpy(state.id, logic_msg(*(c++)), ID_SIZE);
	state.id[ID_SIZE] = 0;	// end JUST IN CASE
	//game_id_test();	// oh.. I can't be bothered
	return c;
}

// compare the string
// var8 and vara are agi_string NUMBERS
u16 agi_string_compare(u16 var8, u16 vara)
{
	char temp52[41];	// string vara copy;
	char temp29[41];	// string var8 copy
	char *si, *di;
	
	agi_string_clean(var8, temp29);
	agi_string_clean(vara, temp52);
	
	si = temp29;
	di = temp52;
	while (*si != 0)
	{
		if (*si != *di)
			return 0;
		si++;
		di++;
	}
	
	return (*si == *di);
}

// clean the string? 
static void agi_string_clean(u16 str_agi, char *str_buff)
{
	char *di;
	
	for (di=state.string[str_agi]; *di!=0; di++)
		if (strchr("\x20\x09.,;:'!-", *di) == 0)
			*(str_buff++) = tolower(*di);
	*str_buff = 0;
}

u8 *cmd_get_num(u8 *c)
{
	char temp194[400];
	char temp4[4];
	
	temp4[0] = 0;

	input_edit_on();
	goto_row_col(state.input_pos, 0);
	agi_printf(str_wordwrap(temp194, logic_msg(*(c++)), 0x28));
	input_edit_off();
	string_edit(temp4, 4, 4);
	input_redraw();

	state.var[*(c++)] = string_to_int(temp4);
	return c;
}




static TPOS stredit_pos = {0,0};



// 2/3	-- starting out.. or when backspacing left 
// 1/3	-- moving right

// display an end portion of the str
static void stredit_disp2(const char *port, u16 disp_size, u16 offset)
{
	window_clear(stredit_pos.row, stredit_pos.col+offset, stredit_pos.row, stredit_pos.col+disp_size-1, state.text_bg);
	
	while ( (*port != 0) && (offset != 0)  )
	{
		port++;
		offset--;
	}
	
	while (*port != 0)
	{
		window_put_char(*(port++));
	}
}


static const char *stredit_disp(const char *str_edit, u16 str_size,u16 disp_size, u16 percent)
{
	const char *str_disp;
	// display string
	if ((str_size<=disp_size)||(strlen(str_edit) < disp_size))
		str_disp = str_edit;
	else
		str_disp = str_edit + strlen(str_edit) - ((percent*disp_size)/100);
	
	goto_row_col(stredit_pos.row, stredit_pos.col);
	
	if (str_disp == str_edit)
	{
		stredit_disp2(str_disp, disp_size, 0);
	}
	else
	{
		window_put_char(0x1b);
		stredit_disp2(str_disp, disp_size, 1);
	}
	return str_disp;
}

char string_edit(char *str, u16 str_size, u16 disp_size)
{
	char *str_edit;	// the whole str that gets edited
	const char *str_disp;	// pointer to the bit that gets displayed
	char *str_tail;	// tail of the edited string;
	char *str_end;	// the absolute end
	
	char ch = 0;
	
	// init string
	str_edit = alloca(str_size);
	strcpy(str_edit, str);
	str_tail = str_edit + strlen(str_edit);
	str_end = str_edit + str_size - 1;
	
	ch_pos_get(&stredit_pos);
	
	str_disp = stredit_disp(str_edit, str_size, disp_size, 75);
	
	for (;;)
	{
		input_edit_off();	// remove cursor
		ch_update();
		ch = char_wait();
		input_edit_on();	// add cursor

		switch(ch)
		{
			case 0x3:	// ctrl-c
			case 0x18:	// ctrl-x ??
				str_tail = str_edit;
				str_disp = str_edit;
				*str_tail = 0;
				stredit_disp(str_edit, str_size, disp_size, 0);
				break;
			
			case 0x8:	// bs
				if (str_edit < str_tail)
					str_tail--;
				if (str_tail > str_disp)
				{
					window_put_char(ch);

				}
				else
				{
					*str_tail = 0;
					str_disp = stredit_disp(str_edit, str_size, disp_size, 90);
				}
				break;

			case 0xD:	// enter
				*str_tail = 0;
				strcpy(str, str_edit);
				ch_update();
				return ch;	// LEAVE!
				// break;
			
			case 0x1B:	// esc
				ch_update();
				return ch;	// LEAVE!
				// break;
			
			default:
				if (str_tail < str_end)
				{
					*(str_tail++) = ch;
					
					if ((str_tail - str_disp) < disp_size)
					{
						window_put_char(ch);
					}
					else
					{
						*str_tail = 0;
						str_disp = stredit_disp(str_edit, str_size, disp_size, 20);
					}
				}
		}
	}
	// we should never reach this point, we only `return` from the for loop above.
	// ch_update();
	// return ch;
}
