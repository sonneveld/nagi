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
#include "res/res.h"


#include <setjmp.h>
#include "sys/error.h"

/*
old:
u8 *object_ptr = 0;
s16 obj_size_minus_3;
*//*
u8 *object = 0;
u8 *object_name = 0;
u16 object_size = 0;*/


int object_file_load(void);
void object_file_unload(void);
INV_OBJ *invent_find(u8 **c);
INV_OBJ *invent_find_v(u8 **c);

INV_OBJ *inv_obj_table = 0;
int inv_obj_table_size = 0;
u8 *inv_obj_string = 0;
int inv_obj_string_size = 0;

int object_file_load()
{
	u8 *obj_data;
	u8 *ptr;
	u16 count;
	u8 item_size;
	u16 head_anim_obj_max;
	u16 head_string_offset;
	u16 string_size;
	u8 *string_ptr;
	
	if (c_game_res == RES_V3_AMIGA) item_size = 4;
		else item_size = 3;
	
	if (inv_obj_table != 0)
		object_file_unload();
	
	obj_data = file_load("object", 0);
	
	if (res_size < 3)
	{
		printf("object_file_load(): object incorrect size\n");
		return 1;
	}
		
	count = 3;
	
	while (count)
	{
		switch(count)
		{
			case 1: // still didn't work.. fail
				printf("object_file_load(): invalid object file\n");
				a_free(obj_data);
				return 2;
				break;
			
			case 2: // 2nd go.. we'll have to decrypt now
				printf("object_file_load(): default decryption option invalid.  trying opposite\n");
				decrypt_string(obj_data, obj_data + res_size );
				break;
			
			case 3: // first go round... decrypt if necessary
				// decrypt or not decrypt
				if (c_game_object_decrypt != 0)
					decrypt_string(obj_data, obj_data + res_size);
				break;
				
			default:
				break;	
		}
		
		count --;
		
		// read name's offset
		head_string_offset = load_le_16(obj_data);
		if ((head_string_offset + item_size) >= res_size)
			goto obj_continue;	// decrypt and try again
		
		// read animated obj_max
		if (c_game_res == RES_V3_AMIGA)
			head_anim_obj_max = load_le_16(obj_data + 2) + 1;
		else
			head_anim_obj_max = obj_data[2] + 1;
		
		printf("object_file_load(): number of animated object's defined = %d\n", head_anim_obj_max);

		// load up table
		count = head_string_offset / item_size;
		inv_obj_table_size = count;
		if (count)
		{
			ptr = obj_data + item_size*(count);
			inv_obj_table = (INV_OBJ *)a_malloc(count * sizeof(INV_OBJ));
			while (count)
			{
				count--;
				inv_obj_table[count].name = load_le_16(ptr );
				if ((inv_obj_table[count].name + item_size) >= res_size)
					goto obj_continue;	// decrypt and try again
				inv_obj_table[count].name -= head_string_offset;
				if (c_game_res == RES_V3_AMIGA)
					inv_obj_table[count].location = load_le_16(ptr  + 2);
				else
					inv_obj_table[count].location = ptr[2];
				ptr -= item_size;
			}
		}
		
		count = 0;
	obj_continue:
	}
	
	// bung in a '\0' at the end if one doesn't exist
	string_size = res_size - head_string_offset - item_size ;
	string_ptr = obj_data+item_size+head_string_offset;
	
	// create string stuff
	if (string_ptr[string_size-1] != '\0')
	{
		printf("object_file_load(): warning! object string does not end in '\\0'.\n");
		inv_obj_string = (u8 *)a_malloc(string_size + 1);
		inv_obj_string_size = string_size + 1;
		memcpy(inv_obj_string, string_ptr, string_size);
		inv_obj_string[string_size] = '\0';				
	}
	else
	{
		inv_obj_string = (u8 *)a_malloc(string_size);
		inv_obj_string_size = string_size;
		memcpy(inv_obj_string, string_ptr, string_size);
	}

	// free old obj_file
	a_free(obj_data);
	
	// call obj_table create
	objtable_new(head_anim_obj_max);	// new view table baby
	
	return 0;
}

void object_file_unload()
{
	// free table
	if (inv_obj_table != 0)
	{
		a_free(inv_obj_table);
		inv_obj_table = 0;
	}
	// free names
	if (inv_obj_string != 0)
	{
		a_free(inv_obj_string);
		inv_obj_string = 0;
	}
}


void inv_obj_table_print()
{
	int i;
	
	for (i=0; i< inv_obj_table_size; i++)
	{
		printf("%s @ %d\n",inv_obj_string + inv_obj_table[i].name, inv_obj_table[i].location);
		
	}
	
}

u8 *cmd_get(u8 *c)
{
	invent_find(&c)->location = 0xFF;
	return c;
}


u8 *cmd_get_v(u8 *c)
{
	invent_find_v(&c)->location = 0xFF;
	return c;
}

u8 *cmd_drop(u8 *c)
{
	invent_find(&c)->location = 0x00;
	return c;
}

u8 *cmd_put(u8 *c)
{
	INV_OBJ *obj;
	obj = invent_find(&c);
	obj->location = state.var[*(c++)];
	return c;
}


u8 *cmd_put_v(u8 *c)
{
	INV_OBJ *obj;
	obj = invent_find_v(&c);
	obj->location = state.var[*(c++)];
	return c;
}

u8 *cmd_get_room_v(u8 *c)
{
	INV_OBJ *obj;
	obj = invent_find_v(&c);
	state.var[*(c++)] = obj->location;
	return c;
}


INV_OBJ *invent_find(u8 **c)
{
	if ((**c) >= inv_obj_table_size)
		set_agi_error(0x17,  (**c) - inv_obj_table_size);
	
	return &inv_obj_table[*((*c)++)];
}

INV_OBJ *invent_find_v(u8 **c)
{
	if (state.var[(**c)] >= inv_obj_table_size)
		set_agi_error(0x17,  state.var[(**c)] - inv_obj_table_size);
	
	return &inv_obj_table[state.var[*((*c)++)]];
}