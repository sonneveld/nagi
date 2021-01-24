#include "../agi.h"

// print
#include "../ui/msg.h"
#include "../base.h"

#include "../sys/time.h"



// 0 = run normal
// 1 = pause
// 2 = turn off
u16 clock_state = 0;

static SDL_Thread *agi_clock_thread;

#define SDL_TICK_SCALE 50

// TODO: check base.c   disable clock and denit time


static int clock_thread(void *unused)
{	
	u16 time_counter = 0;
	u32 sdl_tick_prev = 0;
	u32 sdl_tick = 0;
	
	(void) unused;
	
	sdl_tick_prev = SDL_GetTicks();
	while (clock_state != 2)
	{
		sdl_tick = SDL_GetTicks();
		state.ticks += (sdl_tick - sdl_tick_prev) / SDL_TICK_SCALE;

		switch(clock_state)
		{
			case 0:
				// it's in 1/20's of seconds
				time_counter += (sdl_tick - sdl_tick_prev) ;
			
				while (time_counter >= 20*SDL_TICK_SCALE)
				{ 
					time_counter -= 20*SDL_TICK_SCALE;
					state.var[V11_SECONDS]++;
			
					if (state.var[V11_SECONDS] >= 60)
					{
						state.var[V11_SECONDS] = 0;
						state.var[V12_MINUTES]++;
					}
					if (state.var[V12_MINUTES] >= 60)
					{
						state.var[V12_MINUTES] = 0;
						state.var[V13_HOURS]++;
					}
					if (state.var[V13_HOURS] >= 24)
					{
						state.var[V13_HOURS] = 0;
						state.var[V14_DAYS]++;
					}
				}

			case 1:
				sdl_tick_prev = sdl_tick;
				break;
		}

		SDL_Delay(500);	// it won't update for a second anyways
					// a bit less though to account for overhead
	}

	return 0;
}



void clock_init()
{
	state.var[V11_SECONDS] = 0;
	state.var[V12_MINUTES] = 0;
	state.var[V13_HOURS] = 0;
	state.var[V14_DAYS] = 0;
	clock_state = 0;
	agi_clock_thread = SDL_CreateThread(clock_thread, "nagi_clock", NULL);
	if ( agi_clock_thread == NULL )
	{
		printf("Unable to create SDL thread!");
		agi_exit();
		return;
	}
}

void clock_denit()
{
	clock_state = 2; // turn off
	printf("Waiting for clock thread to die...");
	SDL_WaitThread(agi_clock_thread, NULL);
	printf("done.\n");
}

