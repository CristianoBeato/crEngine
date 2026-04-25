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

#include "idlib/precompiled.h"
#include "renderer/renderer_common.h"
#include "Core.hpp"
#include "Commandbuffer.hpp"

crCommandBuffer::crCommandBuffer( void ) :
    m_frameID( 0 ),
    m_frameCount( 0 ),
    m_queue( nullptr )
{
}

crCommandBuffer::~crCommandBuffer( void )
{
}

bool crCommandBuffer::Create( const uint32_t in_frameCount, const vkDeviceQueuep in_queue, const bool in_primary )
{
    crVulkanRenderDevicep device = tr.GetRenderDevice();
    assert( device != nullptr );
    
    m_queue = in_queue;
    m_frameCount = in_frameCount;
    
    m_commandBuffers.SetNum( m_frameCount );

    // allocate command buffers
    VkCommandBufferAllocateInfo commandBufferAllocateCI{};
    commandBufferAllocateCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateCI.pNext = nullptr;
    commandBufferAllocateCI.commandPool = m_queue->CommandPool();
    commandBufferAllocateCI.level = in_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    commandBufferAllocateCI.commandBufferCount = m_frameCount;
    VkResult result = vkAllocateCommandBuffers( *device, &commandBufferAllocateCI, m_commandBuffers );
    if( result != VK_SUCCESS )
    {
        idLib::Error( "crCommandBuffer::Create::vkAllocateCommandBuffers:%s\n", VulkanErrorString( result ).c_str() );
        return false;
    }

    return true;
}

void crCommandBuffer::Destroy(void)
{
    uint32_t i = 0;
    crVulkanRenderDevicep device = tr.GetRenderDevice();
 
    if ( m_commandBuffers[0] != nullptr )
        vkFreeCommandBuffers( *device, m_queue->CommandPool(), SMP_FRAMES, m_commandBuffers );
}


void crCommandBuffer::Begin( void )
{
    VkResult result = VK_SUCCESS;
    crVulkanRenderDevicep device = tr.GetRenderDevice();

    ///
    /// Reset the main render command buffer
    result = vkResetCommandBuffer( m_commandBuffers[m_frameID], 0 );
    if( result != VK_SUCCESS )
        common->Warning( "crCommandBuffer::Begin::vkResetCommandBuffer::FAILED!\n" );

    ///
    /// Begin register commands in curren buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT /*VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT*/; // we only submit one time per frame 
    result = vkBeginCommandBuffer( m_commandBuffers[m_frameID], &beginInfo );
    if( result != VK_SUCCESS )
        common->Warning( "crCommandBuffer::Begin::vkBeginCommandBuffer FAILED to begin!\n" );
}

void crCommandBuffer::Execute(const crCommandBuffer *in_commandBuffer)
{
    auto subcmd = in_commandBuffer->CommandBuffer();
    vkCmdExecuteCommands( CommandBuffer(), 1, &subcmd );
}

void crCommandBuffer::Submit(  const crSemaphore* in_imageAvailable, const crSemaphore* in_renderDone, const crFence* in_frameFence )
{
    VkResult result = VK_SUCCESS;

    //
    // Finish record draw commands
    result = vkEndCommandBuffer( m_commandBuffers[m_frameID] );
    if( result != VK_SUCCESS )
        idLib::Error( "vkCommandBuffer::Begin FAILED!\n" );

    ///
    /// Wait for semaphores
    /// We wait for swap chain aquire a image
    VkSemaphoreSubmitInfo wait = *in_imageAvailable;
    
    ///
    /// Signal semaphores
    /// Signal that render is done
    VkSemaphoreSubmitInfo       signal = *in_renderDone;
    
    ///
    /// Set command buffer to be submited
    VkCommandBufferSubmitInfo   commandBufferSubmit{};
    commandBufferSubmit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmit.pNext = nullptr;
    commandBufferSubmit.commandBuffer = m_commandBuffers[m_frameID];
    commandBufferSubmit.deviceMask = 0;

    ///
    /// Sumit frame command buffer to GPU
    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &wait;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmit;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signal;
    result = vkQueueSubmit2( m_queue->Queue(), 1, &submitInfo, *in_frameFence );
    if( result != VK_SUCCESS )
        idLib::Error( "crCommandBuffer::Submit::vkQueueSubmit2 FAILED!\n" );
}

