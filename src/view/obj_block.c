/*
CmdBlock                         cseg     00007A87 0000003C
CmdUnblock                       cseg     00007AC3 00000012
CmdIgnoreBlocks                  cseg     00007AD5 00000025
CmdObserveBlock                  cseg     00007AFA 00000025
_CheckBlock                      cseg     00007B1F 00000032
*/

#include "../agi.h"
#include "../view/obj_blit.h"
#include "../view/obj_base.h"



u8 *cmd_block(u8 *c)
{
	state.block_state = 1;
        state.block_x1 = *c++;
        state.block_y1 = *c++;
        state.block_x2 = *c++;
        state.block_y2 = *c++;
	return(c);
}


u8 *cmd_unblock(u8 *c)
{
	state.block_state = 0;
	return(c);
}



u8 *cmd_ignore_blocks(u8 *c)
{
	objtable[*(c++)].flags |= O_BLOCKIGNORE;
	return(c);
}



u8 *cmd_observe_blocks(u8 *c)
{
	objtable[*(c++)].flags &= ~O_BLOCKIGNORE;
	return c;
}



u16 block_chk_pos(u16 x, u16 y)
{
	return ( (state.block_x1 < x)&&(state.block_x2 > x)&&
		(state.block_y1 < y)&&(state.block_y2 > y) );
}

