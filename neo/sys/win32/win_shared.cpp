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

#include "win_local.h"
#include <lmerr.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <errno.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#undef StrCmpN
#undef StrCmpNI
#undef StrCmpI

// RB begin
#if !defined(__MINGW32__)
#include <comdef.h>
#include <comutil.h>
#include <Wbemidl.h>


// RB: no <atlbase.h> with Visual C++ 2010 Express
#if defined(USE_MFC_TOOLS)
#include <atlbase.h>
#else
#include "win_nanoafx.h"
#endif

#endif // #if !defined(__MINGW32__)
// RB end

#pragma comment (lib, "wbemuuid.lib")

#pragma warning(disable:4740)	// warning C4740: flow in or out of inline asm code suppresses global optimization



/*
================
Sys_GetDriveFreeSpace
returns in megabytes
================
*/
int Sys_GetDriveFreeSpace( const char* path )
{
	DWORDLONG lpFreeBytesAvailable;
	DWORDLONG lpTotalNumberOfBytes;
	DWORDLONG lpTotalNumberOfFreeBytes;
	int ret = 26;
	//FIXME: see why this is failing on some machines
	if( ::GetDiskFreeSpaceEx( path, ( PULARGE_INTEGER )&lpFreeBytesAvailable, ( PULARGE_INTEGER )&lpTotalNumberOfBytes, ( PULARGE_INTEGER )&lpTotalNumberOfFreeBytes ) )
	{
		ret = ( double )( lpFreeBytesAvailable ) / ( 1024.0 * 1024.0 );
	}
	return ret;
}

/*
========================
Sys_GetDriveFreeSpaceInBytes
========================
*/
int64_t Sys_GetDriveFreeSpaceInBytes( const char* path )
{
	DWORDLONG lpFreeBytesAvailable;
	DWORDLONG lpTotalNumberOfBytes;
	DWORDLONG lpTotalNumberOfFreeBytes;
	int64_t ret = 1;
	//FIXME: see why this is failing on some machines
	if( ::GetDiskFreeSpaceEx( path, ( PULARGE_INTEGER )&lpFreeBytesAvailable, ( PULARGE_INTEGER )&lpTotalNumberOfBytes, ( PULARGE_INTEGER )&lpTotalNumberOfFreeBytes ) )
	{
		ret = lpFreeBytesAvailable;
	}
	return ret;
}

/*
================
Sys_GetVideoRam
returns in megabytes
================
*/
int Sys_GetVideoRam()
{
	unsigned int retSize = 64;
	
	// RB begin
#if !defined(__MINGW32__)
	CComPtr<IWbemLocator> spLoc = NULL;
	HRESULT hr = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_SERVER, IID_IWbemLocator, ( LPVOID* ) &spLoc );
	if( hr != S_OK || spLoc == NULL )
	{
		return retSize;
	}
	
	CComBSTR bstrNamespace( _T( "\\\\.\\root\\CIMV2" ) );
	CComPtr<IWbemServices> spServices;
	
	// Connect to CIM
	hr = spLoc->ConnectServer( bstrNamespace, NULL, NULL, 0, NULL, 0, 0, &spServices );
	if( hr != WBEM_S_NO_ERROR )
	{
		return retSize;
	}
	
	// Switch the security level to IMPERSONATE so that provider will grant access to system-level objects.
	hr = CoSetProxyBlanket( spServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );
	if( hr != S_OK )
	{
		return retSize;
	}
	
	// Get the vid controller
	CComPtr<IEnumWbemClassObject> spEnumInst = NULL;
	hr = spServices->CreateInstanceEnum( CComBSTR( "Win32_VideoController" ), WBEM_FLAG_SHALLOW, NULL, &spEnumInst );
	if( hr != WBEM_S_NO_ERROR || spEnumInst == NULL )
	{
		return retSize;
	}
	
	ULONG uNumOfInstances = 0;
	CComPtr<IWbemClassObject> spInstance = NULL;
	hr = spEnumInst->Next( 10000, 1, &spInstance, &uNumOfInstances );
	
	if( hr == S_OK && spInstance )
	{
		// Get properties from the object
		CComVariant varSize;
		hr = spInstance->Get( CComBSTR( _T( "AdapterRAM" ) ), 0, &varSize, 0, 0 );
		if( hr == S_OK )
		{
			retSize = varSize.uintVal / (1024 * 1024);
			

			if( retSize == 0 )
			{
				retSize = 64;
			}
		}
	}
#endif
	// RB end
	
	return retSize;
}

/*
================
Sys_GetCurrentUser
================
*/
char* Sys_GetCurrentUser()
{
	static char s_userName[1024];
	unsigned long size = sizeof( s_userName );
	
	
	if( !GetUserName( s_userName, &size ) )
	{
		strcpy( s_userName, "player" );
	}
	
	if( !s_userName[0] )
	{
		strcpy( s_userName, "player" );
	}
	
	return s_userName;
}

