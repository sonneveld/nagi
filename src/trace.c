/* FUNCTION list 	---	---	---	---	---	---	---
CmdTraceOn                       cseg     00008BCA 0000001D
_TraceInit                       cseg     00008BE7 0000008F
CmdTraceInfo                     cseg     00008C76 0000003C
_TraceClear                      cseg     00008CB2 0000002A
_TraceCmd                        cseg     00008CDC 00000026
_TraceEval                       cseg     00008D02 00000042
_TraceAdd                        cseg     00008D44 0000011E
_TraceFuncPrint                  cseg     00008E62 0000010E
R_Debug8F70                      cseg     00008F70 0000003D
_TraceScroll                     cseg     00008FAD 00000023
*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "agi.h" 

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */

#include "logic/cmd_table.h"
#include "logic/logic_base.h"
#include "flags.h"
#include "sys/video_misc.h"
#include "ui/events.h"
#include "sys/endian.h"
#include "sys/video.h"

#include "ui/printf.h"
#include "ui/window.h"
#include "ui/agi_text.h"
#include "ui/msg.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
u8 *cmd_trace_on(u8 *c);
void trace_init(void);
u8 *cmd_trace_info(u8 *c);
void trace_clear(void);
void trace_cmd(u16 op, u8 *log_data);
void trace_eval(u16 result, u8 *log_data);
void trace_add(u16 op, FUNC *table, u8 *log_data, u16 table_offset, u16 result);
void trace_var_print(FUNC *table, u8 *log_data);
u16 trace_var_read(u8 *log_data, u16 var_cur);
void trace_scroll(void);



/* VARIABLES	---	---	---	---	---	---	--- */

u16 trace_top_given = 1;
u16 trace_height = 0x0F;
// this is ONLY accessed by trace.clear...  used by cga/hgc???
u16 trace_unknown = 0xFFFF;
// determines if said eval cmd was called
// 1=said 0=other
u16 said_state = 0;
// trace state, 0=uninit 1=init'd... 2=??
u16 trace_state = 0;
// number of logic that contains cmd text
u16 trace_logic = 0;

// text positions of the trace window
// trace_top
u16 trace_top = 0;
// trace_left
u16 trace_left = 0;
// trace_bottom
u16 trace_bottom = 0;
// trace_right
u16 trace_right = 0;

// pic buff positions of the trace window
// pos of trace bg window
//u16 word_1d12 = 0;
u8 trace_win_x = 0;
u8 trace_win_y = 0;
// dimensions pos of trace bg window
//u16 word_1d14 = 0;
u8 trace_win_w = 0;
u8 trace_win_h = 0;

// set to 1 whenever a logic.call is umm.. called
// ONLY LOGIC 0!!
// prints a nice "====" banner
u16 logic_called = 0;

/* CODE	---	---	---	---	---	---	---	--- */

// presing '+' will skip to the next logic_call man

u8 *cmd_trace_on(u8 *c)
{
	if (trace_state != 0)
		return c + 1;
	else
	{
		trace_init();
		return c;
	}
}

// trace initialise
void trace_init(void)
{
	if (  (trace_state==0) && (flag_test(F10_DEBUG)!=0)  )
	{
		trace_state = 1;
		trace_top = state.window_row_min + trace_top_given + 1;
		trace_bottom =  trace_height + trace_top - 1;
		trace_left = 2;
		trace_right = trace_left + 0x23;
		
		trace_win_x = trace_left * 4 - 5;
		trace_win_y = trace_bottom * 8 + 5;
		trace_win_h = trace_height * 8 + 0xA;
		trace_win_w = 0x9A;
		box_n_border(trace_win_x,trace_win_y,trace_win_w,trace_win_h,0x0F,0x04);
	}
}


u8 *cmd_trace_info(u8 *c)
{
	trace_logic = *(c++);
	trace_top_given = *(c++);
	trace_height = *(c++);
	if (trace_height < 2)
		trace_height = 2;
	return c;
}

void trace_clear(void)
{
	if (trace_state != 0)
	{
		trace_state = 0;
		trace_unknown = 0xFFFF;
		render_update(trace_win_x,trace_win_y,trace_win_w,trace_win_h);
	}
}

void trace_cmd(u16 op, u8 *log_data)
{
	said_state = 0;
	trace_add(op, cmd_table, log_data, 0, 0xFFFF);
}

void trace_eval(u16 result, u8 *log_data)
{
	u16 temp2;
	
	temp2 = *(log_data++);
	if (temp2 == 0xE)
		said_state = 1;
	else
		said_state = 0;
	trace_add(temp2, eval_table, log_data, 0xDC, result);
}

// trace_add?
// var8, vara, varc, vare, var10
void trace_add(u16 op, FUNC *table, u8 *log_data, u16 table_offset, u16 result)
{
	AGI_EVENT *temp4 = 0;
	LOGIC *logic_orig = 0;	// orig logic_cur
	u8 *msg;
	
	//table += op<<2;
	table += op;
	push_row_col();
	text_attrib_push();
	text_colour(0, 0xF);
	trace_scroll();
	
	if (logic_called != 0)
	{
		logic_called = 0;
		agi_printf("==========================");
		trace_scroll();
	}
	
	logic_orig = logic_cur;
	if ((trace_logic==0) || ((logic_cur=logic_list_find(trace_logic)) == 0))
	{
		agi_printf("%d: %d",  logic_cur->num, op);
	}
	else
	{
		if (op == 0)
			agi_printf("%d: %s",  logic_orig->num, "return");
		else
		{
			msg = logic_msg(op + table_offset);
			if (msg != 0)
				agi_printf("%d: %s",  logic_orig->num, logic_msg(op + table_offset));
			else
			{
				if (result != 0xFFFF)
					agi_printf("%d: eval.%d", logic_orig->num, op);
				else
					agi_printf("%d: cmd.%d", logic_orig->num, op);
			}
		}
	}
	logic_cur = logic_orig;
	
	// print function name?
	trace_var_print(table, log_data);
	
	if (result != 0xFFFF)
	{
		goto_row_col(trace_bottom, trace_right-2);
		if (result == 0)
			agi_printf(" :%c", 'F');
		else
			agi_printf(" :%c", 'T');
	}

	while (trace_state != 0)
	{
		temp4 = event_read();
		if (temp4 != 0)
			if (temp4->type == 1)
				break;
	}

	if (temp4 != 0) 
		if (temp4->data == '+')
			trace_state = 2;
		
	pop_row_col();
	text_attrib_pop();
}
// trace var print more like I think
void trace_var_print(FUNC *table, u8 *log_data)
{
	u16 var_value;
	u16 var_cur;
	u16 var_total;
	u16 bitmask;
	u16 var_flags;
	
	push_row_col();
	if (said_state != 0)
		var_total = *(log_data++);
	else
		var_total = table->param_total;	// number of vars

	var_flags = table->param_flag;	// type of stuff
	window_put_char('(');
	var_cur = 0;
	
	while (var_cur < var_total)
	{
		// get a number
		var_value = trace_var_read(log_data, var_cur);
		if (said_state != 1)
			agi_printf("%d", var_value);
		else
			agi_printf("%u", var_value);
		var_cur++;
	
		if (var_cur < var_total)
			window_put_char(',');
	}
	window_put_char(')');
	
	if (var_flags != 0)
		trace_scroll();
	pop_row_col();
	
	if (var_flags != 0)
	{
		bitmask = 0x80;
		window_put_char('(');
		for (var_cur=0; var_cur<var_total; bitmask>>=1)
		{
			var_value = trace_var_read(log_data, var_cur);
			if ((var_flags & bitmask) == 0)
				agi_printf("%d",var_value);
			else
				agi_printf("%d", state.var[var_value]);
			var_cur++;
			if (var_cur < var_total)
				window_put_char(',');
		}
		window_put_char(')');
	}
}


u16 trace_var_read(u8 *log_data, u16 var_cur)
{
	if ( said_state != 0)
		return load_le_16(log_data + (var_cur<<1));
	else
		return *(log_data+var_cur);
}

void trace_scroll(void)
{
	window_scroll(trace_top, trace_left, trace_bottom, trace_right, 0xFF);
}