/* picture render video related functions
_SBuffFillAX                     cseg     0000518F 00000018
_SBuffXLine                      cseg     000051A7 0000003C
_SBuffYLine                      cseg     000051E3 0000004E
_SBuffPlotPixel                  cseg     00005231 00000042
_SBuffPicFill                    cseg     00005273 000001ED
END picture render video related functions */


#include "../agi.h"


#include "../picture/sbuf_util.h"
//#include "../picture/pic_stack.h"
#include "../picture/pic_render.h"

#include "../sys/drv_video.h"
#include "../sys/gfx.h"

/*
CGARelated135B
CGARelated135C
CGARelated135D
SBuffDrawMask
Colour_PictPart
Colour_PriPart
PriPicCol_Odd
PriPicCol_Even
SBuff_Seg
wFBufSeg
wHGCFontData
GivenPicData
mult_lookup_tab
InitPicPosOrY
InitPicX
FinalPicPosOrY
FinalPicX
*/




void sbuff_fill(u8 colour)
{
	memset(gfx_picbuff, colour, 160*168);
}

void sbuff_testpattern()
{
	int i, m,n;
	u8 *b;
	
	m=0;
	n = 0;

	b= gfx_picbuff;
	for (i=0; i<(160*168); i++)
	{
		if (m != 7)
			*(b++) = m;
		else
				*(b++) = 15;
		n++;
		if (n == 10)
		{
			m++;
			if (m > 15)
				m= 0;
			n = 0;
		}
	}
}


// y pos's should be equal or you suck
void sbuff_xline()
{
	u8 x1, x2, x_orig, len;
	u8 *b;
	u8 colour;
	
	x1 = pos_init_x;
	x2 = pos_final_x;
	x_orig = x2;	// push init position

	if (pos_init_x > pos_final_x)
	{
		u8 temp;
		
		temp = x1;
		x1 = x2;
		x2 = temp;
		pos_init_x = x1;
		pos_final_x = x2;
	}

#warning replicated from sbuff_plot code
	b = gfx_picbuff + PBUF_MULT(pos_init_y) + pos_init_x;
	if ((pos_init_y & 1) == 0)
		colour = col_even;
	else
		colour = col_odd;
	*b = (*b | sbuff_drawmask) & colour;

	len = x2 - x1;
	while (len != 0)
	{
		b++;		// b is given from sbuff_plot
		*b = (*b | sbuff_drawmask) & colour;
		len--;
	}
	
	pos_init_x= x_orig;	// pop init position
	
}


// yline needs a bit o' work
#warning fix yline
#warning pos_init needs to equal the final.. bug!
void sbuff_yline()
{
	u8 y1, y2, y_orig, len;
	u8 *b;
	u8 colour;
	
	y1 = pos_init_y;
	y2 = pos_final_y;
	y_orig = y2;
	
	if (y1 >= y2)
	{
		u8 temp = y1;
		y1 = y2;
		y2 = temp;
		pos_final_y = y2;	
		pos_init_y = y1;
	}
	
	b = gfx_picbuff + PBUF_MULT(pos_init_y) + pos_init_x;
	if ((pos_init_y & 1) == 0)
		colour = col_even;
	else
		colour = col_odd;
	*b = (*b | sbuff_drawmask) & colour;
	
	len = y2 - y1;
	
	while (len != 0)
	{	
		if ((y1 & 1) == 0)
			colour = col_odd;
		else
			colour = col_even;

		b += 160;
		*b = (*b | sbuff_drawmask) & colour;
		len--;
		y1++;
	}

	pos_init_y = y_orig;
}

void sbuff_plot()
{
	u8 *b;
	u8 colour, pixel;
	
	b = gfx_picbuff + PBUF_MULT(pos_init_y) + pos_init_x;
	
	if ((pos_init_y & 1) == 0)
		colour = col_even;
	else
		colour = col_odd;
	pixel = (*b | sbuff_drawmask) & colour;
	
	*b = pixel;
}


u8 left=0, right=0;
u8 direction=0, old_direction=0;
u8 toggle=0, old_toggle=0;		//, pos_x3;
u8 old_initx=0, old_inity=0;
u8 stack_left=0, stack_right=0;
u8 old_right=0, old_left=0;
// FILL
void sbuff_picfill(u8 ypos, u8 xpos)
{
	u8 *b; // buffer
	u8 fill_stack[3000];	// the agi stack is 2560.. and that's got a few function names in it. so this should be right
	u8 *stack_ptr;
	//DATA *d = 0;

	u8 al;		// temp al register
	
	// al = old, ah = new
	u8 mask_bh, mask_dl, colour_bl;
	u8 *old_buff, *temp;
	u16 counter;
	u8 colour_new, colour_old;
	
	b = gfx_picbuff + PBUF_MULT(ypos) + xpos;
	mask_bh = sbuff_drawmask;
	colour_bl = 0x4F;
	stack_ptr = fill_stack;
	
/*
	dl = bitmask?
	bl = some sort of colour after bitmask
		the colour that you can only fill in??
*/	

	// ***** Initialise masks and colours
	
	// depending on the bitmask.. sees if it's worth bothering
	if ((sbuff_drawmask & 0x0F) != 0)
	{
		mask_dl = 0xF;
		// filling in a white area.. already white man
		if (colour_pictpart == 0x0F) return;

	}
	else
	{
		if ((sbuff_drawmask & 0xF0) == 0) return;
		mask_dl = 0xF0;
		if (colour_pripart == 0x40) return;
	}

	colour_bl = colour_bl & mask_dl;
	if ( (*b & mask_dl) != colour_bl )
		return;

	memset(stack_ptr, 0xFF, 7);
	stack_ptr += 7;
	
	//d = fill_push(d, 255,255,255,255,255,255,255 );	
	//#warning check for NO leaks in push /pop

	// ***** Fill in a *line*

	left = 161;
	right = 0;
	direction = 1;
	toggle = 0;

loc52ca:			// fill a new line. 
	old_right = right;
	old_left = left;
	old_toggle = toggle;
	old_initx = pos_init_x;
	
	counter = pos_init_x;

	if ( (pos_init_y & 1) == 0)
		colour_new = col_even;
	else
		colour_new = col_odd;

	old_buff = b;
	
	counter++;
	colour_old = *b;

	// fill backwards from this point...
	do
	{
		*b = (colour_old | sbuff_drawmask) & colour_new;
		b--;
		colour_old = *b;
		counter--;
	} while ( ( (colour_old & mask_dl) == colour_bl) && (counter != 0) );

	b++;
	counter = 159 - pos_init_x;
	left = pos_init_x - (old_buff - b);
	pos_init_x = left;

	temp = old_buff;
	old_buff = b;
	b = temp;		// exchange buff, old_buff
	b++;

	// fill forwards
	while (counter != 0)
	{
		colour_old = *b;
		if ((colour_old & mask_dl) != colour_bl) break;

		*b = ( colour_old | sbuff_drawmask ) & colour_new;
		b++;
		counter--;
	}

	right =  left + (b - old_buff) - 1;
	// remember old_buff was swapped with di

	// ***** Find the next line to fill

	if (old_left != 161)
	{
		if (right == old_right)
		{
			if (left != old_left) goto loc5376;
			if (toggle == 1) goto locnext;
			toggle = 1;
			old_initx = right;
		}
		else if (right > old_right)
		{
		loc5376:
			toggle = 0;
			old_initx = old_right;
		}
		else 
		{
			toggle = 0;
			old_initx = right;	
		}
		
		*(stack_ptr++) = old_toggle;
		*(stack_ptr++) = direction;
		*(stack_ptr++) = old_direction;
		*(stack_ptr++) = old_inity;
		*(stack_ptr++) = old_initx;
		*(stack_ptr++) = old_right;
		*(stack_ptr++) = old_left;

		//d = fill_push(d, old_left, old_right, old_initx, old_inity, old_direction, direction, old_toggle);
	}

locnext:

	old_direction = direction;
	old_inity = pos_init_y;
	pos_init_y += direction;
	
	for(;;)
	{
		if (pos_init_y > 167)
			goto loc5413;
	loc53A8:
		b = gfx_picbuff + PBUF_MULT(pos_init_y) + pos_init_x;
		if ((*b & mask_dl) == colour_bl) goto loc52ca;
	
		// redirected position isn't a fill colour??
		if (direction == old_direction) goto loc5406;
		if (toggle == 1) goto loc5406;
		if (pos_init_x < stack_left) goto loc5406;
		if (pos_init_x > stack_right) goto loc5406;
		if (stack_right >= right) goto loc5413;
		
		pos_init_x = stack_right +1;
	loc5406:
		if (pos_init_x < right)
		{
			pos_init_x++;
			goto loc53A8;
		}
		// reached the edge of screen??
	loc5413:
		// al = direction;
		if ( (direction == old_direction) && (toggle == 0) )
		{
			direction = -direction;
			pos_init_x = left;
			pos_init_y = old_inity;
			al = old_inity;
		}
		else
		{
			left = *(--stack_ptr);
			right = *(--stack_ptr);
			pos_init_x = *(--stack_ptr);
			pos_init_y = *(--stack_ptr);
			old_direction = *(--stack_ptr);
			direction = *(--stack_ptr);
			toggle = *(--stack_ptr);
			//d = fill_pop(d, &left, &right, &pos_init_x, &pos_init_y, &old_direction, &direction, &toggle);
	
			al = pos_init_y;
			if (pos_init_y == 0xFF)
				return;
			old_inity = pos_init_y;
		}
	
		// last pushed onto stack.. pos_old_1_rl
		stack_left = *(stack_ptr-1);
		stack_right = *(stack_ptr-2);
		//fill_topab(d, &stack_left, &stack_right);
		pos_init_y = al + direction;
	}

}


