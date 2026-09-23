
#include "Windows_platform.hpp"

#include <shellapi.h>

// Unique identifier for your application (change to your project name)
// Avoid spaces or special characters in the Windows Mutex name.
#define APP_UNIQUE_ID "crEngine_Unique_Instance_ID_000"
static HANDLE s_instanceLock = nullptr;

idCVar sys_cpustring( "sys_cpustring", "detect", CVAR_SYSTEM | CVAR_INIT, "" );

crPlatform *crPlatform::Get(void)
{
    static crWindowsPlatform gWindowsPlatform = crWindowsPlatform();
    return &gWindowsPlatform;
}

crWindowsPlatform::crWindowsPlatform( void )
{
}

crWindowsPlatform::~crWindowsPlatform( void )
{
}

void crWindowsPlatform::Init(void)
{
	uint32_t cpuid = 0;
    CoInitialize( nullptr ); // TODO: Move to Xaudio

	//
	// Windows user name
	//
	// win_username.SetString( Sys_GetCurrentUser() );


	//
	// CPU type
	//
	if ( !idStr::Icmp( sys_cpustring.GetString(), "detect" ) ) {
		idStr string;

		//common->Printf( "%1.0f MHz ", Sys_ClockTicksPerSecond() / 1000000.0f );

		auto cpuid = crCPUInfo::Get()->GetProcessorId();

		string.Clear();

		if ( cpuid & crCPUInfo::CPUID_AMD ) 
		{
			string += "AMD CPU";
		} 
		else if ( cpuid & crCPUInfo::CPUID_INTEL ) 
		{
			string += "Intel CPU";
		} 
		else if ( cpuid & crCPUInfo::CPUID_UNSUPPORTED ) 
		{
			string += "unsupported CPU";
		} 
		else 
		{
			string += "generic CPU";
		}

		string += " with ";
		if ( cpuid & crCPUInfo::CPUID_MMX ) 
			string += "MMX & ";

		if ( cpuid & crCPUInfo::CPUID_SSE ) 
			string += "SSE & ";
		
		if ( cpuid & crCPUInfo::CPUID_SSE2 ) 
            string += "SSE2 & ";
		
		if ( cpuid & crCPUInfo::CPUID_SSE3 ) 
			string += "SSE3 & ";
		
		string.StripTrailing( " & " );
		string.StripTrailing( " with " );
		sys_cpustring.SetString( string );
	} 

	common->Printf( "%s\n", sys_cpustring.GetString() );
	common->Printf( "%d MB System Memory\n", crCPUInfo::Get()->GetSystemRam() );
	//common->Printf( "%d MB Video Memory\n", GetVideoRam() );
	if ( ( cpuid & crCPUInfo::CPUID_SSE2 ) == 0 ) 
	{
		common->Error( "SSE2 not supported!" );
	}

	//g_Joystick.Init();
}

void crWindowsPlatform::Shutdown(void)
{
    // Release firt instance global mutex
	if( s_instanceLock == nullptr )
	{
		CloseHandle( s_instanceLock );
		s_instanceLock = nullptr;
	}

    CoUninitialize();
}

void crWindowsPlatform::Exit(const int code)
{
	// ExitProcess cause reasource leaks 
	exit( code ); // ExitProcess( 0 );
}

/*
==============
crWindowsPlatform::Quit
==============
*/
void crWindowsPlatform::Quit( void ) 
{
	crInputSystem::Get()->Shutdown();
	crConsole::Get()->Shutdown();
	Exit( 0 );
}

bool crWindowsPlatform::AlreadyRunning(void)
{
    // Creates a named mutex in the global scope of Windows.
	s_instanceLock = CreateMutexA( nullptr, FALSE, APP_UNIQUE_ID );

	// If the mutex already existed, it means that another instance created it first.
	if ( ::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED ) 
		return true;
}

bool crWindowsPlatform::LockMemory(void *ptr, const size_t bytes)
{
    return ( VirtualLock( ptr, ( SIZE_T )bytes ) != FALSE );
}

bool crWindowsPlatform::UnlockMemory(void *ptr, const size_t bytes)
{
    return ( VirtualUnlock( ptr, ( SIZE_T )bytes ) != FALSE );
}

void crWindowsPlatform::ReLaunch(void *data, const size_t dataSize)
{
	TCHAR				szPathOrig[MAX_PRINT_MSG];
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

	SDL_snprintf( szPathOrig, MAX_PRINT_MSG, ("\"%s\" %s", crPaths::Get()->EXEPath(), (const char *)data) );

	CloseHandle( hProcessMutex );

	if ( !CreateProcess( nullptr, szPathOrig, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi ) ) 
    {
		idLib::Error( "Could not start process: '%s' ", szPathOrig );
		return;
	}
	cmdSystem->AppendCommandText( "quit\n" );
}

/*
==================
crWindowsPlatform::StartProcess
==================
*/
void crWindowsPlatform::StartProcess(const char *exePath, const bool doexit)
{
	TCHAR				szPathOrig[_MAX_PATH];
	STARTUPINFO			si;
	PROCESS_INFORMATION	pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);

	strncpy( szPathOrig, exePath, _MAX_PATH );

	if( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) 
    {
        common->Error( "Could not start process: '%s' ", szPathOrig );
	    return;
	}

	if ( doexit ) 
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
}

/*
==================
crWindowsPlatform::OpenURL
==================
*/
void crWindowsPlatform::OpenURL(const char *url, const bool doexit)
{
	static bool doexit_spamguard = false;
	HWND wnd;

	if (doexit_spamguard) 
    {
		common->DPrintf( "OpenURL: already in an exit sequence, ignoring %s\n", url );
		return;
	}

	common->Printf("Open URL: %s\n", url);

	if ( !ShellExecute( NULL, "open", url, NULL, NULL, SW_RESTORE ) ) 
    {
		common->Error( "Could not open url: '%s' ", url );
		return;
	}

#if 0
	wnd = GetForegroundWindow();
	if ( wnd ) 
    {
		ShowWindow( wnd, SW_MAXIMIZE );
	}
#else
    crVideo::Get()->ShowWindow( true );
#endif

	if ( doexit ) 
    {
		doexit_spamguard = true;
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "quit\n" );
	}
}

/*
==================
crWindowsPlatform::GetCurrentMemoryStatus
==================
*/
void crWindowsPlatform::GetCurrentMemoryStatus(sysMemoryStats_t &stats)
{
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
}

void crWindowsPlatform::GetExeLaunchMemoryStatus(sysMemoryStats_t &stats)
{
    stats = exeLaunchMemoryStats;
}

const char *crWindowsPlatform::GetCmdLine(void)
{
    return m_cmdline;
}

const char *crWindowsPlatform::GetCurrentUser(void)
{
	static const uint32_t K_NAME_LEN = 1024;
	static char s_userName[K_NAME_LEN];
	unsigned long size = sizeof( s_userName );
	
	if( !GetUserName( s_userName, &size ) )
		std::strncpy( s_userName, "player", K_NAME_LEN );
	
	if( !s_userName[0] )
		std::strncpy( s_userName, "player", K_NAME_LEN );
	
	return s_userName;
}