//~ RaDIaT1oN (2002-04-29):
//~ proper calls to ftime

#include "../agi.h"
//#include "../sys/delay.h"

// gettimeofday() does not exist in mingw libraries
#include <sys/timeb.h>

u16 agi_rand_seed = 0;

#define IBM_CLOCK_PER_SEC 18.2
#define TANDY_CLOCK_PER_SEC 20

u8 agi_rand(void)
{
	struct timeb t;
	u8 r;
		
	if (agi_rand_seed == 0)
        {
		//printf("Creating new randomised seed...\n");
		/* ah = 0; int(1Ah); */	// number of ticks since midnight
		ftime(&t);
		//printf("time = %ld seconds %ld milliseconds\n", t.time, t.millitm );
		agi_rand_seed = t.time * TANDY_CLOCK_PER_SEC;
		agi_rand_seed += (u16) ((double)t.millitm  / 1000 * TANDY_CLOCK_PER_SEC);
        	//printf("seed = 0x%04X\n\n", agi_rand_seed);
	}

        agi_rand_seed = 0x7C4D * agi_rand_seed + 1;
	r = agi_rand_seed ^ (agi_rand_seed>>8);
        return( r );
}


u8 *cmd_random(u8 *c)
{
	u16 diff;
	u16 min;
	u16 max;
	
	min =  *(c++);
	max = *(c++);
	diff = max - min +1;
	state.var[*(c++)] = (agi_rand() % diff) + min;
	return c;
}
