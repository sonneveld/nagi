/*
_PicDecompress                   cseg     00009A5B 000000D4
_PdBuffFill                      cseg     00009B2F 00000041
_PdBuffShift                     cseg     00009B70 0000001E
*/

#include "../agi.h"
#include "res.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

void pd_buff_fill(u16 byte_cur);
u8 *pd_buff_shift(u8 *buff_cur);


FILE *pd_stream = 0;
u16 pd_stream_size = 0;
u8 *pd_buff = 0;
u8 *pd_buff_end = 0;
u16 pd_buff_size = 0;
u8 *pd_uncomp_buff = 0;
u8 nibble_pad = 0;
u8 nibble_align = 0;

u16 pic_decompress(FILE *cfile, u8 *cbuff, u16 cfile_size, u8 *buff, u16 cbuff_size)
{
	u8 *di, *si;
	u8 pic_code;
	
	pd_stream = cfile;
	pd_buff = cbuff;
	pd_stream_size = cfile_size;
	pd_uncomp_buff = buff;
	pd_buff_size = cbuff_size;
	
	pd_buff_end = cbuff + cbuff_size - 2;
	nibble_pad = 0;
	nibble_align = 0;
	pd_buff_fill(0);
	di = pd_uncomp_buff;
	
	if (errno == 0)
	{
		si = cbuff;
		do
		{
			if (si >= pd_buff_end)
			{
				si = pd_buff_shift(si);
				
				if ( errno != 0)
					break;
			}
			pic_code = *si;
			if (nibble_pad == 1)
			{
				if (nibble_align == 0)
					pic_code >>= 4;
				else
				{
					pic_code &= 0xF;
					si++;
				}
				nibble_align ^= 1;
				nibble_pad = 0;
			}
			else
			{
				si++;
				if (nibble_align == 1)
				{
					pic_code <<= 4;
					pic_code |= (*si)>>4;
				}
				if ( (pic_code == 0xF0) || (pic_code == 0xF2) )
					nibble_pad = 1;
				else
					nibble_pad = 0;
			}
			
			*(di++) = pic_code;
		} while (pic_code != 0xFF);
	}
	
	return di - pd_uncomp_buff;	
}

void pd_buff_fill(u16 byte_cur)
{
	u16 size;
	size = pd_buff_size-byte_cur;
	if ( size < pd_stream_size)
		pd_stream_size -= size;
	else
	{
		size = pd_stream_size;
		pd_stream_size = 0;
	}
	if ( size != 0)
		fread(pd_buff+byte_cur, sizeof(u8), size, pd_stream);
}

u8 *pd_buff_shift(u8 *buff_cur)
{
	u16 cx;
	cx = pd_buff_size-(buff_cur-pd_buff);
	memmove(pd_buff, buff_cur, cx);
	pd_buff_fill(cx);
	return pd_buff;
}
