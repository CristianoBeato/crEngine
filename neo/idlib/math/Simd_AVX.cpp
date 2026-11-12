/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2025-2026 Cristiano B. Santos.


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

#include "Simd_AVX.hpp"

#include <immintrin.h>

void VPCALL	idSIMD_AVX::AtemporalCopy( void* dst,	const void* src,    const size_t bytes )
{
    idassert(reinterpret_cast<uintptr_t>( dst ) % 32 == 0);
    idassert(reinterpret_cast<uintptr_t>( src ) % 32 == 0);

    auto* __restrict d = static_cast<__m256i*>( dst );
    const auto* __restrict s = static_cast<const __m256i*>( src );
    
    // process tow blocks of 32 bytes
    size_t num_blocks_256 = bytes / sizeof(__m256i);
    for ( size_t i = 0; i < num_blocks_256; i += 2 ) 
    {
        // Load a cache line width ( 64 bytes ) 
        __m256i r0 = _mm256_loadu_si256( &s[i] );
        __m256i r1 = _mm256_loadu_si256( &s[i + 1] );

        // stor two blocks to the memory location
        _mm256_stream_si256(&d[i],     r0);
        _mm256_stream_si256(&d[i + 1], r1);
    }

    // operation fence
    _mm_sfence(); 
}