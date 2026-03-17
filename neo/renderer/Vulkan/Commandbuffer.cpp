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

#include "Commandbuffer.hpp"
#include "Core.hpp"

vkCommandbuffer::vkCommandbuffer( void )
{
}

vkCommandbuffer::~vkCommandbuffer( void )
{
}

bool vkCommandbuffer::Create(void)
{
    VkResult result = VK_SUCCESS;
    crVulkanRenderDevicep device = tr.GetRenderDevice();
    m_graphicQueue = device->GraphicQueue();

    /// reserve array 
    m_commandBuffers.SetNum( SMP_FRAMES );
    m_submitFinish.SetNum( SMP_FRAMES );
    m_frameFences.SetNum( SMP_FRAMES );

    // allocate command buffers
    VkCommandBufferAllocateInfo commandBufferAllocateCI{};
    commandBufferAllocateCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateCI.pNext = nullptr;
    commandBufferAllocateCI.commandPool = m_graphicQueue->CommandPool();
    commandBufferAllocateCI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateCI.commandBufferCount = SMP_FRAMES;
    result = vkAllocateCommandBuffers( *device, &commandBufferAllocateCI, m_commandBuffers.Ptr() );
    if( result != VK_SUCCESS )
    {
        common->Error( "vkCommandbuffer::Create::vkAllocateCommandBuffers:%s\n", VulkanErrorString( result ).c_str() );
        return false;
    }

    // Semaphore configuration
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

        // Fence configuration
    VkFenceCreateInfo fenceCI{};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Signaled, so we don't stuck at first frames 

        // alloc the structures arrays 
    for ( uint32_t i = 0; i < SMP_FRAMES; i++)
    {
        result = vkCreateSemaphore( *device, &semaphoreInfo, k_allocationCallbacks, &m_submitFinish[i] );
        if( result != VK_SUCCESS )
        {
            common->Error( "crvkSwapchain::Create::vkCreateSemaphore %s\n", VulkanErrorString( result ).c_str() );
            return false;
        }

        // create the fence object
        result = vkCreateFence( *device, &fenceCI, k_allocationCallbacks, &m_frameFences[i] );
        if( result != VK_SUCCESS )
        {
            common->Error( "crvkSwapchain::Create::vkCreateFence %s\n", VulkanErrorString( result ).c_str() );
            return false;
        }
    }

    return true;
}

void vkCommandbuffer::Destroy(void)
{
    uint32_t i = 0;
    crVulkanRenderDevicep device = tr.GetRenderDevice();
 
    for ( i = 0; i < SMP_FRAMES; i++)
    {
        vkDestroySemaphore( *device, m_submitFinish[i], k_allocationCallbacks );
        vkDestroyFence( *device, m_frameFences[i], k_allocationCallbacks );
    }
 
    if ( m_commandBuffers[0] != nullptr )
        vkFreeCommandBuffers( *device, m_graphicQueue->CommandPool(), SMP_FRAMES, m_commandBuffers.Ptr() );
}


void vkCommandbuffer::Begin( const uint32_t in_bufferID )
{
    VkResult result = VK_SUCCESS;
    m_bufferID = in_bufferID;
    crVulkanRenderDevicep device = tr.GetRenderDevice();

    //
    // Wait for the device finish last render in previous match frame, before reuse command buffer
    result = vkWaitForFences( *device, 1, &m_frameFences[m_bufferID], VK_TRUE, UINT64_MAX );
    if ( result != VK_SUCCESS )
        common->Error( "vkCommandbuffer::Begin::vkWaitForFences: %s\n", VulkanErrorString( result ).c_str() );
    else
        vkResetFences( *device, 1, &m_frameFences[m_bufferID] );

    ///
    /// Reset the main render command buffer
    result = vkResetCommandBuffer( CommandBuffer(), 0 );
    if( result != VK_SUCCESS )
        common->Warning( "vkCommandbuffer::Begin::vkResetCommandBuffer::FAILED!\n" );

    ///
    /// Begin register commands in curren buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT /*VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT*/; // we only submit one time per frame 
    result = vkBeginCommandBuffer( CommandBuffer(), &beginInfo );
    if( result != VK_SUCCESS )
        common->Warning( "vkCommandbuffer::Begin::vkBeginCommandBuffer FAILED to begin!\n" );
}

void vkCommandbuffer::Submit(  const VkSemaphore in_imageAvailable )
{
    VkResult result = VK_SUCCESS;

    //
    // Finish record draw commands
    result = vkEndCommandBuffer( m_commandBuffers[m_bufferID] );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandBuffer::Begin FAILED!\n" );

    ///
    /// Wait for semaphores
    /// We wait for swap chain aquire a image
    VkSemaphoreSubmitInfo       wait{};
    wait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    wait.pNext = nullptr;
    wait.semaphore = in_imageAvailable;
    wait.value = 0;
    wait.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; 
    wait.deviceIndex = 0; 

    ///
    /// Signal semaphores
    /// Signal that render is done
    VkSemaphoreSubmitInfo       signal{};
    signal.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signal.pNext = nullptr;
    signal.semaphore = m_submitFinish[m_bufferID];
    signal.value = 0;
    signal.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; 
    signal.deviceIndex = 0; 

    ///
    /// Set command buffer to be submited
    VkCommandBufferSubmitInfo   commandBufferSubmit{};
    commandBufferSubmit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmit.pNext = nullptr;
    commandBufferSubmit.commandBuffer = CommandBuffer();
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
    result = vkQueueSubmit2( m_graphicQueue->Queue(), 1, &submitInfo, m_frameFences[m_bufferID] );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandbuffer::Submit::vkQueueSubmit2 FAILED!\n" );
}