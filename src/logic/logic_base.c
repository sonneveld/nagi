/*
_LogicListInit                   cseg     000010E8 0000000F
_LogicListFree                   cseg     000010F7 00000018
_LogicListFind                   cseg     0000110F 0000002E
CmdLoadLogics                    cseg     0000113D 0000001C
CmdLoadLogicsV                   cseg     00001159 00000024
_LogicLoad                       cseg     0000117D 0000001D
_LogicLoad2                      cseg     0000119A 000000C0

CmdCall                          cseg     0000125A 00000026
CmdCallV                         cseg     00001280 0000002E
_LogicCall                       cseg     000012AE 00000087

CmdSetScanStart                  cseg     00001335 00000015
CmdResetScanSta                  cseg     0000134A 0000001A
R_Logic1364                      cseg     00001364 00000041
R_Logic13A5                      cseg     000013A5 0000002F
*/

#include <stdlib.h>

#include "../agi.h"

#include "../logic/logic_base.h"
#include "../logic/logic_execute.h"

// script
#include "../sys/script.h"
// erase/blit both
#include "../view/obj_update.h"
// resources
#include "../res/res.h"
// decrypt
#include "../decrypt.h"
// get msg offset
#include "../ui/cmd_input.h"

#include "../sys/endian.h"

// logic_msg
#include "../ui/msg.h"

#include "../sys/mem_wrap.h"

#include <assert.h>

#include "../trace.h"

static LOGIC logic_head = {0,0,0,0,0,0,0};
LOGIC *logic_cur = 0;
static LOGIC *logic_last = 0;
u16 scan_start_list[60];

// initialising the list for the first time
void logic_list_init()
{
	LOGIC *cur, *next;
	
	cur = logic_head.next;
	
	while (cur != 0)
	{
		next = cur->next;
		if (cur->data != 0)
			free(cur->data);
		
		free(cur);
		cur = next;
	}
	
	logic_head.next = 0;
}

// frees up the list for a new instance of a room
void logic_list_new_room()
{
	LOGIC *cur, *next;
	
	// doesn't remove the first one because it's probably logic.0
	if (logic_head.next != 0)
	{
		cur = (logic_head.next)->next;
		
		while (cur != 0)
		{
			next = cur->next;
			if (cur->data != 0)
				free(cur->data);
			assert(cur->num != 0);
			free(cur);
			cur = next;
		}
		
		(logic_head.next)->next = 0;
	}
}

// returns a pointer to the logic node if it's found
// if not, returns 0
// logic_last is set so if it's not found, you can add it onto the last one
LOGIC *logic_list_find(u16 logic_num)
{
	LOGIC *cur, *prev;
	
	cur = logic_head.next;
	prev = &logic_head;
	
	while ( (cur != 0) && (cur->num != logic_num) )
	{
		prev = cur;
		cur = cur->next;
	}
	
	logic_last = prev;
	return cur;
}
	
u8 *cmd_load_logics(u8 *c)
{
	logic_load( *(c++) );
	return c;
}

u8 *cmd_load_logics_v(u8 *c)
{
	logic_load( state.var[*(c++)] );
	return c;
}

void logic_load(u16 logic_num)
{
	logic_load_2(logic_num);
	script_write(0, logic_num);
}

LOGIC *logic_load_2(u16 logic_num)
{
	LOGIC *log;	// logic
	u8 *log_data;
	u8 *msg;	// ptr to messages
	LOGIC *logic_cur_orig;	// orig logic_cur

	log = logic_list_find(logic_num);
	if ( log == 0)
	{
		blists_erase();
		
		log = (LOGIC *)a_malloc( sizeof(LOGIC) );
		
		logic_last->next = log;
		log->next = 0;
		log->num = logic_num;
		
		log_data = vol_res_load( dir_logic(logic_num), 0 );
		
		log->data = log_data;
		log->code = log_data + 2;	// skip header
		log->scan_start = log->code;
		msg = log->code + load_le_16(log_data);	// pointer to messages
		
		log->msg_total = msg[0];	// number of messages
		msg++;	// skip the number of messages
		log->msg = msg;

		if ((!c_game_compression) || not_compressed)
		{
			if ( log->msg_total != 0)
			{
				logic_cur_orig = logic_cur;
				logic_cur = log;
				decrypt_string(  log->msg + ((log->msg_total + 1)<<1),
							logic_msg(0)  );
				logic_cur = logic_cur_orig;
			}
		}

		blists_draw();
	}
	return log;
}

u8 *cmd_call(u8 *c)
{
	//printf("%d ", *c);
	
	if (logic_call(*(c++)) == 0)
		return 0;
	else
		return c;
}

u8 *cmd_call_v(u8 *c)
{
	//printf("%d ", state.var[*c]);
	if (logic_call(state.var[*(c++)]) == 0)
		return 0;
	else
		return c;
}

u8 *logic_call(u16 logic_num)
{
	LOGIC *logic_new = 0;	// the new struct opened.. so you can free it later
	u8 *code;			// logic code return
	LOGIC *last_orig = 0;		// original last node
	LOGIC *cur_orig;		// oringal current node
	u16 untouched;
	
	cur_orig = logic_cur;
	untouched = 1;
	
	logic_cur = logic_list_find(logic_num);
	
	//~ if logic_cur == 0, then we're running logic.0 (since logic_cur is initialised to
	//~ a null pointer)
	// ie.. we don't need to remember the previous logic
	if (logic_cur == NULL)
	{
		last_orig = logic_last;
		logic_cur = logic_load_2(logic_num);
		logic_new = logic_cur;
		untouched = 0;
	}

	if (trace_state == 2)
		trace_state = 1;
	if (logic_num == 0)
		logic_called = 1;
	
	code = logic_execute(logic_cur);

	// TODO: warning another AGI bug.  if restore/restart/newroom is called from a called logic, havoc may be wrecked when returning.
	// you don't want to try and free the previous logic code if it was just free'd by a returning restore command.
	if ( (untouched == 0) && (code != NULL) )
	{
		assert(logic_new != NULL);
		assert(last_orig != NULL);
		
		last_orig->next = 0;
		blists_erase();
		//set_mem_ptr(logic_new);
		if (logic_new->data != 0)
			free(logic_new->data);	// hope this works
		free(logic_new);
		blists_draw();
	}
	
	logic_cur = cur_orig;
	return code;
}

u8 *cmd_set_scan_start(u8 *c)
{
	logic_cur->scan_start = c;
	return c;
}

u8 *cmd_reset_scan_start(u8 *c)
{
	logic_cur->scan_start = logic_cur->code;
	return c;
}

// store a list of scan starts
// returns number of bytes used
u16 logic_save_scan_start()
{
	u16 *scan;
	LOGIC *log;
	
	log = &logic_head;
	scan = scan_start_list;
	
	while (log != 0)
	{
		scan[0] = log->num;
		scan[1] =  log->scan_start - log->code;
		log = log->next;
		scan += 2;
	}
	
	scan[0] = 0xFFFF;
	return ((u8*)scan - (u8*)scan_start_list) + 4;
}

void logic_restore_scan_start(LOGIC *log)
{
	u16 *scan;
	scan = scan_start_list;

	while (  (scan[0] != 0xFFFF) && (scan[0] != log->num)  )
		scan += 2;
	if (scan[0] == log->num)
		log->scan_start = log->code + scan[1];
}
