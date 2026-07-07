
#include "Windows_platform.hpp"

#include <windows.h>

// Unique identifier for your application (change to your project name)
// Avoid spaces or special characters in the Windows Mutex name.
#define APP_UNIQUE_ID "crEngine_Unique_Instance_ID_000"
static HANDLE s_instanceLock = nullptr;

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

void crWindowsPlatform::StartUp(void)
{
    CoInitialize( nullptr ); // TODO: Move to Xaudio 
}

void crWindowsPlatform::ShutDown(void)
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
    
}

bool crWindowsPlatform::CreateInstanceLock(void)
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

	strcpy(szPathOrig, va("\"%s\" %s", Sys_EXEPath(), (const char *)data));

	CloseHandle( hProcessMutex );

	if ( !CreateProcess( NULL, szPathOrig, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) 
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
