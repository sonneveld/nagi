/*
CmdRestartGame                   cseg     00002472 0000009F
CmdRestoreGame                   cseg     00002512 0000019E
_StateRead                       cseg     000026B0 00000075
CmdUnknown170                    cseg     00002726 0000002D
CmdSaveGame                      cseg     00002753 00000173
_StateWrite                      cseg     000028C6 00000074
*/

#include <stdio.h>
#include <string.h>

#include "agi.h"
#include "sys/vstring.h"
#include "state_io.h"
#include "state_info.h"

#include "decrypt.h"

#include "ui/events.h"
#include "ui/controller.h"
#include "sys/drv_video.h"
#include "sys/gfx.h"
#include "objects.h"
#include "sys/glob_sys.h"


#include "flags.h"

#include "sound/sound.h"
#include "ui/msg.h"
#include "ui/cmd_input.h"
#include "initialise.h"
#include "ui/menu.h"
#include "res/res.h"

#include "sys/time.h"

#include "ui/status.h"
#include "sys/script.h"
#include "picture/pic_res.h"
#include "picture/pic_add.h"
#include "view/view_base.h"
#include "view/obj_base.h"
#include "view/obj_update.h"
#include "view/obj_drawerase.h"
#include "logic/logic_base.h"

#include "base.h"

#include "trace.h"





u8 state_name_auto[0x32] = "";

u8 *cmd_restart_game(u8 *c)
{
	u16 sound_state;
	u16 user_result;
	u16 input_state;
	
	sound_stop();
	input_state = input_edit_status();
	input_edit_on();
	
	if (flag_test(F16_RESTARTMODE) != 0)
		user_result = 1;
	else
		user_result = message_box("Press ENTER to restart\nthe game.\n\nPress ESC to continue\nthis game.");

	if ( user_result != 0)
	{
		cmd_cancel_line(0);
		sound_state = flag_test(F09_SOUND);
		//clear_memory();	// shouldn't be necessary
		game_init();
		volumes_close();
		flag_set(F06_RESTART);
		if ( sound_state != 0)
			flag_set(F09_SOUND);
		state.ticks = 0;
		if (trace_logic != 0)
			logic_load_2(trace_logic);
		menu_enable_all();
	}
	
	
	if ( (user_result != 0) || (input_state != 0) )
		input_edit_off();
	
	if (user_result != 0)
		return 0;
	else
		return c;
}


u8 *cmd_restore_game(u8 *c)
{
	u8 *code_ret;	// result to pass at the end
	u8 newline_orig;	// d0d orig
	FILE *rest_stream;
	//u8 msg[200];
	u8 *msg;
	
	clock_state = 1;
	code_ret = c;
	newline_orig = msgstate.newline_char;
	msgstate.newline_char = '@';
	
	if (save_dir == 0)
		save_dir = vstring_new(0, 200);
	if (save_filename == 0)
		save_filename = vstring_new(0, 250);
	
	if (state_get_info('r') != 0)
	{
		if (strlen(save_filename->data) > strlen(save_dir->data))
			msg = alloca(200 + strlen(save_filename->data));
		else
			msg = alloca(200 + strlen(save_dir->data));
		if (state_name_auto[0] == 0)
		{
			sprintf(msg, "About to restore the game\ndescribed as:\n\n%s\n\nfrom file:\n %s\n\n%s",
				save_description, save_filename->data, "Press ENTER to continue.\nPress ESC to cancel.");
			message_box_draw(msg, 0, 0x23, 0);
			if ( user_bolean_poll() == 0)
				goto rest_end;
		}
		rest_stream = fopen(save_filename->data, "rb");
		if ( rest_stream == 0)
		{
			sprintf(msg, "Can't open file:\n%s", save_filename->data);
			message_box(msg);
		}
		else
		{
			fseek(rest_stream, 0x1F, SEEK_SET);
			if (state_read(rest_stream, &state) == 0)
				goto loc2630;
			if (state_read(rest_stream, objtable) == 0)
				goto loc2630;
			if (state_read(rest_stream, inv_obj_table) == 0)
				goto loc2630;
			if (state_read(rest_stream, inv_obj_string) == 0)
				goto loc2630;
			if (state_read(rest_stream, script_head) == 0)
				goto loc2630;
			if (state_read(rest_stream, scan_start_list) != 0)
				goto loc2647;
		loc2630:
			fclose(rest_stream);
			message_box("Error in restoring game.\nPress ENTER to quit.");
			agi_exit();  // can't recover.. we possibly just overwrote some of the
					// structures with stuff
		loc2647:
			fclose(rest_stream);
			decrypt_string(inv_obj_string, inv_obj_string+inv_obj_string_size);
			state.var[V20_COMPUTER] = computer_type;
			state.var[V26_MONITORTYPE] = display_type;
			if (computer_type == 0)
				state.var[V22_SNDTYPE] = 1;
			else
			{
				state.var[V22_SNDTYPE] = 3;
				flag_set(F11_NEWLOGIC0);
			}
			state_reload();
			control_state_clear();
			flag_set(F12_RESTORE);
			volumes_close();
			code_ret = 0;
			menu_enable_all();
		}
	}
rest_end:
	cmd_close_window(0);
	msgstate.newline_char = newline_orig;
	clock_state = 0;
	return code_ret;
}

u16 state_read(FILE *data_stream, void *data_alloc)
{
	u8 buff;
	u16 data_size;
	
	if ( fread(&buff, sizeof(u8), 1, data_stream) == 1)
	{
		data_size = buff;
		if (fread(&buff, sizeof(u8), 1, data_stream) == 1)
		{
			data_size += buff << 8;
			if ( fread(data_alloc, sizeof(u8), data_size, data_stream) == data_size)
				return 1;
		}
	}
	return 0;
}


u8 *cmd_unknown_170(u8 *c)
{
	strncpy(state_name_auto, state.string[*(c++)], 31);
	return c;
}

u8 *cmd_save_game(u8 *c)
{
	u8 newline_orig;
	FILE *save_stream;	// file handle
	u8 *msg;
	
	clock_state = 1;
	newline_orig = msgstate.newline_char;
	msgstate.newline_char = '@';
		
	decrypt_string(inv_obj_string, inv_obj_string+inv_obj_string_size);
	
	if (save_dir == 0)
		save_dir = vstring_new(0, 200);
	if (save_filename == 0)
		save_filename = vstring_new(0, 250);
	
	if ( state_get_info('s') != 0)// select the game
	{
		if (strlen(save_filename->data) > strlen(save_dir->data))
			msg = alloca(200 + strlen(save_filename->data));
		else
			msg = alloca(200 + strlen(save_dir->data));
		if (state_name_auto[0] == 0)
		{
			sprintf(msg, "About to save the game\ndescribed as:\n\n%s\n\nin file:\n %s\n\n%s",
					save_description, save_filename->data,
					"Press ENTER to continue.\nPress ESC to cancel.");
			message_box_draw(msg, 0, 0x23, 0);
			
			if ( user_bolean_poll() == 0)
				goto save_end;
		}
		save_stream = fopen(save_filename->data, "wb");
		if ( save_stream == 0)
		{
			sprintf(msg, "The directory\n   %s\n is full or the disk is write-protected.\nPress ENTER to continue."
				, save_dir->data);
			message_box(msg);
		}
		else
		{
			if (fwrite(save_description, sizeof(u8), 0x1f, save_stream) != 0x1f)
				goto save_err;
			if (state_write(save_stream, &state, sizeof(AGI_STATE)) == 0)
				goto save_err;
			if (state_write(save_stream, objtable, objtable_size) == 0)
				goto save_err;
			if (state_write(save_stream, inv_obj_table, inv_obj_table_size) == 0)
				goto save_err;
			if (state_write(save_stream, inv_obj_string, inv_obj_string_size) == 0)
				goto save_err;
			if (state_write(save_stream, script_head, state.script_size<<1) == 0)
				goto save_err;
			if (state_write(save_stream, (void *)scan_start_list, logic_save_scan_start()) != 0)
				goto save_close;
		save_err:
			fclose(save_stream);
			remove(save_filename->data);
			message_box("The disk is full.\nPress ENTER to continue.");
			goto save_end;
		save_close:
			fclose(save_stream);
		}
	}
save_end:
	cmd_close_window(0);
	msgstate.newline_char = newline_orig;
	clock_state = 0;
	decrypt_string(inv_obj_string, inv_obj_string+inv_obj_string_size);
	return c;
}


u16 state_write(FILE *stream, void *write_data, u16 write_size)
{
	u8 temp;
	
	// writes the size in little endian format.. then the data.
	
	temp = write_size & 0xFF;
	if ( fwrite(&temp, sizeof(u8), 1, stream) == 1 )
	{
		temp = write_size >> 8;
		if ( fwrite(&temp, sizeof(u8), 1, stream)  == 1 )
			if ( fwrite(write_data, sizeof(u8), write_size, stream) == write_size )
				return 1;
	}
	return 0;
}

// used to reinitialise all the restored data so the game will run properly (ie, fix pointers 'n stuff man)
void state_reload(void)
{
	u16 temp4;
	VIEW *v;
	u8 *si;
	u16 di;
	
	sound_stop();
	room_init();
	script_block();
	
	for (v=objtable ; v<objtable_tail ; v++)
	{
		v->num = (u8)v->x;
		v->x =  v->flags;	// store the flags for later use????
		if (( v->flags & O_ANIMATE) != 0)	// bit six
			v->flags = (v->flags & ~O_DRAWN) | O_UPDATE;
		// turn off bit 0.. turn on 4
	}

	blists_erase();
	#warning clear_memory() not implemented
	//clear_memory();
	pic_visible = 0;
	script_first();

	while ((si=script_get_next()) != 0)
	{
		switch(si[0])
		{
			case 0:
				logic_restore_scan_start(logic_load_2(si[1]));
				break;
			case 1:
				view_load(si[1], 1);
				break;
			case 2:
				pic_load(si[1]);
				break;
			case 3:
				sound_load(si[1]);
				break;
			case 4:
				pic_draw(si[1]);
				break;
			case 5:
				si = script_get_next();
				add_num = si[0];
				add_loop = si[1];
				si = script_get_next();
				add_cel = si[0];
				add_x = si[1];
				si = script_get_next();
				add_y = si[0];
				add_pri = si[1];
				add_to_pic();
				break;
			case 6:
				pic_discard(si[1]);
				break;
			case 7:
				view_discard(si[1]);
				break;
			case 8:
				pic_overlay(si[1]);
				break;
		}
	}

	script_allow();
	
	di=0;
	for (v=objtable ; v<objtable_tail ; v++)
	{
		temp4 = v->x;	// the old flags stored in x
		v->x = v->num;
		v->num = di++;	// ahh.. back to normality
		if ( view_find( v->view_cur) != 0)
			obj_view_set(v, v->view_cur);
		if ((temp4 & 0x40) != 0)		// 6
		{
			if ((temp4 & 0x01) != 0) 	// 0
			{
				obj_draw(v->num);
				if ( v->motion == MT_FOLLOW)
					v->follow.count = 0xFF;
					//v->unknown29 = 0xFF;
			}
			if ( (temp4 & 0x11) == 1)	// 4 0
				obj_stop_update(v);
			v->flags = temp4;
		}
	}

	input_edit_on();
	cmd_cancel_line(0);
	gfx_picbuff_update();
	pic_visible = 1;
	status_line_write();
	input_redraw();
}