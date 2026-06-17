#include "precompiled.h"
#include "sys/sys_local.h"
#include "sys/sys_public.h"

#include <SDL3/SDL_process.h>

#if __PLATFORM_WINDOWS__
#   include <errno.h>
#   include <shellapi.h>
#   include <shlobj.h>
#endif //__PLATFORM_WINDOWS__

#if __PLATFORM_LINUX__ || __PLATFORM_FBSD__
#   include "posix/posix_public.h" 
#endif //__PLATFORM_LINUX__ || __PLATFORM_FBSD__

/*
==================
idSysLocal::OpenURL
==================
*/
void idSysLocal::OpenURL( const char *url, bool doexit )
{
#if __PLATFORM_WINDOWS__
	static bool doexit_spamguard = false;
	HWND wnd;

	if (doexit_spamguard) 
    {
		common->DPrintf( "OpenURL: already in an exit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf("Open URL: %s\n", url);

	if ( !ShellExecuteA( nullptr, "open", url, nullptr, nullptr, SW_RESTORE ) ) 
    {
		common->Error( "Could not open url: '%s' ", url );
		return;
	}

	wnd = GetForegroundWindow();
	if ( wnd ) 
		ShowWindow( wnd, SW_MAXIMIZE );

	if ( doexit ) 
    {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}

#elif __PLATFORM_LINUX__ || __PLATFORM_FBSD__

    const char*	script_path;
	idFile*		script_file;
	char		cmdline[ 1024 ];
	
	static bool	quit_spamguard = false;
	
	if( quit_spamguard )
	{
		common->DPrintf( "Sys_OpenURL: already in a doexit sequence, ignoring %s\n", url );
		return;
	}
	
	common->Printf( "Open URL: %s\n", url );
	// opening an URL on *nix can mean a lot of things ..
	// just spawn a script instead of deciding for the user :-)
	
	// look in the savepath first, then in the basepath
	script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_savepath" ), "", "openurl.sh" );
	script_file = fileSystem->OpenExplicitFileRead( script_path );
	if( !script_file )
	{
		script_path = fileSystem->BuildOSPath( cvarSystem->GetCVarString( "fs_basepath" ), "", "openurl.sh" );
		script_file = fileSystem->OpenExplicitFileRead( script_path );
	}
	if( !script_file )
	{
		common->Printf( "Can't find URL script 'openurl.sh' in either savepath or basepath\n" );
		common->Printf( "OpenURL '%s' failed\n", url );
		return;
	}
	fileSystem->CloseFile( script_file );
	
	// if we are going to quit, only accept a single URL before quitting and spawning the script
	if( quit )
	{
		quit_spamguard = true;
	}
	
	common->Printf( "URL script: %s\n", script_path );
	
	// StartProcess is going to execute a system() call with that - hence the &
	idStr::snPrintf( cmdline, 1024, "%s '%s' &",  script_path, url );
	sys->StartProcess( cmdline, quit );

#endif // !__PLATFORM_LINUX__ !__PLATFORM_FBSD__
}

/*
==================
idSysLocal::StartProcess
if !quit, start the process asap
otherwise, push it for execution at exit
(i.e. let complete shutdown of the game and freeing of resources happen)
NOTE: might even want to add a small delay?
==================
*/
void idSysLocal::StartProcess( const char* exeName, bool quit )
{
#if __PLATFORM_WINDOWS__
	char				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strncpy( szPathOrig, exeName, _MAX_PATH );

	if( !CreateProcessA( nullptr, szPathOrig, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi ) ) 
    {
        common->Error( "Could not start process: '%s' ", szPathOrig );
	    return;
	}

	if ( quit )
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	
#else
	if( quit )
	{
		common->DPrintf( "Sys_StartProcess %s (delaying until final exit)\n", exeName );
		Posix_SetExitSpawn( exeName );
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
		return;
	}
	
	common->DPrintf( "Sys_StartProcess %s\n", exeName );
	Sys_DoStartProcess( exeName );
#endif // !__PLATFORM_LINUX__ !__PLATFORM_FBSD__
}

/*
========================
Sys_ReLaunch
========================
*/

// motorsep 12-28-2014; reverted back to the original Sys_ReLaunch; guys from RBDoom 3 BFG team made it impossible to pass any cmds on restart
void Sys_ReLaunch( void * data, const unsigned int dataSize ) 
{
#if __PLATFORM_WINDOWS__
	char                szPathOrig[MAX_PRINT_MSG];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	/*
	// DG: we don't have function arguments in Sys_ReLaunch() anymore, everyone only passed
	//     the command-line +" +set com_skipIntroVideos 1" anyway and it was painful on POSIX systems
	//     so let's just add it here.
	idStr cmdLine = Sys_GetCmdLine();
	if( cmdLine.Find( "com_skipIntroVideos" ) < 0 )
	{
		cmdLine.Append( " +set com_skipIntroVideos 1" );
	}

	strcpy( szPathOrig, va( "\"%s\" %s", Sys_EXEPath(), cmdLine.c_str() ) );
	// DG end
	*/

	strcpy(szPathOrig, va("\"%s\" %s", Sys_EXEPath(), (const char *)data));

	if ( !CreateProcessA( nullptr, szPathOrig, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi ) ) {
		idLib::Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}
	cmdSystem->AppendCommandText( "quit\n" );

#elif __PLATFORM_LINUX__ || __PLATFORM_FBSD__
    // DG: implementing this... basic old fork() exec() (+ setsid()) routine..
	// NOTE: this function used to have parameters: the commandline arguments, but as one string..
	//       for Linux/Unix we want one char* per argument so we'll just add the friggin'
	//       " +set com_skipIntroVideos 1" to the other commandline arguments in this function.
	
	int ret = fork();
	if( ret < 0 )
		idLib::Error( "Sys_ReLaunch(): Couldn't fork(), reason: %s ", strerror( errno ) );
		
	if( ret == 0 )
	{
		// child process
		
		// get our own session so we don't depend on the (soon to be killed)
		// parent process anymore - else we'll freeze
		pid_t sId = setsid();
		if( sId == ( pid_t ) - 1 )
		{
			idLib::Error( "Sys_ReLaunch(): setsid() failed! Reason: %s ", strerror( errno ) );
		}
		
		// close all FDs (except for stdin/out/err) so we don't leak FDs
		DIR* devfd = opendir( "/dev/fd" );
		if( devfd != nullptr )
		{
			struct dirent entry;
			struct dirent* result;
			while( readdir_r( devfd, &entry, &result ) == 0 )
			{
				const char* filename = result->d_name;
				char* endptr = nullptr;
				long int fd = strtol( filename, &endptr, 0 );
				if( endptr != filename && fd > STDERR_FILENO )
					close( fd );
			}
		}
		else
		{
			idLib::Warning( "Sys_ReLaunch(): Couldn't open /dev/fd/ - will leak file descriptors. Reason: %s", strerror( errno ) );
		}
		
		// + 3 because "+set" "com_skipIntroVideos" "1" - and note that while we'll skip
		// one (the first) cmdargv argument, we need one more pointer for nullptr at the end.
		int argc = cmdargc + 3;
		const char** argv = ( const char** )calloc( argc, sizeof( char* ) );
		
		int i;
		for( i = 0; i < cmdargc - 1; ++i )
			argv[i] = cmdargv[i + 1]; // ignore cmdargv[0] == executable name
			
		// add +set com_skipIntroVideos 1
		argv[i++] = "+set";
		argv[i++] = "com_skipIntroVideos";
		argv[i++] = "1";
		// execv expects nullptr terminated array
		argv[i] = nullptr;
		
		const char* exepath = Sys_EXEPath();
		
		errno = 0;
		execv( exepath, ( char** )argv );
		// we only get here if execv() fails, else the executable is restarted
		idLib::Error( "Sys_ReLaunch(): WTF exec() failed! Reason: %s ", strerror( errno ) );
		
	}
	else
	{
		// original process
		// just do a clean shutdown
		cmdSystem->AppendCommandText( "quit\n" );
	}
	// DG end
#endif
}
