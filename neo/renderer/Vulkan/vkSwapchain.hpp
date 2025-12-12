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

class vkSwapchain : public crSwapchain
{
public:
    vkSwapchain( const uint32_t in_width, const uint32_t in_height );
    ~vkSwapchain( void );
    virtual bool    Recreate( const uint32_t in_width, const uint32_t in_height );
    virtual void    AcquireImage( void );
    virtual void    PresentImage( void );

private:
    uint32_t                                    m_currentImage;
    uint64_t                                    m_frameOperationsFenceCount;
    VkFormat                                    m_format;
    VkDevice                                    m_device;
    VkSwapchainKHR                              m_swapChain;
    VkQueue                                     m_presentQueue;
    VkQueue                                     m_graphicQueue;
    VkSemaphore                                 m_frameOperationsFence;
    idList<VkImage>                             m_colorImages;
    idList<VkImageView>                         m_colorViews;
    idList<VkImage>                             m_depthStencilImages;
    idList<VkImageView>                         m_depthStencilViews;
    idList<VkDeviceMemory>                      m_depthStencilMemory;
    idStaticList<VkSemaphore, SMP_FRAMES>       m_imageAvailable;
    idStaticList<VkSemaphore, SMP_FRAMES>       m_renderFinished;
    idStaticList<VkFence, SMP_FRAMES>           m_frameFences;
    idStaticList<VkFence, SMP_FRAMES>           m_renderSubmit;
    idStaticList<VkCommandBuffer, SMP_FRAMES>   m_commandBuffers;       // the main render command buffer
    void    CreateSwapChain( const VkFormat in_format, const VkColorSpaceKHR in_colorSpace, const VkPresentModeKHR in_presentMode, const uint32_t in_presentFamily, const uint32_t in_graphycFamily );
    void    PrepareImages( const bool in_recreate, const VkFormat in_format, const uint32_t in_graphycFamily );
    void    CreateFences( void );
};

#endif //!__VK_SWAPCHAIN_HPP__