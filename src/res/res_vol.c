/*
_VolResLoad                      cseg     00002E32 00000024
_ResLoad                         cseg     00002E56 0000019B
_ErrInsertDisk                   cseg     00002FF1 00000025
_ErrMsg                          cseg     00003016 00000025
_ErrWrongDisk                    cseg     0000303B 00000045
_VolumesOpen                     cseg     00003080 00000056
_VolumesClose                    cseg     000030D6 0000003D
_FileLoad                        cseg     00003113 000000C5
*/

#define RES_HEAD_SIZE 5

#include "../agi.h"

#include <stdlib.h>
#include <stdio.h>

#include "res.h"

// print_err_code
#include <setjmp.h>
#include "../sys/error.h"
// agi_exit
#include "../base.h"
// endian
#include "../sys/endian.h"
// print
#include "../ui/msg.h"

#include "../sys/mem_wrap.h"

#include "../sys/sys_dir.h"



u16 volume_error = 0;
u8 res_header[8];
// size 16 for v3,  10 for v2
FILE *vol_handle_table[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0};
u16 free_mem_check = 0;
u16 res_size = 0;
u16 vol_disk_num = 0;
	
u16 not_compressed = 0;

u8 *vol_res_load(u8 *dir_entry, u8 *buff)
{
	u8 *si=0;
	
	do
	{
		if (c_game_compression)
			si = v3_res_load(dir_entry, buff);
		else
			si = v2_res_load(dir_entry, buff);
	} while (  (si==0) && (volume_error != 5)  );
	
	return si;
}

u8 *v2_res_load(u8 *dir_entry, u8 *buff)
{	
	u8 res_head[5];
	//u8 *mem_ptr_orig;		// orig mem ptr
	u16 vol_num;		// vol num
	FILE *vol_stream;	// vol stream
	u32 vol_pos;
	
	//mem_ptr_orig = get_mem_ptr();
	if (vol_handle_table[0] == 0)
		volumes_open();

	vol_num = dir_entry[0] >> 4;	// vol num
	if ( vol_num != 0)
		vol_disk_num = vol_num;
	if ( vol_disk_num == 0)
		vol_disk_num = 1;

	vol_stream = vol_handle_table[vol_num];
	

	if ( vol_stream == 0)
	{
		volumes_close();
		volume_error = 1;
		err_insert_disk(vol_num);
		volumes_open();
	}
	else
	{
		vol_pos = dir_entry[2];
		vol_pos |= dir_entry[1] << 8;
		vol_pos |= (dir_entry[0] & 0x0F) << 16;
	
		fseek(vol_stream, vol_pos, SEEK_SET);
		if ( fread(res_head, sizeof(u8), RES_HEAD_SIZE, vol_stream) == RES_HEAD_SIZE)
		{
			if (  (res_head[0]!=0x12)||(res_head[1]!=0x34)||(res_head[2]!=vol_num)  )
			{
				volumes_close();
				volume_error = 1;
				if ( err_wrong_disk(vol_num) == 0 )
					agi_exit();
				volumes_open();
				//set_mem_ptr(mem_ptr_orig);
				return 0;
			}

			res_size = load_le_16(res_head +3);
			if ( buff == 0)
			{
				if ( free_mem_check != 0)
				{
					/*
					if ( update_var8() < res_size)
					{
						volume_error = 5;
						//set_mem_ptr(mem_ptr_orig);
						return 0;
					} */
					buff = a_malloc(res_size);
					if ( buff == 0)
					{
						volume_error = 5;
						return 0;
					}
				}
				else
				{
					buff = a_malloc(res_size);
				}
			}
			
			if ( fread(buff, sizeof(u8), res_size, vol_stream) == res_size)
				return buff;
		}
		if ( print_err_code() != 0)
		{
			//set_mem_ptr(mem_ptr_orig);
			if (buff != 0)
				free(buff);
			return 0;
		}
		agi_exit();
	}
	if (buff != 0)
		a_free(buff);
	return 0;
}

u8 *v3_res_load(u8 *dir_entry, u8 *buff)
{
	u16 pic_compressed;		// 1 = picture compression
	u8 decomp_buff[0x400];
	u16 res_comp_size;		// compressed size
	//void *mem_orig;			// original mem_ptr
	u16 vol_num;			// volume number
	FILE *vol_stream;		// vol handle
	u32 res_pos;			// position of resource in vol
	
	//mem_orig = mem_ptr_get();
	if (vol_handle_table[0] == 0)
		volumes_open();
	vol_num = dir_entry[0] >> 4;	// vol num
	if ( (vol_num!=0) && (vol_num<=8) )
		vol_disk_num = vol_num;
	if (vol_disk_num == 0)
		vol_disk_num = 1;
	vol_stream = vol_handle_table[vol_num];
	if (vol_stream == 0)
	{
		volumes_close();
		volume_error = 1;
		err_insert_disk(vol_num);
		volumes_open();
		goto res_error_2;
	}
	else
	{
		res_pos = dir_entry[2];
		res_pos |= dir_entry[1] << 8;
		res_pos |= (dir_entry[0] & 0x0F) << 16;
		fseek(vol_stream, res_pos, SEEK_SET);
		if (fread(&res_header, sizeof(u8), 7, vol_stream) != 7)
			goto res_error;
		if ( (res_header[2] & 0x80) != 0)
		{
			pic_compressed = 1;
			res_header[2] = res_header[2] & 0xF;	// vol num
		}
		else
			pic_compressed = 0;

		if ( (res_header[0]!=0x12)||(res_header[1]!=0x34)||(res_header[2]!=vol_num) )
		{
			volumes_close();
			volume_error = 1;
			if ( err_wrong_disk(vol_num) == 0)
				agi_exit();
			volumes_open();
			goto res_error_2;
		}
		res_size = load_le_16(res_header + 3);	// uncompressed
		res_comp_size = load_le_16(res_header + 5); 	// compressed
		if (buff == 0)
		{
			/*if ( free_mem_check != 0)
				if (var8update() < res_size)
				{
					volume_error = 5;
					goto res_error_2;
				}*/
			buff = a_malloc(res_size);
		}

		if ( pic_compressed != 0)
		{
			if (pic_decompress(vol_stream, decomp_buff, res_comp_size, buff, 0x400) != res_size)
				goto res_error;
		}
		else if (res_size == res_comp_size)
		{
			if (fread(buff, sizeof(u8), res_size, vol_stream) != res_size)
				goto res_error;
			not_compressed = 1;
		}
		else
		{
			if ( lzw_decompress(vol_stream, decomp_buff, res_comp_size, buff, 0x400) != res_size)
				goto res_error;
			not_compressed = 0;
		}
		return buff;
	}
	
res_error:
	if (print_err_code() == 0)
		agi_exit();
res_error_2:
	if (buff != 0)
		free(buff);
	//mem_ptr_set(mem_orig);
	return 0;
}




void err_insert_disk(u16 num)
{
	u8 msg[100];
	err_msg(msg, num);
	message_box(msg);
	//disk_reset();
}

void err_msg(u8 *msg, u16 num)
{
	if ((num == 0) || (c_game_compression && (num > 8)) )
		sprintf(msg, "Please insert disk %d\nand press ENTER.",
				vol_disk_num);
	else
		sprintf(msg, "Please insert disk %d\nand press ENTER.",
				num);
}

u16 err_wrong_disk(u16 num)
{
	u16 ret_value;
	u8 msg_insert[100];
	u8 msg_main[200];
	
	beep_speaker();
	err_msg(msg_insert, num);
	sprintf(msg_main, "%s%s\n%s", "That is the wrong disk.\n\n",
			msg_insert, "\nPress ESC to quit.");
	ret_value =  message_box(msg_main);
	//disk_reset();
	return ret_value;
}

void volumes_open()
{
	u8 name[strlen("vol.XXXXXXX") + ID_SIZE + 1];
	//u16 vol_max;
	u16 i;
	/*
	if ((c_game_res==RES_V2) ||(c_game_res==RES_V3_4))
		vol_max = 0x10;
	else
		vol_max = 0x5;*/
	dir_preset_change(DIR_PRESET_GAME);

	for (i=0 ; i<0x10 ; i++)
	{
		if (c_game_file_id[0] != '\0')
			sprintf(name, "%svol.%d", c_game_file_id, i);
		else
			sprintf(name, "vol.%d", i);
		//do
		//{
			//errno = 0;
			vol_handle_table[i] = fopen(name, "rb");
			/*
			if ( (errno != 0) && (errno != ENOENT)  )
				if (print_err_code() == 0)
					agi_exit();
			*/
		//} while (errno != 0);
	}
	errno=0;
}

void volumes_close()
{
	u16 i;
	
	for (i=0 ; i<5 ; i++)
	{
		if (vol_handle_table[i] != 0)
		{
			fclose(vol_handle_table[i]);
			vol_handle_table[i] = 0;
		}
	}
	#warning LOGGING IS NOT COMPLETE YET
	//log_file_close();
}

u8 *file_load(u8 *name, u8 *buff)
{
	u8 msg[100];
	fpos_t file_size;
	FILE *file_stream;
	u8 newline_orig;

	newline_orig = msgstate.newline_char;
	msgstate.newline_char = '@';
	while (  (file_stream=fopen(name, "rb")) == 0  )
	{
		sprintf(msg, "Can't find %s.%s%s", name,
			"\nPress ENTER to try again.", "\nPress ESC to quit.");
		if (message_box(msg) == 0)
		{
			agi_exit();
		}
	}
	msgstate.newline_char = newline_orig;
	
	fseek(file_stream, 0, SEEK_END);
	fgetpos(file_stream, &file_size);
	fseek(file_stream, 0, SEEK_SET);
	res_size = file_size;
	if (buff == 0)
		buff = (u8 *)a_malloc(file_size);
	
	if ( fread(buff, sizeof(u8), file_size, file_stream) != file_size)
		if (print_err_code == 0)
		{
			agi_exit();
		}
		
	fclose(file_stream);
	return buff;
}