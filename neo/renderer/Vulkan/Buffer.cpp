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

#include "Buffer.hpp"
#include "Core.hpp"

VkAccessFlags2 bufferAccess[] =
{
    VK_ACCESS_2_TRANSFER_WRITE_BIT,
    VK_ACCESS_2_TRANSFER_READ_BIT
};

VkBufferUsageFlags bufferUsageFlags[]
{
    0, // BUFFER_TYPE_UNDEFINED
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,    // BUFFER_TYPE_INDEX
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,   // BUFFER_TYPE_VERTEX
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, // BUFFER_TYPE_COMMANDS
    /* VK_BUFFER_USAGE_TRANSFER_DST_BIT |*/ VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,  // BUFFER_TYPE_SHADER
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,   // BUFFER_TYPE_SOURCE
    VK_BUFFER_USAGE_TRANSFER_DST_BIT    // BUFFER_TYPE_DESTINATION
};

crBuffer::crBuffer( void )
{
}

crBuffer::~crBuffer( void )
{
    Destroy();
}

bool crBuffer::Create( const type_t in_type, const access_t in_acess, const size_t in_size )
{
    uint32_t graphicFamily = 0;
    VkResult result = VK_SUCCESS;
    auto device = tr.GetRenderDevice();
    auto graphic = device->GraphicQueue();
    graphicFamily = graphic->Family(); 

    //m_access = bufferAccess[in_acess];
    m_usage = bufferUsageFlags[in_acess];

    ///
    ///
    /// Create buffer object
    VkBufferCreateInfo buffer{};
    buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer.pNext = nullptr;
    buffer.flags = 0;
    buffer.size = in_size;
    buffer.usage = m_usage;
    buffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer.queueFamilyIndexCount = 1;
    buffer.pQueueFamilyIndices = &graphicFamily;
    result = vkCreateBuffer( *device, &buffer, k_allocationCallbacks, &m_buffer );
    if ( result != VK_SUCCESS) 
    {
        common->Error( "crvkBufferStatic::Create::vkCreateBuffer" );
        return false;
    }

    vkGetBufferMemoryRequirements( *device, m_buffer, &m_memoryRequirements );

    return true;
}

bool crBuffer::Storage( crMemoryPool *in_bufferPool )
{
    m_page = in_bufferPool->AllocPage( m_memoryRequirements.size, m_memoryRequirements.alignment );
    m_page->Bind( m_buffer );
    return true;
}

void crBuffer::Destroy( void )
{
    auto device = tr.GetRenderDevice();

    // destroi client handle buffer
    if( m_buffer != nullptr )
    {
        vkDestroyBuffer( *device, m_buffer, k_allocationCallbacks );
        m_buffer = nullptr;
    }
}

void crBuffer::SetState( const crCommandBufferp in_commandBuffer, const state_t in_newState )
{
    /// nothing to update
    if ( m_state == in_newState )
        return;

    // 
    VkBufferMemoryBarrier2 destinationBarrier{};
    destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    destinationBarrier.pNext = nullptr;
    destinationBarrier.srcStageMask = m_state.stage;
    destinationBarrier.srcAccessMask = m_state.access;
    destinationBarrier.dstStageMask = in_newState.stage;
    destinationBarrier.dstAccessMask = in_newState.access;
    destinationBarrier.srcQueueFamilyIndex = m_state.queueFamily;
    destinationBarrier.dstQueueFamilyIndex = in_newState.queueFamily;
    destinationBarrier.buffer = m_buffer;

    // update whole buffer, no region change
    destinationBarrier.offset = 0;  
    destinationBarrier.size = VK_WHOLE_SIZE;

    // insert a state transition to destination
    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;
    dependencyInfo.dependencyFlags = 0;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.pMemoryBarriers = nullptr;
    dependencyInfo.bufferMemoryBarrierCount = 1;
    dependencyInfo.pBufferMemoryBarriers = &destinationBarrier;
    dependencyInfo.imageMemoryBarrierCount = 0;
    dependencyInfo.pImageMemoryBarriers = nullptr;
    vkCmdPipelineBarrier2( *in_commandBuffer, &dependencyInfo );
    
    m_state = in_newState;
}

void crBuffer::Flush( const uintptr_t in_offset, const size_t in_size ) const
{
    idassert( m_page != nullptr );
    m_page->Flush( in_offset, in_size );
}
