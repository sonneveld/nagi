/*
_LzwDecompress                   cseg     000007F4 0000017E
_LzwBuffFill                     cseg     00000972 00000041
_LzwReadNext                     cseg     000009B3 0000004E
_LzwBuffShift                    cseg     00000A01 00000030
*/

#include "../agi.h"
#include "../res/res.h"
#include "../sys/endian.h"

#include <errno.h>
#include <string.h>

void lzw_buff_fill(u16 cur_byte);
u16 lzw_read_next(void);
void lzw_buff_shift(void);

DICT *lzw_dict = 0;

FILE *lzw_res_stream = 0;
u16 lzw_buff_size = 0;
u16 lzw_res_size = 0;
u8 *lzw_buff = 0;
u16 lzw_code_width = 0;
u16 lzw_code_mask[] = {0x1FF, 0x3FF, 0x7FF, 0xFFF}; //code mask (9bits, 10 bits, 11 bits, 12 bits 
u16 lzw_bit_cur = 0;	// current bit
u16 lzw_bit_max = 0;	// max number of bits in lzw_buffer

// var8 = res stream handle
// vara = comp buff
// varc = compressed size
// vare = uncompressed buff
// var10 = decomp buff size



u16 lzw_decompress(FILE *cfile, u8 *cbuff, u16 fsize, u8 *lzw_uncomp_buff, u16 cbuff_size)
{
	u16 code_cur = 0;	// code	
	u16 code_prev = 0;	// code
	u16 code_read = 0;
	u16 code_next = 0x102;	// the next code to be written
	u16 stack_size = 0;		// number of times something is pushed onto stack
	u16 code_max = 0x200;	
	u8 lzw_ascii = 0;		// "ascii"?
	u8 *di;
	u8 code_string[2048];
	u8 *str_ptr;
	
	errno = 0;  // reset file errors that may have occured from vol opening
	
	lzw_res_stream = cfile;
	lzw_buff = cbuff;
	lzw_res_size = fsize;
	lzw_buff_size = cbuff_size;
	lzw_bit_max = (lzw_buff_size - 2)<<3;
	lzw_code_width = 9;
	lzw_bit_cur = 0;
	lzw_buff_fill(0);
	
	di = lzw_uncomp_buff;
	if (errno == 0)
	{
		code_read = lzw_read_next();
		while ( (errno==0)&&(code_read!=0x101) )
		{	
			if (code_read == 0x100)	// start over code
			{
				lzw_code_width = 9;
				code_max = 0x200;
				code_next = 0x102;
				code_read = lzw_read_next();
				if (errno != 0)
					break;
					//return di - lzw_uncomp_buff;
				code_cur = code_read;
				code_prev = code_read;
				lzw_ascii = code_read;
				*(di++) = code_read;
			}
			else
			{
				code_cur = code_read;
				str_ptr = code_string;
				if ( code_cur >= code_next)
				{
					code_cur = code_prev;
					*(str_ptr++) = lzw_ascii;
					stack_size++;
				}
				
				while (code_cur >= 0x100)
				{
					*(str_ptr++) = lzw_dict[code_cur].ascii;
					stack_size++;
					code_cur = lzw_dict[code_cur].prev;
				}
				lzw_ascii = code_cur;
				*(str_ptr++) = lzw_ascii;
				stack_size++;
				
				while (stack_size > 0)
				{
					str_ptr--;
					*(di++) = *str_ptr;
					stack_size--;
				}			
				stack_size = 0;
		
				lzw_dict[code_next].ascii = lzw_ascii;
				lzw_dict[code_next].prev = code_prev;
				code_next++;
				code_prev = code_read;
				if ( (code_next>=code_max) && (lzw_code_width!=11) )
				{
					lzw_code_width++;
					code_max <<= 1;
				}
			}
			code_read = lzw_read_next();
		}
	}

	return di - lzw_uncomp_buff;
}

// cur byte in the lzw_buff.. not in the res
void lzw_buff_fill(u16 cur_byte)	// cur point?
{
	u16 temp2;
	temp2 = lzw_buff_size - cur_byte;
	
	if ( temp2 < lzw_res_size)
		lzw_res_size -= temp2;
	else
	{
		temp2 = lzw_res_size;
		lzw_res_size = 0;
	}
	
	if ( temp2 != 0)
	{
		fread(lzw_buff+cur_byte, sizeof(u8), temp2, lzw_res_stream);
		//lib_read(lzw_res_stream, lzw_buff+cur_byte, temp2);
	}
}



u16 lzw_read_next()
{
	u16 code_new, over_flow;
	
	if (lzw_bit_cur >= lzw_bit_max)
	{
		lzw_buff_shift();
		if (errno != 0)
			return 0;
	}
		
	code_new = load_le_16(lzw_buff + (lzw_bit_cur>>3));
	code_new >>= lzw_bit_cur&7;
	over_flow = lzw_buff[(lzw_bit_cur>>3)+2];
	over_flow <<= (16-(lzw_bit_cur&7));
	
	code_new |= over_flow;
	
	lzw_bit_cur += lzw_code_width;
	return code_new & lzw_code_mask[lzw_code_width-9];
}

// si = bit cur
void lzw_buff_shift()
{
	u16 cx;
	cx = lzw_buff_size - (lzw_bit_cur>>3);
	
	memmove(lzw_buff, lzw_buff+(lzw_bit_cur>>3), cx);
	lzw_buff_fill(cx);
	lzw_bit_cur = lzw_bit_cur & 7;
}

