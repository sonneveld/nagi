/* FUNCTION list 	---	---	---	---	---	---	---
creates a new vstring with string or minsize
VSTRING * vstring_new(u8 *, min_size)

frees all data and sets it to zero.
void vstring_free(VSTRING *)

shrinks the allocated size to the text size
void vstring_shrink(VSTRING *);

strcpys the text into vstring.. sets the size if it's too small
void vstring_set_text(VSTRING *, u8 *)

sets the minimum size.. for shrinking 'n stuff
void vstring_set_min(VSTRING *, min_size)

shift characters a certain number left
void vstring_shift(VSTRING *, shift_size)
*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"
#include "../sys/vstring.h"
#include "../sys/mem_wrap.h"


/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
 

/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


	
#define VSTRING_DEF_MIN 50
	
VSTRING *vstring_new(u8 *str, u32 min_size)
{
	VSTRING *vs_new;
	
	vs_new = (VSTRING *)a_malloc(sizeof(VSTRING));
	
	vs_new->min = min_size;
	if (vs_new->min <= 0)
		vs_new->min = VSTRING_DEF_MIN;
	
	if ( (str != 0) && ((strlen(str)+1) > vs_new->min) )
		vs_new->size = strlen(str)+1;	// +1 for null character at end of string
	else
		vs_new->size = vs_new->min;
	
	vs_new->data = (u8 *)a_malloc(vs_new->size);
	memset(vs_new->data, 0, vs_new->size);
	
	if (str != 0)
	{
		strcpy(vs_new->data, str);
		vs_new->data[strlen(str)] = 0;
	}
	
	return vs_new;
}

void vstring_free(VSTRING *vs)
{
	if (vs != 0)
	{
		if (vs->data != 0)
			a_free(vs->data);
		a_free(vs);
	}
}

void vstring_shrink(VSTRING *vs)
{
	if ( (vs != 0) && (vs->data != 0) )
	{
		u8 *new_data;
		
		vs->size = strlen(vs->data)+1;
		if (vs->size < vs->min)
			vs->size = vs->min;
		
		new_data = (u8 *)a_malloc(vs->size);
		memcpy(new_data, vs->data, vs->size);
		a_free(vs->data);
		vs->data = new_data;
	}
}


void vstring_set_text(VSTRING *vs, u8 *str)
{
	if ( (vs!=0) && (str!=0) )
	{
		u32 str_size;
		str_size = strlen(str)+1;
		
		if (vs->size < str_size)
			vstring_set_size(vs, str_size);

		strcpy(vs->data, str);
		vs->data[str_size-1] = 0;
	}
}

// does not bother to copy old string.
void vstring_set_size(VSTRING *vs, u32 new_size)
{
	if (vs != 0)
	{
		if (new_size < vs->min)
			new_size = vs->min;
		if (vs->size < new_size)
		{
			a_free(vs->data);
			vs->data = (u8 *)a_malloc(new_size);
			vs->size = new_size;
			//memset(vs->data, 0, new_size);
		}
	}
}

// shift left <----
void vstring_shift(VSTRING *vs, u32 shift_size)
{
	if ((vs!=0) && (vs->data != 0) && (shift_size > 0))
	{
		if (strlen(vs->data) < shift_size)
			vs->data[0] = 0;
		else
			memmove(vs->data, vs->data + shift_size, vs->size - shift_size);
	}
}
