/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel
Copyright (C) 2025 Cristiano Beato

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
#include "sys_local.h"
#include "platform/Platform.hpp"

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_loadso.h>
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_messagebox.h>

static idCVar sys_allowMultipleInstances( "sys_allowMultipleInstances", "0", CVAR_SYSTEM | CVAR_BOOL, "allow multiple instances running concurrently" );


idSysLocal			sysLocal;
idSys* 				sys = &sysLocal;

void idSysLocal::DebugPrintf( const char* fmt, ... )
{
	va_list argptr;
	
	va_start( argptr, fmt );
	Sys_DebugVPrintf( fmt, argptr );
	va_end( argptr );
}

void idSysLocal::DebugVPrintf( const char* fmt, va_list arg )
{
	Sys_DebugVPrintf( fmt, arg );
}

double idSysLocal::GetClockTicks( void )
{
	return static_cast<double>(SDL_GetPerformanceCounter());
}

double idSysLocal::ClockTicksPerSecond( void )
{
	return static_cast<double>(SDL_GetPerformanceFrequency());
}

/*
=================
idSysLocal::DLL_GetFileName
=================
*/
void idSysLocal::DLL_GetFileName( const char* baseName, char* dllName, int maxLength )
{
	idStr::snPrintf( dllName, maxLength, "%s" CPUSTRING ".dll", baseName );
}

/*
=================
idSysLocal::GenerateMouseButtonEvent
=================
*/
// sysEvent_t idSysLocal::GenerateMouseButtonEvent( int button, bool down )
// {
// 	sysEvent_t ev;
// 	ev.evType = SE_KEY;
// 	ev.evValue = K_MOUSE1 + button - 1;
// 	ev.evValue2 = down;
// 	ev.evPtrLength = 0;
// 	ev.evPtr = nullptr;
// 	return ev;
// }

/*
=================
idSysLocal::GenerateMouseMoveEvent
=================
*/
//sysEvent_t idSysLocal::GenerateMouseMoveEvent( int deltax, int deltay )
//{
//	sysEvent_t ev;
//	ev.evType = SE_MOUSE;
//	ev.evValue = deltax;
//	ev.evValue2 = deltay;
//	ev.evPtrLength = 0;
//	ev.evPtr = nullptr;
//	return ev;
//}

/*
=================
Sys_TimeStampToStr
=================
*/
const char* Sys_TimeStampToStr( ID_TIME_T timeStamp )
{
	static char timeString[MAX_STRING_CHARS];
	timeString[0] = '\0';
	
	time_t ts = ( time_t )timeStamp;
	tm*	time = localtime( &ts );
	if( time == nullptr )
	{
		return "??/??/???? ??:??";
	}
	
	idStr out;
	
	idStr lang = cvarSystem->GetCVarString( "sys_lang" );
	if( lang.Icmp( ID_LANG_ENGLISH ) == 0 )
	{
		// english gets "month/day/year  hour:min" + "am" or "pm"
		out = va( "%02d", time->tm_mon + 1 );
		out += "/";
		out += va( "%02d", time->tm_mday );
		out += "/";
		out += va( "%d", time->tm_year + 1900 );
		out += " ";	// changed to spaces since flash doesn't recognize \t
		if( time->tm_hour > 12 )
		{
			out += va( "%02d", time->tm_hour - 12 );
		}
		else if( time->tm_hour == 0 )
		{
			out += "12";
		}
		else
		{
			out += va( "%02d", time->tm_hour );
		}
		out += ":";
		out += va( "%02d", time->tm_min );
		if( time->tm_hour >= 12 )
		{
			out += "pm";
		}
		else
		{
			out += "am";
		}
	}
	else
	{
		// europeans get "day/month/year  24hour:min"
		out = va( "%02d", time->tm_mday );
		out += "/";
		out += va( "%02d", time->tm_mon + 1 );
		out += "/";
		out += va( "%d", time->tm_year + 1900 );
		out += " ";	// changed to spaces since flash doesn't recognize \t
		out += va( "%02d", time->tm_hour );
		out += ":";
		out += va( "%02d", time->tm_min );
	}
	idStr::Copynz( timeString, out, sizeof( timeString ) );
	
	return timeString;
}

/*
========================
Sys_SecToStr
========================
*/
const char* Sys_SecToStr( const uint32_t in_sec )
{
	uint32_t sec = in_sec;
	char timeString[MAX_STRING_CHARS];
	
	uint32_t weeks = sec / ( 3600 * 24 * 7 );
	sec -= weeks * ( 3600 * 24 * 7 );
	
	uint32_t days = sec / ( 3600 * 24 );
	sec -= days * ( 3600 * 24 );
	
	uint32_t hours = sec / 3600;
	sec -= hours * 3600;
	
	uint32_t min = sec / 60;
	sec -= min * 60;
	
	if( weeks > 0 )
		std::sprintf( timeString, "%dw, %dd, %d:%02d:%02d", weeks, days, hours, min, sec );
	else if( days > 0 )
		std::sprintf( timeString, "%dd, %d:%02d:%02d", days, hours, min, sec );
	else
		std::sprintf( timeString, "%d:%02d:%02d", hours, min, sec );
	
	return timeString;
}

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error( const char *error, ... )
{
	va_list argptr;
	va_start( argptr, error );
	crConsole::Get()->VError( error, argptr );
	va_end( argptr);

	/// close aplication
	crPlatform::Get()->Exit( EXIT_FAILURE );
}
