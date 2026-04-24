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
#include "renderer_common.h"
#include "Core.hpp"
#include "Swapchain.hpp"

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

crSwapchain::crSwapchain( void ) : 
    m_width( 0 ),
    m_height( 0 ),
    m_currentImage( 0 ),
    m_presentQueue( nullptr ),
    m_graphicQueue( nullptr ),
    m_swapchain( nullptr )
{
}

crSwapchain::~crSwapchain( void )
{
}

bool crSwapchain::Create( const uint32_t in_width, const uint32_t in_height, const bool in_recreate )
{
    uint32_t i = 0;
    uint32_t numImages = 0;
    VkSurfaceFormatKHR  format{};
    VkPresentModeKHR    presentMode;
    VkSwapchainKHR old = m_swapchain;
    VkResult result = VK_SUCCESS;

    auto context = dynamic_cast<crVulkanAPIp>( crRenderAPI::Get() );
    m_device = tr.GetRenderDevice();
    m_width = in_width;
    m_height = in_height;

    // Get queues 
    m_presentQueue = m_device->PresentQueue();
    m_graphicQueue = m_device->GraphicQueue();
    uint32_t queueFamilyIndices[2] { m_presentQueue->Family(), m_graphicQueue->Family() };

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
    if( m_device->SupportedPresentMode( modes[vk_swapchainPresent.GetInteger()].present ) )
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
    swapchainCI.minImageCount = std::min( SMP_FRAMES, 3u ); //TODO: check max m_device suported frames
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

    result = vkCreateSwapchainKHR( *m_device, &swapchainCI, k_allocationCallbacks, &m_swapchain );
    if ( result != VK_SUCCESS )
    { 
        common->FatalError( "vkCreateSwapchainKHR FAILED! %s\n", VulkanErrorString( result ).c_str() );
        return false;
    }

    // we are updating, recreating a new
    if ( old != nullptr )
        vkDestroySwapchainKHR( *m_device, old, k_allocationCallbacks );
    
    // Get the available image count 
    vkGetSwapchainImagesKHR( *m_device, m_swapchain, &numImages, nullptr );
    
    // prepare the arrays
    m_imagesArray.SetNum( numImages );
    m_presentImages.SetNum( numImages );
    
    // Get the image array 
    vkGetSwapchainImagesKHR( *m_device, m_swapchain, &numImages, m_imagesArray.Ptr() );
    
    // create the swap chain views
    for ( i = 0; i < numImages; i++) 
    {
        // destroy old view 
        if ( in_recreate )
            vkDestroyImageView( *m_device, m_presentImages[i], k_allocationCallbacks );

        if( !m_presentImages[i].Create( m_imagesArray[i], crInternalFormat( format.format ), VK_IMAGE_VIEW_TYPE_2D ) )
            return false;
    
    }
    
    return true;
}

void crSwapchain::Destroy(void)
{
    uint32_t i = 0;
    auto m_device = tr.GetRenderDevice();
    
    for ( i = 0; i < m_presentImages.Num(); i++ )
    {
        // release color image view 
        vkDestroyImageView( *m_device, m_presentImages[i].View(), k_allocationCallbacks );
    }
    
    if ( m_swapchain != nullptr )
    {
        vkDestroySwapchainKHR( *m_device, m_swapchain, k_allocationCallbacks );
        m_swapchain = nullptr;
    }
    
    m_width = 0;
    m_height = 0;
    m_currentImage = 0;
    m_presentQueue = nullptr;
    m_graphicQueue = nullptr;
    m_presentImages.Clear();
    m_imagesArray.Clear();
}

void crSwapchain::AcquireImage( const crSemaphore* in_imageAvailable )
{
    VkResult result = VK_SUCCESS;
    auto m_device = tr.GetRenderDevice();

    //
    // Aquire the current frame image idex
    VkAcquireNextImageInfoKHR   acquireNextImageInfo{};
    acquireNextImageInfo.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    acquireNextImageInfo.pNext = nullptr;
    acquireNextImageInfo.swapchain = m_swapchain;
    acquireNextImageInfo.timeout = UINT64_MAX;
    acquireNextImageInfo.semaphore = *in_imageAvailable;
    acquireNextImageInfo.fence = nullptr;
    acquireNextImageInfo.deviceMask = m_device->Mask();
    result = vkAcquireNextImage2KHR( *m_device, &acquireNextImageInfo, &m_currentImage );
    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
        idLib::Error( "crSwapchain::AcquireImage::vkAcquireNextImage2KHR %s\n", VulkanErrorString( result ).c_str() );
}

void crSwapchain::Present( const crSemaphore* in_renderDone )
{
    VkResult result = VK_SUCCESS;

    ///
    /// present to the window
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =  in_renderDone->Pointer();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain;
    presentInfo.pImageIndices = &m_currentImage;
    vkQueuePresentKHR( m_presentQueue->Queue(), &presentInfo );
}

VkSurfaceFormatKHR crSwapchain::GetPresentFormat( uint32_t in_format )
{
    auto m_device = static_cast<crVulkanRenderDevicep>( tr.GetRenderDevice() );
    VkSurfaceFormatKHR format = formats[in_format].format;
    while ( in_format > 0 )
    {
        // suported
        if( m_device->SupportedFormat( format ) )
            break;
            
        /// fall back
        in_format--;
    }
    
    return format;
}
