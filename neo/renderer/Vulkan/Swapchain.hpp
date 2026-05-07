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

#ifndef __SWAPCHAIN_HPP__
#define __SWAPCHAIN_HPP__

typedef class crQueue* crQueuep;
typedef class crVulkanRenderDevice* crVulkanRenderDevicep;

class crSwapchain
{
public:
    crSwapchain( void );
    ~crSwapchain( void );

    bool                            Create( const uint32_t in_width, const uint32_t in_height, const bool in_recreate );
    void                            Destroy( void );
    void                            AcquireImage( const crSemaphore* in_imageAvailable );
    void                            Present( const crSemaphore* in_renderDone );
    ID_INLINE crTexture*            Image( void ) const { return const_cast<crTexture*>( &m_presentImages[m_currentImage] ); }

private:
    uint32_t                                        m_width;
    uint32_t                                        m_height;
    uint32_t                                        m_currentImage;
    crQueuep                                  m_presentQueue;
    crQueuep                                  m_graphicQueue;
    crVulkanRenderDevicep                           m_device;
    VkSwapchainKHR                                  m_swapchain;
    idList<VkRenderingAttachmentInfo, TAG_VULKAN>   m_colorAttachments;
    idList<VkImage, TAG_VULKAN>                     m_imagesArray;
    idList<crTexture, TAG_VULKAN>                   m_presentImages;

    VkSurfaceFormatKHR GetPresentFormat( uint32_t in_format );
};
typedef crSwapchain* crSwapchainp;

#endif //!__SWAPCHAIN_HPP__