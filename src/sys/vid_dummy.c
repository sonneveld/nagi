/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>


/* OTHER headers	---	---	---	---	---	---	--- */
#include "video.h"
#include "vid_render.h"

#include "vid_dummy.h"


/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

void dummyvid_init(RSTATE *rend_state)
{
	printf("dummy video init\n");
}

void dummyvid_shutdown()
{
	printf("dummy video shutdown\n");
}

void dummyvid_update(u16 x, u16 y, u16 width, u16 height)
{
	printf("dummy video update\n");
}

void dummyvid_clear()
{
	printf("dummy video clear\n");
}

void dummyvid_shake(u8 count)
{
	printf("dummy video shake\n");
}

void dummyvid_pos_get(POS *pos)
{
	printf("dummy video pos get\n");
}

void dummyvid_pos_set(POS *pos)
{
	printf("dummy video pos set\n");
}

void dummyvid_char_attrib(u8 colour, u16 flags)
{
	printf("dummy video char attrib\n");
}

void dummyvid_char_put(u8 ch)
{
	printf("dummy video char put\n");
}


void dummyvid_scroll(POS *pos1, POS *pos2, u16 scroll, u8 attrib)
{
	printf("dummy video scroll\n");
}

void dummyvid_char_clear(POS *pos1, POS *pos2, u8 attrib)
{
	printf("dummy video clear\n");
}