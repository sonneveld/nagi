
/*
_StateGetInfo                    cseg     000088EB 000000C4
_StateDiskCheck                  cseg     000089AF 0000006E
_StateGetPath                    cseg     00008A1D 0000008F
_StateGetText                    cseg     00008AAC 00000080
_StateGetFname                   cseg     00008B2C 000003B8
_StateGetDiz                     cseg     00008EE4 000000AF
_StateDrawArrow                  cseg     00008F93 00000020
_StateDrawBlank                  cseg     00008FB3 00000020
*/

#include "agi.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>

#include "sys/vstring.h"

#include "state_io.h"
#include "ui/cmd_input.h"
#include "ui/agi_text.h"
#include "ui/window.h"
#include "sound/sound.h"
#include "ui/msg.h"
#include "ui/string.h"
#include "ui/events.h"
#include "ui/printf.h"
#include "agi_string.h"
#include "res/res.h"

#include "sys/mem_wrap.h"
#include "sys/sys_dir.h"


u16 diskspace_available = 1;

VSTRING *save_dir = 0;
VSTRING *save_filename = 0;

u16 word_1aad = 0;
u8 save_description[0x20];

u8 save_drive = 0;

struct save_struct
{
	u16 num;
	u8 diz[31];
};

typedef struct save_struct SAVE;

u16 state_get_info(u8 state_type);
u16 state_get_path(u8 state_type);
u16 state_get_text(u8 *msg, u8 *str, u16 str_alloc);
u16 state_get_filename(u8 state_type);
u16 state_get_diz(u16 var8, SAVE *vara, u32 *varc);
void state_draw_arrow(u16 var8);
void state_draw_blank(u16 var8);
void state_name_create(u16 save_num, VSTRING *filename);
u16 state_dir_check(VSTRING *dir);


u16 state_get_info(u8 state_type)
{
	u16 edit_state;
	u16 save_num;
	
	edit_state = input_edit_status();
	input_edit_on();
	text_attrib_push();
	push_row_col();
	sound_stop();
	text_colour(0, 0x0F);
	
	////save_filename[0] = 0;
	
	if ( state_get_path(state_type) == 0)	// get save path
		goto info_cancel;

	//printf("f=%s d=%s\n", save_filename->data, save_dir->data);
	
	//if (save_drive == 'b')	// drive letter
	//	if (drives_found == 0)
	//	{
	//		save_directory[0] = 'a';
	//		save_drive = 'a';
	//	}
	//if (save_disk_check(state_type) == 0)	// make sure savedisk is in
	//	goto info_cancel;
	
	word_1aad = 1;	// in savegame dialogue stuff
	save_num = state_get_filename(state_type);	// game select

	if (save_num != 0)
	{
		if ( (state_type=='s') && (state_name_auto[0]==0) )
		{
			if (state_get_text("How would you like to describe this saved game?\n\n", save_description, sizeof(save_description)) == 0)
				save_num = 0;
			else
				state_name_create(save_num, save_filename);
		}
		else
			state_name_create(save_num, save_filename);
	}
	
	
cancel_return:
	pop_row_col();
	text_attrib_pop();
	if (edit_state != 0)
		input_edit_off();
	return save_num;
	
info_cancel:
	save_num = 0;
	goto cancel_return;
}

// check if the specifed save disk is in the drive.
// if not then close all volumes, reset disk 'n all that
// useful for 1 disk drive systems and you have to swap
/*
u16 save_disk_check(u8 state_type)
{
	u16 tempa2;
	u8 tempa0[160];
	tempa2 = 1;
	if ( save_drive <= 'b')
	{
		if (lib_get_disk() == save_drive)
		{
			volumes_close();
			if (state_type == 'r')
				ax = "restore";
			else
				ax = "save";
			
			lib_sprintf(tempa0, "Please put your save game\ndisk in drive %c.\nPress ENTER to continue.\nPress ESC to not %s a game.", save_drive, ax); 
			tempa2 = message_box(tempa0);
		}
		disk_reset();
	}
	return tempa2;
} */

u16 state_get_path(u8 state_type)
{
	u8 user_msg[160];
	VSTRING *msg = 0;
	
	if ( (save_dir->data==0) || (save_dir->data[0]==0) )
		vstring_getcwd(save_dir);
	
	if (state_name_auto[0] != 0)
		return 1;
	
	do
	{
		msg = vstring_new(0, 160);
		
		if (state_type == 's')
			sprintf(msg->data, 
					"         SAVE GAME\n\nOn which disk or in which directory do you wish to save this game?\n\n%s\n\n",
					"(For example, \"B:\" or \"C:\\savegame\")");
		else
			sprintf(msg->data, 
					"        RESTORE GAME\n\nOn which disk or in which directory is the game that you want to restore?\n\n%s\n\n",
					"(For example, \"B:\" or \"C:\\savegame\")");
	
		if (state_get_text(msg->data, save_dir->data, save_dir->size) == 0)
		{
			vstring_free(msg);
			return 0;
		}
		if (state_dir_check(save_dir) != 0)
		{
			vstring_free(msg);
			return 1;
		}
		
		vstring_set_size(msg, 100 + strlen(save_dir->data));
		
		sprintf(msg->data, 
				"There is no directory named\n %s\nPress ENTER to try again.\nPress ESC to cancel.",
				save_dir->data);
	}
	while (message_box(msg->data) != 0);
	
	if (msg != 0)
		vstring_free(msg);
	
	return 0;
}

u16 state_get_text(u8 *msg, u8 *str, u16 str_alloc)
{
	u16 edit_result;
	
	message_box_draw(msg, 0, 0x1F, 1);
	goto_row_col(msgstate.tpos_edge.row, msgstate.tpos.col);
	window_clear(msgstate.tpos_edge.row, msgstate.tpos.col, msgstate.tpos_edge.row, msgstate.tpos_edge.col-1, 0);
	text_attrib_push();
	text_colour(0xF, 0);
	
	edit_result = string_edit(str, str_alloc, 0x1f);
	
	text_attrib_pop();
	cmd_close_window(0);
	
	return (edit_result==0xD);
}

u16 state_get_filename(u8 state_type)
{
	//u16 temp202;	// cur file time	lo
	//u16 temp200;	// date		hi
	u32 temp202;
	
	//u16 temp1fe;	// highest file time	lo
	//u16 temp1fc;	// date		hi
	u32 temp1fe;
	
	u16 temp1fa;	// text pos
	u16 temp1f8;	// text height
	u16 temp1f6;	// last file saved number
	u16 temp1f4;	// number of games to show?
	u16 temp1f2;	// cur save num counter
	//u8 temp1f0[100];	// msg temp
	SAVE temp18c[12];
	
	AGI_EVENT *state_event;
	
	temp1fe = 0;
	//temp1fc = 0;
	temp1f6 = 0;
	word_1aad = 0;
	
	if (state_type == 's')
	{
		do			// SAVE
		{
			if (word_1aad == 1)
				volumes_close();
			for (temp1f2=0; temp1f2<12; temp1f2++)
				if (state_get_diz(temp1f2+1, &temp18c[temp1f2], &temp202) != 0)
					if (temp202 > temp1fe)
					{
						temp1fe = temp202;
						temp1f6 = temp1f2;
					}
		}
		while ( (word_1aad++) == 0);
		temp1f4 = 12;
	}
	else
	{
		do			// RESTORE
		{
			if (word_1aad == 1)
				volumes_close();
			temp1f4 = 0;
			
			for (temp1f2=0; temp1f2<12; temp1f2++)
				if (state_get_diz(temp1f2+1, &temp18c[temp1f4], &temp202) != 0)
				{
					if (temp202 > temp1fe)
					{
						temp1fe = temp202;
						temp1f6 = temp1f4;
					}
					temp1f4++;
				}
		}
		while ( (word_1aad++) == 0);
	}

	if (  (temp1f4 == 0) && (state_name_auto[0] == 0)  )
	{
		u8 *temp1f0;
		
		temp1f0 = alloca(100 + strlen(save_dir->data));
		
		sprintf(temp1f0, "There are no games to\nrestore in\n\n %s\n\nPress ENTER to continue.",
					save_dir->data);
		message_box(temp1f0);
		return 0;
	}
	
	if ( (state_name_auto[0]!=0) && (diskspace_available==0) )
	{
		strcpy(save_description, state_name_auto);
		for (temp1f2=0; temp1f2<12; temp1f2++)
			if (strcmp(state_name_auto, temp18c[temp1f2].diz) == 0)
				return temp18c[temp1f2].num;
	
		if (state_type == 's')
			for (temp1f2=0; temp1f2<12; temp1f2++)
				if (strcmp("", temp18c[temp1f2].diz) == 0)
					return temp18c[temp1f2].num;
		
		if (state_type == 'r')
			return 0;
	}

	//if ( (state_name_auto[0]!=0) && (diskspace_available!=0) )
	//	save_drive = lib_get_disk();
	
	temp1fa = 5;
	temp1f8 = temp1fa + temp1f4;
	
	if (state_name_auto[0] == 0)
	{
		// select game
		if (state_type == 's')
			message_box_draw("Use the arrow keys to select the slot in which you wish to save the game. Press ENTER to save in the slot, ESC to not save a game.",
				temp1f8, 0x22, 1);
		else
			message_box_draw("Use the arrow keys to select the game which you wish to restore. Press ENTER to restore the game, ESC to not restore a game.",
				temp1f8, 0x22, 1);
	}
	else if (diskspace_available == 0)
	{
		// disk full
		message_box_draw("   Sorry, this disk is full.\nPosition pointer and press ENTER\n    to overwrite a saved game\nor press ESC and try again \n    with another disk\n",
				temp1f8, 0x22, 1);
	}
	else
	{
		// select name
		message_box_draw("Use the arrow keys to move\n     the pointer to your name.\nThen press ENTER\n",
				temp1f8, 0x22, 1);	
	}
	
	temp1fa += msgstate.tpos.row;
	diskspace_available = 0;
	
	for (temp1f2=0; temp1f2<temp1f4; temp1f2++)
	{
		goto_row_col(temp1fa+temp1f2, msgstate.tpos.col);
		agi_printf(" - %s", temp18c[temp1f2].diz);
	}
	
	temp1f2 = temp1f6;
	state_draw_arrow(temp1f2+temp1fa);
	for(;;)
	{
		state_event = event_wait();
		//joy_butt_map(state_event);
		switch (state_event->type)
		{
			case 1:
				switch (state_event->data)
				{
					case 0xD:
						if (state_name_auto[0] == 0)
							strcpy(save_description, temp18c[temp1f2].diz);
						else
						{
							if (state_type == 'r')
								strcpy(state_name_auto, temp18c[temp1f2].diz);
						}
						cmd_close_window(0);
						return temp18c[temp1f2].num;
						
					case 0x1B:
						cmd_close_window(0);
						return 0;
				}
				break;
				
			case 2:
				state_draw_blank(temp1f2+temp1fa);
				switch(state_event->data)
				{
					case 1:
						if (temp1f2 == 0)
							temp1f2 = temp1f4 - 1;
						else
							temp1f2 = temp1f2 - 1;
						state_draw_arrow(temp1f2 + temp1fa);
						break;
						
					case 5:
						if ( temp1f2 == (temp1f4-1))
							temp1f2 = 0;
						else
							temp1f2 = temp1f2 + 1;
						state_draw_arrow(temp1f2 + temp1fa);
						break;
						
					default:
						state_draw_arrow(temp1f2 + temp1fa);
				}
				break;
		}
	}
}



u32 stream_get_date(FILE *fstream)
{
	int file_des;
	struct stat stat_buf;
		
	file_des = fileno(fstream);
	if (file_des != -1)
		if (fstat(file_des, &stat_buf) == 0)
			return (u32)stat_buf.st_mtime;
	return 0;
}


// read file description?
u16 state_get_diz(u16 s_num, SAVE *s_item, u32 *s_date)
{
	FILE *state_stream;	// file handle
	u8 state_id[ID_SIZE+1];
	//u8 temp40[64];
	VSTRING *name_temp;
	
	s_item->num = s_num;
	
	name_temp = vstring_new(0, 300);
	state_name_create(s_num, name_temp);
	state_stream = fopen(name_temp->data, "rb");
	vstring_free(name_temp);
	
	if (state_stream == 0)
	{
		s_item->diz[0] = 0;
		return 0;
	}

	*s_date = stream_get_date(state_stream);
	fread(s_item->diz, sizeof(u8), sizeof(s_item->diz), state_stream);
	fseek(state_stream, 2, SEEK_CUR);	
	fread(state_id, sizeof(u8), sizeof(state.id), state_stream);	
	fclose(state_stream);

	if (strcmp(state_id, state.id) != 0)
	{
		s_item->diz[0] = 0;
		return 0;
	}
	else
		return 1;
}

void state_draw_arrow(u16 var8)
{
	goto_row_col(var8, msgstate.tpos.col);
	window_put_char(0x1A);
	ch_update();
}

void state_draw_blank(u16 var8)
{
	goto_row_col(var8, msgstate.tpos.col);
	window_put_char(' ');
	ch_update();
}


//u8 *state_name_create(u16 save_num, u8 *buff, int *buff_size)
void state_name_create(u16 save_num, VSTRING *filename)
{
	u8 *dir_sep;
	int size_new;
	
	if ((strlen(save_dir->data)>0)&&(strchr("\\/", save_dir->data[strlen(save_dir->data)-1]) != 0))
		dir_sep = "";
	else if (strchr(save_dir->data, '/')==0)
		dir_sep = "\\";
	else
		dir_sep = "/";
	
	size_new = sizeof(save_dir->data) + sizeof(dir_sep) + sizeof(state.id) + 15;
	
	vstring_set_size(filename, size_new);
	
	// "c:\savegame\pq4sg.1"
	sprintf(filename->data, "%s%s%ssg.%d", save_dir->data, dir_sep, string_lower(state.id), save_num);
	//string_lower(filename->data);
}

// save path exists
/*
u16 save_5f6b(u8 *save_path)
{
	u8 *save_path_tail;
	u16 save_path_size;
	u8 temp2b[0x2B];
	
	while (*save_path == ' ')
		save_path++;
	
	save_path_size = strlen(save_path);
	if (save_path_size == 0)
	{
		lib_get_cwd(save_path);
		save_path_size = strlen(save_path);
	}

	save_path_tail = save_path + save_path_size - 1;
	
	if ((strchr("\\/", *save_path_tail) != 0) && (save_path_size != 1))
	{
		save_path_size--;
		*save_path_tail = 0;
	}
	
	if (save_path[1] == ':')
		save_drive = tolower(save_path[0]);
	else
		save_drive = lib_get_disk();

	if (save_path_size == 1)
		if (strchr("\\/", *save_path) != 0)
			return 1;

	if ( (save_path[1] == ':') && (save_path_size==2) )
		return drive_exists(tolower(*save_path));	// change drive
	else
		return (lib_find_first(save_path, 0x10, temp2b) != 0xFFFF);
}

*/
// save path exists
u16 state_dir_check(VSTRING *dir)
{
	u8 *path_tail;
	u32 path_size;
	
	vstring_shift(dir, strspn(dir->data, " "));
	
	if (strlen(dir->data) == 0)
		vstring_getcwd(dir);
	
	path_size = strlen( dir->data);
	path_tail = dir->data + path_size - 1;
	
	if ((strchr("\\/", *path_tail) != 0) && (path_size != 1))
	{
		//path_size--;
		*path_tail = 0;
	}
	
	//if (save_path[1] == ':')
	//	save_drive = tolower(save_path[0]);
	//else
	//	save_drive = lib_get_disk();

	//if (path_size == 1)
	//	if (strchr("\\/", *path) != 0)
	//		return 1;


	if ( (dir->data[1] == ':') && (strlen(dir->data)==2) )
	{
		vstring_set_size(dir, 4);
		dir->data[2] = '\\';
		dir->data[3] = 0;
	}
	
	//if ( (save_path[1] == ':') && (save_path_size==2) )
	//	return drive_exists(tolower(*save_path));	// change drive
	//else
	return (dir_exists(dir->data) != 0);
}