/*
sample_init
sample_denit
table_new
sample_open
sample_close
mix_new
mix_add
*/

#include <stdlib.h>
#include <stdio.h>

#include "../agi.h"

#include "../sound/sound.h"  
#include "../sys/endian.h"
#include "../sys/mem_wrap.h"

#include "../sys/sys_dir.h"

#define TABLE_SIZE 1024
// 111843.75
#define FREQ_DIV 111844
#define MULTIPLE 111844

SAMPLE *sample[2];

void sample_init(void);
void sample_denit(void);
SAMPLE *sample_open(u8 *file_name, int freq_base);
void sample_close(SAMPLE *s);
int *table_new(u16 freq_base);
void mix_new(u8 *stream, CHANNEL *c, int len);
void mix_add(u8 *stream, CHANNEL *c, int len);


void sample_init()
{ 
	// open up an array of two samples
	dir_preset_change(DIR_PRESET_NAGI);
	sample[0] = sample_open("tone50.pcm", 50);
	sample[1] = sample_open("tone50.pcm", 50);

}


void sample_denit()
{
	sample_close(sample[0]);
	sample_close(sample[1]);
}


SAMPLE *sample_open(u8 *file_name, int freq_base)
{
	FILE *file;
	SAMPLE *s;
	
	s = (SAMPLE *)a_malloc(sizeof(SAMPLE) );
	
	// base frequency
	s->freq_base = freq_base;

	if ( (file = fopen(file_name, "rb")) == 0 )
	{
		printf("sample_open(): Can't find %s.\n", file_name);
		exit(1);
	}
	rewind(file);
	
	if ( fseek(file, 0, SEEK_END) != 0)
	{
		printf("sample_open(): Error seeking to end of %s\n", file_name);
		exit(1);
	}
	#warning will probably only work on gnu systems
	// file size
	s->size = ftell(file);
	rewind(file);
	
	// read data
	s->data = (u8 *)a_malloc(s->size);
	if (fread(s->data, sizeof(u8), s->size, file) != s->size)
	{
		printf("sample_open(): Error occured reading %s into memory\n", file_name);
		exit(1);
	}
	
	// scale table
	s->scale_table = table_new(s->freq_base);
	
	fclose(file);
	return(s);	
}


void sample_close(SAMPLE *s)
{
	if (s != 0)
	{
		a_free(s->scale_table);
		a_free(s->data);
		a_free(s);
	}
}


int *table_new(u16 freq_base)
{
	int i;
	int *table;
	
	table = (int *)a_malloc(TABLE_SIZE * sizeof(int));
	
	for (i=1; i<TABLE_SIZE; i++)
		table[i] = (int)( (double)freq_base * i / FREQ_DIV * MULTIPLE + 0.5);
	return table;
}

u32 tickz;

void mix_new(u8 *stream, CHANNEL *c, int len)
{
	int samp_cur;
	SAMPLE *s;
	
	s = sample[c->sample_num];

	if ( (c->freq_count == 0) || (c->attenuation >= 0x0F) )
	{
		memset(stream, 0, len);
		
		c->count = 0;
		//c->scale = 1;
		c->sample_cur = s->data;
		c->freq_prev = 0;
		#warning add an ending wave
	}
	else
	{
		
		if (c->freq_count != c->freq_prev)
		{
			c->count = 0;
			c->scale = s->scale_table[ c->freq_count ];
			if (c->freq_prev == 0)
				c->sample_cur = s->data;
			c->freq_prev = c->freq_count;
		}
		
		samp_cur=0;
		while (samp_cur < len)
		{
			while (  (c->count < c->scale) && (samp_cur < len)  )
			{
				//add in extra sample;
				*(stream++) = c->sample_cur[0];
				*(stream++) = c->sample_cur[1];
				samp_cur += 2;
				c->count += MULTIPLE;
			}
			if ( c->count >= c->scale )
			{
				c->sample_cur += 2;
				if ( (c->sample_cur - s->data) >= (s->size) )
					c->sample_cur = s->data;
				c->count = c->count - c->scale;
			}
		}
		
	}
	
}


void mix_add(u8 *stream, CHANNEL *c, int len)
{
	int samp_cur;
	s16 value;
	SAMPLE *s;

	s = sample[c->sample_num];

	if ( (c->freq_count == 0) || (c->attenuation >= 0x0F) )
	{
		//memset(stream, 0, len);
		c->count = 0;
		//c->scale = 1;
		c->sample_cur = s->data;
		c->freq_prev = c->freq_count;
		#warning add an ending wave
	}
	else
	{
		if (c->freq_count != c->freq_prev)
		{
			c->count = 0;
			c->scale = s->scale_table[ c->freq_count ];
			if (c->freq_prev == 0)
				c->sample_cur = s->data;
			c->freq_prev = c->freq_count;
		}
			
		samp_cur=0;

		while (samp_cur < len)
		{
			while (  (c->count < c->scale) && (samp_cur < len)  )
			{
				//add in extra sample;
				value = load_le_16(stream);
				value += load_le_16(c->sample_cur);
				store_le_16(stream, value);
				stream += 2;
				samp_cur += 2;
				c->count += MULTIPLE;
			}
			if ( c->count >= c->scale )
			{
				c->sample_cur += 2;
				if ( (c->sample_cur - s->data) >= (s->size) )
					c->sample_cur = s->data;
				c->count = c->count - c->scale;
			}
		}
	}
}
