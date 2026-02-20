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
#include "vkSwapchain.hpp"

vkSwapchain::vkSwapchain( const uint32_t in_width, const uint32_t in_height ) : crSwapchain( in_width, in_height ),
    m_currentImage( 0 ),
    m_format( VK_FORMAT_UNDEFINED ),
    m_device( nullptr ),
    m_swapChain( nullptr )
{
    VkSurfaceFormatKHR sformat = tr.vkContext->SurfaceFormat();
    auto presentQueue = tr.vkContext->GetPresentQueue();
    auto graphicQueue = tr.vkContext->GetGraphicQueue();

    if ( presentQueue.queue == nullptr )
        common->FatalError( "can't create a swapchain no valid present queue" );
        
    if ( graphicQueue.pool == nullptr )
        common->FatalError( "can't create a swapchain no valid graphic queue" );
        
    m_presentQueue = presentQueue.queue;

    ///
    /// Create Swapchain and Imageview 
    /// ==========================================================================
    CreateSwapChain( sformat.format, sformat.colorSpace, tr.vkContext->PresentMode(), presentQueue.family.value(), graphicQueue.family.value() );
    PrepareImages( false, sformat.format, graphicQueue.family.value() );

    //
    // Create race condition structures 
    CreateFences();
}

vkSwapchain::~vkSwapchain( void )
{
    for ( uint32_t i = 0; i < m_colorImages.Num(); i++)
    {
        // release depth stencil image view 
        vkDestroyImageView( m_device, m_depthStencilViews[i], k_allocationCallbacks );

        // release color image view 
        vkDestroyImageView( m_device, m_colorViews[i], k_allocationCallbacks );

        // release image meory 
        vkFreeMemory( m_device, m_depthStencilMemory[i], k_allocationCallbacks );

        //
        vkDestroyImage( m_device, m_depthStencilImages[i], k_allocationCallbacks );
    }

    if ( m_swapChain != nullptr )
    {
        vkDestroySwapchainKHR( m_device, m_swapChain, k_allocationCallbacks );
        m_swapChain = nullptr;
    }
}

bool vkSwapchain::Recreate(const uint32_t in_width, const uint32_t in_height)
{
    // wait we for finish evetirthing before we recreate the swap chain
    vkDeviceWaitIdle( m_device );

    return true;
}

void vkSwapchain::AcquireImage(void)
{
    VkResult result = VK_SUCCESS;

    //
    // Wait for the device finish last render in previous match frame
    result = vkWaitForFences( m_device, 1, &m_frameFences[m_frame], VK_TRUE, UINT64_MAX );
    if ( result != VK_SUCCESS )
        common->Error( "vkSwapchain::AcquireImage::vkWaitForFences", result );
    else
        vkResetFences(m_device, 1, &m_frameFences[m_frame] );


    //
    //
    // Aquire the current frame image idex
    VkAcquireNextImageInfoKHR   acquireNextImageInfo{};
    acquireNextImageInfo.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    acquireNextImageInfo.pNext = nullptr;
    acquireNextImageInfo.swapchain = m_swapChain;
    acquireNextImageInfo.timeout = UINT64_MAX;
    acquireNextImageInfo.semaphore = m_imageAvailable[m_frame];
    acquireNextImageInfo.fence = nullptr;
    acquireNextImageInfo.deviceMask = 0;
    result = vkAcquireNextImage2KHR( m_device, &acquireNextImageInfo, &m_currentImage );
    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
        common->Error( "vkSwapchain::AcquireImage::vkAcquireNextImage2KHR %s\n", result );

    //
    //
    // Reset the main render command buffer
    result = vkResetCommandBuffer( m_commandBuffers[m_frame], 0 );

    //
    // 
    // Begin register commands in curren buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT /*VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT*/; // we only submit one time per frame 
    result = vkBeginCommandBuffer( m_commandBuffers[m_frame], &beginInfo );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandBuffer::Begin FAILED to begin!\n" );

    /// now we bind to render
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO; 
    colorAttachment.pNext = nullptr; 
    colorAttachment.imageView = m_colorViews[m_currentImage]; 
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT; 
    colorAttachment.resolveImageView = nullptr;
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // clear to black
    colorAttachment.clearValue.color.float32[0] = 0.0f; 
    colorAttachment.clearValue.color.float32[0] = 0.0f; 
    colorAttachment.clearValue.color.float32[0] = 0.0f; 
    colorAttachment.clearValue.color.float32[0] = 1.0f; 

    /// now we render depth stencil attachament
    VkRenderingAttachmentInfo depthStencilAttachment{};
    depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthStencilAttachment.pNext = nullptr;
    depthStencilAttachment.imageView = m_depthStencilViews[m_frame];
    depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    depthStencilAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    depthStencilAttachment.resolveImageView = nullptr;
    depthStencilAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo renderingInfo{};
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthStencilAttachment;
    renderingInfo.pStencilAttachment = &depthStencilAttachment;

    vkCmdBeginRendering(m_commandBuffers[m_frame], &renderingInfo );
}

void vkSwapchain::PresentImage(void)
{
    VkResult result = VK_SUCCESS;
    VkSubmitInfo2               submitInfo{};
    VkSemaphoreSubmitInfo       wait{};
    VkSemaphoreSubmitInfo       signal{};
    VkCommandBufferSubmitInfo   commandBufferSubmit{};

    //
    // 
    // Finish record draw commands
    result = vkEndCommandBuffer( m_commandBuffers[m_frame] );
    if( result != VK_SUCCESS )
        common->Error( "vkCommandBuffer::Begin FAILED!\n" );

    ///
    ///
    /// Wait for semaphores
    wait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    wait.pNext = nullptr;
    wait.semaphore = m_imageAvailable[m_frame];
    wait.value = 0;
    wait.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; 
    wait.deviceIndex = 0; 

    ///
    ///
    /// Signal semaphores
    signal.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signal.pNext = nullptr;
    signal.semaphore = m_renderFinished[m_frame];
    signal.value = 0;
    signal.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT; 
    signal.deviceIndex = 0; 

    ///
    ///
    ///
    commandBufferSubmit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmit.pNext = nullptr;
    commandBufferSubmit.commandBuffer = m_commandBuffers[m_frame];
    commandBufferSubmit.deviceMask = 0;

    ///
    ///
    ///
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = 0;
    submitInfo.pWaitSemaphoreInfos = &wait;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmit;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signal;

    result = vkQueueSubmit2( m_graphicQueue, 1, &submitInfo, m_renderSubmit[m_frame] );
    if( result != VK_SUCCESS )
        common->Error( "vkQueueSubmit2 FAILED!\n" );

    //
    // present to the window
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_imageAvailable[m_frame];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain;
    presentInfo.pImageIndices = &m_currentImage;
    vkQueuePresentKHR( m_presentQueue, &presentInfo );

    m_frame = ( m_frame + 1 ) % SMP_FRAMES;
}

void vkSwapchain::CreateSwapChain( const VkFormat in_format, const VkColorSpaceKHR in_colorSpace, const VkPresentModeKHR in_presentMode, const uint32_t in_presentFamily, const uint32_t in_graphycFamily )
{
    VkResult result = VK_SUCCESS;
    uint32_t queueFamilyIndices[2] { in_presentFamily, in_graphycFamily };
    VkSwapchainKHR old = m_swapChain;
    VkSwapchainCreateInfoKHR swapchainCI{};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.pNext = nullptr;
    swapchainCI.flags = 0;
    swapchainCI.surface = tr.vkContext->Surface();
    swapchainCI.minImageCount = std::min( SMP_FRAMES, 3u ); //TODO: check max device suported frames
    swapchainCI.imageFormat = in_format;
    swapchainCI.imageColorSpace = in_colorSpace;
    swapchainCI.imageExtent = { m_width, m_height };
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    // if we have a independent present queue 
    swapchainCI.imageSharingMode = ( in_presentFamily != in_graphycFamily ) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    swapchainCI.queueFamilyIndexCount = ( in_presentFamily != in_graphycFamily ) ? 2 : 1;
    swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
    swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // todo: get from context
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = in_presentMode;
    swapchainCI.clipped = VK_TRUE;
    swapchainCI.oldSwapchain = old;

    result = vkCreateSwapchainKHR( m_device, &swapchainCI, k_allocationCallbacks, &m_swapChain );
    if ( result != VK_SUCCESS ) 
        common->FatalError( "vkCreateSwapchainKHR FAILED! %s\n", GetVulkanError( result ) );

    // we are updating, recreating a new
    if ( old != nullptr )
        vkDestroySwapchainKHR( m_device, old, k_allocationCallbacks );    
}

void vkSwapchain::PrepareImages( const bool in_recreate, const VkFormat in_format, const uint32_t in_graphycFamily )
{
    uint32_t i = 0;
    VkResult result = VK_SUCCESS;
    uint32_t numImages = 0;

        // Get the available image count 
    vkGetSwapchainImagesKHR( m_device, m_swapChain, &numImages, nullptr );
    
    // prepare the arrays
    m_colorImages.Resize( numImages );
    m_colorViews.Resize( numImages );
    m_depthStencilImages.Resize( numImages );
    m_depthStencilViews.Resize( numImages );
    m_depthStencilMemory.Resize( numImages );
    
    // Get the image array 
    vkGetSwapchainImagesKHR( m_device, m_swapChain, &numImages, m_colorImages.Ptr() );
    
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = in_format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    // create the swap chain views
    for ( i = 0; i < numImages; i++) 
    {
    
        // destroy old view 
        if ( in_recreate )
            vkDestroyImageView( m_device, m_colorViews[i], k_allocationCallbacks );
    
        createInfo.image = m_colorImages[i];    
        result = vkCreateImageView( m_device, &createInfo, k_allocationCallbacks, &m_colorViews[i] ); 
        if ( result != VK_SUCCESS ) 
            common->FatalError( "vkSwapchain::PrepareImages::vkCreateImageView ERROR: %s\n", GetVulkanError( result ) );
    }

    //
    //
    // Depth Stencil Image Create Info
    VkImageCreateInfo   dsImageCI;
    dsImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    dsImageCI.pNext = nullptr;
    dsImageCI.flags = 0;
    dsImageCI.imageType = VK_IMAGE_TYPE_2D;
    dsImageCI.format = VK_FORMAT_D24_UNORM_S8_UINT;
    dsImageCI.extent = { m_width, m_height, 1 };
    dsImageCI.mipLevels = 1;
    dsImageCI.arrayLayers = 1;
    dsImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    dsImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    dsImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    // only graphic queue will use depth stencil, we don't present these to final screen 
    dsImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    dsImageCI.queueFamilyIndexCount = 1;
    dsImageCI.pQueueFamilyIndices = &in_graphycFamily;
    dsImageCI.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //
    //
    // Depth Stencil View Create Infor
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.image = nullptr;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = in_format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    //
    //
    //
    VkMemoryAllocateInfo dsSllocInfo{};
    dsSllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    dsSllocInfo.pNext = nullptr;
    dsSllocInfo.allocationSize = 0;
    dsSllocInfo.memoryTypeIndex = 0;

    for ( i = 0; i < m_depthStencilImages.Num(); i++)
    {
        VkMemoryRequirements memRequirements{};

        // create depth stencil image handle 
        result = vkCreateImage( m_device, &dsImageCI, k_allocationCallbacks, &m_depthStencilImages[i] );
        if ( result != VK_SUCCESS ) 
            common->FatalError( "vkSwapchain::PrepareImages::vkCreateImage ERROR: %s\n", GetVulkanError( result ) );

        // get image memory requeriments 
        vkGetImageMemoryRequirements( m_device, m_depthStencilImages[i], &memRequirements );

        //
        dsSllocInfo.allocationSize = memRequirements.size;
        dsSllocInfo.memoryTypeIndex = tr.vkContext->FindMemoryType( memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

        // allocate memory for the image
        result = vkAllocateMemory( m_device, &dsSllocInfo, k_allocationCallbacks, &m_depthStencilMemory[i] );
        if( result != VK_SUCCESS )
            common->FatalError( "vkSwapchain::PrepareImages::vkCreateImage ERROR: %s\n", GetVulkanError( result ) );

        // bind image to the handle 
        result = vkBindImageMemory( m_device, m_depthStencilImages[i], m_depthStencilMemory[i], 0 );
        if( result != VK_SUCCESS )
            common->FatalError( "vkSwapchain::PrepareImages::vkBindImageMemory ERROR: %s\n", GetVulkanError( result ) );

        // create image view
        createInfo.image = m_depthStencilImages[i];
        result = vkCreateImageView( m_device, &createInfo, k_allocationCallbacks, &m_depthStencilViews[i] );
        if( result != VK_SUCCESS )
            common->FatalError( "vkSwapchain::PrepareImages::vkCreateImageView ERROR: %s\n", GetVulkanError( result ) );
    }
}

void vkSwapchain::CreateFences(void)
{
    VkResult result = VK_SUCCESS;
    // Semaphore configuration
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    // Fence configuration
    VkFenceCreateInfo fenceCI{};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = 0;

    // alloc the structures arrays 
    m_imageAvailable.SetNum( SMP_FRAMES, nullptr );
    m_frameFences.SetNum( SMP_FRAMES, nullptr );
    for ( uint32_t i = 0; i < SMP_FRAMES; i++)
    {
        // create the semaphore object
        result = vkCreateSemaphore( m_device, &semaphoreInfo, k_allocationCallbacks, &m_imageAvailable[i] ); 
        if( result != VK_SUCCESS )
            common->FatalError( "crvkSwapchain::Create::vkCreateSemaphore %s\n", GetVulkanError( result ) );

        // create the fence object
        result = vkCreateFence( m_device, &fenceCI, k_allocationCallbacks, &m_frameFences[i] );
        if( result != VK_SUCCESS )
            common->FatalError( "crvkSwapchain::Create::vkCreateFence %s\n", GetVulkanError( result ) );
    }
}
