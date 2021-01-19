
#include "../agi.h"
#include <stdlib.h>
#include <stdio.h>


void *a_malloc (size_t size)
{
	void *m;

	m = malloc(size);
	if ( m == 0)
	{
		printf("malloc error\n");
		exit(-1);
	}
	return m;
}

void a_free(void *m)
{
	free(m);
}

