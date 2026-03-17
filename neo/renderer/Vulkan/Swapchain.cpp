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

#include "Swapchain.hpp"
#include "sys/sys_vulkan.hpp"
#include "Core.hpp"

static struct scMode_t
{
    const char*         mode;
    VkPresentModeKHR    present;
} modes[] =
{
    { "Imediate",           VK_PRESENT_MODE_IMMEDIATE_KHR },
    { "Mail Box",           VK_PRESENT_MODE_MAILBOX_KHR },
    { "Fifo",               VK_PRESENT_MODE_FIFO_KHR },
    { "Fifo Relaxed",       VK_PRESENT_MODE_FIFO_RELAXED_KHR },
    { "Shared Demand",      VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR },
    { "Shared Continuous",  VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR },
};

static struct scFormat_t
{
    const char*         descrip;
    VkSurfaceFormatKHR  format;
} formats[]
{
    { "RGB8-sRGB", { VK_FORMAT_B8G8R8A8_SRGB,    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }},
    { "RGB8A8-sRGB", { VK_FORMAT_R8G8B8A8_SRGB,    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }},
    { "BGR8A8-sRGB", { VK_FORMAT_B8G8R8A8_UNORM,   VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }},
    { "RGB10A02-ST2084", { VK_FORMAT_A2B10G10R10_UNORM_PACK32, VK_COLOR_SPACE_HDR10_ST2084_EXT }},
    { "RGB16A16-sRGB", { VK_FORMAT_R16G16B16A16_SFLOAT,  }}
};

idCVar vk_swapchainFormat( "vk_swapChainFormat", "5", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, " TODO " );
idCVar vk_swapchainPresent( "vk_swapchainPresent", "1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, " TODO " );

vkSwapchain::vkSwapchain( void ) : 
    m_width( 0 ),
    m_height( 0 ),
    m_currentImage( 0 ),
    m_presentQueue( nullptr ),
    m_graphicQueue( nullptr ),
    m_swapchain( nullptr )
{
}

vkSwapchain::~vkSwapchain( void )
{
}

bool vkSwapchain::Create( const uint32_t in_width, const uint32_t in_height, const bool in_recreate )
{
    uint32_t i = 0;
    uint32_t numImages = 0;
    VkSurfaceFormatKHR  format{};
    VkPresentModeKHR    presentMode;
    VkSwapchainKHR old = m_swapchain;
    VkResult result = VK_SUCCESS;
    uint32_t queueFamilyIndices[2] { m_presentQueue->Family(), m_graphicQueue->Family() };
    
    auto context = dynamic_cast<crVulkanAPIp>( crRenderAPI::Get() );
    auto device = tr.GetRenderDevice();
    m_width = in_width;
    m_height = in_height;

    // Get queues 
    m_presentQueue = device->PresentQueue();
    m_graphicQueue = device->GraphicQueue();

    if ( m_presentQueue == nullptr )
    {
        printf( "can't create a swapchain no valid present queue" );
        return false;
    }

    if ( m_graphicQueue == nullptr )
    {
        printf( "can't create a swapchain no valid graphic queue" );
        return false;
    }
    
    /// Get present mode
    auto selected = modes[0]; 
    if( device->SupportedPresentMode( modes[vk_swapchainPresent.GetInteger()].present ) )
        selected = modes[vk_swapchainPresent.GetInteger()]; 

    printf( "Using Present Mode: %s\n", selected.mode );
    presentMode = selected.present;

    // Get Surface format
    format = GetPresentFormat( vk_swapchainFormat.GetInteger() );

    ///
    /// Create Swapchain and Imageview 
    /// ==========================================================================
    VkSwapchainCreateInfoKHR swapchainCI{};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.pNext = nullptr;
    swapchainCI.flags = 0;
    swapchainCI.surface = context->Surface();
    swapchainCI.minImageCount = std::min( SMP_FRAMES, 3u ); //TODO: check max device suported frames
    swapchainCI.imageFormat = format.format;
    swapchainCI.imageColorSpace = format.colorSpace;
    swapchainCI.imageExtent.width = m_width;
    swapchainCI.imageExtent.height = m_height;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // if we have a independent present queue 
    swapchainCI.imageSharingMode = ( queueFamilyIndices[0] != queueFamilyIndices[1] ) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    swapchainCI.queueFamilyIndexCount = ( queueFamilyIndices[0] != queueFamilyIndices[1] ) ? 2 : 1;
    swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
    swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR; // todo: get from context
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = presentMode;
    swapchainCI.clipped = VK_TRUE;
    swapchainCI.oldSwapchain = old;

    result = vkCreateSwapchainKHR( *device, &swapchainCI, k_allocationCallbacks, &m_swapchain );
    if ( result != VK_SUCCESS )
    { 
        common->FatalError( "vkCreateSwapchainKHR FAILED! %s\n", VulkanErrorString( result ).c_str() );
        return false;
    }

    // we are updating, recreating a new
    if ( old != nullptr )
        vkDestroySwapchainKHR( *device, old, k_allocationCallbacks );
    
    ///
    ///
    ///
    
    // Get the available image count 
    vkGetSwapchainImagesKHR( *m_device, m_swapchain, &numImages, nullptr );
    
    // prepare the arrays
    m_imagesArray.Resize( numImages );
    m_presentImages.Resize( numImages );
    
    // Get the image array 
    vkGetSwapchainImagesKHR( *m_device, m_swapchain, &numImages, m_imagesArray.Ptr() );
    
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format.format;
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
        m_presentImages[i].image = m_imagesArray[i];

        // destroy old view 
        if ( in_recreate )
            vkDestroyImageView( *m_device, m_presentImages[i], k_allocationCallbacks );
    
        createInfo.image = m_presentImages[i];    
        result = vkCreateImageView( *m_device, &createInfo, k_allocationCallbacks, &m_presentImages[i].view ); 
        if ( result != VK_SUCCESS ) 
            common->FatalError( "vkSwapchain::PrepareImages::vkCreateImageView ERROR: %s\n", VulkanErrorString( result ).c_str() );
    }
    
    ///
    ///
    /// Create race condition structures 

    // Semaphore configuration
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;
    
    // alloc the structures arrays 
    m_imageAvailable.SetNum( SMP_FRAMES );
    for ( i = 0; i < SMP_FRAMES; i++)
    {
        // create the semaphore object
        result = vkCreateSemaphore( *device, &semaphoreInfo, k_allocationCallbacks, &m_imageAvailable[i] ); 
        if( result != VK_SUCCESS )
        {
            common->Error( "crvkSwapchain::Create::vkCreateSemaphore %s\n", VulkanErrorString( result ).c_str() );
            return false;
        }
    }

    return true;
}

void vkSwapchain::Destroy(void)
{
    uint32_t i = 0;
    auto device = tr.GetRenderDevice();
    
    for ( i = 0; i < SMP_FRAMES; i++)
    {
        vkDestroySemaphore( *device, m_imageAvailable[i], k_allocationCallbacks );
    }

    for ( i = 0; i < m_presentImages.Num(); i++ )
    {
        // release color image view 
        vkDestroyImageView( *device, m_presentImages[i].view, k_allocationCallbacks );
    }
    
    if ( m_swapchain != nullptr )
    {
        vkDestroySwapchainKHR( *device, m_swapchain, k_allocationCallbacks );
        m_swapchain = nullptr;
    }
    
    m_width = 0;
    m_height = 0;
    m_currentImage = 0;
    m_presentQueue = nullptr;
    m_graphicQueue = nullptr;
    m_imageAvailable.Clear();
    m_presentImages.Clear();
    m_imagesArray.Clear();
}

void vkSwapchain::AcquireImage( const uint32_t in_bufferID )
{
    VkResult result = VK_SUCCESS;
    auto device = tr.GetRenderDevice();
    m_bufferID = in_bufferID;

    //
    // Aquire the current frame image idex
    VkAcquireNextImageInfoKHR   acquireNextImageInfo{};
    acquireNextImageInfo.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    acquireNextImageInfo.pNext = nullptr;
    acquireNextImageInfo.swapchain = m_swapchain;
    acquireNextImageInfo.timeout = UINT64_MAX;
    acquireNextImageInfo.semaphore = m_imageAvailable[m_bufferID];
    acquireNextImageInfo.fence = nullptr;
    acquireNextImageInfo.deviceMask = device->Mask();
    result = vkAcquireNextImage2KHR( *device, &acquireNextImageInfo, &m_currentImage );
    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
        common->Error( "vkSwapchain::AcquireImage::vkAcquireNextImage2KHR %s\n", VulkanErrorString( result ).c_str() );
}

void vkSwapchain::SwapBuffers( const VkSemaphore in_renderDone )
{
    VkResult result = VK_SUCCESS;

    ///
    /// present to the window
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &in_renderDone;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentImage;
    vkQueuePresentKHR( m_presentQueue->Queue(), &presentInfo );
}

VkSurfaceFormatKHR vkSwapchain::GetPresentFormat( uint32_t in_format )
{
    auto device = static_cast<crVulkanRenderDevicep>( tr.GetRenderDevice() );
    VkSurfaceFormatKHR format = formats[in_format].format;
    while ( in_format > 0 )
    {
        // suported
        if( device->SupportedFormat( format ) )
            break;
            
        /// fall back
        in_format--;
    }
    
    return format;
}
