
#include "precompiled.h"
#include "sys_public.h"
#include "sys_local.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

idCVar sys_viewlog( "sys_viewlog", "0", CVAR_SYSTEM | CVAR_INTEGER, "" );

void ToolsFrame( void )
{
    
}

int main( int argc, char *argv[] )
{
    idStr cmdLine;

    if( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO ) )
    {
		Sys_Error( SDL_GetError() );
        return EXIT_FAILURE;    
    }

    for ( int i = 0; i < argc; i++)
    {
        // store command line arguments
        cmdLine += idStr( argv[i] );
    }

    crConsole::Get()->Startup();
    crPlatform::Get()->Init( cmdLine );

    // Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
	
	common->Init( argc, argv, nullptr );

	// hide or show the early console as necessary
	if ( sys_viewlog.GetBool() )
		crConsole::Get()->ShowConsole( 1, true );
	else
		crConsole::Get()->ShowConsole( 0, false );

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

