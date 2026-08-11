
#include "precompiled.h"
#include "sys_public.h"
#include "sys_local.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static struct sys_main
{
    idList<idStr>       argVec;
    idStr               cmdLine;
} sysMain;

/*
 ==================
 Sys_DoPreferences
 ==================
 */
void Sys_DoPreferences( void ) 
{
}

/*
========================
Sys_GetCmdLine
========================
*/
const char* Sys_GetCmdLine( void )
{
	// DG: don't use this, use cmdargv and cmdargc instead!
	return "TODO Sys_GetCmdLine";
}

/*
================
Sys_GetExeLaunchMemoryStatus
================
*/
void Sys_GetExeLaunchMemoryStatus( sysMemoryStats_t &stats ) 
{
	stats = sysMain.exeLaunchMemoryStats;
}


void ToolsFrame( void )
{
    
}

int main( int argc, char *argv[] )
{
    if( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO ) )
    {
		Sys_Error( SDL_GetError() );
        return EXIT_FAILURE;    
    }

    for ( int i = 0; i < argc; i++)
    {
        // store command line arguments
        sysMain.argVec.Append( argv[i] );
        sysMain.cmdLine += idStr( argv[i] );
    }

    Sys_GetCurrentMemoryStatus( sysMain.exeLaunchMemoryStats );

    //	Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
	Sys_FPU_SetPrecision( FPU_PRECISION_DOUBLE_EXTENDED );

#if __PLATFORM_LINUX__ || __PLATFORM_FBSD__
#ifdef ID_MCHECK
	// must have -lmcheck linkage
	mcheck( abrt_func );
	Sys_Printf( "memory consistency checking enabled\n" );
#endif

	Posix_EarlyInit( );
#endif

	common->Init( argc, argv, nullptr );

#if __PLATFORM_WINDOWS__
	// hide or show the early console as necessary
	if ( win_viewlog.GetBool() )
		Sys_ShowConsole( 1, true );
	else
		Sys_ShowConsole( 0, false );

#if defined( SET_THREAD_AFFINITY ) 
	// give the main thread an affinity for the first cpu
	SetThreadAffinityMask( GetCurrentThread(), 1 );
#endif //SET_THREAD_AFFINITY

#else //!__PLATFORM_WINDOWS__
    Posix_LateInit( );
#endif 

    while ( true )
    {
        // debug clien frame
        DebuggerClientUpdate();

		// run the game
		common->Frame();
    }
    
    // we never get here
    return EXIT_FAILURE;
}

