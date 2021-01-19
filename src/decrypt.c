#include <stdio.h>

#include "agi.h"
#include "decrypt.h"

#define DECRYPT_STRING "Avis Durgan"

void decrypt_string(u8 *start, u8 *end)
{
	u8 *d_string = "";
	u8 *cur_char = start;
	
	//printf("\n\nDecrypting information..\nVery Important: some files aren't encrypted in older AGI versions\n");
	//printf("Errors may occur\n");
	while (cur_char < end)
	{
		if (*d_string == 0)
			d_string = DECRYPT_STRING;
		*cur_char = (*cur_char) ^ (*d_string);
		cur_char++;
		d_string++;
	}
}
