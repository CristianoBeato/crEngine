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

VkAccessFlags2 access[]
{
    VK_ACCESS_2_TRANSFER_WRITE_BIT,
    VK_ACCESS_2_TRANSFER_READ_BIT
};

VkBufferUsageFlags usages[]
{
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT
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
    VkResult result = VK_SUCCESS;
    auto device = tr.GetRenderDevice();
    
    ///
    ///
    /// Create buffer object
    VkBufferCreateInfo buffer{};
    buffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer.pNext = nullptr;
    buffer.flags = 0;
    buffer.size = in_size;
    buffer.usage = ;
    buffer.sharingMode = ;
    buffer.queueFamilyIndexCount = ;
    buffer.pQueueFamilyIndices = ;
    result = vkCreateBuffer( *device, &buffer, k_allocationCallbacks, &m_buffer );
    if ( result != VK_SUCCESS) 
    {
        common->Error( "crvkBufferStatic::Create::vkCreateBuffer" );
        return false;
    }

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

void crBuffer::SetState(const vkCommandbufferp in_commandBuffer, const state_t in_newState)
{
    // 
    VkBufferMemoryBarrier2 destinationBarrier{};
    destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    destinationBarrier.pNext = nullptr;
    destinationBarrier.srcStageMask = m_stage;
    destinationBarrier.srcAccessMask = m_access;
    destinationBarrier.dstStageMask = in_newState.stage;
    destinationBarrier.dstAccessMask = in_newState.access;
    destinationBarrier.srcQueueFamilyIndex = m_queueFamily;
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
    
    m_stage = in_newState.stage; 
    m_access = in_newState.access;
    m_queueFamily = in_newState.queueFamily;
}

void crBuffer::Flush(const uintptr_t in_offset, const size_t in_size) const
{
    idassert( m_page != nullptr );
    m_page->Flush( in_offset, in_size );
}


bool vkBuffer::Create( const type_t in_type, const access_t in_access, const size_t in_size )
{
    
    VkBufferCreateInfo      clientBufferInfo{};
    VkBufferCreateInfo      hostBufferInfo{};
    VkMemoryRequirements    clientMemRequirements{};
    VkMemoryRequirements    hostMemRequirements{};
    VkMemoryAllocateInfo    clientAllocInfo{};
    VkMemoryAllocateInfo    hostAllocInfo{};
    VkMemoryPropertyFlags   clientMemoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryPropertyFlags   hostMemoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    auto device             = tr.GetRenderDevice();

    // strore the buffer type and the acess
    m_type = in_type;
    m_access = in_access;

    /// check if transfer and compute 
    auto graphic = device->GraphicQueue();
    auto transfer = device->TransferQueue();

    ///
    /// host buffer are the device ( GPU ) side of the memory, stored in vram
    hostBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    hostBufferInfo.pNext = nullptr;
    hostBufferInfo.flags = 0;
    hostBufferInfo.size = in_size;
    hostBufferInfo.usage = 0;
    hostBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    hostBufferInfo.queueFamilyIndexCount = 0;
    hostBufferInfo.pQueueFamilyIndices = 0;
    
    ///
    /// client buffer are a CPU side of the memory buffer, stored by the driver 
    clientBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    clientBufferInfo.pNext = nullptr;
    clientBufferInfo.flags = 0;
    clientBufferInfo.size = in_size;
    clientBufferInfo.usage = 0;
    clientBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    clientBufferInfo.queueFamilyIndexCount = 0;
    clientBufferInfo.pQueueFamilyIndices = 0;

    /// check if the transfer and graphic compute
    if( graphic->Family() != transfer->Family() )
    {
        // the host are used by the tranfer queue and the 
        hostBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
        // client are used only by the transfer queue 
        clientBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 
    }
    else
    {
        // buffers are used only by the graphic queue
        hostBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        clientBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;        
    }

    /// clien is a sorce or destination to data 
    switch ( m_access )
    {
            break;
    default:
        common->Error( "vkBuffer::Create: Error invalid \"in_acess\"\n" );
        return false;
    }
    
    switch ( m_type )
    {
        /// Index buffer 
        case BUFFER_TYPE_INDEX:
        break;
        /// Vertex buffer 
        case BUFFER_TYPE_VERTEX:
        break;
        /// Shader Storage buffer
        case BUFFER_TYPE_SHADER:
        break;
        /// Indirect draw command buffer 
        case BUFFER_TYPE_COMMANDS:
        break;
        
        /// pixel buffer are a CPU Side only
        case BUFFER_TYPE_PIXEL:
            break;

    default:
        return false; // invalid
        break;
    };

    

    vkGetBufferMemoryRequirements( *device, m_bufferClient, &clientMemRequirements );

    /// try find the required memory 
    clientAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    clientAllocInfo.allocationSize = clientMemRequirements.size;
    clientAllocInfo.memoryTypeIndex = device->FindMemoryType( clientMemRequirements.memoryTypeBits, clientMemoryProperty );
    result = vkAllocateMemory( *device, &clientAllocInfo, k_allocationCallbacks, &m_memoryClient );
    if ( result != VK_SUCCESS ) 
    {
        common->Warning( "vkBuffer::Create::vkAllocateMemory failed" );
        return false;
    }

    result = vkBindBufferMemory( *device, m_bufferClient, m_memoryClient, 0 );
    if ( result != VK_SUCCESS )
    {
        common->Warning( "vkBuffer::Create::vkBindBufferMemory" );
        return false;
    }

    // map client buffer memory
    result = vkMapMemory( *device, m_memoryClient, 0, VK_WHOLE_SIZE, 0, &m_data );

    // pixel buffers are client only
    if ( m_type != BUFFER_TYPE_PIXEL )
    {
        ///
        ///
        /// Create the host buffer
        result = vkCreateBuffer( *device, &hostBufferInfo, k_allocationCallbacks, &m_bufferHost );
        if ( result != VK_SUCCESS) 
        {
            common->Error( "crvkBufferStatic::Create::vkCreateBuffer" );
            return false;
        }

        vkGetBufferMemoryRequirements( *device, m_bufferHost, &hostMemRequirements );

        /// try find the required memory 
        hostAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        hostAllocInfo.allocationSize = hostMemRequirements.size;
        hostAllocInfo.memoryTypeIndex = device->FindMemoryType( hostMemRequirements.memoryTypeBits, hostMemoryProperty );
        result = vkAllocateMemory( *device, &hostAllocInfo, k_allocationCallbacks, &m_memoryHost );
        if ( result != VK_SUCCESS ) 
        {
            common->Warning( "vkBuffer::Create::vkAllocateMemory failed" );
            return false;
        }

        result = vkBindBufferMemory( *device, m_bufferHost, m_memoryHost, 0 );
        if ( result != VK_SUCCESS )
        {
            common->Warning( "vkBuffer::Create::vkBindBufferMemory" );
            return false;
        }

        //
        // allocate a command buffer to copy buffer content
        VkCommandBufferAllocateInfo commandBufferAllocateCI{};
        commandBufferAllocateCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateCI.pNext = nullptr;
        commandBufferAllocateCI.commandPool = transfer->CommandPool();
        commandBufferAllocateCI.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        commandBufferAllocateCI.commandBufferCount = 1;
        result = vkAllocateCommandBuffers( *device, &commandBufferAllocateCI, &m_copyCmd );
        if( result != VK_SUCCESS )
        {
            common->Error( "Failed to create command buffer" );
            return false;
        }
    }

    return true;
}

bool vkBuffer::Resize(const size_t in_newSize)
{
    // TODO: reacreate the buffer and copy his content

    return false;
}

void vkBuffer::Destroy(void)
{
    auto device = tr.GetRenderDevice();

    //
    if ( m_copyCmd != nullptr )
    {
        vkFreeCommandBuffers( *device, nullptr, 1, &m_copyCmd );
        m_copyCmd = nullptr;
    }
    
    // release client memory 
    if( m_memoryClient != nullptr )
    {
        vkUnmapMemory( *device, m_memoryClient );
        vkFreeMemory( *device, m_memoryClient, k_allocationCallbacks );
        m_memoryClient = nullptr;
    }



    // release host memorys
    if( m_memoryHost != nullptr )
    {
        vkFreeMemory( *device, m_memoryHost, k_allocationCallbacks );
        m_memoryHost = nullptr;
    }

    // release host buffer handle
    if ( m_bufferHost != nullptr )
    {
        vkDestroyBuffer( *device, m_bufferHost, k_allocationCallbacks );
        m_bufferHost = nullptr;
    }
}

void vkBuffer::Flush(const uintptr_t in_offset, const size_t in_size ) const
{
    
    VkMappedMemoryRange memoryRange{};
    memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.pNext = nullptr;
    memoryRange.memory = m_memoryClient;
    memoryRange.offset = in_offset;
    memoryRange.size = in_size;
    vkFlushMappedMemoryRanges( *device, 1, &memoryRange );

    if( m_bestate.usage != BUFFER_TYPE_PIXEL )
    {
        VkBuffer srcBuffer = nullptr;
        VkBuffer dstBuffer = nullptr;
        VkCopyBufferInfo2 copyBufferInfo{};
        VkBufferCopy2 bufferCopy{};

        // we need to perform a internal state transition to be surre
        if ( m_access == BUFFER_ACCESS_READ )
        {
            // we gona copy the host memory to client 
            const_cast<vkBuffer*>( this )->StateTransition( RESOURCE_STATE_COPY_SOURCE );
            srcBuffer = m_bufferHost;
            dstBuffer = m_bufferClient;
        }
        else
        {
            // we gona copy the client data to the host
            const_cast<vkBuffer*>( this )->StateTransition( RESOURCE_STATE_COPY_DESTINATION );
            srcBuffer = m_bufferClient;
            dstBuffer = m_bufferHost;
        }

        // Region of the buffer to perform a copy
        bufferCopy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
        bufferCopy.pNext = nullptr;
        bufferCopy.srcOffset = in_offset;
        bufferCopy.dstOffset = in_offset;
        bufferCopy.size = in_size;

        copyBufferInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
        copyBufferInfo.pNext = nullptr;
        copyBufferInfo.srcBuffer = srcBuffer;
        copyBufferInfo.dstBuffer = dstBuffer;
        copyBufferInfo.regionCount = 1;
        copyBufferInfo.pRegions = &bufferCopy;
    
        // registes our copy command 
        vkCmdCopyBuffer2( m_copyCmd, &copyBufferInfo );
    }
}

// help to remember
// VK_BUFFER_USAGE_TRANSFER_SRC_BIT
// VK_BUFFER_USAGE_TRANSFER_DST_BIT
// VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
// VK_BUFFER_USAGE_INDEX_BUFFER_BIT
// VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
// VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
// VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT

void vkBuffer::StateTransition( const state_t in_state )
{
    bufferState_t   state{};

    switch ( in_state )
    {
        case RESOURCE_STATE_UNKNOW:
        {
            common->Warning( "Unknow state passed to buffer" );
        } break;

        /// we gone use buffer as data copy destination 
        case RESOURCE_STATE_COPY_DESTINATION:
        {
            state.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            state.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            state.access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            state.queueFamily = VK_QUEUE_FAMILY_IGNORED;
        } break;

        /// we gona use buffer as data copy source
        case RESOURCE_STATE_COPY_SOURCE:
        {
            state.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            state.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            state.access = VK_ACCESS_2_TRANSFER_READ_BIT;
            state.queueFamily = VK_QUEUE_FAMILY_IGNORED;
        } break;

        /// we gona use data as shader 
        case RESOURCE_STATE_USE_RENDER:
        {
            switch ( m_type )
            {
                case BUFFER_TYPE_INDEX:
                {
                    state.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                    state.stage = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
                    state.access = VK_ACCESS_2_INDEX_READ_BIT;
                } break;
                case BUFFER_TYPE_VERTEX:
                {
                    state.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                    state.stage = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
                    state.access = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                } break;
                case BUFFER_TYPE_SHADER:
                {
                    state.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
                    state.stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
                    state.access = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                } break;
                case BUFFER_TYPE_COMMANDS:
                {
                    state.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
                    state.stage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
                    state.access = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                } break;
                case BUFFER_TYPE_PIXEL:
                {
                    /// TODO: may we never get here 
                    state.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
                    state.stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
                    state.access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
                } break;

                default:
                {
                    common->Warning( "vkBuffer::StateTransition: invalid buffer type!\n" );
                } break;
            }

            state.queueFamily = VK_QUEUE_FAMILY_IGNORED;
        } break;
        case RESOURCE_STATE_USE_COMPUTE:
        {
            state.usage = m_bestate.usage;
            state.stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            state.access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT; // TODO: Fix
            state.queueFamily = VK_QUEUE_FAMILY_IGNORED;

        } break;
        case RESOURCE_STATE_WRITE_COMPUTE:
        {
            state.usage = m_bestate.usage;
            state.stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            state.access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;   // TODO: Fix
            state.queueFamily = VK_QUEUE_FAMILY_IGNORED;

        } break;
        case RESOURCE_STATE_WRITE_RENDER:
        {
            //TODO:
            //state.usage = m_bestate.usage;
            //state.stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            //state.access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
            //state.queueFamily = VK_QUEUE_FAMILY_IGNORED;
        } break;
    }

    SetState( state );

    // update state 
    m_bestate = state;
}

void vkBuffer::SetState( const bufferState_t &in_state )
{
    VkBufferMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    barrier.pNext = nullptr;

    // ignore if no change
    if ( m_bestate == in_state )
        return;

    // source state
    barrier.srcStageMask = m_bestate.stage;
    barrier.srcAccessMask = m_bestate.stage;
    barrier.srcQueueFamilyIndex = m_family;
    
    // destination state state 
    barrier.dstStageMask = in_state.stage;
    barrier.dstAccessMask = in_state.access;
    barrier.dstQueueFamilyIndex = in_state.queueFamily;
    
    // we update whole buffer 
    barrier.buffer = m_bufferHost;
    barrier.offset = 0;
    barrier.size = VK_WHOLE_SIZE;

    // submit barrier
    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;
    depInfo.dependencyFlags = 0;
    depInfo.memoryBarrierCount = 0;
    depInfo.pMemoryBarriers = nullptr;
    depInfo.bufferMemoryBarrierCount = 1;
    depInfo.pBufferMemoryBarriers = &barrier;
    depInfo.imageMemoryBarrierCount = 0;
    depInfo.pImageMemoryBarriers = nullptr;
    vkCmdPipelineBarrier2( m_copyCmd, &depInfo );

    // update buffer state
    m_bestate = in_state;
}
