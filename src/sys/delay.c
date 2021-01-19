//_DoClock                       cseg     00007E35 0000007C
//_DoDelay                       cseg     00007EB1 0000001D

#include "../agi.h"
#include "delay.h"

#include "../sound/sound_base.h"
#include "../sound/sound_gen.h"

#include "../ui/cmd_input.h"
#include "../flags.h"

u32 tick_prev = 0;
// 1/20 sec intervals
#define DELAY_MULT 50

u32 calc_agi_tick()
{
	// if delay_mult == 50;
	return SDL_GetTicks() / DELAY_MULT;
}


void delay_init()
{
	tick_prev = SDL_GetTicks();
}

void do_delay()
{
	SDL_PumpEvents();	// we have to poll at least once
	input_poll();
	SDL_Delay(1);	// so fastest speed doesn't run too fast
	
	while ( ( (state.var[V10_DELAY] * DELAY_MULT) > (SDL_GetTicks() - tick_prev) )
			&& (!flag_test(F02_PLAYERCMD)) )
	{
		SDL_PumpEvents();
		input_poll();
		//sndgen_poll();
		SDL_Delay(5);	// to prevent it taking 100% cpu
	}

	tick_prev = SDL_GetTicks();
	
	/*
	s32 delay_len;
	//s32 stuff;
	//s32 more;
	
	SDL_Delay(1);	// so it doesn't take up all cpu time

	delay_len = state.var[V10_DELAY] * DELAY_MULT - (SDL_GetTicks() - tick_prev);
	//stuff = SDL_GetTicks();
	if (delay_len > 0)
		SDL_Delay(delay_len);

	//more = SDL_GetTicks() - stuff;
	//printf("len=%d d=%d diff=%d\n", delay_len, more, more-delay_len);
	*/
	//tick_prev = SDL_GetTicks();
	//SDL_PumpEvents();
}



