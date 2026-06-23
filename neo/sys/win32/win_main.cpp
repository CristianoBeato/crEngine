/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#pragma hdrstop
#include "precompiled.h"

#include <errno.h>
#include <float.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <mapi.h>
#include <shellapi.h>
#include <shlobj.h>

#ifndef __MRC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "../sys_local.h"
#include "win_local.h"
#include "renderer/renderer_common.h"

#ifdef ID_ALLOW_TOOLS
#include "tools/edit_public.h"
#endif

//idCVar Win32Vars_t::sys_arch( "sys_arch", "", CVAR_SYSTEM | CVAR_INIT, "" );
//idCVar Win32Vars_t::sys_cpustring( "sys_cpustring", "detect", CVAR_SYSTEM | CVAR_INIT, "" );
//idCVar Win32Vars_t::in_mouse( "in_mouse", "1", CVAR_SYSTEM | CVAR_BOOL, "enable mouse input" );
//idCVar Win32Vars_t::win_allowAltTab( "win_allowAltTab", "0", CVAR_SYSTEM | CVAR_BOOL, "allow Alt-Tab when fullscreen" );
//idCVar Win32Vars_t::win_notaskkeys( "win_notaskkeys", "0", CVAR_SYSTEM | CVAR_INTEGER, "disable windows task keys" );
//idCVar Win32Vars_t::win_username( "win_username", "", CVAR_SYSTEM | CVAR_INIT, "windows user name" );
//idCVar Win32Vars_t::win_timerUpdate( "win_timerUpdate", "0", CVAR_SYSTEM | CVAR_BOOL, "allows the game to be updated while dragging the window" );

Win32Vars_t	win32;

bool enableToolsSupport = true;


/*
==============
Sys_DebugPrintf
==============
*/
#define MAXPRINTMSG 4096
void Sys_DebugPrintf( const char *fmt, ... ) {
	char msg[MAXPRINTMSG];

	va_list argptr;
	va_start( argptr, fmt );
	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, argptr );
	msg[ sizeof(msg)-1 ] = '\0';
	va_end( argptr );

	OutputDebugString( msg );
}

/*
==============
Sys_DebugVPrintf
==============
*/
void Sys_DebugVPrintf( const char *fmt, va_list arg ) {
	char msg[MAXPRINTMSG];

	idStr::vsnPrintf( msg, MAXPRINTMSG-1, fmt, arg );
	msg[ sizeof(msg)-1 ] = '\0';

	OutputDebugString( msg );
}

// Vista shit
typedef HRESULT (WINAPI * SHGetKnownFolderPath_t)( const GUID & rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath );
// NOTE: FOLIDERID_SavedGames is already exported from in shell32.dll in Windows 7.  We can only detect
// the compiler version, but that doesn't doesn't tell us which version of the OS we're linking against.
// This GUID value should never change, so we name it something other than FOLDERID_SavedGames to get
// around this problem.
const GUID FOLDERID_SavedGames_IdTech5 = { 0x4c5c32ff, 0xbb9d, 0x43b0, { 0xb5, 0xb4, 0x2d, 0x72, 0xe5, 0x4e, 0xaa, 0xa4 } };

/*
========================
ExecOutputFn
========================
*/
static void ExecOutputFn( const char * text ) 
{
	idLib::Printf( text );
}


/*
========================
Sys_Exec

if waitMsec is INFINITE, completely block until the process exits
If waitMsec is -1, don't wait for the process to exit
Other waitMsec values will allow the workFn to be called at those intervals.
========================
*/
bool Sys_Exec(	const char * appPath, const char * workingPath, const char * args, 
	execProcessWorkFunction_t workFn, execOutputFunction_t outputFn, const int waitMS,
	unsigned int & exitCode ) {
		exitCode = 0;
		SECURITY_ATTRIBUTES secAttr;
		secAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
		secAttr.bInheritHandle = TRUE;
		secAttr.lpSecurityDescriptor = nullptr;

		HANDLE hStdOutRead;
		HANDLE hStdOutWrite;
		CreatePipe( &hStdOutRead, &hStdOutWrite, &secAttr, 0 );
		SetHandleInformation( hStdOutRead, HANDLE_FLAG_INHERIT, 0 );

		HANDLE hStdInRead;
		HANDLE hStdInWrite;
		CreatePipe( &hStdInRead, &hStdInWrite, &secAttr, 0 );
		SetHandleInformation( hStdInWrite, HANDLE_FLAG_INHERIT, 0 );										

		STARTUPINFO si;
		std::memset( &si, 0, sizeof( si ) );
		si.cb = sizeof( si );
		si.hStdError = hStdOutWrite;
		si.hStdOutput = hStdOutWrite;
		si.hStdInput = hStdInRead;
		si.wShowWindow = FALSE;
		si.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		PROCESS_INFORMATION pi;
		std::memset ( &pi, 0, sizeof( pi ) );

		if ( outputFn != nullptr ) {
			outputFn( va( "^2Executing Process: ^7%s\n^2working path: ^7%s\n^2args: ^7%s\n", appPath, workingPath, args ) );
		} else {
			outputFn = ExecOutputFn;
		}

		// we duplicate args here so we can concatenate the exe name and args into a single command line
		const char * imageName = appPath;
		char * cmdLine = nullptr;
		{
			// if we have any args, we need to copy them to a new buffer because CreateProcess modifies
			// the command line buffer.
			if ( args != nullptr ) {
				if ( appPath != nullptr ) {
					int len = idStr::Length( args ) + idStr::Length( appPath ) + 1 /* for space */ + 1 /* for nullptr terminator */ + 2 /* app quotes */;
					cmdLine = (char*)Mem_Alloc( len, TAG_TEMP );
					// note that we're putting quotes around the appPath here because when AAS2.exe gets an app path with spaces
					// in the path "w:/zion/build/win32/Debug with Inlines/AAS2.exe" it gets more than one arg for the app name,
					// which it most certainly should not, so I am assuming this is a side effect of using CreateProcess.
					idStr::snPrintf( cmdLine, len, "\"%s\" %s", appPath, args );
				} else {
					int len = idStr::Length( args ) + 1;
					cmdLine = (char*)Mem_Alloc( len, TAG_TEMP );
					idStr::Copynz( cmdLine, args, len );
				}
				// the image name should always be nullptr if we have command line arguments because it is already
				// prefixed to the command line.
				imageName = nullptr;
			}
		}

		BOOL result = CreateProcess( imageName, (LPSTR)cmdLine, nullptr, nullptr, TRUE, 0, nullptr, workingPath, &si, &pi );

		if ( result == FALSE ) {
			TCHAR szBuf[1024]; 
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError(); 

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM,
				nullptr,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, nullptr );

			// foresthale 2014-05-19: /analyze warned about passing LPVOID here, so cast it
			wsprintf( szBuf, "%d: %s", dw, (LPTSTR)lpMsgBuf );
			if ( outputFn != nullptr ) {
				outputFn( szBuf );
			}
			LocalFree( lpMsgBuf );
			if ( cmdLine != nullptr ) {
				Mem_Free( cmdLine );
			}
			return false;
		} else if ( waitMS >= 0 ) {	// if waitMS == -1, don't wait for process to exit
			DWORD ec = 0;
			DWORD wait = 0;
			char buffer[ 4096 ];
			for ( ; ; ) {
				wait = WaitForSingleObject( pi.hProcess, waitMS );
				GetExitCodeProcess( pi.hProcess, &ec );

				DWORD bytesRead = 0;
				DWORD bytesAvail = 0;
				DWORD bytesLeft = 0;
				BOOL ok = PeekNamedPipe( hStdOutRead, nullptr, 0, nullptr, &bytesAvail, &bytesLeft );
				if ( ok && bytesAvail != 0 ) {
					ok = ReadFile( hStdOutRead, buffer, sizeof( buffer ) - 3, &bytesRead, nullptr );
					if ( ok && bytesRead > 0 ) {
						buffer[ bytesRead ] = '\0';
						if ( outputFn != nullptr ) {
							int length = 0;
							for ( int i = 0; buffer[i] != '\0'; i++ ) {
								if ( buffer[i] != '\r' ) {
									buffer[length++] = buffer[i];
								}
							}
							buffer[length++] = '\0';
							outputFn( buffer );
						}
					}
				}

				if ( ec != STILL_ACTIVE ) {
					exitCode = ec;
					break;
				}

				if ( workFn != nullptr ) {
					if ( !workFn() ) {
						TerminateProcess( pi.hProcess, 0 );
						break;
					}
				}
			}
		}

		// this assumes that windows duplicates the command line string into the created process's
		// environment space.
		if ( cmdLine != nullptr ) {
			Mem_Free( cmdLine );
		}

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		CloseHandle( hStdOutRead );
		CloseHandle( hStdOutWrite );
		CloseHandle( hStdInRead );
		CloseHandle( hStdInWrite );
		return true;
}

//=======================================================================

//#define SET_THREAD_AFFINITY


/*
====================
GetExceptionCodeInfo
====================
*/
const char *GetExceptionCodeInfo( UINT code ) 
{
	switch( code ) {
		case EXCEPTION_ACCESS_VIOLATION: return "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.";
		case EXCEPTION_BREAKPOINT: return "A breakpoint was encountered.";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.";
		case EXCEPTION_FLT_DENORMAL_OPERAND: return "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "The thread tried to divide a floating-point value by a floating-point divisor of zero.";
		case EXCEPTION_FLT_INEXACT_RESULT: return "The result of a floating-point operation cannot be represented exactly as a decimal fraction.";
		case EXCEPTION_FLT_INVALID_OPERATION: return "This exception represents any floating-point exception not included in this list.";
		case EXCEPTION_FLT_OVERFLOW: return "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.";
		case EXCEPTION_FLT_STACK_CHECK: return "The stack overflowed or underflowed as the result of a floating-point operation.";
		case EXCEPTION_FLT_UNDERFLOW: return "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "The thread tried to execute an invalid instruction.";
		case EXCEPTION_IN_PAGE_ERROR: return "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "The thread tried to divide an integer value by an integer divisor of zero.";
		case EXCEPTION_INT_OVERFLOW: return "The result of an integer operation caused a carry out of the most significant bit of the result.";
		case EXCEPTION_INVALID_DISPOSITION: return "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "The thread tried to continue execution after a noncontinuable exception occurred.";
		case EXCEPTION_PRIV_INSTRUCTION: return "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.";
		case EXCEPTION_SINGLE_STEP: return "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.";
		case EXCEPTION_STACK_OVERFLOW: return "The thread used up its stack.";
		default: return "Unknown exception";
	}
}

/*
====================
EmailCrashReport

  emailer originally from Raven/Quake 4
====================
*/
void EmailCrashReport( LPSTR messageText ) 
{
	static int lastEmailTime = 0;

	if ( Sys_Milliseconds() < lastEmailTime + 10000 ) {
		return;
	}

	lastEmailTime = Sys_Milliseconds();

	HINSTANCE mapi = LoadLibrary( "MAPI32.DLL" ); 
	if( mapi ) {
		LPMAPISENDMAIL	MAPISendMail = ( LPMAPISENDMAIL )GetProcAddress( mapi, "MAPISendMail" );
		if( MAPISendMail ) {
			MapiRecipDesc toProgrammers =
			{
				0,										// ulReserved
					MAPI_TO,							// ulRecipClass
					"DOOM 3 Crash",						// lpszName
					"SMTP:programmers@idsoftware.com",	// lpszAddress
					0,									// ulEIDSize
					0									// lpEntry
			};

			MapiMessage		message = {};
			message.lpszSubject = "DOOM 3 Fatal Error";
			message.lpszNoteText = messageText;
			message.nRecipCount = 1;
			message.lpRecips = &toProgrammers;

			MAPISendMail(
				0,									// LHANDLE lhSession
				0,									// ULONG ulUIParam
				&message,							// lpMapiMessage lpMessage
				MAPI_DIALOG,						// FLAGS flFlags
				0									// ULONG ulReserved
				);
		}
		FreeLibrary( mapi );
	}
}

// RB: disabled unused FPU exception debugging
#if !defined(__MINGW32__) && !defined(_WIN64)

/*
====================
_except_handler
====================
*/
EXCEPTION_DISPOSITION __cdecl _except_handler( struct _EXCEPTION_RECORD *ExceptionRecord, void * EstablisherFrame,
												struct _CONTEXT *ContextRecord, void * DispatcherContext ) {

	static char msg[ 8192 ];
	char FPUFlags[2048];

	Sys_FPU_PrintStateFlags( FPUFlags, ContextRecord->FloatSave.ControlWord,
										ContextRecord->FloatSave.StatusWord,
										ContextRecord->FloatSave.TagWord,
										ContextRecord->FloatSave.ErrorOffset,
										ContextRecord->FloatSave.ErrorSelector,
										ContextRecord->FloatSave.DataOffset,
										ContextRecord->FloatSave.DataSelector );


	sprintf( msg, 
		"Please describe what you were doing when DOOM 3 crashed!\n"
		"If this text did not pop into your email client please copy and email it to programmers@idsoftware.com\n"
			"\n"
			"-= FATAL EXCEPTION =-\n"
			"\n"
			"%s\n"
			"\n"
			"0x%x at address 0x%08p\n"
			"\n"
			"%s\n"
			"\n"
			"EAX = 0x%08x EBX = 0x%08x\n"
			"ECX = 0x%08x EDX = 0x%08x\n"
			"ESI = 0x%08x EDI = 0x%08x\n"
			"EIP = 0x%08x ESP = 0x%08x\n"
			"EBP = 0x%08x EFL = 0x%08x\n"
			"\n"
			"CS = 0x%04x\n"
			"SS = 0x%04x\n"
			"DS = 0x%04x\n"
			"ES = 0x%04x\n"
			"FS = 0x%04x\n"
			"GS = 0x%04x\n"
			"\n"
			"%s\n",
			com_version.GetString(),
			ExceptionRecord->ExceptionCode,
			ExceptionRecord->ExceptionAddress,
			GetExceptionCodeInfo( ExceptionRecord->ExceptionCode ),
			ContextRecord->Eax, ContextRecord->Ebx,
			ContextRecord->Ecx, ContextRecord->Edx,
			ContextRecord->Esi, ContextRecord->Edi,
			ContextRecord->Eip, ContextRecord->Esp,
			ContextRecord->Ebp, ContextRecord->EFlags,
			ContextRecord->SegCs,
			ContextRecord->SegSs,
			ContextRecord->SegDs,
			ContextRecord->SegEs,
			ContextRecord->SegFs,
			ContextRecord->SegGs,
			FPUFlags
		);

	EmailCrashReport( msg );
	common->FatalError( msg );

    // Tell the OS to restart the faulting instruction
    return ExceptionContinueExecution;
}
#endif
// RB end

#define TEST_FPU_EXCEPTIONS	/*	FPU_EXCEPTION_INVALID_OPERATION |		*/	\
							/*	FPU_EXCEPTION_DENORMALIZED_OPERAND |	*/	\
							/*	FPU_EXCEPTION_DIVIDE_BY_ZERO |			*/	\
							/*	FPU_EXCEPTION_NUMERIC_OVERFLOW |		*/	\
							/*	FPU_EXCEPTION_NUMERIC_UNDERFLOW |		*/	\
							/*	FPU_EXCEPTION_INEXACT_RESULT |			*/	\
								0

typedef enum STORM_PROCESS_DPI_AWARENESS {
	STORM_PROCESS_DPI_UNAWARE			= 0,
	STORM_PROCESS_SYSTEM_DPI_AWARE		= 1,
	STORM_PROCESS_PER_MONITOR_DPI_AWARE = 2
} STORM_PROCESS_DPI_AWARENESS;

#if 0
/*
==================
WinMain
==================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) {

	const HCURSOR hcurSave = ::SetCursor( LoadCursor( 0, IDC_WAIT ) );

	Sys_SetPhysicalWorkMemory( 192 << 20, 1024 << 20 );
	
#if 0
    DWORD handler = (DWORD)_except_handler;
    __asm
    {                           // Build EXCEPTION_REGISTRATION record:
        push    handler         // Address of handler function
        push    FS:[0]          // Address of previous handler
        mov     FS:[0],ESP      // Install new EXECEPTION_REGISTRATION
    }
#endif

	win32.hInstance = hInstance;
	idStr::Copynz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );

	// done before Com/Sys_Init since we need this for error output
	Sys_CreateConsole();

	// no abort/retry/fail errors
	SetErrorMode( SEM_FAILCRITICALERRORS );

	for ( int i = 0; i < MAX_CRITICAL_SECTIONS; i++ ) {
		InitializeCriticalSection( &win32.criticalSections[i] );
	}

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod( 1 );

	// get the initial time base
	Sys_Milliseconds();

#ifdef DEBUG
	// disable the painfully slow MS heap check every 1024 allocs
	_CrtSetDbgFlag( 0 );
#endif

//	Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );
	Sys_FPU_SetPrecision( FPU_PRECISION_DOUBLE_EXTENDED );

	common->Init( 0, nullptr, lpCmdLine );
	
#if TEST_FPU_EXCEPTIONS != 0
	common->Printf( Sys_FPU_GetState() );
#endif

	if ( win32.win_notaskkeys.GetInteger() ) {
		DisableTaskKeys( TRUE, FALSE, /*( win32.win_notaskkeys.GetInteger() == 2 )*/ FALSE );
	}

	// hide or show the early console as necessary
	if ( win32.win_viewlog.GetInteger() ) {
		Sys_ShowConsole( 1, true );
	} else {
		Sys_ShowConsole( 0, false );
	}

#ifdef SET_THREAD_AFFINITY 
	// give the main thread an affinity for the first cpu
	SetThreadAffinityMask( GetCurrentThread(), 1 );
#endif

	::SetCursor( hcurSave );

	::SetFocus( win32.hWnd );

    // main game loop
	while( 1 ) {

		Win_Frame();

#ifdef DEBUG
		Sys_MemFrame();
#endif

		// set exceptions, even if some crappy syscall changes them!
		Sys_FPU_EnableExceptions( TEST_FPU_EXCEPTIONS );

#ifdef ID_ALLOW_TOOLS
		if ( com_editors && enableToolsSupport) {
			if ( com_editors & EDITOR_GUI ) {
				// GUI editor
				GUIEditorRun();
			} else if ( com_editors & EDITOR_RADIANT ) {
				// Level Editor
				RadiantRun();
			}
			else if (com_editors & EDITOR_MATERIAL ) {
				//BSM Nerve: Add support for the material editor
				MaterialEditorRun();
			}
			else {
				if ( com_editors & EDITOR_LIGHT ) {
					// in-game Light Editor
					LightEditorRun();
				}
				if ( com_editors & EDITOR_SOUND ) {
					// in-game Sound Editor
					SoundEditorRun();
				}
				if ( com_editors & EDITOR_DECL ) {
					// in-game Declaration Browser
					DeclBrowserRun();
				}
				if ( com_editors & EDITOR_AF ) {
					// in-game Articulated Figure Editor
					AFEditorRun();
				}
				if ( com_editors & EDITOR_PARTICLE ) {
					// in-game Particle Editor
					ParticleEditorRun();
				}
				if ( com_editors & EDITOR_SCRIPT ) {
					// in-game Script Editor
					ScriptEditorRun();
				}
				if ( com_editors & EDITOR_PDA ) {
					// in-game PDA Editor
					PDAEditorRun();
				}
			}
		}
#endif

		DebuggerClientUpdate();

		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}
#endif 

/*
================
Sys_SetLanguageFromSystem
================
*/
extern idCVar sys_lang;
void Sys_SetLanguageFromSystem( void ) 
{
	sys_lang.SetString( Sys_DefaultLanguage() );
}
