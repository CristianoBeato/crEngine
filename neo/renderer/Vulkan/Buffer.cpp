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
#include "renderer_common.h"
#include "Vulkan/Vulkan.hpp"
#include "vkBuffer.hpp"

bool vkBuffer::Create( const crBuffer::type_t in_type, const crBuffer::access_t in_access, const size_t in_size )
{
    VkResult result = VK_SUCCESS;
    VkBufferCreateInfo      clientBufferInfo{};
    VkBufferCreateInfo      hostBufferInfo{};
    VkMemoryRequirements    clientMemRequirements{};
    VkMemoryRequirements    hostMemRequirements{};
    VkMemoryAllocateInfo    clientAllocInfo{};
    VkMemoryAllocateInfo    hostAllocInfo{};
    VkMemoryPropertyFlags   clientMemoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VkMemoryPropertyFlags   hostMemoryProperty = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    vkRenderDevice*         device = tr.vkContext->Device();

    // strore the buffer type and the acess
    m_type = in_type;
    m_access = in_access;
    m_device = *device;

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
        case BUFFER_ACCESS_WRITE:
            m_state.access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            clientBufferInfo.usage = m_state.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        case BUFFER_ACCESS_READ:
            m_state.access = VK_ACCESS_2_TRANSFER_READ_BIT;
            clientBufferInfo.usage = m_state.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
    default:
        common->Error( "vkBuffer::Create: Error invalid \"in_acess\"\n" );
        return false;
    }
    
    switch ( m_type )
    {
        /// Index buffer 
        case BUFFER_TYPE_INDEX:
            hostBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        /// Vertex buffer 
        case BUFFER_TYPE_VERTEX:
            hostBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        /// Shader Storage buffer
        case BUFFER_TYPE_SHADER:
            hostBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        /// Indirect draw command buffer 
        case BUFFER_TYPE_COMMANDS:
            hostBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            break;

        /// pixel buffer are a CPU Side only
        case BUFFER_TYPE_PIXEL:
            break;

    default:
        return false; // invalid
        break;
    };

    ///
    ///
    /// Create the client buffer
    result = vkCreateBuffer( m_device, &clientBufferInfo, k_allocationCallbacks, &m_bufferClient );
    if ( result != VK_SUCCESS) 
    {
        common->Error( "crvkBufferStatic::Create::vkCreateBuffer" );
        return false;
    }

    vkGetBufferMemoryRequirements( m_device, m_bufferClient, &clientMemRequirements );

    /// try find the required memory 
    clientAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    clientAllocInfo.allocationSize = clientMemRequirements.size;
    clientAllocInfo.memoryTypeIndex = device->FindMemoryType( clientMemRequirements.memoryTypeBits, clientMemoryProperty );
    result = vkAllocateMemory( m_device, &clientAllocInfo, k_allocationCallbacks, &m_memoryClient );
    if ( result != VK_SUCCESS ) 
    {
        common->Warning( "vkBuffer::Create::vkAllocateMemory failed" );
        return false;
    }

    result = vkBindBufferMemory( m_device, m_bufferClient, m_memoryClient, 0 );
    if ( result != VK_SUCCESS )
    {
        common->Warning( "vkBuffer::Create::vkBindBufferMemory" );
        return false;
    }

    // map client buffer memory
    result = vkMapMemory( m_device, m_memoryClient, 0, VK_WHOLE_SIZE, 0, &m_data );

    // pixel buffers are client only
    if ( m_type != BUFFER_TYPE_PIXEL )
    {
        ///
        ///
        /// Create the host buffer
        result = vkCreateBuffer( m_device, &hostBufferInfo, k_allocationCallbacks, &m_bufferHost );
        if ( result != VK_SUCCESS) 
        {
            common->Error( "crvkBufferStatic::Create::vkCreateBuffer" );
            return false;
        }

        vkGetBufferMemoryRequirements( m_device, m_bufferHost, &hostMemRequirements );

        /// try find the required memory 
        hostAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        hostAllocInfo.allocationSize = hostMemRequirements.size;
        hostAllocInfo.memoryTypeIndex = device->FindMemoryType( hostMemRequirements.memoryTypeBits, hostMemoryProperty );
        result = vkAllocateMemory( m_device, &hostAllocInfo, k_allocationCallbacks, &m_memoryHost );
        if ( result != VK_SUCCESS ) 
        {
            common->Warning( "vkBuffer::Create::vkAllocateMemory failed" );
            return false;
        }

        result = vkBindBufferMemory( m_device, m_bufferHost, m_memoryHost, 0 );
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
        result = vkAllocateCommandBuffers( m_device, &commandBufferAllocateCI, &m_copyCmd );
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
    //
    if ( m_copyCmd != nullptr )
    {
        vkFreeCommandBuffers( m_device, nullptr, 1, &m_copyCmd );
        m_copyCmd = nullptr;
    }
    
    // release client memory 
    if( m_memoryClient != nullptr )
    {
        vkUnmapMemory( m_device, m_memoryClient );
        vkFreeMemory( m_device, m_memoryClient, k_allocationCallbacks );
        m_memoryClient = nullptr;
    }

    // destroi client handle buffer
    if( m_bufferClient != nullptr )
    {
        vkDestroyBuffer( m_device, m_bufferHost, k_allocationCallbacks );
        m_bufferClient = nullptr;
    }

    // release host memorys
    if( m_memoryHost != nullptr )
    {
        vkFreeMemory( m_device, m_memoryHost, k_allocationCallbacks );
        m_memoryHost = nullptr;
    }

    // release host buffer handle
    if ( m_bufferHost != nullptr )
    {
        vkDestroyBuffer( m_device, m_bufferHost, k_allocationCallbacks );
        m_bufferHost = nullptr;
    }

    m_device = nullptr;
}

void vkBuffer::CopyBuffer(const crBuffer *in_source, const uintptr_t in_srcOffset, const uintptr_t in_dstOffset, const size_t in_size) const
{
    VkCopyBufferInfo2 copyBufferInfo{};
    VkBufferCopy2 bufferCopy{};
    assert( in_source != nullptr && in_source->Handle() != nullptr );

    // Region of the buffer to perform a copy
    bufferCopy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
    bufferCopy.pNext = nullptr;
    bufferCopy.srcOffset = in_srcOffset;
    bufferCopy.dstOffset = in_dstOffset;
    bufferCopy.size = in_size;

    copyBufferInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
    copyBufferInfo.pNext = nullptr;
    copyBufferInfo.srcBuffer = static_cast<VkBuffer>( in_source->Handle() );
    copyBufferInfo.dstBuffer = m_type == BUFFER_TYPE_PIXEL ? m_bufferClient : m_bufferHost; // we direct copy to host buffer ( if not a pixel buffer )
    copyBufferInfo.regionCount = 1;
    copyBufferInfo.pRegions = &bufferCopy;
    
    // registes our copy command 
    vkCmdCopyBuffer2( m_copyCmd, &copyBufferInfo );
}

void vkBuffer::Flush(const uintptr_t in_offset, const size_t in_size ) const
{
    VkMappedMemoryRange memoryRange{};
    memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    memoryRange.pNext = nullptr;
    memoryRange.memory = m_memoryClient;
    memoryRange.offset = in_offset;
    memoryRange.size = in_size;
    vkFlushMappedMemoryRanges( m_device, 1, &memoryRange );

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
            const_cast<vkBuffer*>( this )->StateTransition( BUFFER_STATE_COPY_SOURCE, in_offset, in_size );
            srcBuffer = m_bufferHost;
            dstBuffer = m_bufferClient;
        }
        else
        {
            // we gona copy the client data to the host
            const_cast<vkBuffer*>( this )->StateTransition( BUFFER_STATE_COPY_DESTINATION, in_offset, in_size );
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

#if 0
void vkBuffer::StateTransition(const state_t in_state, const uintptr_t in_offset, const size_t in_size )
{
    VkResult                result = VK_SUCCESS;
    uint32_t                family = 0;
    VkAccessFlags2          access = VK_ACCESS_2_NONE;
    VkPipelineStageFlags2   stage = VK_PIPELINE_STAGE_2_NONE;
    vkRenderDevice*         device = tr.vkContext->Device();

    // no need to change
    if ( in_state == m_state )
        return;

    if ( in_state == BUFFER_STATE_COPY_DESTINATION ||  in_state == BUFFER_STATE_COPY_SOURCE )
    {
        // Reset command buffer before begin re use it
        result = vkResetCommandBuffer( m_copyCmd, 0 );
        if( result != VK_SUCCESS )
            common->Error( "vkBuffer::StateTransition::vkResetCommandBuffer FAILED to reset!\n" );

        // Begin register commands in curren buffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // we submit multiple times per frame 
        result = vkBeginCommandBuffer( m_copyCmd, &beginInfo );
        if( result != VK_SUCCESS )
            common->Error( "vkBuffer::StateTransition::vkBeginCommandBuffer FAILED to begin!\n" );
    }
    
    switch ( in_state )
    {    
    case BUFFER_STATE_COPY_DESTINATION:
    {
        stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        family = device->TransferQueue()->Family();
    } break;
    case BUFFER_STATE_COPY_SOURCE:
    {
        stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        access = VK_ACCESS_2_TRANSFER_READ_BIT;
        family = device->TransferQueue()->Family();
    } break;
    case BUFFER_STATE_USE_RENDER:
    {
        switch ( m_type )
        {
        case BUFFER_TYPE_INDEX:
        {
            stage = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
            access = VK_ACCESS_2_INDEX_READ_BIT;
        } break;
        case BUFFER_TYPE_VERTEX:
        {
            stage = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
            access = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
        } break;
        case BUFFER_TYPE_SHADER:
        {
            stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
            access = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        } break;
        case BUFFER_TYPE_COMMANDS:
        {
            stage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
            access = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        } break;
        default:
            break;
        }

        family = device->GraphicQueue()->Family();
    } break;
    case BUFFER_STATE_USE_COMPUTE:
    {
        switch ( m_type )
        {
        case BUFFER_TYPE_SHADER:
        {
            stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            access = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        } break;
        case BUFFER_TYPE_COMMANDS:
        {
            stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            access = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        } break;
        default:
            break;
        }

        family = device->ComputeQueue()->Family();
    } break;
    case BUFFER_STATE_WRITE_COMPUTE:
    {
        // the output is indiferent, we just write to buffer 
        stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        access = VK_ACCESS_2_SHADER_WRITE_BIT;
        family = device->ComputeQueue()->Family();
    } break;
    default:
        break;
    }

    // if we change the state to use, we can flush the copy operation
    if ( in_state == BUFFER_STATE_USE_RENDER || in_state == BUFFER_STATE_USE_COMPUTE )
    {
        crCommandBuffer* cmd = nullptr;
        // End register commands in current buffer
        result = vkEndCommandBuffer( m_copyCmd );
        if( result != VK_SUCCESS )
            common->Error( "vkBuffer::StateTransition::vkEndCommandBuffer FAILED!\n" );

        // flush to main command buffer
        if ( in_state == BUFFER_STATE_USE_RENDER )
            cmd = backEnd.GetRenderCMD();
        else if( in_state == BUFFER_STATE_USE_COMPUTE  )
            cmd = backEnd.GetComputeCMD();
        dynamic_cast<vkCommandBuffer*>( cmd )->ExecuteCommands( 1, &m_copyCmd );
    }
}
#endif

void *vkBuffer::Handle(void) const
{
    return const_cast<VkBuffer*>( &m_bufferHost );
}

// help to remember
// VK_BUFFER_USAGE_TRANSFER_SRC_BIT
// VK_BUFFER_USAGE_TRANSFER_DST_BIT
// VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
// VK_BUFFER_USAGE_INDEX_BUFFER_BIT
// VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
// VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT
// VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT

void vkBuffer::StateTransition(const state_t in_state, const crCommandBuffer *in_commandBuffer)
{
    bufferState_t   state{};
    auto commandbuffer = dynamic_cast<const vkCommandBuffer*>( in_commandBuffer );

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
            state.queueFamily = commandbuffer->Family();
        } break;

        /// we gona use buffer as data copy source
        case RESOURCE_STATE_COPY_SOURCE:
        {
            state.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            state.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            state.access = VK_ACCESS_2_TRANSFER_READ_BIT;
            state.queueFamily = commandbuffer->Family();
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

            state.queueFamily = commandbuffer->Family();
        } break;
        case RESOURCE_STATE_USE_COMPUTE:
        {
        } break;
        case RESOURCE_STATE_WRITE_COMPUTE:
        {
        } break;
        case RESOURCE_STATE_WRITE_RENDER:
        {
            
        } break;
    }



    case BUFFER_STATE_USE_COMPUTE:
    {
        switch ( m_type )
        {
        case BUFFER_TYPE_SHADER:
        {
            stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            access = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        } break;
        case BUFFER_TYPE_COMMANDS:
        {
            stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            access = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        } break;
        default:
            break;
        }

        family = device->ComputeQueue()->Family();
    } break;
    case BUFFER_STATE_WRITE_COMPUTE:
    {
        // the output is indiferent, we just write to buffer 
        stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        access = VK_ACCESS_2_SHADER_WRITE_BIT;
        family = device->ComputeQueue()->Family();
    } break;
 
    SetState( state, commandbuffer->CommandBuffer() );
}

void vkBuffer::SetState(const bufferState_t &in_state, const VkCommandBuffer in_commandBuffer)
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
