/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

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
#ifndef __SYS_THREADING_H__
#define __SYS_THREADING_H__

#ifndef __TYPEINFOGEN__

/*
================================================================================================

	Platform specific mutex, signal, atomic integer and memory barrier.

================================================================================================
*/

// BEATO Begin

typedef int						interlockedInt_t;

// _ReadWriteBarrier() does not translate to any instructions but keeps the compiler
// from reordering read and write instructions across the barrier.
// MemoryBarrier() inserts and CPU instruction that keeps the CPU from reordering reads and writes.
#if defined(_MSC_VER)
#pragma intrinsic(_ReadWriteBarrier)
#define SYS_MEMORYBARRIER		_ReadWriteBarrier(); MemoryBarrier()
#elif defined(__GNUC__) // FIXME: what about clang?
// according to http://en.wikipedia.org/wiki/Memory_ordering the following should be equivalent to the stuff above..
//#ifdef __sync_syncronize
#define SYS_MEMORYBARRIER		asm volatile("" ::: "memory");__sync_synchronize()
#endif

/*
================================================================================================

	Platform specific thread local storage.
	Can be used to store either a pointer or an integer.

================================================================================================
*/

#endif // __TYPEINFOGEN__

/*
================================================================================================

	Platform independent threading functions.

================================================================================================
*/

enum core_t
{
	CORE_ANY = -1,
	CORE_0A,
	CORE_0B,
	CORE_1A,
	CORE_1B,
	CORE_2A,
	CORE_2B
};

typedef unsigned int ( *xthread_t )( void* );

enum xthreadPriority
{
	THREAD_LOWEST,
	THREAD_BELOW_NORMAL,
	THREAD_NORMAL,
	THREAD_ABOVE_NORMAL,
	THREAD_HIGHEST
};

inline constexpr int32_t DEFAULT_THREAD_STACK_SIZE = 256 * 1024;

interlockedInt_t	Sys_InterlockedIncrement( interlockedInt_t& value );
interlockedInt_t	Sys_InterlockedDecrement( interlockedInt_t& value );

interlockedInt_t	Sys_InterlockedAdd( interlockedInt_t& value, interlockedInt_t i );
interlockedInt_t	Sys_InterlockedSub( interlockedInt_t& value, interlockedInt_t i );

interlockedInt_t	Sys_InterlockedExchange( interlockedInt_t& value, interlockedInt_t exchange );
interlockedInt_t	Sys_InterlockedCompareExchange( interlockedInt_t& value, interlockedInt_t comparand, interlockedInt_t exchange );

void* 				Sys_InterlockedExchangePointer( void*& ptr, void* exchange );
void* 				Sys_InterlockedCompareExchangePointer( void*& ptr, void* comparand, void* exchange );

extern void			Sys_Yield( void );

#endif	// !__SYS_THREADING_H__
