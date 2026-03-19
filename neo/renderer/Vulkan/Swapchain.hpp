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

#ifndef __VK_SWAPCHAIN_HPP__
#define __VK_SWAPCHAIN_HPP__

typedef class vkDeviceQueue* vkDeviceQueuep;
typedef class crVulkanRenderDevice* crVulkanRenderDevicep;

class vkSwapchain
{
public:
    vkSwapchain( void );
    ~vkSwapchain( void );

    bool                            Create( const uint32_t in_width, const uint32_t in_height, const bool in_recreate );
    void                            Destroy( void );
    void                            AcquireImage( const uint32_t in_bufferID );
    void                            SwapBuffers( const VkSemaphore in_renderDone );
    ID_INLINE VkSemaphore           ImageAvailableSemaphore( void ) const { return m_imageAvailable[m_bufferID]; }
    ID_INLINE vkImageHandle_t*      Image( void ) const { return const_cast<vkImageHandle_t*>( &m_presentImages[m_currentImage] ); }

private:
    uint32_t                                        m_width;
    uint32_t                                        m_height;
    uint32_t                                        m_currentImage;
    uint32_t                                        m_bufferID;
    vkDeviceQueuep                                  m_presentQueue;
    vkDeviceQueuep                                  m_graphicQueue;
    crVulkanRenderDevicep                           m_device;
    VkSwapchainKHR                                  m_swapchain;
    idStaticList<VkSemaphore, SMP_FRAMES>           m_imageAvailable;
    idList<VkRenderingAttachmentInfo, TAG_VULKAN>   m_colorAttachments;
    idList<VkImage, TAG_VULKAN>                     m_imagesArray;
    idList<vkImageHandle_t, TAG_VULKAN>             m_presentImages;

    VkSurfaceFormatKHR GetPresentFormat( uint32_t in_format );
};
typedef vkSwapchain* vkSwapchainp;

#endif //!__VK_SWAPCHAIN_HPP__