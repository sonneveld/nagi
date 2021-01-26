/*
load_le_16
load_be_16
store_le_16
store_be_16

little endian is intel
big endian is motorola

*/

#include "../agi.h"

// byte-order support
#include "endian.h"


u16 load_le_16(const void *data)
{
	const u8 *datab = (const u8 *)(data);
	return (datab[1] << 8) | datab[0];
}

u16 load_be_16(const void *data)
{
	const u8 *datab = (const u8 *)(data);
	return (datab[0] << 8) | datab[1];
}


void store_le_16(void *data, u16 value)
{
	u8 *datab = (u8 *)(data);
	datab[0] = value & 0xFF;
	datab[1] = (value >> 8) & 0xFF;
}

void store_be_16(void *data, u16 value)
{
	u8 *datab = (u8 *)(data);
	datab[0] = (value >> 8) & 0xFF;
	datab[1] = value & 0xFF;
}
