
#include "precompiled.h"
#include "sys_public.h"
#include "sys_local.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if __PLATFORM_WINDOWS__
#   incldue <windows.h>
#   include "win32/win_local.h"
#elif __PLATFORM_LINUX__ || __PLATFORM_FBSD__
#   include <sys/mman.h>
#   include "posix/posix_public.h"
#endif

static struct sys_main
{
    sysMemoryStats_t    exeLaunchMemoryStats;
    idList<idStr>       argVec;
    idStr               cmdLine;
} sysMain;

/*
================
Sys_Init
The cvar system must already be setup
================
*/
void Sys_Init() 
{
#if __PLATFORM_WINDOWS__
	CoInitialize( NULL );
#endif

// BEATO Begin:
	sys->GetVideoSystem()->StartUp( 0 );
// BEATO End
}

/*
===============
Sys_Shutdown
===============
*/
void Sys_Shutdown( void )
{
	sys->GetVideoSystem()->ShutDown();

    Sys_ReleaseAlreadyRunningLock();

#if __PLATFORM_WINDOWS__
    CoUninitialize();
#else
	Posix_Shutdown();
#endif
}

/*
================
Sys_Quit
================
*/
void Sys_Quit( void )
{
#if __PLATFORM_WINDOWS__
	timeEndPeriod( 1 );
	Sys_ShutdownInput();
	Sys_DestroyConsole();
	ExitProcess( 0 );
#else
	Posix_Exit( EXIT_SUCCESS );
#endif
}

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
Sys_LockMemory
================
*/
bool Sys_LockMemory( void* ptr, int bytes )
{
#if __PLATFORM_WINDOWS__
	return ( VirtualLock( ptr, ( SIZE_T )bytes ) != FALSE );
#elif __PLATFORM_LINUX__
    return mlock(ptr, bytes) == 0;
#else
    return false;
#endif
}

/*
================
Sys_UnlockMemory
================
*/
bool Sys_UnlockMemory( void* ptr, int bytes )
{
#if __PLATFORM_WINDOWS__
	return ( VirtualUnlock( ptr, ( SIZE_T )bytes ) != FALSE );
#elif __PLATFORM_LINUX__
    return munlock( ptr, bytes ) == 0;
#else
    return false;
#endif
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


/*
================
Sys_GetCurrentMemoryStatus

	returns OS mem info
	all values are in kB except the memoryload
================
*/
void Sys_GetCurrentMemoryStatus( sysMemoryStats_t& stats )
{
#if __PLATFORM_WINDOWS__
	MEMORYSTATUSEX statex = {};
	unsigned __int64 work;
	
	statex.dwLength = sizeof( statex );
	GlobalMemoryStatusEx( &statex );
	
	memset( &stats, 0, sizeof( stats ) );
	
	stats.memoryLoad = statex.dwMemoryLoad;
	
	work = statex.ullTotalPhys >> 20;
	stats.totalPhysical = *( int* )&work;
	
	work = statex.ullAvailPhys >> 20;
	stats.availPhysical = *( int* )&work;
	
	work = statex.ullAvailPageFile >> 20;
	stats.availPageFile = *( int* )&work;
	
	work = statex.ullTotalPageFile >> 20;
	stats.totalPageFile = *( int* )&work;
	
	work = statex.ullTotalVirtual >> 20;
	stats.totalVirtual = *( int* )&work;
	
	work = statex.ullAvailVirtual >> 20;
	stats.availVirtual = *( int* )&work;
	
	work = statex.ullAvailExtendedVirtual >> 20;
	stats.availExtendedVirtual = *( int* )&work;
#else __PLATFORM_LINUX__
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) 
    {
        stats.totalPhysical = stats.availPhysical = 0;
        stats.totalVirtual = stats.availVirtual = 0;
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), f)) 
    {
        if (sscanf(buffer, "MemTotal: %d kB", &stats.totalPhysical) == 1) continue;
        if (sscanf(buffer, "MemAvailable: %d kB", &stats.availPhysical) == 1) continue;
        if (sscanf(buffer, "SwapTotal: %d kB", &stats.totalVirtual) == 1) continue;
        if (sscanf(buffer, "SwapFree: %d kB", &stats.availVirtual) == 1) continue;
    }
    fclose(f);

    // converte de kB para bytes
    stats.totalPhysical     *= 1024;
    stats.availPhysical *= 1024;
    stats.totalVirtual      *= 1024;
    stats.availVirtual  *= 1024;
#endif //__PLATFORM_LINUX__ 
}

void ToolsFrame( void )
{
    
}

int main( int argc, char *argv[] )
{
    if( !SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO ) )
    {
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
	if ( win32.win_viewlog.GetInteger() )
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

