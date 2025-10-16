#include "precompiled.h"
#include "sys/sys_local.h"
#include "sys/sys_public.h"

#include <SDL3/SDL_process.h>


#if __PLATFORM_WINDOWS__
    #include <windows.h>
    static HANDLE g_singletonMutex = NULL;
#else
    #include <sys/file.h>
    #include <unistd.h>
    #include <fcntl.h>
    static int g_lockFile = -1;
#endif

idCVar sys_allowMultipleInstances( "sys_allowMultipleInstances", "0", CVAR_SYSTEM | CVAR_BOOL, "allow multiple instances running concurrently" );

bool Sys_AlreadyRunning( void ) 
{
	// if we allow open multiple instances we always return true
	if ( sys_allowMultipleInstances.GetBool() )
		return true; 

#if __PLATFORM_WINDOWS__

#elif __PLATFORM_LINUX__
#else
	return false;
#endif 
}


void Sys_ReleaseAlreadyRunningLock( void ) 
{
#if defined(_WIN32)
    if (g_singletonMutex) {
        CloseHandle(g_singletonMutex);
        g_singletonMutex = NULL;
    }
#else
    if (g_lockFile >= 0) {
        flock(g_lockFile, LOCK_UN);
        close(g_lockFile);
        g_lockFile = -1;
    }
#endif
}