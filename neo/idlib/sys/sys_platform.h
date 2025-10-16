/*
===========================================================================

Beato idTech 4x Source Code
Copyright (C) 2016-2022 Cristiano B. Santos <cristianobeato_dm@hotmail.com>.

This file is part of the Beato idTech 4x  GPL Source Code (?Beato idTech 4  Source Code?).

Beato idTech 4  Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Beato idTech 4x  Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Beato idTech 4x  Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef __SYS_PLATFORM_H__
#define __SYS_PLATFORM_H__

/*
#define _ARCH_x86_32_ 0
#define _ARCH_ARM_32_ 0
#define _ARCH_PPC_32_ 0
#define _ARCH_x86_64_ 0
#define _ARCH_ARM_64_ 0
#define _ARCH_PPC_64_ 0
*/

//Get current architecture, detectx nearly every architecture. Coded by Freak
// https://stackoverflow.com/questions/152016/detecting-cpu-architecture-compile-time
#if defined(__x86_64__) || defined(_M_X64)
#define _ARCH_x86_64_ 1
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
#define _ARCH_x86_32_ 1
#elif defined(__arm__) || defined(_M_ARM)
#elif defined(__aarch64__) || defined(_M_ARM64)
#define _ARCH_ARM_64_ 1
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
#define _ARCH_PPC_32_ 1
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
#define _ARCH_PPC_64_ 1
#endif

/*
#define __PLATFORM_WINDOWS__ 0
#define __PLATFORM_LINUX__ 0 
#define __PLATFORM_FBSD__ 0 
*/

// SO
#if defined(WIN32) || defined(_WIN32) // Windows
#   define __PLATFORM_WINDOWS__ 1
#elif defined( __linux__ )
#   define __PLATFORM_LINUX__ 1
#elif defined( __FreeBSD__ )
#   define __PLATFORM_FBSD__ 1
#elif defined( __ANDROID__ )
#   define __PLATFORM_ANDROID__ 1
#endif

/*
#define __COMPILER_MSVC__ 0
#define __COMPILER_GCC__ 0
#define __COMPILER_CLANG__ 0
#define __COMPILER_INTEL__ 0
*/
// COMPILER PLATFORM
#if defined( _MSC_VER ) || defined( _MSVC_LANG )
#define __COMPILER_MSVC__ 1
#elif defined( __clang__ )
#define __COMPILER_CLANG__ 1
#elif defined(__GNUC__) || defined(__GNUG__)
#define __COMPILER_GCC__ 1
#elif defined( __INTEL_COMPILER )
#define __COMPILER_INTEL__ 1
#endif

#endif //!__SYS_PLATFORM_H__