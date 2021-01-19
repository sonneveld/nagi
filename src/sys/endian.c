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


#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	// ----------- little endian -------------------------------------------------------
	u16 load_le_16(void *data)
	{
		return *(  (u16 *)data  );
	}
	
	u16 load_be_16(void *data)
	{
		return ((u8 *)data)[1] |  (     ((u16)((u8 *)data)[0]) << 8  );
	}
	
	
	void store_le_16(void *data, u16 value)
	{
		*(  (u16 *)data  ) = value;
	}
	
	void store_be_16(void *data, u16 value)
	{
		((u8 *)data)[1] = value & 0xFF;
		((u8 *)data)[0] = (value & 0xFF00) >> 8;
	}
	
#else
	
	// ----------- big endian -------------------------------------------------------
	#warning big endian is untested..  I hope it works
	u16 load_le_16(void *data)
	{
		return ((u8 *)data)[0] |  (   ((u16)((u8 *)data)[1]) << 8  );
	}
	
	u16 load_be_16(void *data)
	{
		return *(  (u16 *)data  );
	}
	
	
	void store_le_16(void *data, u16 value)
	{
		((u8 *)data)[0] = value & 0xFF;
		((u8 *)data)[1] = (value & 0xFF00) >> 8;
	}
	
	void store_be_16(void *data, u16 value)
	{
		*(  (u16 *)data  ) = value;
	
	}
#endif

