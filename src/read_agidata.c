typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;
typedef signed char		s8;
typedef signed short		s16;
typedef signed long		s32;
typedef unsigned int		uint;
typedef signed int		sint;

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	fpos_t file_size;
	FILE *file_stream;
	u8 *buff;
	int i;
	
	while (  (file_stream=fopen("agidata.ovl", "rb")) == 0  )
	{
		printf("bad loading stuff\n");
	}
	
	//fseek(file_stream, 0, SEEK_END);
	//fgetpos(file_stream, &file_size);
	fseek(file_stream, 0x1d2c, SEEK_SET);

	if (buff == 0)
		buff = (u8 *)malloc(0x150);
	
	fread(buff, sizeof(u8), 0x150, file_stream);
	
	for (i=1; i <= 0x150; i++)
	{
		printf("0x%02X, ", *(buff++));
		if ((i%8) == 0)
			printf("\n");
	}

		
	fclose(file_stream);
	return 0;

}