/*
_DirLoad                         cseg     00004305 00000049
_DirFind                         cseg     0000434E 00000023

_DirLogic                        cseg     00004371 00000034
_DirView                         cseg     000043A5 00000034
_DirPicture                      cseg     000043D9 00000034
_DirSound                        cseg     0000440D 00000034

_ResNotFound                     cseg     00004441 0000002F
*/

#include "../agi.h"
#include "res.h"

#include <stdio.h>
#include "../base.h"
#include "../ui/msg.h"
#include "../sys/endian.h"

#include "../game_id.h"

#define DIR_ITEM_SIZE 3

u8 *dir_log_data = 0;
u8 *dir_pic_data = 0;
u8 *dir_view_data = 0;
u8 *dir_snd_data = 0;

u8 dir_id[10] = "";

void v2_dir_load(void)
{
	dir_log_data = file_load("logdir", 0);
	dir_pic_data = file_load("picdir", 0);
	dir_view_data = file_load("viewdir", 0);
	dir_snd_data = file_load("snddir", 0);
}

void v3_dir_load(void)
{
	u8 dir_v3_name[15];
	FILE *dir_stream;
	u8 *dir_data;
	
	if (dir_id[0] == 0)
		game_id_copy();
	
	sprintf(dir_v3_name, "%sdir", dir_id);
	dir_stream = fopen(dir_v3_name, "rb");
	if ( dir_stream != 0)
	{
		fclose(dir_stream);
		dir_data = file_load(dir_v3_name, 0);
		dir_log_data = dir_data + load_le_16(dir_data+0);
		dir_pic_data = dir_data + load_le_16(dir_data+2);
		dir_view_data = dir_data + load_le_16(dir_data+4);
		dir_snd_data = dir_data + load_le_16(dir_data+6);
	}
	else
	{
		v2_dir_load();
		/*dir_log_data = file_load("logdir", 0);
		dir_pic_data = file_load("picdir", 0);
		dir_view_data = file_load("viewdir", 0);
		dir_snd_data = file_load("snddir", 0);*/
	}
}



void dir_load(void)
{
	switch(standard.res_type)
	{
		case RES_V3:
		case RES_V3_4:
			v3_dir_load();
			break;
		case RES_V2:
			v2_dir_load();
			break;
		default:
			printf("dir_load(): unknown resource type\n");
			agi_exit();
	}
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


