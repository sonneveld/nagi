/*
_DirLoad                         cseg     00004305 00000049
_DirFind                         cseg     0000434E 00000023

_DirLogic                        cseg     00004371 00000034
_DirView                         cseg     000043A5 00000034
_DirPicture                      cseg     000043D9 00000034
_DirSound                        cseg     0000440D 00000034

_ResNotFound                     cseg     00004441 0000002F
*/

//~ RaDIaT1oN (2002-04-29):
//~ open first lowercase name changes

#include <string.h>

#include "../agi.h"
#include "res.h"

#include <stdio.h>
#include "../base.h"
#include "../ui/msg.h"
#include "../sys/endian.h"

#include "../game_id.h"
#include "../sys/sys_dir.h"

#include "../sys/agi_file.h"
#include "../sys/mem_wrap.h"

#define DIR_ITEM_SIZE 3

u8 dir_ver = 0;
u8 *dir_data = 0;
u8 *dir_log_data = 0;
u8 *dir_pic_data = 0;
u8 *dir_view_data = 0;
u8 *dir_snd_data = 0;


void dir_load(void)
{
	// -1 is skip
	int dir_type_list[] = {1, 2, 3, 0};
	int *dir_type_ptr = 0;
	int dir_type;
	int dir_loaded = 0;
	
	switch(c_game_dir_type)
	{
		case DIR_COMB:
			dir_type = 2;
			dir_type_list[1] = -1;
			break;
		case DIR_AMIGA:
			dir_type = 3;
			dir_type_list[2] = -1;
			break;
		case DIR_SEP:
			dir_type = 1;
			dir_type_list[0] = -1;
			break;
		default:
			printf("dir_load(): unknown resource type\n");
			dir_type = -1;
	}
	
	dir_preset_change(DIR_PRESET_GAME);
	
	while (!dir_loaded)
	{
		u8* dir_v3_name = alloca(strlen("dir") + ID_SIZE + 1);
		FILE *dir_stream;
		
		dir_stream = 0;
		
		switch (dir_type)
		{
			case 0:
				printf("dir_load(): unable to load an AGI directory.\n");
				agi_exit();
				break;
			case 1:
				if (dir_type_ptr)
					printf("dir_load(): attempting to load seperated dir structure.\n");
				dir_log_data = file_to_buf("logdir");
				dir_pic_data = file_to_buf("picdir");
				dir_view_data = file_to_buf("viewdir");
				dir_snd_data = file_to_buf("snddir");
				dir_ver = 2;
				if ( (dir_log_data != 0) && (dir_pic_data != 0) && 
					(dir_view_data != 0) && (dir_snd_data != 0) )
					dir_loaded = 1;
				
				// test if it loaded properly
				break;
			case 2:
				if (dir_type_ptr)
					printf("dir_load(): attempting to load combined PC dir structure.\n");
				sprintf(dir_v3_name, "%sdir", c_game_file_id);
//				dir_stream = fopen(dir_v3_name, "rb");
				dir_stream = fopen_nocase(dir_v3_name);
				break;
			case 3:
				if (dir_type_ptr)
					printf("dir_load(): attempting to load combined Amiga dir structure.\n");
				sprintf(dir_v3_name, "dirs");
//				dir_stream = fopen(dir_v3_name, "rb");
				dir_stream = fopen_nocase(dir_v3_name);
				break;
			case -1:
			default:
			;
		}
		
		if (dir_stream != 0)
		{
			fclose(dir_stream);
			dir_data = file_load(dir_v3_name, 0);
			dir_log_data = dir_data + load_le_16(dir_data+0);
			dir_pic_data = dir_data + load_le_16(dir_data+2);
			dir_view_data = dir_data + load_le_16(dir_data+4);
			dir_snd_data = dir_data + load_le_16(dir_data+6);
			dir_ver = 3;
			dir_loaded = 1;
		}
		
		if (!dir_loaded)
		{
			if (dir_type_ptr == 0)
			{
				printf("dir_load(): original dir option failed.  Trying alternative dir options.\n");
				dir_type_ptr = dir_type_list;
			}
			dir_type = *(dir_type_ptr++);
		}
		
	}
	
}

void dir_unload(void)
{
	switch (dir_ver)
	{
		case 2:
			a_free(dir_log_data);
			a_free(dir_pic_data);
			a_free(dir_view_data);
			a_free(dir_snd_data);
			break;
		case 3:
			a_free(dir_data);
			break;
	}
	
	dir_data = 0;
	dir_log_data = 0;
	dir_pic_data = 0;
	dir_view_data = 0;
	dir_snd_data = 0;
}

// checks the first four bits of a vol_item.
u8 *dir_check(u8 *v)
{
	if ( (v[0] & 0xF0) == 0xF0)
		return 0;
	else
		return v;
}

void dir_error(u8 *name, u16 num)
{
	u8 str[100];
	sprintf(str, "%s %d not found", name, num);
	message_box(str);
	agi_exit();
}

u8 *dir_logic(u16 num)
{
	u8 *entry;
	entry = dir_check(dir_log_data + num * DIR_ITEM_SIZE);
	if ( entry == 0)
		dir_error("logic", num);
	return entry;
}

u8 *dir_view(u16 num)
{
	u8 *entry;
	entry = dir_check(dir_view_data + num * DIR_ITEM_SIZE);
	if ( entry == 0)
		dir_error("view", num);
	return entry;
}

u8 *dir_picture(u16 num)
{
	u8 *entry;
	entry = dir_check(dir_pic_data + num * DIR_ITEM_SIZE);
	if ( entry == 0)
		dir_error("picture", num);
	return entry;
}

u8 *dir_sound(u16 num)
{
	u8 *entry;
	entry = dir_check(dir_snd_data + num * DIR_ITEM_SIZE);
	if ( entry == 0)
		dir_error("sound", num);
	return entry;
}


