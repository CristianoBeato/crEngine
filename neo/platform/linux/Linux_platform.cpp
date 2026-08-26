
#include "Linux_platform.hpp"
#include "../platform.hpp"

#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>

static const char** cmdargv = nullptr;
static int cmdargc = 0;
static int s_instanceLock = 0; 

// Unique identifier for your application (change to your project name)
// Avoid spaces or special characters in the Windows Mutex name.
#define APP_UNIQUE_ID "crEngine_Unique_Instance_ID_000"

crPlatform *crPlatform::Get(void)
{
    static crLinuxPlatform gLinuxPlatform = crLinuxPlatform();
    return &gLinuxPlatform;
}

crConsole *crConsole::Get(void)
{
	static crLinuxConsole gLinuxConsole = crLinuxConsole();
    return &gLinuxConsole;
}

crLinuxConsole::crLinuxConsole( void )
{
}

crLinuxConsole::~crLinuxConsole( void )
{
}

void crLinuxConsole::StartUp(void)
{
}

void crLinuxConsole::ShutDown(void)
{
}

void crLinuxConsole::VPrintf(const char *fmt, va_list arg)
{
	TtyHide();
	vprintf( fmt, arg );
	TtyShow();
}
void crLinuxPlatform::ShutDown(void)
{
	// Release firt instance lock file
	if( s_instanceLock > 0 )
	{
		close( s_instanceLock );
		s_instanceLock = 0;
	}
}

void crLinuxPlatform::Exit( const int code )
{
	Posix_ConsoleExit();
	
	// at this point, too late to catch signals
	ClearSigs();
	
	// process spawning. it's best when it happens after everything has shut down
	if( m_exitSpawn[0] )
		Sys_DoStartProcess( m_exitSpawn, false );
	
	// in case of signal, handler tries a common->Quit
	// we use set_exit to maintain a correct exit code
	if( m_setExit )
		exit( m_setExit );

	exit( code );
}

bool crLinuxPlatform::AlreadyRunning(void)
{
    // Path to the lock file in the Linux/Unix temporary directory
	// The "/tmp/" prefix ensures that the file is visible to all local users
	const char* lockFile = "/tmp/" APP_UNIQUE_ID ".lock";

	// Opens or creates the file with read/write permissions
    s_instanceLock = open( lockFile, O_CREAT | O_RDWR, 0666 );
    if ( s_instanceLock < 0 ) 
        return false; // The lock file could not be created.

	// Attempts to apply a non-blocking exclusive lock ( LOCK_EX | LOCK_NB )
    if ( flock( s_instanceLock, LOCK_EX | LOCK_NB) < 0 ) 
	{
        // If it fails, it means that another instance has already locked this file.
        close( s_instanceLock );
        return true;
    }
}

/*
========================
crLinuxPlatform::LockMemory
========================
*/
bool crLinuxPlatform::LockMemory(void *ptr, const size_t bytes)
{
    return mlock( ptr, bytes ) == 0;
}

/*
========================
crLinuxPlatform::UnlockMemory
========================
*/
bool crLinuxPlatform::UnlockMemory(void *ptr, const size_t bytes)
{
    return munlock( ptr, bytes ) == 0;
}

/*
========================
crLinuxPlatform::ReLaunch
========================
*/
void crLinuxPlatform::ReLaunch(void *data, const size_t dataSize)
{
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
		if( devfd != NULL )
		{
			struct dirent entry;
			struct dirent* result;
			while( readdir_r( devfd, &entry, &result ) == 0 )
			{
				const char* filename = result->d_name;
				char* endptr = NULL;
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
		// one (the first) cmdargv argument, we need one more pointer for NULL at the end.
		int argc = cmdargc + 3;
		const char** argv = ( const char** )calloc( argc, sizeof( char* ) );
		
		int i;
		for( i = 0; i < cmdargc - 1; ++i )
			argv[i] = cmdargv[i + 1]; // ignore cmdargv[0] == executable name
			
		// add +set com_skipIntroVideos 1
		argv[i++] = "+set";
		argv[i++] = "com_skipIntroVideos";
		argv[i++] = "1";
		// execv expects NULL terminated array
		argv[i] = NULL;
		
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
}

/*
==================
crLinuxPlatform::StartProcess
if we don't fork, this function never returns
the no-fork lets you keep the terminal when you're about to spawn an installer

if the command contains spaces, system() is used. Otherwise the more straightforward execl ( system() blows though )
==================
*/
void crLinuxPlatform::StartProcess(const char *exeName, const bool dofork )
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
				printf( "cmod +x %s failed: %s\n", exeName, strerror( errno ) );
			
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
					execl( exeName, exeName, NULL );
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
			execl( exeName, exeName, NULL );
			printf( "execl failed: %s\n", strerror( errno ) );
		}

		// terminate
		_exit( 0 );
	}
}

/*
========================
crLinuxPlatform::OpenURL
========================
*/
void crLinuxPlatform::OpenURL(const char *url, const bool doexit)
{
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
		if( devfd != NULL )
		{
			struct dirent entry;
			struct dirent* result;
			while( readdir_r( devfd, &entry, &result ) == 0 )
			{
				const char* filename = result->d_name;
				char* endptr = NULL;
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
		// one (the first) cmdargv argument, we need one more pointer for NULL at the end.
		int argc = cmdargc + 3;
		const char** argv = ( const char** )calloc( argc, sizeof( char* ) );
		
		int i;
		for( i = 0; i < cmdargc - 1; ++i )
			argv[i] = cmdargv[i + 1]; // ignore cmdargv[0] == executable name
			
		// add +set com_skipIntroVideos 1
		argv[i++] = "+set";
		argv[i++] = "com_skipIntroVideos";
		argv[i++] = "1";
		// execv expects NULL terminated array
		argv[i] = NULL;
		
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
}

void crLinuxPlatform::GetCurrentMemoryStatus(sysMemoryStats_t &stats)
{
	/// read linux memory info file
	FILE *f = fopen("/proc/meminfo", "r");

	/// can't reade file, set zero 
    if (!f) 
    {
        stats.totalPhysical = stats.availPhysical = 0;
        stats.totalVirtual = stats.availVirtual = 0;
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), f)) 
    {
        if ( sscanf(buffer, "MemTotal: %d kB", &stats.totalPhysical) == 1) 
			continue;

        if ( sscanf(buffer, "MemAvailable: %d kB", &stats.availPhysical) == 1) 
			continue;

        if ( sscanf(buffer, "SwapTotal: %d kB", &stats.totalVirtual) == 1) 
			continue;

        if ( sscanf(buffer, "SwapFree: %d kB", &stats.availVirtual) == 1) 
			continue;
    }
    fclose(f);

    // convert from kB to bytes
    stats.totalPhysical *= 1024;
    stats.availPhysical *= 1024;
    stats.totalVirtual *= 1024;
    stats.availVirtual  *= 1024;
}

void crLinuxPlatform::GetExeLaunchMemoryStatus(sysMemoryStats_t &stats)
{
	stats = exeLaunchMemoryStats;
}

const char *crLinuxPlatform::GetCurrentUser(void)
{
	static const uint32_t NAME_LEN = 256;
	static char usrname[NAME_LEN] = {0};

	/// Retrieve current user id and name.
	if( usrname[0] == '\0' )
	{
		uid_t uid = getuid();
        struct passwd* pw = getpwuid( uid );
        if ( pw && pw->pw_name )
        {
            std::strncpy( usrname, pw->pw_name, NAME_LEN - 1 );
            usrname[NAME_LEN - 1] = '\0';
        }
        else
        {
            return "current_user"; // Failed to retrieve the name
        }
	}

	return usrname;
}

/*
===============
crLinuxPlatform::SetExitSpawn
set the process to be spawned when we quit
===============
*/
void crLinuxPlatform::SetExitSpawn( const char *exeName )
{
	idStr::Copynz( m_exitSpawn, exeName, 1024 );
}

/*
================
Posix_ClearSigs
================
*/
void crLinuxPlatform::ClearSigs( void )
{
	struct sigaction action;
	int i;
	
	// Set up the structure
	action.sa_handler = SIG_DFL;
	sigemptyset( &action.sa_mask );
	action.sa_flags = 0;
	
	i = 0;
	while( siglist[ i ] != -1 )
	{
		if( sigaction( siglist[ i ], &action, nullptr ) != 0 )
		{
			Sys_Printf( "Failed to reset %s handler: %s\n", signames[ i ], strerror( errno ) );
		}
		i++;
	}
}
