#include <stdlib.h>
#include <stdio.h>



typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;
typedef signed char		s8;
typedef signed short		s16;
typedef signed long		s32;
typedef unsigned int		uint;
typedef signed int		sint;


struct font_struct
{
	u16 width;
	u16 height;
	u16 size;	// number of chars
	u8 *data;
	u32 align; // size of each char in bytes
	u32 line_size;  // size of each line in bytes
};
typedef struct font_struct FONT;
	
//#include "sys/font_8x8.h"


#define FONTWIDTH (4)
#define FONTHEIGHT (8)
#define FONTNUM (256)
#define FONTALIGN (8)
#define FONTDATAMAX (FONTALIGN * FONTNUM)

unsigned char font_4x8_data[FONTDATAMAX] = {

  

};

FONT font_4x8 = {FONTWIDTH, FONTHEIGHT, 
			FONTNUM, font_4x8_data, FONTALIGN,1};

	



// width, height, size

// char number....   data

// charnumber == 0xFF ends

int main(int argc, char *argv[])
{
	int i, j, k;
	u8 b;
	FONT *this_font;
	u8 data;
	FILE *file;
	u8 *ch_ptr;
	u8 ch_check;
	
	file = fopen("font_4x8.fnt", "w");
	
	this_font = &font_4x8;

	
	data = this_font->width;
	fwrite(&data, sizeof(u8), 1, file);
	data = this_font->height;
	fwrite(&data, sizeof(u8), 1, file);
	data = this_font->align;
	fwrite(&data, sizeof(u8), 1, file);
	
	
	for (i=0; i<128; i++)   // each char
	{
		// get a ptr to char
		ch_ptr = this_font->data + this_font->align*i;
		
		// check if it equals 0
		ch_check = 0;
		for (j=0; j<this_font->align; j++)
		{
			ch_check |= ch_ptr[j];
		}
		
		// if not.. write char number and then data
		if ( (ch_check != 0) && ( (i==0x1a) || (i> 32) ) )
		{
			data = i;
			fwrite(&data, sizeof(u8), 1, file);
			
			fwrite(ch_ptr, this_font->align, 1, file);
		}
		else
		{
			printf("skipping ch %d\n", i);
		}

	}
	
	// end of file
	data = 0xFF;
	fwrite(&data, sizeof(u8), 1, file);
	
	fclose(file);
	return 0;
	
}
