/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos.

This file is part of the crEngine GPL Source Code ("crEngine Source Code").

crEngine Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

crEngine Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with crEngine Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#include "precompiled.h"
#include "renderer/renderer_common.h"
#include "Vulkan.hpp"

#include <SDL3/SDL_stdinc.h>

// our onw allocation structure using SDL_malloc
static const VkAllocationCallbacks allocationCallbacksLocal = 
{
    nullptr,
    crvkAllocation,
    crvkReallocation,
    crvkFree,
    crvkInternalAllocation,
    crvkInternalFree
};

#if 1 // TODO: a define to easy control
const VkAllocationCallbacks* k_allocationCallbacks = &allocationCallbacksLocal;
#else 
const VkAllocationCallbacks* k_allocationCallbacks = nullptr;
#endif 

/*
==============================================
crvkAllocation
==============================================
*/
void *VKAPI_ATTR crvkAllocation( void * in_userData, size_t in_size, size_t in_alignment, VkSystemAllocationScope in_allocationScope )
{
    void* memptr = nullptr;
#if 1
    void* original = SDL_malloc( in_size + in_alignment - 1 + sizeof(void*) );
    uintptr_t aligned = ( reinterpret_cast<uintptr_t>( original ) + sizeof(void*) + in_alignment - 1) & ~( in_alignment - 1 );
    (reinterpret_cast<void**>(aligned))[-1] = original;
    memptr = reinterpret_cast<void*>( aligned );
    // check if memory is aligned 
    assert( memptr && ( (uintptr_t)memptr % in_alignment ) == 0 );
#else
    memptr = SDL_aligned_alloc( in_alignment, in_size );
#endif
    return memptr;
}

/*
==============================================
crvkReallocation
==============================================
*/
void* VKAPI_CALL crvkReallocation( void* in_userData, void* in_original, size_t in_size, size_t in_alignment, VkSystemAllocationScope in_allocationScope )
{
    void* memptr = nullptr;
#if 1
    void* original = static_cast<void**>( in_original )[-1];
    if( original != nullptr )
        original = SDL_realloc( original, in_size + in_alignment - 1 + sizeof(void*) );
    else
        original = SDL_malloc( in_size + in_alignment - 1 + sizeof(void*) );

    uintptr_t aligned = ( reinterpret_cast<uintptr_t>( original ) + sizeof(void*) + in_alignment - 1) & ~( in_alignment - 1 );
    (reinterpret_cast<void**>(aligned))[-1] = original;
    memptr = reinterpret_cast<void*>( aligned );

    // check if memory is aligned 
    assert( memptr && ( (uintptr_t)memptr % in_alignment ) == 0 );
#else
    memptr = SDL_aligned_alloc( in_alignment, in_size );
    SDL_memcpy( memptr, in_original, sizeof( in_original ) );
    SDL_free( in_original );    
#endif
    return memptr;
}

/*
==============================================
crvkFree
==============================================
*/
void VKAPI_CALL crvkFree( void* in_userData, void* in_memory )
{
#if 1
    if ( in_memory ) 
    {
        void* original = static_cast<void**>( in_memory )[-1];
        SDL_free( original );
    }
#else
    SDL_aligned_free( in_memory );
#endif
}

/*
==============================================
crvkInternalAllocation
==============================================
*/
void VKAPI_CALL crvkInternalAllocation( void* in_userData, size_t in_size, VkInternalAllocationType in_allocationType, VkSystemAllocationScope in_allocationScope )
{
    //vkCtx.allocedMemory += size;
    //std::printf("[Vulkan] Internal allocation of %zu bytes, total %i\n", in_size, 0 );
}

/*
==============================================
crvkInternalFree
==============================================
*/
void VKAPI_CALL crvkInternalFree( void* in_userData, size_t in_size, VkInternalAllocationType in_allocationType, VkSystemAllocationScope in_allocationScope )
{
    // vkCtx.allocedMemory -= size; 
    //std::printf("[Vulkan] Internal free of %zu bytes, total %i\n", in_size, 0 );
}
