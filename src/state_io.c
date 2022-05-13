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

#include "sys/sys_dir.h"


#include "flags.h"

#include "sound/sound_base.h"
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

#include "sys/endian.h"

static u16 state_read(FILE *data_stream, void *data_alloc, size_t size_multiple, size_t size_max);
static u16 state_write(FILE *stream, void *write_data, u16 write_size);

char state_name_auto[0x32] = {0};

u8 *cmd_restart_game(u8 *c)
{
	u16 snd_state;
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
		snd_state = flag_test(F09_SOUND);
		//clear_memory();	// shouldn't be necessary
		game_init();
		volumes_close();
		flag_set(F06_RESTART);
		if ( snd_state != 0)
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
	char newline_orig;	// d0d orig
	FILE *rest_stream;
	//u8 msg[200];
	char *msg;
	
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
		dir_preset_change(DIR_PRESET_GAME);
		rest_stream = fopen(save_filename->data, "rb");
		if ( rest_stream == 0)
		{
			sprintf(msg, "Can't open file:\n %s", save_filename->data);
			message_box(msg);
		}
		else
		{
			fseek(rest_stream, 0x1F, SEEK_SET);
			if (state_read(rest_stream, &state, sizeof(AGI_STATE), sizeof(AGI_STATE)) == 0)
				goto loc2630;
			if (state_read(rest_stream, objtable, sizeof(VIEW), (objtable_tail - objtable) * sizeof(VIEW)) == 0)
				goto loc2630;
			if (state_read(rest_stream, inv_obj_table, sizeof(INV_OBJ), inv_obj_table_size * sizeof(INV_OBJ)) == 0)
				goto loc2630;
			if (state_read(rest_stream, inv_obj_string, 1, inv_obj_string_size) == 0)
				goto loc2630;
			if (state_read(rest_stream, script_head, 1, (state.script_size << 1)) == 0)
				goto loc2630;
			if (state_read(rest_stream, scan_start_list, sizeof(u16), sizeof(u16)*60) == 0)
				goto loc2630;

			goto loc2647;
		loc2630:
			fclose(rest_stream);
			message_box("Error in restoring game.\nPress ENTER to quit.");
			agi_exit();  // can't recover.. we possibly just overwrote some of the
					// structures with stuff
		loc2647:
			fclose(rest_stream);
			decrypt_string((u8*)inv_obj_string, (u8*)(inv_obj_string+inv_obj_string_size));
			state.var[V20_COMPUTER] = computer_type;
			state.var[V26_MONITORTYPE] = display_type;
			state.var[V08_FREEMEM] = 10;
			
			if (computer_type == 0)
				state.var[V22_SNDTYPE] = 1;
			else
			{
				state.var[V22_SNDTYPE] = 3;
				flag_set(F11_HAS_NOISE_CHANNEL);
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

// Read in object array from save game file
//
// `size_multiple` and `size_max` are used to ensure some integrity.
// (because this format is based on the original dos format, it is possible that nagi
// could inadvertently try to read it and crash because of struct differences)
// Kudos to @Ritchie333 for suggesting and implementing the first iteration of this check.
//
// Format is:
//   0..1 - little endian word - data size
//   2..  - bytes[data_size] - data
static u16 state_read(FILE *data_stream, void *data_alloc, size_t size_multiple, size_t size_max)
{
	uint8_t size_bytes[2];
	if (fread(&size_bytes, sizeof(size_bytes), 1, data_stream) != 1) { return 0; }
	size_t data_size = load_le_16(size_bytes);
	if ((data_size % size_multiple) != 0) { return 0; }
	if ((data_size > size_max) != 0) { return 0; }
	if (fread(data_alloc, data_size, 1, data_stream) != 1) { return 0; }
	return 1;
}

u8 *cmd_unknown_170(u8 *c)
{
	strncpy(state_name_auto, state.string[*(c++)], 31);
	return c;
}

u8 *cmd_save_game(u8 *c)
{
	char newline_orig;
	FILE *save_stream;	// file handle
	char *msg;
	
	clock_state = 1;
	newline_orig = msgstate.newline_char;
	msgstate.newline_char = '@';
		
	decrypt_string((u8*)inv_obj_string, (u8*)(inv_obj_string+inv_obj_string_size));
	
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
		dir_preset_change(DIR_PRESET_GAME);
		save_stream = fopen(save_filename->data, "wb");
		if ( save_stream == 0)
		{
			sprintf(msg, "The directory\n   %s\n is full or the disk is write-protected.\nPress ENTER to continue."
				, save_dir->data);
			message_box(msg);
		}
		else
		{
			if (fwrite(save_description, sizeof(char), 0x1f, save_stream) != 0x1f)
				goto save_err;
			if (state_write(save_stream, &state, sizeof(AGI_STATE)) == 0)
				goto save_err;
			if (state_write(save_stream, objtable, objtable_size) == 0)
				goto save_err;
			if (state_write(save_stream, inv_obj_table, inv_obj_table_size*sizeof(INV_OBJ)) == 0)
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
	decrypt_string((u8*)inv_obj_string, (u8*)(inv_obj_string+inv_obj_string_size));
	return c;
}


// writes the size in little endian format.. then the data.
// return 1 on success, 0 on error
//
// Format is:
//   0..1 - little endian word - data size
//   2..  - bytes[data_size] - data
static u16 state_write(FILE *stream, void *write_data, u16 write_size)
{
	u8 header[2];
	store_le_16(header, write_size);
	if (fwrite(&header, sizeof(header), 1, stream) != 1) { return 0; }

	if (fwrite(write_data, write_size, 1, stream) != 1) { return 0; }

	return 1;
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
	// TODO: clear_memory() not implemented
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
		if ((temp4 & O_ANIMATE) != 0)		// 6
		{
			if ((temp4 & O_DRAWN) != 0) 	// 0
			{
				obj_draw(v->num);
				if ( v->motion == MT_FOLLOW)
					v->follow.count = 0xFF;
					//v->unknown29 = 0xFF;
			}
			if ( (temp4 & (O_DRAWN & O_ANIMATE))
					== O_DRAWN)	// 4 0
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
