/*
CmdSet                           cseg     000073BD 00000007
CmdReset                         cseg     000073C4 00000007
CmdToggle                        cseg     000073CB 00000007
CmdSetV                          cseg     000073D2 0000000F
CmdResetV                        cseg     000073E1 0000000F
CmdToggleV                       cseg     000073F0 0000000F
_SetFlag                         cseg     000073FF 0000000A
_ResetFlag                       cseg     00007409 0000000A
_ToggleFlag                      cseg     00007413 0000000A
_TestFlag                        cseg     0000741D 0000000A
_SetFlagAL                       cseg     00007427 00000006
_ResetFlagAL                     cseg     0000742D 00000008
_ToggleFlagAL                    cseg     00007435 00000006
_TestFlagAL                      cseg     0000743B 0000000F
_FlagMaskAL                      cseg     0000744A 00000019
_ClearFlags                      cseg     00007463 0000000E
*/
#include "agi.h"
#include "flags.h"

u8 *cmd_set(u8 *c)
{
	flag_set(*(c++));
	return c;
}

u8 *cmd_reset(u8 *c)
{
	flag_reset(*(c++));
	return c;
}

u8 *cmd_toggle(u8 *c)
{
	flag_toggle(*(c++));
	return c;
}

u8 *cmd_set_v(u8 *c)
{
	flag_set(state.var[*(c++)]);
	return c;
}

u8 *cmd_reset_v(u8 *c)
{
	flag_reset(state.var[*(c++)]);
	return c;
}

u8 *cmd_toggle_v(u8 *c)
{
	flag_toggle(state.var[*(c++)]);
	return c;
}


/*
the flags have been compressed into bits

*(state.flag+(flag_num>>3))
is the same as
u8 *bx;
bx = state.flag+flag_num/8
*bx = .....

0x80>>(flag_num % 8)
is the same as
100000000b >> (flag_num % 8)
just dragging one bit down to the right

then or, xor 'n all that is used to set, reset, 'n stuff
*/

void flag_set(u8 flag_num)
{
	*(state.flag+(flag_num>>3)) |= 0x80>>(flag_num % 8);
	//state.flag[flag_num] = 1;
}

void flag_reset(u8 flag_num)
{
	*(state.flag+(flag_num>>3)) &= (0x80>>(flag_num % 8))^0xFF;
	//state.flag[flag_num] = 0;
}

void flag_toggle(u8 flag_num)
{
	*(state.flag+(flag_num>>3)) ^= 0x80>>(flag_num % 8);
	//state.flag[flag_num] = state.flag[flag_num] ^ 1;
}

u8 flag_test(u8 flag_num)
{
	return ( (*(state.flag+(flag_num>>3)) & (0x80>>(flag_num % 8))) != 0);
	//return (state.flag[flag_num] != 0);
}

void flags_clear()
{
	memset(state.flag, 0 , sizeof(state.flag));
}
