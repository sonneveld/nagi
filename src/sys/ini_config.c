/*
open 
close
read
write
delete
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../agi.h"
#include "../sys/ini_config.h"

u8 *ini_data = 0;
u8 *ini_name = 0;
u8 *key_data = 0;
fpos_t ini_size = 0;


//int ini_write(u8 *key_name, u8 *key_data);
//int ini_delete(u8 *key_name);


u8 *line_next(u8 *data, u8 *last);
u8 *section_find(u8 *data, u8 *sect_name, u8 *last);
u8 *key_find(u8 *data, u8 *key_name, u8 *last);
u8 *key_read(u8 *data, u8 *last);

// return 0 if ok
int ini_open(u8 *ini_given_name)
{
	FILE *file_stream;
	
	if (((int)ini_data|(int)ini_name) != 0)	// something's open..... damnit!
		return 3;
	
	ini_name = strdup(ini_given_name);
	
	if ( (file_stream=fopen(ini_name, "rb")) == 0)
		return 1;
	
	fseek(file_stream, 0, SEEK_END);
	fgetpos(file_stream, &ini_size);
	fseek(file_stream, 0, SEEK_SET);

	ini_data = (u8 *)malloc(ini_size);
	
	if ( fread(ini_data, sizeof(u8), ini_size, file_stream) !=(size_t)ini_size)
		return 2;
	
	fclose(file_stream);
	return 0;
}

int ini_close()
{
	FILE *file_stream;
	
	// write the ini file
	if ( (ini_name!=0) && (ini_data!=0) )
	{
		if ( (file_stream=fopen(ini_name, "wb")) != 0)
		{
			fseek(file_stream, 0, SEEK_SET);
			fwrite(ini_data, sizeof(u8), ini_size, file_stream);
			fclose(file_stream);
		}
	}
	// free memory
	if (ini_name != 0)
		free(ini_name);
	ini_name = 0;
	if (ini_data != 0)
		free(ini_data);
	ini_data = 0;
	if (key_data != 0)
		free(key_data);
	key_data = 0;
	
	return 0;
}


u8 *line_next(u8 *data, u8 *last)
{
	u8 lstate;
	u8 *cur;
	// 0 = looking for new line .. 1 = currently reading new line characters
	// 2 = after the new line
	
	cur = data;
	lstate = 0;	// looking
	
	while ( (cur<=last) && (lstate!=2) && (cur != 0) )
	{
		switch (lstate)
		{
			case 0:
				if ( (*cur==0xA) || (*cur==0xD) )
					lstate = 1;	// reading newline characters
				cur++;
				break;
			case 1:
				if ( (*cur!=0xA) && (*cur!=0xD) )
					lstate = 2;	// new line!
				else
					cur++;
				break;
		}
	}
	
	return (lstate==2)? cur: 0;
}

u8 *section_find(u8 *data, u8 *sect_name, u8 *last)
{
	u8 *ini_cur;
	u8 *sect_cur;
	u8 result;
	
	if (data==0)
		return 0;
	
	ini_cur = data;
	result = 0;

	while ( (ini_cur != 0) && (result != 1) )
	{
		// find a line starting with '['
		while ( (ini_cur != 0) && (ini_cur[0] != '[') )
			ini_cur = line_next(ini_cur, last);
		if (ini_cur != 0)
		{
			ini_cur++;	// skip the '['
			sect_cur = sect_name;
			
			while ( (ini_cur != 0) && (result == 0) )
			{
				//printf("%c ? %c ", *sect_cur, *ini_cur);
				if (ini_cur > last) 
				{
					//printf("over size\n");
					ini_cur=0;
					result=1;
				}
				else if ( (*sect_cur==0) && (*ini_cur == ']') )
				{
					//printf("found\n");
					result = 1;
				}
				else if  ( (*sect_cur!=*ini_cur) || (*sect_cur==0)  )
				{
					//printf("next line\n");
					while ( (ini_cur != 0) && (ini_cur[0] != '[') )
						ini_cur = line_next(ini_cur, last);
					if (ini_cur != 0)
						ini_cur++;
					sect_cur = sect_name;
					//printf("blah");
					//system("PAUSE");
				}
				else
				{
					//printf("equal\n");
					sect_cur++;
					ini_cur++;
				}
				
			}
			
			ini_cur = line_next(ini_cur, last);
		}
		
	}
	
	return ini_cur;
}


u8 *key_find(u8 *data, u8 *key_name, u8 *last)
{
	u8 *cur;
	u8 *key_cur;
	u8 result;
	
	if (data==0)
		return 0;
	
	cur = data;
	result = 0;
	key_cur = key_name;
	if (cur[0] == '[')
		cur = 0;
	
	while (  (result != 1) && (cur!=0)  )
	{
		//printf("%c(%X) ? %c(%X) ", *key_cur, *key_cur, *cur, *cur);
		if (cur > last)
		{
			//printf("oversize\n");
			cur=0;
		}
		else if (  (*key_cur == 0) && (*cur == '=')  )
		{
			//printf("found\n");
			result = 1;
		}
		else if  ( (*key_cur!=*cur) || ((*key_cur == 0) && (*cur != '=')) )
		{
			//printf("not eq\n");
			key_cur = key_name;
			cur = line_next(cur, last);
			if (cur != 0)
				if (cur[0] == '[')
					cur = 0;
		}
		else
		{
			//printf("equal\n");
			key_cur++;
			cur++;
		}
	}

	return cur;
	
}

u8 *key_read(u8 *data, u8 *last)
{
	u8 *next, *cur;
	int key_size;
	u8 *key_cur=0;
	
	if (data==0)
		return 0;
	
	next = line_next(data,last);
	if (next == 0)
		key_size = last-data+1;
	else
		key_size = next-data+1;
	
	if (key_size != 0)
	{
		if (key_data == 0)
			key_data = (u8 *)malloc(key_size);
		else
			key_data = (u8 *)realloc(key_data, key_size);
		key_cur = key_data;
		
		cur=data;
		
		if (*cur == '=')
		{
			cur++;
			if (cur > last)
				cur=0;
		}
		
		while(cur!=0)
		{
			if ((cur>last)||(*cur==0xA)||(*cur==0xD))
				cur = 0;
			else 
			{
				//printf("%c %c\n", *key_cur, *cur);
				*(key_cur++) = *(cur++);
			}
		}
		*key_cur = 0;
		return key_data;
	}
	else
		return 0;
}

u8 *ini_read(u8 *sect_name, u8 *key_name)
{
	u8 *ini_cur;
	u8 *ini_last;
	u8 *key_value = 0;

	if ((ini_data ==0)||(sect_name==0)||(key_name==0))
		return 0;
	
	ini_cur = ini_data;
	if (ini_size != 0)
		ini_last = ini_data + ini_size - 1;
	else
		ini_last = ini_data;
	
	ini_cur = section_find(ini_cur, sect_name, ini_last);
	if (ini_cur != 0)
	{
		ini_cur = key_find(ini_cur, key_name, ini_last);
		if (ini_cur != 0)
			key_value = key_read(ini_cur, ini_last);
		
	}
	
	return key_value;
}

int ini_boolean(u8 *sect, u8 *sect_parent, u8 *key, int val_default)
{
	u8 *key_value;
	// child section
	if ( (key_value=ini_read(sect, key)) != 0)
	{
		//printf("  [child|%s] is set to \"%s\".\n", key, key_value);
		return (strtoul (key_value, 0, 10) != 0);
	}
	else
		if (sect_parent != 0)	// parent section
			if ( (key_value=ini_read(sect_parent, key)) != 0)
			{
				//printf("  [parent|%s] is set to \"%s\".\n", key, key_value);
				return (strtoul(key_value, 0, 10) != 0);
			}
	// default value
	//printf("  [%s] is NOT set!  Default to %d.\n", key, val_default);
	return (val_default != 0);
}

int ini_int(u8 *sect, u8 *sect_parent, u8 *key, int val_default)
{
	u8 *key_value;
	// child section
	if ( (key_value=ini_read(sect, key)) != 0)
	{
		//printf("  [child|%s] is set to \"%s\".\n", key, key_value);
		return strtol(key_value, 0, 10);
	}
	else
		if (sect_parent != 0)	// parent section
			if ( (key_value=ini_read(sect_parent, key)) != 0)
			{
				//printf("  [parent|%s] is set to \"%s\".\n", key, key_value);
				return strtol(key_value, 0, 10);
			}
	// default value
	//printf("  [%s] is NOT set!  Default to %d.\n", key, val_default);
	return val_default;
}

// returns something from a buffer... use it before you call ini_string or ini_read again
// exactly the same as using ini_read but with the parent bit
u8 *ini_string(u8 *sect, u8 *sect_parent, u8 *key)
{
	u8 *key_value;
	// child section
	if ( (key_value=ini_read(sect, key)) != 0)
	{
		//printf("  [child|%s] is set to \"%s\".\n", key, key_value);
		return key_value;
	}
	else
		if (sect_parent != 0)	// parent section
			if ( (key_value=ini_read(sect_parent, key)) != 0)
			{
				//printf("  [parent|%s] is set to \"%s\".\n", key, key_value);
				return key_value;
			}
	// default value
	//printf("  [%s] is NOT set!\n", key);
	return 0;
}

