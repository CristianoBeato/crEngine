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

#ifndef __SYS_VULKAN_HPP__
#define __SYS_VULKAN_HPP__

class crVulkanRenderDevice
{
public:
    crVulkanRenderDevice( void );
    ~crVulkanRenderDevice( void );

    inline VkPhysicalDevice PhysicDevice( void ) const { return m_phisicDevice; }
    inline VkDevice LogicDevice( void ) const { return m_logicDevce; }
    inline operator VkPhysicalDevice( void ) const { return m_phisicDevice; }
    inline operator VkDevice( void ) const { return m_logicDevce; }
    
private: 
    VkPhysicalDevice    m_phisicDevice;
    VkDevice            m_logicDevce;
};

class crVulkanAPI : public crRenderAPI
{
public:
    crVulkanAPI( void );
    ~crVulkanAPI( void );

	virtual bool				StartUp( void ) override;
	virtual void				ShutDown( void ) override;
	virtual uint32_t			GetDevices( crRenderDevice** m_deviceArray ) override;
    inline VkInstance           Instance( void ) const { return m_instance; }
    inline VkSurfaceKHR         Surface( void )  const { return m_surface; }

private:
    bool                                        m_hasDebugUtils;    // Enable debug layer
    bool                                        m_portabilityEnumerationAvailable; //
    uint32_t                                    m_currentDevice;    // current initialized device 
    VkInstance                                  m_instance;
    VkDebugUtilsMessengerEXT                    m_debugUtilsMessenger;    // The debug utility messenger callback.
	VkSurfaceKHR                                m_surface;          // The surface we will render to.
    idList<VkExtensionProperties, TAG_VULKAN>   m_availableInstanceExtensions;  //
    idList<VkLayerProperties, TAG_VULKAN>       m_supportedInstanceLayers;
    idList<VkPhysicalDevice, TAG_VULKAN>        m_availablePhysicalDevices; // list of vulkan compatible devices

    void    ListExtensionsAndLayers( void );
    bool    InitInstance( const idList<const char*> in_requestedInstanceLayers, const idList<const char*> &in_requiredInstanceExtensions, const VkDebugUtilsMessengerCreateInfoEXT* in_debugUtilsMessengerCI );
    void    ReleaseInstance( void );
    bool    InitSurface( void );
    void    ReleaseSurface( void );
    bool    InitDebugUtilsMessenger( const VkDebugUtilsMessengerCreateInfoEXT *in_debugUtilsMessengerCI );
    void    ReleaseDebugUtilsMessenger( void );
    bool    IsExtensionAvailable( const idStr &in_ext ) const;
    bool    IsLayersAvailable( const idStr &in_layer ) const;
    void    GetInstanceProcs( void );
    void    LoadVulkanProcs( void );
};

#endif //!__SYS_VULKAN_HPP__