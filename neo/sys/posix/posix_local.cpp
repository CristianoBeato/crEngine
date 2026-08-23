/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#include "precompiled.h"
#include "sys/sys_local.h"

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>


#include <fnmatch.h>

// RB begin
#include <sys/statvfs.h>
// RB end
// DG: needed for Sys_ReLaunch()
#include <dirent.h>

static const char** cmdargv = nullptr;
static int cmdargc = 0;
// DG end

#include "posix_public.h"

#define					MAX_OSPATH 256

// pid - useful when you attach to gdb..
idCVar com_pid( "com_pid", "0", CVAR_INTEGER | CVAR_INIT | CVAR_SYSTEM, "process id" );

// exit - quit - error --------------------------------------------------------

static int set_exit = 0;
static char exit_spawn[ 1024 ];

/*
==================
Sys_DoStartProcess
if we don't fork, this function never returns
the no-fork lets you keep the terminal when you're about to spawn an installer

if the command contains spaces, system() is used. Otherwise the more straightforward execl ( system() blows though )
==================
*/
void Sys_DoStartProcess( const char* exeName, bool dofork )
{
	bool use_system = false;
	if( strchr( exeName, ' ' ) )
	{
		use_system = true;
	}
	else
	{
		// set exec rights when it's about a single file to execute
		struct stat buf;
		if( stat( exeName, &buf ) == -1 )
		{
			printf( "stat %s failed: %s\n", exeName, strerror( errno ) );
		}
		else
		{
			if( chmod( exeName, buf.st_mode | S_IXUSR ) == -1 )
			{
				printf( "cmod +x %s failed: %s\n", exeName, strerror( errno ) );
			}
		}
	}
	if( dofork )
	{
		switch( fork() )
		{
			case -1:
				// main thread
				break;
			case 0:
				if( use_system )
				{
					printf( "system %s\n", exeName );
					system( exeName );
					_exit( 0 );
				}
				else
				{
					printf( "execl %s\n", exeName );
					execl( exeName, exeName, nullptr );
					printf( "execl failed: %s\n", strerror( errno ) );
					_exit( -1 );
				}
				break;
		}
	}
	else
	{
		if( use_system )
		{
			printf( "system %s\n", exeName );
			system( exeName );
			sleep( 1 );	// on some systems I've seen that starting the new process and exiting this one should not be too close
		}
		else
		{
			printf( "execl %s\n", exeName );
			execl( exeName, exeName, nullptr );
			printf( "execl failed: %s\n", strerror( errno ) );
		}
		// terminate
		_exit( 0 );
	}
}

/*
================
Posix_Exit
================
*/
void Posix_Exit( int ret )
{
	Posix_ConsoleExit();
	
	// at this point, too late to catch signals
	Posix_ClearSigs();
	
	//if( asyncThread.threadHandle )
	//{
	//	Sys_DestroyThread( asyncThread );
	//}
	
	// process spawning. it's best when it happens after everything has shut down
	if( exit_spawn[0] )
		Sys_DoStartProcess( exit_spawn, false );
	
	// in case of signal, handler tries a common->Quit
	// we use set_exit to maintain a correct exit code
	if( set_exit )
		exit( set_exit );

	exit( ret );
}

/*
================
Posix_SetExit
================
*/
void Posix_SetExit( int ret )
{
	set_exit = 0;
}

/*
===============
Posix_SetExitSpawn
set the process to be spawned when we quit
===============
*/
void Posix_SetExitSpawn( const char* exeName )
{
	idStr::Copynz( exit_spawn, exeName, 1024 );
}

/*
=================
Posix_Shutdown
=================
*/
void Posix_Shutdown()
{
	
}

// RB end

/*
===============
Posix_EarlyInit
===============
*/
void Posix_EarlyInit()
{
	//std::memset( &asyncThread, 0, sizeof( asyncThread ) );
	
	exit_spawn[0] = '\0';
	Posix_InitSigs();
	
	// set the base time
	Sys_Milliseconds();	
}

/*
===============
Posix_LateInit
===============
*/
void Posix_LateInit()
{
	Posix_InitConsoleInput();
	com_pid.SetInteger( getpid() );
	common->Printf( "pid: %d\n", com_pid.GetInteger() );
	common->Printf( "%d MB System Memory\n", Sys_GetSystemRam() );
	
//#ifndef ID_DEDICATED
	//common->Printf( "%d MB Video Memory\n", Sys_GetVideoRam() );
//#endif

	//Posix_StartAsyncThread( );
}

/*
================
Sys_SetLanguageFromSystem
================
*/
extern idCVar sys_lang;
void Sys_SetLanguageFromSystem()
{
	sys_lang.SetString( Sys_DefaultLanguage() );
}
