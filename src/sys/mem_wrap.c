
#include "../agi.h"
#include <stdlib.h>
#include <stdio.h>

int total = 0;

void *a_malloc (size_t size)
{
	void *m;

	m = malloc(size);
	if ( m == 0)
	{
		printf("malloc error\n");
		exit(-1);
	}

	//	total += size;
	//printf("m=%d ", size);
	//printf("m=%X \n", (u32)m);
	return m;
}

void a_free(void *m)
{
	//		total += size;
	//printf("m=%d ", size);
	//printf("f=%X \n", (u32)m);
	free(m);
}