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
#include "vkCommandBuffer.hpp"

vkCommandBuffer::vkCommandBuffer( const VkQueue in_queue, const VkCommandPool in_commandPool ) : crCommandBuffer(),
    m_frame( 0 ),
    m_frameOperationsFenceCount( 0 ),
    m_frameOperationsFence( 0 ),
    m_device( nullptr ),
    m_commandPool( nullptr ),
    m_queue( nullptr )
{
    VkResult result = VK_SUCCESS;
    auto device = tr.vkContext->Device(); 
    m_device = *device; 
    m_queue = in_queue;
    m_commandPool = in_commandPool;

    //
    // allocate command buffers
    //
    VkCommandBufferAllocateInfo commandBufferAllocateCI{};
    commandBufferAllocateCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateCI.pNext = nullptr;
    commandBufferAllocateCI.commandPool = m_commandPool;
    commandBufferAllocateCI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateCI.commandBufferCount = SMP_FRAMES;
    result = vkAllocateCommandBuffers( m_device, &commandBufferAllocateCI, m_command );
    if( result != VK_SUCCESS )
        throw idException( "Failed to create command buffer" );

    ///
    /// Desciptor pool 
    ///
    VkDescriptorPoolSize poolSizes[2]{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = MAX_BINDING_SAMPLERS; // por ex. 8192
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 16; // quantos SSBOs você planeja (normalmente 1 por binding)

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1; // provavelmente 1 set global

    result = vkCreateDescriptorPool( m_device, &poolInfo, nullptr, &m_descriptorPool);
    if ( result != VK_SUCCESS )
        throw idException( "Failed to create command buffer" );

    //
    //
    //
    VkSemaphoreTypeCreateInfo timelineType{};
    timelineType.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineType.initialValue = 0; // initial timeline value

    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semInfo.pNext = &timelineType;

    result = vkCreateSemaphore( m_device, &semInfo, k_allocationCallbacks, &m_frameOperationsFence );
    if( result != VK_SUCCESS )
        throw idException( "Failed to create command buffer" );
}

vkCommandBuffer::~vkCommandBuffer(void)
{
    if ( m_frameOperationsFence != nullptr )
    {
        vkDestroySemaphore( m_device, m_frameOperationsFence, k_allocationCallbacks );
        m_frameOperationsFence = nullptr;
    }
    
    if ( m_descriptorPool )
    {
        vkDestroyDescriptorPool( m_device, m_descriptorPool, k_allocationCallbacks );
        m_descriptorPool = nullptr;
    }

    if ( m_command[0] != nullptr )
    {
        vkFreeCommandBuffers( m_device, m_commandPool, SMP_FRAMES, m_command );
        std::memset( m_command, 0x00, sizeof( VkCommandBuffer ) * SMP_FRAMES );
    }
}

void vkCommandBuffer::Begin(void)
{
    VkResult result = VK_SUCCESS;

    //
    // wait for buffer be freed
    //
    result = vkWaitForFences( m_device, 1, &m_renderDone[m_frame], VK_TRUE, UINT64_MAX );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandBuffer::Begin FAILED Wait previous frame!\n" );

    //
    // Reset command buffer before begin re use it
    //
    result = vkResetCommandBuffer( m_command[m_frame], 0 );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandBuffer::Begin FAILED to reset!\n" );

    //
    // Begin register commands in curren buffer
    // 
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT /*VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT*/; // we only submit one time per frame 
    result = vkBeginCommandBuffer( m_command[m_frame], &beginInfo );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandBuffer::Begin FAILED to begin!\n" );
}

void vkCommandBuffer::End(void)
{
    VkResult result = VK_SUCCESS;

    //
    // End register commands in current buffer
    //
    result = vkEndCommandBuffer( m_command[m_frame] );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandBuffer::Begin FAILED!\n" );
}

void vkCommandBuffer::Sumit(void)
{
    VkResult                    result = VK_SUCCESS;
    VkSubmitInfo2               submitInfo{};
    VkSemaphoreSubmitInfo       wait{};
    VkSemaphoreSubmitInfo       signal{};
    VkCommandBufferSubmitInfo   commandBufferSubmit{};

    ///
    ///
    ///    wait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    wait.pNext = nullptr;
    wait.semaphore = m_frameOperationsFence;
    wait.value = m_frameOperationsFenceCount;
    wait.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; 
    wait.deviceIndex = 0; 

    ///
    ///
    ///
    commandBufferSubmit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmit.pNext = nullptr;
    commandBufferSubmit.commandBuffer = m_command[m_frame];
    commandBufferSubmit.deviceMask = 0;

    ///
    ///
    ///
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = 0;
    submitInfo.pWaitSemaphoreInfos = &wait;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmit;
#if 0
    submitInfo.signalSemaphoreInfoCount = 0;
    submitInfo.pSignalSemaphoreInfos = &signal;
#else
    submitInfo.signalSemaphoreInfoCount = 0;
    submitInfo.pSignalSemaphoreInfos = nullptr;
#endif

    result = vkQueueSubmit2( m_queue, 1, &submitInfo, m_renderDone[m_frame] );

    // swap buffer
    m_frame = ( m_frame + 1 ) % SMP_FRAMES;
}

void vkCommandBuffer::LineWidth(const float in_lineWidth) const
{
    vkCmdSetLineWidth( m_command[m_frame], in_lineWidth );
}

void vkCommandBuffer::BindFrameBuffer(const crFramebuffer *in_framebuffer )
{
    VkRect2D renderArea{};
    assert( in_framebuffer );
    auto fbh = static_cast<fbHandler_t*>( in_framebuffer->Handle() );
    
    ///
    ///
    ///
    renderArea.offset.x = 0;
    renderArea.offset.y = 0;
    renderArea.extent.width = in_framebuffer->Width();
    renderArea.extent.height = in_framebuffer->Height();

    ///
    ///
    ///
    VkRenderPassBeginInfo   renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = fbh->rp;
    renderPassBeginInfo.framebuffer = fbh->fb;
    renderPassBeginInfo.renderArea = renderArea;
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &m_clearValues;

    ///
    ///
    ///
    VkSubpassBeginInfo subpassBeginInfo{};
    subpassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    subpassBeginInfo.pNext = nullptr;
    subpassBeginInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

    vkCmdBeginRenderPass2( m_command[m_frame], &renderPassBeginInfo, &subpassBeginInfo );
}

void vkCommandBuffer::BindIndexBuffer(const crBuffer *in_buffer)
{
    VkBuffer** buffer = static_cast<VkBuffer**>( in_buffer->Handle() );
    vkCmdBindIndexBuffer( m_command[m_frame], **buffer, 0, VK_INDEX_TYPE_UINT16 );
}

void vkCommandBuffer::BindVertexBuffers(const crBuffer *in_buffer, uint32_t in_binding, const uintptr_t in_offsets, const size_t in_sizes, const size_t in_strides)
{
    VkBuffer* buffer = *static_cast<VkBuffer**>( in_buffer->Handle() );
    VkDeviceSize offsets = in_offsets;
    VkDeviceSize sizes = in_sizes;
    VkDeviceSize strides = in_strides;
    vkCmdBindVertexBuffers2( m_command[m_frame], in_binding, 1, buffer, &offsets, &sizes, &strides );
}

void vkCommandBuffer::BindPipeline(const crPipeline *in_pipeline)
{
    vkCmdBindPipeline( m_command[m_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, dynamic_cast<const vkPipeline*>( in_pipeline )->Pipeline() );
}

void vkCommandBuffer::EndRenderPass(void) const
{
    VkSubpassEndInfo subpassEndInfo{};
    subpassEndInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO;
    vkCmdEndRenderPass2( m_command[m_frame], &subpassEndInfo );
}

void vkCommandBuffer::Draw(const uint32_t in_vertexCount, const uint32_t in_instanceCount, const uint32_t in_firstVertex, const uint32_t in_firstInstance) const
{
    vkCmdDraw( m_command[m_frame], in_vertexCount, in_instanceCount, in_firstVertex, in_firstInstance );
}

void vkCommandBuffer::DrawIndexed(const uint32_t in_indexCount, const uint32_t in_instanceCount, const uint32_t in_firstIndex, const int32_t in_vertexOffset, const uint32_t in_firstInstance) const
{
    vkCmdDrawIndexed( m_command[m_frame], in_indexCount, in_instanceCount, in_firstIndex, in_vertexOffset, in_firstInstance );
}

void vkCommandBuffer::Dispatch(const uint32_t in_groupCountX, const uint32_t in_groupCountY, const uint32_t in_groupCountZ) const
{
    vkCmdDispatch( m_command[m_frame], in_groupCountX, in_groupCountY, in_groupCountZ );
}

void vkCommandBuffer::Clear(bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a)
{
    m_clearValues.color.float32[0] = r;
    m_clearValues.color.float32[1] = g;
    m_clearValues.color.float32[2] = b;
    m_clearValues.color.float32[3] = a;
    m_clearValues.depthStencil.depth = 0.0f;
    m_clearValues.depthStencil.stencil = stencilValue;
}

void vkCommandBuffer::PolygonOffset(const float scale, const float bias)
{
    vkCmdSetDepthBias( m_command[m_frame], scale, 0.0f, bias );
}

void vkCommandBuffer::DepthBoundsTest(const float zmin, const float zmax)
{
    vkCmdSetDepthBounds( m_command[m_frame], zmin, zmax );
}

void vkCommandBuffer::Scissor( const int x, const int y, const int w, const int h) const
{
    VkRect2D r{};
    r.offset.x = x;
    r.offset.y = y;
    r.extent.width = w;
    r.extent.height = h;
    vkCmdSetScissor( m_command[m_frame], 0, 1, &r );
}

void vkCommandBuffer::Viewport( const int x, const int y, const int w, const int h ) const
{
    VkViewport vp{};
    vp.x = x;
    vp.y = y + h;
    vp.width = w;
    vp.height = -std::abs(h);;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport( m_command[m_frame], 0, 1, &vp );
}

void vkCommandBuffer::ExecuteCommands(const uint32_t in_commandBufferCount, const VkCommandBuffer *in_commandBuffers)
{
    vkCmdExecuteCommands( m_command[m_frame], in_commandBufferCount, in_commandBuffers );
}
