/*
CmdTextScreen                    cseg     00007603 00000038
CmdGraphics                      cseg     0000763B 00000012
CmdClearLines                    cseg     0000764D 0000003F
CmdClrTextRec                    cseg     0000768C 0000005C
CmdSetTextAttri                  cseg     000076E8 00000026
_TextColour                      cseg     0000770E 0000002E
_TextColour2                     cseg     0000773C 0000009E
subDoNothing                     cseg     000077DA 0000000C
subChckTxtMode                   cseg     000077E6 0000001E
_RedrawScreen                    cseg     00007804 00000025
CmdConfigScreen                  cseg     00007829 0000005C
CmdToggleMonitr                  cseg     00007885 0000003D
_PushTextAtt                     cseg     000078C2 0000003A
_PopTextAtt                      cseg     000078FC 0000003A
*/

#include "../agi.h"
#include "../ui/agi_text.h"

// input_edit_on/off
#include "../ui/cmd_input.h"
// window_portion_clear
#include "../ui/window.h"
// state.window_row_min, max
#include "../ui/msg.h"
// vstate.pic_buf_row
#include "../sys/video.h"
// rstate
#include "../sys/vid_render.h"
// state.input_pos
#include "../ui/cmd_input.h"
// status
#include "../ui/status.h"



//u8 vstate.text_mode = 0;	// 0x174b

#define ATTRIB_MAX 5

struct attrib_struct
{
	u16 fg;
	u16 bg;
	u16 comb;
};

typedef struct attrib_struct ATTRIB;
ATTRIB attrib_list[ATTRIB_MAX];
u16 attrib_cur = 0;

u8 *cmd_text_screen(u8 *c)
{
	input_edit_on();
	vstate.text_mode = 1;
	text_colour(state.text_fg, state.text_bg);
	vid_clear();
	window_portion_clear(0, 24, state.text_comb);
	return c; 
}

u8 *cmd_graphics(u8 *c)
{
	input_edit_on();
	screen_redraw();
	return c;
}

u8 *cmd_clear_lines(u8 *c)
{
	u16 upper;
	u16 lower;
	u16 attrib;
	u16 temp;

	upper = *(c++);
	lower = *(c++);
	attrib = calc_text_bg(*(c++));
	
	if (upper > lower)
	{
		lower = upper;
	}
	window_portion_clear(upper, lower, attrib);
	return c;
}

u8 *cmd_clear_text_rect(u8 *c)
{
	u16 lower_col;
	u16 upper_col;
	u16 lower_row;
	u16 upper_row;
	u16 attrib;
	
	upper_row = *(c++);
	upper_col = *(c++);
	lower_row = *(c++);
	lower_col = *(c++);
	attrib = calc_text_bg(*(c++) );
	window_clear(upper_row, upper_col, lower_row, lower_col, attrib);
	return c;
}

u8 *cmd_set_text_attribute(u8 *c)
{
	u16 fg;
	u16 bg;
	
	fg = *(c++);
	bg = *(c++);
	text_colour(fg, bg);
	return c;
}

// var8 =fg  vara = bg
void text_colour(u16 fg, u16 bg)
{
	state.text_comb = text_combine(fg, bg);
	state.text_fg = fg;
	state.text_bg = calc_text_bg(bg);
}

u8 cga_text_conv[] = {0, 1, 1, 1, 2, 2, 2, 3, 3, 1, 1, 1, 2, 2, 2};

// performs colour conversion for cga  'n stuff
u16 text_combine(u16 fg, u16 bg)
{
	u16 comb;
	
	if ( vstate.text_mode == 1) 
		comb = fg | (bg << 4);
	else
	{
		switch(rstate.drv->type)
		{
			case R_CGA0:
			case R_CGA1:
				if ( bg != 0)
					comb = 0x83;
				else if (fg > 0xE)
					comb = 3;
				else
					comb = cga_text_conv[fg];
				break;
			default:
				if ( bg != 0)
					comb = 0x8F;
				else
					comb = fg;
				break;
		}
	} 

	return comb;
}

u16 do_nothing(u16 nothin_but_crap)
{
	return nothin_but_crap;
}

u16 calc_text_bg(u16 colour)
{
	if ( (vstate.text_mode == 0) && (colour != 0) )
		return 0xFF;
	else
		return 0;
}

void screen_redraw()
{
	vstate.text_mode = 0;
	text_colour(state.text_fg, state.text_bg);
	vid_clear();
	pic_buff_update();
	status_line_write();
	input_redraw();	// cmd_input stuff
}

u8 *cmd_config_screen(u8 *c)
{
	state.window_row_min = *(c++);
	state.window_row_max = state.window_row_min + 21;
	//vstate.pic_buf_row = (state.window_row_min<<3); // * 8
	state.input_pos = *(c++);
	state.status_line_row = *(c++);	// status.c 
	return c;
}

u8 *cmd_toggle_monitor(u8 *c)
{
	return c;
}

void text_attrib_push()
{
	if ( attrib_cur < ATTRIB_MAX) 
	{
		attrib_list[attrib_cur].fg = state.text_fg;
		attrib_list[attrib_cur].bg = state.text_bg;
		attrib_list[attrib_cur].comb = state.text_comb;
		attrib_cur ++;
	}
}

void text_attrib_pop()
{
	if ( attrib_cur != 0)
	{
		attrib_cur --;
		state.text_fg = attrib_list[attrib_cur].fg;
		state.text_bg = attrib_list[attrib_cur].bg;
		state.text_comb = attrib_list[attrib_cur].comb;
	}
}