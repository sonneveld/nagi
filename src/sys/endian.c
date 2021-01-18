/*
load_le_16
load_be_16
store_le_16
store_be_16

little endian is intel
big endian is motorola

*/

#include <assert.h>

#include "../agi.h"

// byte-order support
#include "endian.h"

uint16_t load_le_16(const void *data)
{
	assert(data != 0);
	const uint8_t *data8 = (uint8_t *)data;
	return (data8[1]<<8) | data8[0];
}

uint16_t load_be_16(const void *data)
{
	assert(data != 0);
	const uint8_t *data8 = (uint8_t *)data;
	return (data8[0]<<8) | data8[1];
}

void store_le_16(void *data, uint16_t value)
{
	assert(data != 0);
	uint8_t *data8 = (uint8_t *)data;
	data8[0] = value & 0xFF;
	data8[1] = (value >> 8) & 0xFF;
}

void store_be_16(void *data, uint16_t value)
{
	assert(data != 0);
	uint8_t *data8 = (uint8_t *)data;
	data8[0] = (value >> 8) & 0xFF;
	data8[1] = value & 0xFF;
}
