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

#include "Core.hpp"
#include "Memory.hpp"

/// Helper ( i just can't save the do Right now )
/// https://www.khronos.org/assets/uploads/developers/library/2018-vulkan-devday/03-Memory.pdf

crMemoryPool::crMemoryPool( void ) : m_size( 0 )
{
}

crMemoryPool::~crMemoryPool( void )
{
    Destroy();
}

bool crMemoryPool::Create( const size_t in_size )
{


    return true;
}

void crMemoryPool::Destroy(void)
{
    if( m_memory != nullptr )
    {
        vkFreeMemory( m_device, m_memory, k_allocationCallbacks );
        m_memory = nullptr;
    }
}
