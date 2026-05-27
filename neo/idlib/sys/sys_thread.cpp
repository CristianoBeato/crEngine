/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.

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

/*
================================================================================================
================================================================================================
*/


/*
========================
Sys_Yield
========================
*/
void Sys_Yield( void )
{
#if __PLATFORM_WINDOWS__
	SwitchToThread();
#elif __PLATFORM_LINUX__
	sched_yield();
#endif //__PLATFORM_LINUX__
}


/*
================================================================================================

	Interlocked Integer

================================================================================================
*/

/*
========================
Sys_InterlockedIncrement
========================
*/
interlockedInt_t Sys_InterlockedIncrement( interlockedInt_t& value )
{
#if __COMPILER_MSVC__
	return InterlockedIncrementAcquire( & value ); // googling suggests that some experimental mingw code supports this too..
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	return __sync_add_and_fetch( &value, 1 );
#endif
}

/*
========================
Sys_InterlockedDecrement
========================
*/
interlockedInt_t Sys_InterlockedDecrement( interlockedInt_t& value )
{
#if __COMPILER_MSVC__
	return InterlockedDecrementRelease( & value );
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	return __sync_sub_and_fetch( &value, 1 );
#endif
}

/*
========================
Sys_InterlockedAdd
========================
*/
interlockedInt_t Sys_InterlockedAdd( interlockedInt_t& value, interlockedInt_t i )
{
#if __COMPILER_MSVC__
	return InterlockedExchangeAdd( & value, i ) + i;
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	return __sync_add_and_fetch( &value, i );
#endif
}

/*
========================
Sys_InterlockedSub
========================
*/
interlockedInt_t Sys_InterlockedSub( interlockedInt_t& value, interlockedInt_t i )
{
#if __COMPILER_MSVC__
	return InterlockedExchangeAdd( & value, - i ) - i;
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	return __sync_sub_and_fetch( &value, i );
#endif
}

/*
========================
Sys_InterlockedExchange
========================
*/
interlockedInt_t Sys_InterlockedExchange( interlockedInt_t& value, interlockedInt_t exchange )
{
#if __COMPILER_MSVC__
	return InterlockedExchange( & value, exchange );
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	// source: http://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Atomic-Builtins.html
	// These builtins perform an atomic compare and swap. That is, if the current value of *ptr is oldval, then write newval into *ptr.
	return __sync_val_compare_and_swap( &value, value, exchange );
#endif
}

/*
========================
Sys_InterlockedCompareExchange
========================
*/
interlockedInt_t Sys_InterlockedCompareExchange( interlockedInt_t& value, interlockedInt_t comparand, interlockedInt_t exchange )
{
#if __COMPILER_MSVC__
	return InterlockedCompareExchange( & value, exchange, comparand );
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	return __sync_val_compare_and_swap( &value, comparand, exchange );
#endif
}

/*
================================================================================================

	Interlocked Pointer

================================================================================================
*/

/*
========================
Sys_InterlockedExchangePointer
========================
*/
void* Sys_InterlockedExchangePointer( void*& ptr, void* exchange )
{
#if __COMPILER_MSVC__
	return InterlockedExchangePointer( & ptr, exchange );
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	return __sync_val_compare_and_swap( &ptr, ptr, exchange );
#endif
}

/*
========================
Sys_InterlockedCompareExchangePointer
========================
*/
void* Sys_InterlockedCompareExchangePointer( void*& ptr, void* comparand, void* exchange )
{
#if __COMPILER_MSVC__
	return InterlockedCompareExchangePointer( & ptr, exchange, comparand );
#elif __COMPILER_MINGW__ || __COMPILER_GCC__ || __COMPILER_CLANG__
	return __sync_val_compare_and_swap( &ptr, comparand, exchange );
#endif
}
