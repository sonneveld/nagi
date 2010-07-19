// memory allocations
// _Malloc                        cseg     000013D6 0000005A
// _GetMemPtr                     cseg     00001430 0000000C
// _SetMemPtr                     cseg     0000143C 0000000F
// _SaveMemPtr                    cseg     0000144B 0000000F
// _RestoreMemPtr                 cseg     0000145A 0000001C
// _SetMemRm0                     cseg     00001476 0000000F
// _ClearMemory                   cseg     00001485 0000001B
// _UpdateVar8                    cseg     000014A0 0000001D
// CmdShowMem                     cseg     000014BD 0000004D
// END memory allocations

#define HEAP_SIZE 0x10000

#include "../agi.h"

// for blists_free()
#include "../view/obj_update.h"


// for mem_script
#include "script.h"

#include <stdlib.h>
#include <stdio.h>

#include <setjmp.h>
#include "../sys/error.h"

void *mem_base;
void *mem_end;
void *mem_ptr;
void *mem_rm0;
void *mem_ptr2;
void *mem_max;

void init_agi_heap(void)
{
#warning text mode support needed
	//text_mode = 1;
	//text_colour(0xFF, 0);

	// allocate memory for heap
	// = 64k - AGIDATA.OVL = 56k

	if ( (mem_ptr = malloc(HEAP_SIZE)) == 0)
		goto out_mem_err;
	mem_base = mem_ptr;
	mem_end = (u8 *)mem_ptr + HEAP_SIZE;
	
	// allocate room for screen buffer
#warning move this screen buffer thingy to somewhere else
	// other = 0x6900    hgc = 0xD200
	
	// sbuff = malloc(0x6900);
	//text_mode = 0;
	
out_mem_err:
	printf("Sorry, your computer does\nnot have enough memory to\n");
	printf("play this game.\n256K of RAM is required.");
	beep_speaker();
	exit(1);
}

u16 update_var8(void)	// return via ax
{
	// store in multiples of 0x100 or 256
	// free_mem
	state.var[8] = ((u8 *)mem_end-(u8 *)mem_ptr) / 0x100;
	return(state.var[8]);
}

void *agi_malloc(u16 size)
{
	void *return_ptr;
	
	if (size > ((u8 *)mem_end-(u8 *)mem_ptr) )
	{
	/*	_sprintf(&temp, "No memory. \nWant %d, have %d", num_bytes, mem_heap-mem_ptr);
		_Print(temp);
		agi_exit();	*/
		exit(1);
#warning need to fix error statement
	}
	
	return_ptr = mem_ptr;
	mem_ptr = (u8 *)mem_ptr + size;
	update_var8();
	if (mem_ptr > mem_max)
		mem_max = mem_ptr;
	return return_ptr;
}

void *get_memptr(void)
{
	return(mem_ptr);
}

void set_memptr(void *ptr)
{
	mem_ptr = ptr;
}

// save and restore mem ptr are not used

void save_memptr(void)
{
	mem_ptr2 = mem_ptr;
}

void restore_memptr(void)
{
	if (mem_ptr2 != 0)
	{
		mem_ptr = mem_ptr2;
		mem_ptr2 = 0;
	}
}

void set_memrm0(void)
{
	mem_rm0 = mem_ptr;
}

void clear_memory(void)
{
	blists_free();
	mem_ptr2 = 0;
	mem_ptr = mem_rm0;
	update_var8();
}

#warning finish cmd_show_mem
/* word CmdShowMem(var param)
{
	word string[0x64];

	sprintf(&string, 
		"heapsize: %u\nnow: %u  max: %u\nrm.0, etc.: %u\nmax script: %d",
		mem_heap-mem_base,
		mem_ptr-mem_base,
		mem_max-mem_base,
		mem_rm0-mem_base,
		mem_script);
	_Print(&string);
	return(param);		
} */
