/*
_ScriptBlock                     cseg     00006F97 0000000F
_ScriptAllow                     cseg     00006FA6 0000000F
_ScriptNew                       cseg     00006FB5 00000035
_ScriptWrite                     cseg     00006FEA 0000007E
_ScriptFirst                     cseg     00007068 0000001D
_ScriptNext                      cseg     00007085 0000001E
CmdScriptSize                    cseg     000070A3 00000021
CmdUnknown171                    cseg     000070C4 00000012
CmdUnknown172                    cseg     000070D6 00000020
*/

#include <stdlib.h>

#include "../agi.h"
#include "../flags.h"

#include "../view/obj_update.h"
#include <setjmp.h>
#include "error.h"
#include "script.h"

#include "../sys/mem_wrap.h"




u8 *script_head = 0;		// dseg:16FD	*// top of script
u8 *script_next = 0;		// dseg:16ff
u8 *script_cur = 0;		// dseg:1701
u16 write_ok = 0;		// dseg:1703
u16 mem_script = 0;		// dseg:1705	*used by cmdshowmem


void script_block()
{
	write_ok = 0;
}

void script_allow()
{
	write_ok = 1;
}

// initialise script
u8 *script_new()
{
	if (  (state.script_size > 0) && (script_head == 0)  )
	{
		script_head = a_malloc(state.script_size << 1);
		#warning set_mem_rm0() not implemented
		//set_mem_rm0();
	}
	script_next = script_head;
	state.script_count = 0;
	return script_head;
}

void script_write(u16 var8, u16 vara)
{
	u8 *script_name = 0;
	
	switch (var8)
	{
		case 0:
			script_name = "load.logic";
			break;
		case 1:
			script_name = "load.view";
			break;
		case 2:
			script_name = "load.pic";
			break;
		case 3:
			script_name = "load.sound";
			break;
		case 4:
			script_name = "draw.pic";
			break;
		case 5:
			script_name = "add.to.pic";
			break;
		case 6:
			script_name = "discard.pic";
			break;
		case 7:
			script_name = "discard.view";
			break;
		case 8:
			script_name = "overlay.pic";
			break;
		default:
			script_name = "unknown";
			break;
	}
	
	//printf("script write %s - %d\n", script_name, vara);
	
	if (flag_test(0x07) == 0)
	{
		if (write_ok != 0)
		{
			if (script_next >= ((state.script_size << 1) + script_head)  )
				set_agi_error(0xB, mem_script);
			script_next[0] = (u8)var8;
			script_next[1] = (u8)vara;
			script_next += 2;
			state.script_count++;
		}
		if (  ((script_next - script_head) / 2)  > mem_script)
		{
			mem_script = (script_next - script_head) / 2;
			//printf("mem_script = %d\n", mem_script);
		}
	}
}


// init some search?
void script_first()
{
	script_cur = script_head;
	script_next = script_head + (state.script_count << 1);
}

// return a ptr?  so the returning function can change it instead??
// get next script item
u8 *script_get_next()
{
	u8 *ax;

	if (script_cur >= script_next)
		ax = 0;
	else
	{
		ax = script_cur;
		script_cur += 2;
	}
	return ax;
}


u8 *cmd_script_size(u8 *c)
{
	state.script_size = *(c++);
	
	blists_erase();
	script_new();
	blists_draw();
	
	return c;
}

u8 *cmd_unknown_171(u8 *c)
{
	state.script_saved = state.script_count;
	return c;
}

u8 *cmd_unknown_172(u8 *c)
{
	state.script_count = state.script_saved;	
	script_next = script_head + (state.script_count << 1);
	return c;
}


