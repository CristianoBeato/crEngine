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

#ifndef __VK_CONTEXT_HPP__
#define __VK_CONTEXT_HPP__

class vkContext
{
public:
    vkContext( void );
    ~vkContext( void );

    // Tryes to initialize a valid funtional vulkan 1.3 instance
    bool    Init( void );

    // Destroys the rendering context, closes the window, resets the resolution,
    // and resets the gamma ramps.
    void    Shutdown( void );

    /// @brief access the instance handle 
    VkInstance          Instance( void ) const { return m_instance; }
    
    /// access the window surface handle
    VkSurfaceKHR        Surface( void )  const { return m_surface; }
    VkSurfaceFormatKHR  SurfaceFormat( void ) const;
    VkPresentModeKHR    PresentMode( void ) const;
    vkRenderDevice *    Device( void ) { return &m_devices[m_currentDevice]; }

private:
    bool                        m_hasDebugUtils;    // Enable debug layer
    bool                        m_portabilityEnumerationAvailable; //
    uint32_t                    m_currentDevice;    // current initialized device 
    VkInstance                  m_instance;         // The Vulkan instance.
    VkDebugUtilsMessengerEXT    m_debugCallback;    // The debug utility messenger callback.
	VkSurfaceKHR                m_surface;          // The surface we will render to.
    idList<VkExtensionProperties, TAG_VULKAN>       m_availableInstanceExtensions;  //
    idList<VkLayerProperties, TAG_VULKAN>           m_supportedInstanceLayers;
    idList<VkPhysicalDevice, TAG_VULKAN>            m_availablePhysicalDevices; // list of vulkan compatible devices
	idList<vkRenderDevice, TAG_VULKAN>              m_devices; // our interal device list 

	void	InitInstanceProcs( void );
	void	LoadVulkanProcs( void );
    bool    ExtensionAvailable( const idStr &in_ext ) const;
    bool    LayersAvailable( const idStr &in_layer ) const;
};

#endif //!__VK_CONTEXT_HPP__