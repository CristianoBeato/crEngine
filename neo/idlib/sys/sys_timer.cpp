
#include "precompiled.h"
#include "sys_timer.h"

#include <SDL3/SDL_timer.h>

/*
==============
Sys_Sleep
==============
*/
void Sys_Sleep( const uint32_t in_msec ) 
{
	SDL_Delay( in_msec );
}


/*
================
Sys_Milliseconds
================
*/
uint32_t Sys_Milliseconds( void )
{
	return SDL_GetTicks();
}

/*
========================
Sys_Microseconds
========================
*/
uint64_t Sys_Microseconds( void )
{
	static uint64_t baseCounter = 0;
	static uint64_t frequency = 0;

	// init the timer 
	if ( frequency == 0)
	{
		frequency = SDL_GetPerformanceFrequency();
    	baseCounter = SDL_GetPerformanceCounter();
	}
	
	return ( ( SDL_GetPerformanceCounter() - baseCounter ) * 1000000ULL) / frequency;
}