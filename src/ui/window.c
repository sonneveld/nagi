/*
_WindowPutChar                   cseg     000029F6 00000117
_GotoRowCol                      cseg     00002B0D 0000001B
_PushRowCol                      cseg     00002B28 00000027
_PopRowCol                       cseg     00002B4F 00000029
_WindPortnClr                    cseg     00002B78 0000002E
_WindowLineClr                   cseg     00002BA6 0000001E
_WindowClear                     cseg     00002BC4 00000043
_WindowScroll                    cseg     00002C07 00000030
_CharInvert                      cseg     00002C37 0000000F
_CharDisable                     cseg     00002C46 0000000F
_Char0x80Read                    cseg     00002C55 00000025
*/
 
#include "../agi.h"

// getpos, setpos, scroll, clear and putchar

// state.text_bg, text_comb 
#include "../ui/agi_text.h"

#include "../ui/window.h"

#include "../sys/chargen.h"

u8 window_col = 0;	// set by messagebox so it wrap inside the window
u8 window_row = 0;

#define PA_MAX 5
u8 pos_count = 0;
TPOS pos_array[PA_MAX];

u16 text_shade = 0;	// if set, text is shaded

#define TEXT_INVERT 0x1
#define TEXT_SHADE 0x2
// if char position isn't within window, move it there?
void window_put_char(u16 given_char)
{
	//u16 temp;
	TPOS char_pos;
	//u8 x,  y;	
	u8 bl;
	u8 conversion = 0;

	ch_pos_get(&char_pos);
	
	if (given_char == 0x08)	// backspace
	{
		if (char_pos.col != 0)
			char_pos.col--;
		else if (char_pos.row > 21)
		{
			char_pos.col = 39;
			char_pos.row--;
		}

		ch_clear(&char_pos, &char_pos, state.text_bg);	// one square
		ch_pos_set(&char_pos);
	}
	else if (  (given_char == 0x0D) || (given_char == 0x0A)  )	// return/ linefeed ??
	{
		if (char_pos.row < 24)
			char_pos.row++;
		char_pos.col = window_col;
		ch_pos_set(&char_pos);
	}
	else
	{
		// normal character
		bl = state.text_comb;
		
		if (chgen_textmode == 0)	// if not graphical
		{
			if ((state.text_comb & 0x80) != 0)	// invert character
				conversion = conversion | TEXT_INVERT;
			if (text_shade != 0)	// disabled item
				conversion = conversion | TEXT_SHADE;
			
			// load up an individual character
			
			bl = state.text_comb & 0x7F;	// turn off invert bit
		}
		
		//#warning fix this one day
		/*
		Bitfields for character's display attribute:
		Bit(s)	Description	(Table 00014)
		 7	foreground blink or (alternate) background bright (see also AX=1003h)
		 6-4	background color (see #00015)
		 3	foreground bright or (alternate) alternate character set (see AX=1103h)
		 2-0	foreground color (see #00015)
		*/

		ch_attrib(state.text_comb, conversion);
		ch_put(given_char);
	
		char_pos.col++;
		if (char_pos.col <= 39)
			ch_pos_set(&char_pos);
		else
			window_put_char(0xD);	// carriage return
	
		// restore character
	}
}

// dh = row / y
// dl = col / x

void goto_row_col(u16 row, u16 col)
{
	TPOS da_pos = {row, col};
	ch_pos_set(&da_pos);
}


void push_row_col()
{
	if ( pos_count < PA_MAX )
	{
		ch_pos_get(&pos_array[pos_count]);
		pos_count++;
	}
}

void pop_row_col()
{
	if ( pos_count > 0 )
	{
		pos_count--;
		ch_pos_set(&pos_array[pos_count]);
	}
}

// draw a big window from column 0 to column 39
void window_portion_clear(u16 upper_row, u16 lower_row, u16 attrib)
{
	window_clear(upper_row, 0x00, lower_row, 39, attrib);
}

// I'm just a line.. in a lonely little world or something...
void window_line_clear(u16 row, u16 attrib)
{
	window_portion_clear(row, row, attrib);
}

void window_clear(u16 upper_row, u16 upper_col, u16 lower_row, u16 lower_col, u16 attrib)
{
	TPOS old_pos;
	TPOS pos1 = {upper_row, upper_col};
	TPOS pos2 = {lower_row, lower_col};
	
	ch_pos_get(&old_pos);
	ch_clear(&pos1, &pos2, attrib);
	ch_pos_set(&old_pos);
}

// scroll up one 
// var8 = upper_row vara = upper_col   varc = lower_row   vare = lower_col  var10 attrib
void window_scroll(u16 upper_row, u16 upper_col, u16 lower_row, u16 lower_col, u16 attrib)
{
	TPOS pos1 = {upper_row, upper_col};
	TPOS pos2 = {lower_row, lower_col};
	ch_scroll(&pos1, &pos2, 1, attrib);
	pos1.row = lower_row;
	ch_pos_set(&pos1);
}

/*

u8 invert_char()
{
	get_char_0x80();
	ax = 0xFFFF;
loc2c3d:
	*di = *di ^ ax;
	di++;
	di++;
	cx--;
	if (cx !=0) goto loc2c3d;
	al = 0x80;
}

u8 disable_char()
{
	get_char_0x80();
	ax = 0xAA55;
loc2c4c:
	*di = *di | ax;
	di++;
	di++;
	cx--;
	if ( cx != 0) goto loc2c4c;
	al = 0x80;
}


void get_char_0x80()
{
	ah = 0;
	if ( al != 0x80)
	{
		cx = 0xF000;
		ds = cx;
		di = 0xe9e;
		si = 0xFa6e;
		
}*/