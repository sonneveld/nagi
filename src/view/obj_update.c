/*
_IsUpdated                       cseg     0000691D 00000021
_IsNotUpdated                    cseg     0000693E 00000021
_BuildUpdList                    cseg     0000695F 00000017
_BuildNoUpdList                  cseg     00006976 00000017
_EraseBoth                       cseg     0000698D 0000001D
_FreeBoth                        cseg     000069AA 0000001D
_BlitBoth                        cseg     000069C7 0000001D
_CheckMoveBoth                   cseg     000069E4 0000001D
CmdStopUpdate                    cseg     00006A01 00000028
CmdStartUpdate                   cseg     00006A29 00000028
CmdForceUpdate                   cseg     00006A51 0000002C
_StopUpdate                      cseg     00006A7D 0000001E
_StartUpdate                     cseg     00006A9B 0000001E
*/

#include "../agi.h"

#include "obj_base.h"
#include "obj_update.h"

static void obj_start_update(VIEW *v);

static u16 obj_updated(VIEW *v)
{
	return ((v->flags & (O_DRAWN|O_UPDATE|O_ANIMATE)) == (O_DRAWN|O_UPDATE|O_ANIMATE));
}

static u16 obj_static(VIEW *v)
{
	return ((v->flags & (O_DRAWN|O_UPDATE|O_ANIMATE)) == (O_DRAWN|O_ANIMATE));
}


// head 1 = updated
BLIT *build_updated_list()
{
	return blitlist_build(obj_updated, &blitlist_updated);
}

//sprite_list_head_2 = not updated
BLIT *build_static_list()
{
	return blitlist_build(obj_static, &blitlist_static);
}

void blists_erase()
{
	blitlist_erase(&blitlist_updated);
	blitlist_erase(&blitlist_static);
}

void blists_free()
{
	blitlist_free(&blitlist_updated);
	blitlist_free(&blitlist_static);
}

void blists_draw()
{
	blitlist_draw( build_static_list() );
	blitlist_draw( build_updated_list() );
}

void blists_update()
{
	blitlist_update(&blitlist_static);
	blitlist_update(&blitlist_updated);
}

u8 *cmd_stop_update(u8 *c)
{
	obj_stop_update(&objtable[*(c++)]);
	return c;
}

u8 *cmd_start_update(u8 *c)
{
	obj_start_update(&objtable[*(c++)]);
	return c;
}

u8 *cmd_force_update(u8 *c)
{
	blists_erase();
	blists_draw();
	blists_update();
	
	c++;		// this is because agi is meant to just update the one
			// but from at least 2.917 it updates them all
	return c;
}

void obj_stop_update(VIEW *v)
{
	if ((v->flags & O_UPDATE) != 0)
	{
		blists_erase();
		v->flags &= ~O_UPDATE;
		blists_draw();
	}
} 

static void obj_start_update(VIEW *v)
{
	if ((v->flags & O_UPDATE) == 0)
	{
		blists_erase();
		v->flags |= O_UPDATE;
		blists_draw();
	}
}

