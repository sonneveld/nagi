/*
CmdIncrement                     cseg     0000728E 00000013
CmdDecrement                     cseg     000072A1 00000013
CmdAssignN                       cseg     000072B4 0000000D
CmdAssignV                       cseg     000072C1 00000013
CmdAddN                          cseg     000072D4 0000000D
CmdAddV                          cseg     000072E1 00000013
CmdSubN                          cseg     000072F4 0000000D
CmdSubV                          cseg     00007301 00000013
CmdLIndirectV                    cseg     00007314 00000019
CmdLIndirectN                    cseg     0000732D 00000011
CmdRIndirect                     cseg     0000733E 00000019
CmdMultN                         cseg     00007357 00000013
CmdMultV                         cseg     0000736A 0000001B
CmdDivN                          cseg     00007385 00000019
CmdDivV                          cseg     0000739E 0000001F
*/

/*
 state.var[V06_DIRECTION]
*/

#include "../agi.h"
#include "arithmetic.h"


// command increment
// if an agi variable < 255, increment
u8 *cmd_increment(u8 *code)
{
	u8 a;

        a = *code++;
	if (state.var[a] < 0xFF)
		state.var[a]++;

	return(code);
}

// command decrement
// if an agi variable != 0, decrement
u8 *cmd_decrement(u8 *code)
{
	u8 a;

        a = *code++;
        // b = *code++;
        // c = *code++;
	if (state.var[a] != 0x00)
		state.var[a]--;

	return(code);
}

/* ---------------- */

u8 *cmd_assignn(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] = b;

	return(code);
}


u8 *cmd_assignv(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] = state.var[b];

	return(code);
}

/* ---------------- */


u8 *cmd_addn(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] += b;

	return(code);
}

u8 *cmd_addv(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] += state.var[b];

	return(code);
}

u8 *cmd_subn(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] -= b;

	return(code);
}

u8 *cmd_subv(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] -= state.var[b];

	return(code);
}

/* ---------------- */


u8 *cmd_lindirectv(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
	state.var[ state.var[a] ] = state.var[b];

	return(code);
}

u8 *cmd_lindirectn(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
	state.var[ state.var[a] ] = b;

	return(code);
}

u8 *cmd_rindirect(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
	state.var[a] = state.var[ state.var[b] ];

	return(code);
}

/* ---------------- */

u8 *cmd_multn(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] *= b;

	return(code);
}

u8 *cmd_multv(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] *= state.var[b];

	return(code);
}

u8 *cmd_divn(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
        // c = *code++;
	state.var[a] /= b;

	return(code);
}

u8 *cmd_divv(u8 *code)
{
	u8 a,b;

        a = *code++;
        b = *code++;
	state.var[a] /= state.var[b];

	return(code);
}


