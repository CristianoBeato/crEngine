/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel
Copyright (C) 2025-2026 Cristiano B. Santos

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
#ifndef __SYS_PUBLIC__
#define __SYS_PUBLIC__

#include "../idlib/CmdArgs.h"

#ifdef __GNUC__
#define id_attribute(x) __attribute__(x)
#else
#define id_attribute(x)  
#endif

/*
===============================================================================

	Non-portable system services.

===============================================================================
*/

enum joystickAxis_t
{
	AXIS_LEFT_X,
	AXIS_LEFT_Y,
	AXIS_RIGHT_X,
	AXIS_RIGHT_Y,
	AXIS_LEFT_TRIG,
	AXIS_RIGHT_TRIG,
	MAX_JOYSTICK_AXIS
};

enum sysEventType_t
{
	SE_NONE,				// evTime is still valid
	SE_KEY,					// evValue is a key code, evValue2 is the down flag
	SE_CHAR,				// evValue is an ascii char
	SE_MOUSE,				// evValue and evValue2 are reletive signed x / y moves
	SE_MOUSE_ABSOLUTE,		// evValue and evValue2 are absolute coordinates in the window's client area.
	SE_MOUSE_LEAVE,			// evValue and evValue2 are meaninless, this indicates the mouse has left the client area.
	SE_JOYSTICK,		// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE				// evPtr is a char*, from typing something at a non-game console
};

enum sys_mEvents
{
	M_ACTION1,
	M_ACTION2,
	M_ACTION3,
	M_ACTION4,
	M_ACTION5,
	M_ACTION6,
	M_ACTION7,
	M_ACTION8,
	M_DELTAX,
	M_DELTAY,
	M_DELTAZ,
	M_INVALID
};

enum sys_jEvents
{
	J_ACTION1,
	J_ACTION2,
	J_ACTION3,
	J_ACTION4,
	J_ACTION5,
	J_ACTION6,
	J_ACTION7,
	J_ACTION8,
	J_ACTION9,
	J_ACTION10,
	J_ACTION11,
	J_ACTION12,
	J_ACTION13,
	J_ACTION14,
	J_ACTION15,
	J_ACTION16,
	J_ACTION17,
	J_ACTION18,
	J_ACTION19,
	J_ACTION20,
	J_ACTION21,
	J_ACTION22,
	J_ACTION23,
	J_ACTION24,
	J_ACTION25,
	J_ACTION26,
	J_ACTION27,
	J_ACTION28,
	J_ACTION29,
	J_ACTION30,
	J_ACTION31,
	J_ACTION32,
	J_ACTION_MAX = J_ACTION32,
	
	J_AXIS_MIN,
	J_AXIS_LEFT_X = J_AXIS_MIN + AXIS_LEFT_X,
	J_AXIS_LEFT_Y = J_AXIS_MIN + AXIS_LEFT_Y,
	J_AXIS_RIGHT_X = J_AXIS_MIN + AXIS_RIGHT_X,
	J_AXIS_RIGHT_Y = J_AXIS_MIN + AXIS_RIGHT_Y,
	J_AXIS_LEFT_TRIG = J_AXIS_MIN + AXIS_LEFT_TRIG,
	J_AXIS_RIGHT_TRIG = J_AXIS_MIN + AXIS_RIGHT_TRIG,
	
	J_AXIS_MAX = J_AXIS_MIN + MAX_JOYSTICK_AXIS - 1,
	
	J_DPAD_UP,
	J_DPAD_DOWN,
	J_DPAD_LEFT,
	J_DPAD_RIGHT,
	
	MAX_JOY_EVENT
};

/*
================================================
The first part of this table maps directly to Direct Input scan codes (DIK_* from dinput.h)
But they are duplicated here for console portability
================================================
*/
enum keyNum_t
{
	K_NONE,
	
	K_ESCAPE,
	K_1,
	K_2,
	K_3,
	K_4,
	K_5,
	K_6,
	K_7,
	K_8,
	K_9,
	K_0,
	K_MINUS,
	K_EQUALS,
	K_BACKSPACE,
	K_TAB,
	K_Q,
	K_W,
	K_E,
	K_R,
	K_T,
	K_Y,
	K_U,
	K_I,
	K_O,
	K_P,
	K_LBRACKET,
	K_RBRACKET,
	K_ENTER,
	K_LCTRL,
	K_A,
	K_S,
	K_D,
	K_F,
	K_G,
	K_H,
	K_J,
	K_K,
	K_L,
	K_SEMICOLON,
	K_APOSTROPHE,
	K_GRAVE,
	K_LSHIFT,
	K_BACKSLASH,
	K_Z,
	K_X,
	K_C,
	K_V,
	K_B,
	K_N,
	K_M,
	K_COMMA,
	K_PERIOD,
	K_SLASH,
	K_RSHIFT,
	K_KP_STAR,
	K_LALT,
	K_SPACE,
	K_CAPSLOCK,
	K_F1,
	K_F2,
	K_F3,
	K_F4,
	K_F5,
	K_F6,
	K_F7,
	K_F8,
	K_F9,
	K_F10,
	K_NUMLOCK,
	K_SCROLL,
	K_KP_7,
	K_KP_8,
	K_KP_9,
	K_KP_MINUS,
	K_KP_4,
	K_KP_5,
	K_KP_6,
	K_KP_PLUS,
	K_KP_1,
	K_KP_2,
	K_KP_3,
	K_KP_0,
	K_KP_DOT,
	K_F11			= 0x57,
	K_F12			= 0x58,
	K_F13			= 0x64,
	K_F14			= 0x65,
	K_F15			= 0x66,
	K_KANA			= 0x70,
	K_CONVERT		= 0x79,
	K_NOCONVERT		= 0x7B,
	K_YEN			= 0x7D,
	K_KP_EQUALS		= 0x8D,
	K_CIRCUMFLEX	= 0x90,
	K_AT			= 0x91,
	K_COLON			= 0x92,
	K_UNDERLINE		= 0x93,
	K_KANJI			= 0x94,
	K_STOP			= 0x95,
	K_AX			= 0x96,
	K_UNLABELED		= 0x97,
	K_KP_ENTER		= 0x9C,
	K_RCTRL			= 0x9D,
	K_KP_COMMA		= 0xB3,
	K_KP_SLASH		= 0xB5,
	K_PRINTSCREEN	= 0xB7,
	K_RALT			= 0xB8,
	K_PAUSE			= 0xC5,
	K_HOME			= 0xC7,
	K_UPARROW		= 0xC8,
	K_PGUP			= 0xC9,
	K_LEFTARROW		= 0xCB,
	K_RIGHTARROW	= 0xCD,
	K_END			= 0xCF,
	K_DOWNARROW		= 0xD0,
	K_PGDN			= 0xD1,
	K_INS			= 0xD2,
	K_DEL			= 0xD3,
	K_LWIN			= 0xDB,
	K_RWIN			= 0xDC,
	K_APPS			= 0xDD,
	K_POWER			= 0xDE,
	K_SLEEP			= 0xDF,
	
	//------------------------
	// K_JOY codes must be contiguous, too
	//------------------------
	
	K_JOY1 = 256,
	K_JOY2,
	K_JOY3,
	K_JOY4,
	K_JOY5,
	K_JOY6,
	K_JOY7,
	K_JOY8,
	K_JOY9,
	K_JOY10,
	K_JOY11,
	K_JOY12,
	K_JOY13,
	K_JOY14,
	K_JOY15,
	K_JOY16,
	
	K_JOY_STICK1_UP,
	K_JOY_STICK1_DOWN,
	K_JOY_STICK1_LEFT,
	K_JOY_STICK1_RIGHT,
	
	K_JOY_STICK2_UP,
	K_JOY_STICK2_DOWN,
	K_JOY_STICK2_LEFT,
	K_JOY_STICK2_RIGHT,
	
	K_JOY_TRIGGER1,
	K_JOY_TRIGGER2,
	
	K_JOY_DPAD_UP,
	K_JOY_DPAD_DOWN,
	K_JOY_DPAD_LEFT,
	K_JOY_DPAD_RIGHT,
	
	//------------------------
	// K_MOUSE enums must be contiguous (no char codes in the middle)
	//------------------------
	
	K_MOUSE1,
	K_MOUSE2,
	K_MOUSE3,
	K_MOUSE4,
	K_MOUSE5,
	K_MOUSE6,
	K_MOUSE7,
	K_MOUSE8,
	
	K_MWHEELDOWN,
	K_MWHEELUP,
	
	K_LAST_KEY
};

struct sysEvent_t
{
	sysEventType_t	evType;
	int				evValue;
	int				evValue2;
	int				evPtrLength;		// bytes of data pointed to by evPtr, for journaling
	void* 			evPtr;				// this must be manually freed if not nullptr
	int				inputDevice;

	bool			IsKeyEvent( void ) const { return evType == SE_KEY; }
	bool			IsMouseEvent( void ) const { return evType == SE_MOUSE; }
	bool			IsCharEvent( void ) const { return evType == SE_CHAR; }
	bool			IsJoystickEvent( void ) const { return evType == SE_JOYSTICK; }
	bool			IsKeyDown( void ) const { return evValue2 != 0; }
	keyNum_t		GetKey( void ) const { return static_cast< keyNum_t >( evValue ); }
	int				GetXCoord( void ) const { return evValue; }
	int				GetYCoord( void ) const { return evValue2; }
};

enum videoMode_t : uint8_t
{
	VIDEO_MODE_WINDOW,
	VIDEO_MODE_BORDERLESS,
	VIDEO_MODE_FULLSCREEN,
	VIDEO_MODE_DEDICATED
};

// BEATO Begin:
typedef struct vidMode_t
{
	uint8_t		format;
	uint8_t		displayHz; // ( uint8 since our engine max is 240hz  )
	uint16_t	width;
	uint16_t	height;
	uint32_t	modeID;
	uint32_t	displayID;
	
	// RB begin
	vidMode_t( void )
	{
		format = 0;
		modeID = UINT32_MAX;
		displayID = UINT32_MAX;
		width = 640;
		height = 480;
		displayHz = 60;
	}
	
	vidMode_t( uint16_t width, uint16_t height, uint8_t displayHz ) : width( width ), height( height ), displayHz( displayHz ) {}
	// RB end
	
	bool operator==( const vidMode_t& a )
	{
		return a.width == width && a.height == height && a.displayHz == displayHz;
	}
} idMode_t;

enum grab_e
{
	GRAB_ENABLE		= ( 1 << 0 ),
	GRAB_REENABLE	= ( 1 << 1 ),
	GRAB_HIDECURSOR	= ( 1 << 2 ),
	GRAB_SETSTATE	= ( 1 << 3 )
};

// typedef unsigned long address_t; // DG: this isn't even used

void			Sys_Error( const char* error, ... );

// will go to the various text consoles
void			Sys_Printf( VERIFY_FORMAT_STRING const char* msg, ... );

// guaranteed to be thread-safe
void			Sys_DebugPrintf( VERIFY_FORMAT_STRING const char* fmt, ... );
void			Sys_DebugVPrintf( const char* fmt, va_list arg );



// returns amount of video ram
int				Sys_GetVideoRam( void );

// returns amount of drive space in path
uint32_t		Sys_GetDriveFreeSpace( const char* path );

// returns amount of drive space in path in bytes
uint64_t		Sys_GetDriveFreeSpaceInBytes( const char* path );

// This really isn't the right place to have this, but since this is the 'top level' include
// and has a function signature with 'FILE' in it, it kinda needs to be here =/

// NOTE: do we need to guarantee the same output on all platforms?
const char* 	Sys_TimeStampToStr( ID_TIME_T timeStamp );
const char* 	Sys_SecToStr( int sec );



// Execute the specified process and wait until it's done, calling workFn every waitMS milliseconds.
// If showOutput == true, std IO from the executed process will be output to the console.
// Note that the return value is not an indication of the exit code of the process, but is false
// only if the process could not be created at all. If you wish to check the exit code of the
// spawned process, check the value returned in exitCode.
typedef bool ( *execProcessWorkFunction_t )();
typedef void ( *execOutputFunction_t )( const char* text );
bool Sys_Exec(	const char* appPath, const char* workingPath, const char* args,
				execProcessWorkFunction_t workFn, execOutputFunction_t outputFn, const int waitMS,
				unsigned int& exitCode );

/*
==============================================================

	Networking

==============================================================
*/

typedef enum
{
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP
} netadrtype_t;

typedef struct
{
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned short	port;
} netadr_t;

#define	PORT_ANY			-1

/*
================================================
idUDP
================================================
*/
class idUDP
{
public:
	// this just zeros netSocket and port
	idUDP();
	virtual		~idUDP();
	
	// if the InitForPort fails, the idUDP.port field will remain 0
	bool		InitForPort( int portNumber );
	
	int			GetPort() const
	{
		return bound_to.port;
	}
	netadr_t	GetAdr() const
	{
		return bound_to;
	}
	uint32_t		GetUIntAdr() const
	{
		return ( bound_to.ip[0] | bound_to.ip[1] << 8 | bound_to.ip[2] << 16 | bound_to.ip[3] << 24 );
	}
	void		Close();
	
	bool		GetPacket( netadr_t& from, void* data, int& size, int maxSize );
	
	bool		GetPacketBlocking( netadr_t& from, void* data, int& size, int maxSize,
								   int timeout );
								   
	void		SendPacket( const netadr_t to, const void* data, int size );
	
	void		SetSilent( bool silent )
	{
		this->silent = silent;
	}
	bool		GetSilent() const
	{
		return silent;
	}
	
	int			packetsRead;
	int			bytesRead;
	
	int			packetsWritten;
	int			bytesWritten;
	
	bool		IsOpen() const
	{
		return netSocket > 0;
	}
	
private:
	netadr_t	bound_to;		// interface and port
	int			netSocket;		// OS specific socket
	bool		silent;			// don't emit anything ( black hole )
};

struct msg_t
{
	msg_t(int bufferSize = 32767, bool compressed = true);
	~msg_t();

	void ResetReadOffset();

	bool ReadBytes(char * bytes, int numBytes);

	template<typename T>
	bool Read(T & output)
	{
		return ReadBytes((char*)&output, sizeof(T));
	}
	bool ReadString(char * buffer, int maxlength);

	bool WriteBytes(const char * bytes, int numBytes);

	template<typename T>
	bool Write(const T & output)
	{
		return WriteBytes((const char *)&output, sizeof(T));
	}

	bool WriteString(const char * output);

	bool ReadPacket(idUDP & socket, netadr_t & addrFrom);
	void SendPacket(idUDP & socket, const netadr_t & addr);
	
	const char * Data () const{ return mData; }
	int Size() const { return mDataOffset; }
private:
	char *			mData;
	int				mDataSize;
	int				mDataOffset;
	const int		mDataMaxSize;
	const bool		mCompressed;
};

// parses the port number
// can also do DNS resolve if you ask for it.
// NOTE: DNS resolve is a slow/blocking call, think before you use
// ( could be exploited for server DoS )
bool			Sys_StringToNetAdr( const char* s, netadr_t* a, bool doDNSResolve );
const char* 	Sys_NetAdrToString( const netadr_t a );
bool			Sys_IsLANAddress( const netadr_t a );
bool			Sys_CompareNetAdrBase( const netadr_t a, const netadr_t b );

int				Sys_GetLocalIPCount();
const char* 	Sys_GetLocalIP( int i );

void			Sys_InitNetworking();
void			Sys_ShutdownNetworking();

/*
==============================================================

	idSys

==============================================================
*/
/// BEATO: on future we gona ignore idSys, and use crPlatform instead for game logic system portable functions, to minimize
/// code polution
class idSys
{
public:
	virtual void			DebugPrintf( VERIFY_FORMAT_STRING const char* fmt, ... ) = 0;
	virtual void			DebugVPrintf( const char* fmt, va_list arg ) = 0;
	
	virtual double			GetClockTicks( void ) = 0;
	virtual double			ClockTicksPerSecond( void ) = 0;
	virtual cpuid_t			GetProcessorId( void ) = 0;
	virtual const char* 	GetProcessorString( void ) = 0;
	virtual void			FPU_SetFTZ( const bool enable ) = 0;
	virtual void			FPU_SetDAZ( const bool enable ) = 0;
	
	virtual void			FPU_EnableExceptions( int exceptions ) = 0;
	
	virtual bool			LockMemory( void* ptr, const size_t bytes ) = 0;
	virtual bool			UnlockMemory( void* ptr, const size_t bytes ) = 0;
	
	virtual void			DLL_GetFileName( const char* baseName, char* dllName, int maxLength ) = 0;
	
	virtual sysEvent_t		GenerateMouseButtonEvent( int button, bool down ) = 0;
	virtual sysEvent_t		GenerateMouseMoveEvent( int deltax, int deltay ) = 0;
	
	virtual void			OpenURL( const char* url, bool quit ) = 0;
	virtual void			StartProcess( const char* exePath, bool quit ) = 0;
};

// TODO: implement as singleton
extern idSys* 				sys;

bool Sys_LoadOpenAL( void );
void Sys_FreeOpenAL( void );


#endif /* !__SYS_PUBLIC__ */
