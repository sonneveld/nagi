/*
CmdGet                           cseg     00007471 0000001C
CmdGetV                          cseg     0000748D 0000001C
CmdDrop                          cseg     000074A9 0000001C
CmdPut                           cseg     000074C5 0000002B
CmdPutV                          cseg     000074F0 0000002B
CmdGetRoomV                      cseg     0000751B 0000002B
_Get                             cseg     00007546 00000056
_GetV                            cseg     0000759C 00000064
*/

#include "agi.h"

#include "objects.h"


#include <setjmp.h>
#include "sys/error.h"

/*
old:
u8 *object_ptr = 0;
s16 obj_size_minus_3;
*/
u8 *object = 0;
u8 *object_name = 0;
u16 object_size = 0;

u8 *cmd_get(u8 *c)
{
	u8 *di;
	//printf("get ");
	di = invent_find(&c);
	di[2] = 0xFF;
	return c;
}


u8 *cmd_get_v(u8 *c)
{
	u8 *di;
	//printf("getv ");
	di = invent_find_v(&c);
	di[2] = 0xFF;
	return c;
}

u8 *cmd_drop(u8 *c)
{
	u8 *di;
	//printf("drop ");
	di = invent_find(&c);
	di[2] = 0;
	return c;
}

u8 *cmd_put(u8 *c)
{
	u8 *di;
	//printf("put ");
	di = invent_find(&c);
	di[2] = state.var[*(c++)];
	return c;
}


u8 *cmd_put_v(u8 *c)
{
	u8 *di;
	//printf("putv ");
	di = invent_find_v(&c);
	di[2] = state.var[*(c++)];
	return c;
}

u8 *cmd_get_room_v(u8 *c)
{
	u8 *di;
	//printf("getroomv ");
	di = invent_find_v(&c);
	state.var[*(c++)] = di[2];
	return c;
}


u8 *invent_find(u8 **c)
{
	u8 *di;
	u8 *si;
	di = *c;
	//printf("%d\n", *di);
	si = object + (*(di++)) * 3;
	if ( si >= object_name)
		set_agi_error(0x17,  (si - object_name)/3);
	*c = di;
	return si;
}

u8 *invent_find_v(u8 **c)
{
	u8 *di;
	u8 *si;
	di = *c;
	//printf("%d\n", state.var[*(di)]);
	si = object + (state.var[*(di++)]) * 3;
	if ( si >= object_name)
		set_agi_error(0x17,  (si - object_name)/3);
	*c = di;
	return si;
}