/*
_LogicCmd                        cseg     000002C4 00000043
...
_LogicEval                       cseg     000007E3 00000040
...
_LogicExecute                    cseg     0000293C 000000BA
*/

// NOTES!!!!
// data += *(data++) is BAD
// data += *(data++) + *(data++) IS WORSE YOU IDIOT!!!
// ahem

//#define LOG_DEBUG 1


#include <stdio.h>
#include "../agi.h"

#include "../logic/logic_base.h"
#include "../logic/logic_execute.h"
#include "../logic/cmd_table.h"

// agi_error
#include <setjmp.h>
#include "../sys/error.h"

// debug
#include "../debug.h"

// byte-order support
#include "../sys/endian.h"
#include "../sys/mem_wrap.h"

#include "../trace.h"

u8 *logic_data;	// si;
u8 op;	// al;




u8 *logic_execute(LOGIC *log)
{
	logic_data = log->scan_start;
	
	#ifdef LOG_DEBUG
	//printf("logic code (40 bytes):  ");
	//print_hex_array(logic_data, 40);
	//printf("\n");
	printf("executing logic %d... \n", log->num);
	
	#endif
	
	op = *(logic_data++);
	while (op != 0)
	{	
		#ifdef LOG_DEBUG
		//printf("\n (0x%X) ", logic_data);
		#endif
		if (op == 0xFF)			// if
		{
			#ifdef LOG_DEBUG
			printf("if: ");
			#endif
			execute_if();
			op = *(logic_data++);
		}
		else if (op == 0xFE)		// else goto
		{
			#ifdef LOG_DEBUG
			printf("goto: 0x%X\n", load_le_16(logic_data));
			#endif
			logic_data += (s16)load_le_16(logic_data) + 2;
			op = *(logic_data++);
		}
		else					// cmd
		{
			#ifdef LOG_DEBUG
			//printf("cmd: ");
			#endif
			logic_cmd();
			// passes si (logic_data) and ax (op);
			if ( logic_data == 0)
				break;
		}
		//printf("op=%d logic_data=%x\n", op, logic_data);
	}
	#ifdef LOG_DEBUG
	printf("return! \n");
	#endif
	return logic_data;
}

void execute_if()
{
	u8 not_mode = 0;
	u8 or_mode = 0;	// bh:  or mode or something
	u8 result;
	
	for(;;)
	{
		op = *(logic_data++);
		if ( (op >= 0xFC) && (op != 0xFE) )
		{
			if ( op == 0xFC)
			{
				// logic here is if you reach the end of an "or"
				// bracket without getting a true.. then the
				// bracket == false
				if (or_mode != 0)
				{	
					or_mode = 0;
					skip_false_and();
					#ifdef LOG_DEBUG
					printf("FALSE!\n");
					#endif
					break;		// !!! return
				}
				or_mode++;
			}
			else if ( op == 0xFF)		// end of "if" conditions (TRUE)
			{
				logic_data += 2;			// skip the word offset
				#ifdef LOG_DEBUG
				printf("TRUE!\n");
				#endif
				break;			// !!! return
			}
			else if ( op == 0xFD)		// not
				not_mode = not_mode ^ 1;
		}
		else
		{
			// this is how the original code works
			// but doesn't look for 0xFE.. so it will cause an agi_error()
			// here....
			result = logic_eval();	// DOES NOT TOUCH BX!!!
							// returns the result in ax.. same as op.
							// passes si (logic_data) and op (result);
			result = result ^ not_mode;
			not_mode = 0;
			
			#ifdef LOG_DEBUG
			//printf("result = %d   or_mode = %d   not_mode = %d", result, or_mode, not_mode);
			#endif
			if ( result != 0)
			{
				if (or_mode != 0) 
				{
					or_mode = 0;
					skip_true_or();
				}
			}
			else if ( or_mode == 0)
			{	
				or_mode = 0;
				skip_false_and();
				#ifdef LOG_DEBUG
				printf("FALSE!\n");
				#endif
				break;			// !!! return
			}
		}
	}
}

void skip_true_or()
{
	op = *(logic_data++);
	while (op != 0xFC)	// while not end of "if" bracket
	{
		if ( op <= 0xFC )
		{
			if ( op == 0xE)	// said command
			{
				#warning may be illegal (in 7 states or something)
				logic_data += (*logic_data << 1) + 1;
			}
			else
			{
				logic_data += eval_table[op].param_total;
			}
		}
		op = *(logic_data++);
	}
}

void skip_false_and()
{
	op = *(logic_data++);
	while (op != 0xFF)	// while not end of "or" bracket
	{
		if (op < 0xFC) 
		{
			if (op == 0xE)
			{
				logic_data += (*(logic_data) << 1) + 1;	// skip said command
			}
			else
			{
				logic_data += eval_table[op].param_total;
			}
		}
		op = *(logic_data++);
	}

	logic_data += load_le_16(logic_data) + 2;	// address
}

#warning FIX LOGIC_CMD and ALL THAT!!!



void logic_cmd()
{
	while (  (op < 0xFC) && (op != 0)  )
	{
		if ( op > CMD_MAX)
			set_agi_error(0x10, op);

		if ( trace_state == 1)
			trace_cmd(op, logic_data);	// does not touch AX

		#ifdef LOG_DEBUG
		printf("%d:0x%X %s (", logic_cur->num, logic_data - logic_cur->data, cmd_table[op].func_name);
		print_hex_array(logic_data, cmd_table[op].param_total);
		printf(")\n");
		#endif
		if (cmd_table[op].func != cmd_do_nothing)	// ADDED
			logic_data = ((CMD_TYPE)cmd_table[op].func)(logic_data);
		//cmd_table[op].func.cmd(logic_data);
		//
		else	// ADDED
		{	
			printf("no cmd=\"%s\"\n", cmd_table[op].func_name);
			logic_data += cmd_table[op].param_total;	// ADDED
		}
		
		if (logic_data == 0) 
			break;
		op = *(logic_data++);
	}
}

u8 *data_orig;

// returns the result.. but changes the global value logic_data;
// op is changed to whatever.. dun worry about it mate
u8 logic_eval()
{
	u8 result;
	
	data_orig = logic_data - 1;
	if ( op >= 0x26)
		set_agi_error(0xF, op);
	
	#ifdef LOG_DEBUG
	printf("%d:0x%X %s (", logic_cur->num, logic_data - logic_cur->data, eval_table[op].func_name);
	print_hex_array(logic_data, eval_table[op].param_total);
	printf(")   ");
	#endif

	if (eval_table[op].func != cmd_ret_false)
		//result = cmd_table[op].func.eval();
		result = ((EVAL_TYPE)eval_table[op].func)();	// return in si, ax
	else
	{
		printf("no eval=\"%s\"    ", eval_table[op].func_name);
		logic_data = logic_data + eval_table[op].param_total;
		result = 0;
	}
	
	if ( trace_state == 1)
	{
		trace_eval(result, data_orig);	// don't touch ax
	}
	return result;
}

