
#include "../agi.h"
#include "events.h"

#include "../sys/mem_wrap.h"
#include "../trace.h"





AGI_EVENT passed_agi_event; 
AGI_EVENT stop_ego = {2, 0};

// -------------------- INIT

void events_init()
{
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_JOYAXISMOTION, SDL_IGNORE);
	SDL_EventState(SDL_JOYBALLMOTION, SDL_IGNORE);
	SDL_EventState(SDL_JOYHATMOTION, SDL_IGNORE);
	SDL_EventState(SDL_QUIT, SDL_IGNORE);
	
	SDL_EnableUNICODE(1);
	
	if (SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL) != 0)
	{
		printf("Couldn't enable key repeat: %s\n", SDL_GetError());
		exit(1);
	}
	
	events_clear();
	
	#warning needs cmd_init_joy ??
}


// ------------------- DIRECTION
			
#define DIR_MAX 8

KEY dir_map[]={ {SDLK_UP, 1}, {SDLK_PAGEUP, 2},
			{SDLK_RIGHT, 3}, {SDLK_PAGEDOWN, 4},
			{SDLK_DOWN, 5}, {SDLK_END, 6}, 
			{SDLK_LEFT, 7}, {SDLK_HOME, 8} , 
			{SDLK_KP8, 1},	{SDLK_KP9, 2},
			{SDLK_KP6, 3},	{SDLK_KP3, 4},
			{SDLK_KP2, 5},	{SDLK_KP1, 6},
			{SDLK_KP4, 7},	{SDLK_KP7, 8},
		{0,0}};

// map directions to key symbols
u16 dir_keymap(SDL_keysym *keysym)
{	
	KEY *k = dir_map;
	
	while ( (k->symbol != 0) && (k->symbol != keysym->sym) )
		k++;

	if (k->symbol != 0)
		return k->value;
	else
		return 0xFFFF;
}


//KEY ibm_map[] = {
/*
	need f1-f15
	need alt-a-z
	need ctrl-a-z
	tab
	esc
	backspace maps to 0x8?
	enter to 10
*/

// i hate the alt key
u16 system_alt_map[] = {30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38, 50, 
				49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44};

// map the keys depending on system (ibm in this case)
// TODO write amiga?  apple ][.. um.. mac??  why don't I scratch my ass?
// for the next gen of agi.. define a set of keys? that's the same over systems?
u16 system_keymap(SDL_keysym *keysym)
{
	if (  (keysym->sym >= SDLK_F1) && (keysym->sym <= SDLK_F10)  )
	{
		// f1-f12
		return (keysym->sym - SDLK_F1 + 0x3B) << 8;
	}
	else if ((keysym->mod & KMOD_ALT) != 0) 
	{
		if ( (keysym->sym >= SDLK_a) && (keysym->sym <= SDLK_z)  )
			return system_alt_map[keysym->sym - SDLK_a] << 8;	// alt a-z
	}
	else if ((keysym->mod & KMOD_CTRL) != 0)
	{
		if ((keysym->sym >= SDLK_a) && (keysym->sym <= SDLK_z)  )
			return keysym->sym - SDLK_a + 0x01;	// ctrl a-z
	}
	else
	{
		switch (keysym->sym)
		{
			case SDLK_TAB:
				return 0x09;
				break;
			case SDLK_ESCAPE:
				return 0x1B;
				break;
			case SDLK_BACKSPACE:
			case SDLK_DELETE:
				return 0x08;
				break;
			case SDLK_KP_ENTER:
			case SDLK_RETURN:
				return 0x0D;
			default:
				if ( ((keysym->unicode & 0xFF80) == 0) && ((keysym->unicode) != 0) )
					return keysym->unicode & 0x7F;
		}
	}
	return 0;
}

// if the key is a direction, then map it to that
// else, return the ascii thing back
AGI_EVENT *key_parse(SDL_keysym *keysym)
{
	u16 direction;
	AGI_EVENT *agi_event = &passed_agi_event;
	
	direction = dir_keymap(keysym);
	if (direction != 0xFFFF)
	{
		agi_event->type = 2;
		agi_event->data = direction;
	}
	else
	{
		agi_event->type = 1;
		agi_event->data = system_keymap(keysym);
	}
	return agi_event;
}



// --------------- SPECIAL KEY

#warning this version skips some keys but will still pass them to the bios_buff read bit

KEY key_special[] = { {SDLK_HOME, 0}, {SDLK_UP, 0},
				{SDLK_PAGEUP, 0}, {SDLK_LEFT, 0}, 
				{SDLK_RIGHT, 0}, {SDLK_END, 0}, 
				{SDLK_DOWN, 0}, {SDLK_PAGEDOWN, 0}, {0,0} };

/* checks the key_up event
if it's alternative walk mode, when the player lifts the arrow key, the ego should stop 
otherwise, it's a waste of space
*/
AGI_EVENT *event_key_up(SDL_keysym *keysym)
{
	KEY *k = key_special;
	AGI_EVENT *agi_event = 0;
	
	while ( (k->symbol != 0) && (k->symbol != keysym->sym) )
		k++;
	
	if (  (k->symbol != 0) && (k->value != 0)  )
	{
		k->value = 0;
		if (state.walk_mode != 0)
			agi_event = &stop_ego;	// stop player
	}
		
	return agi_event;
}

AGI_EVENT *event_key_down(SDL_keysym *keysym)
{
	KEY *k = key_special;
	KEY *t;
	AGI_EVENT *agi_event = 0;
	
	//printf("%d\n", keysym->sym);
	switch (keysym->sym)
	{
		case SDLK_KP5:
		case SDLK_CLEAR:
			//printf("keypad 5 detected(%d)\n", keysym->sym);
			agi_event = &stop_ego;	// stop player
			break;
		
		case SDLK_SCROLLOCK:
			if (trace_state == 0)
				trace_init();
			else
				trace_clear();	// removes the trace window
			break;
		
		default:
			while ( (k->symbol != 0) && (k->symbol != keysym->sym) )
				k++;
			
			if (k->symbol != 0)	// direction key
			{
				if (k->value == 0)	// first press (not repeat)
				{
					for (t=key_special ; t->symbol!=0 ; t++)
						t->value = 0;
					k->value ++;
					agi_event = key_parse(keysym);
				}
			}
			else	// normal key
				agi_event = key_parse(keysym);
	}
	
	return agi_event;
}
	
// return 1 if ok.. 0 if it failed
u16 event_write(u16 type, u16 data)
{
	SDL_Event event;
	AGI_EVENT *new_event;
	
	event.type = SDL_USEREVENT;
	
	new_event = (AGI_EVENT *)a_malloc(sizeof(AGI_EVENT));
	new_event->type = type;
	new_event->data = data;
	
	event.user.data1 = new_event;
	
	if (SDL_PushEvent(&event) == 0)
		return 1;
	else
		return 0;
}

AGI_EVENT *user_event_decode(void *data)
{
	AGI_EVENT *old_event;
	AGI_EVENT *agi_event = &passed_agi_event;	
	
	old_event = (AGI_EVENT *)data;
	agi_event->type = old_event->type;
	agi_event->data = old_event->data;
	a_free(old_event);
	
	return agi_event;
}


AGI_EVENT *event_mouse_button(u8 button, u16 x, u16 y)
{
	AGI_EVENT *agi_event = &passed_agi_event;	
	
	agi_event->type = 10;	// mouse
	switch (button)
	{
		case SDL_BUTTON_LEFT:
			agi_event->data = 1;
		break;
		case SDL_BUTTON_MIDDLE:
			agi_event->data = 3;
		break;
		case SDL_BUTTON_RIGHT:
			agi_event->data = 2;
		break;
		default:
			agi_event->data = 0;
		break;
	}
	agi_event->x = x;
	agi_event->y = y;
	
	return agi_event;
}

AGI_EVENT *event_read(void)
{
	SDL_Event event;
	AGI_EVENT *agi_event;
	
	agi_event = 0;
	
	while (  (SDL_PollEvent(&event) != 0) && (agi_event == 0)  )
	{
		switch (event.type)
		{
			case SDL_KEYUP:	
				agi_event = event_key_up( &(event.key.keysym) );
				break;

			case SDL_KEYDOWN:
				agi_event = event_key_down( &(event.key.keysym) );
				break;
			
			case SDL_USEREVENT:
				agi_event = user_event_decode( event.user.data1 );
				break;
			
			case SDL_MOUSEBUTTONDOWN:
				agi_event = event_mouse_button(event.button.button,event.button.x,event.button.y);
				break;
			
			default:
		}
	}

	return agi_event;
}

// clear input
void events_clear()
{
	SDL_Event event; 
	int one_count = 0;
	int x;
	#warning perhaps another time we could check for non-input events and use them
	#warning doesnt matter for the time being since i dont check for them anyways
	
	while ( (x=SDL_PollEvent(&event)) != 0)
	{
		if ( x == 1 )
			one_count++;
		if (one_count > 10)
		{
			#warning this is because sometimes poll_event always returns 1 event left
			printf("events_clear(): ONE EVENT LEFT ERROR!");
			break;
		}
	}
	
	#warning a bit missing for joystick
	/*
	clear_bios_buff;
	reset_joy
	clear_key_queue;
	*/
	
}


// it's to convert joystick buttons to their equivalant keys
void joy_button_map(AGI_EVENT *agi_event)
{
	u16 di;
	if (agi_event->type == 1)
	{
		di = agi_event->data;
		if ( (di==0x101) || (di==0x301) )
		{
			agi_event->data = 0x0D;	// enter
		}
		else if ( (di==0x201) || (di==0x401) )
		{
			agi_event->data = 0x1B;	// esc
		}
	}
}

// it's to convert joystick buttons to their equivalant keys
void mouse_button_map(AGI_EVENT *agi_event)
{
	u16 di;
	if (agi_event->type == 10)
	{
		if (agi_event->data == 1)
		{
			agi_event->data = 0x0D;
			agi_event->type = 1;
		}
		else if (agi_event->data == 2)
		{
			agi_event->data = 0x1B;
			agi_event->type = 1;
		}
	}
}

u16 char_poll()
{
	AGI_EVENT *agi_event;
	u16 si, di;

	agi_event = event_read();
	if (agi_event == 0)
		return 0;

	joy_button_map(agi_event);
	mouse_button_map(agi_event);

	di = agi_event->type;
	si = agi_event->data;
	
	if ((di==4)||(di==5))
	{
		di = 1;
		si = 0x0D;	// CR
	}
	if (di == 1)
		return si;
	else
		return 0xFFFF;
}

u16 char_wait()
{
	u16 si;
	do
	{
		si = char_poll();
		if  (  (si == 0) || (si == 0xFFFF)  )
			SDL_Delay(10);
	} while (  (si == 0) || (si == 0xFFFF)  );
	return si;
}

u16 has_user_reply()
{
	switch (char_poll())
	{
		case 0x0D:		// Enter
			return 1;
			break;
		
		case 0x1B:		// ESC
			return 0;
			break;
		
		default:
			return 0xFFFF;
	}
}

// wanky name for "wait for user to press enter or esc"
u16 user_bolean_poll()
{
	u16 di;
	
	events_clear();
	
	while (  (di=has_user_reply()) == 0xFFFF  )
		SDL_Delay(10);
		
	return di;
}


u8 *cmd_unknown_173(u8 *c)
{
	state.walk_mode = 1;
	return c;
}

u8 *cmd_unknown_181(u8 *c)
{
	state.walk_mode = 0;
	return c;
}

// event_wait.. waits for an event to happen.. if none.. it forces the joystick
// to make a direction update
AGI_EVENT *event_wait()
{
	AGI_EVENT *si;
	#warning SHEDLOADS MISSING FOR JOYSTICK SUPPORT
	do
	{
		si = event_read();
		if ( si == 0)
			SDL_Delay(10);
	} while (  (si == 0)  );
	return si;
}


