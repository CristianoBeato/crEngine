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

#include <atomic>
#include "renderer/Vulkan/Core.hpp"

struct queueInfo_t
{
    bool        present = false;    // is a present queue
    bool        graphic = false;    // is a graphic queue
    bool        transfer = false;   // is a transfer queue
    bool        compute = false;    // compute queue 
    uint32_t    index = 0;          // queue index 
    uint32_t    family = 0;         // quque family
};

typedef class vkDeviceQueue
{
public:
    vkDeviceQueue( const uint32_t in_family, const uint32_t in_index );
    ~vkDeviceQueue( void );
    bool            Init( const VkDevice in_device );
    uint32_t        Index( void ) const { return m_index; }
    uint32_t        Family( void ) const { return m_index; }
    VkQueue         Queue( void ) const { return m_queue; }
    VkCommandPool   CommandPool( void ) const { return m_commandPool; }

private:
    uint32_t                m_index;        // index in the family 
    uint32_t                m_family;       // the family index
    VkQueue                 m_queue;        // queue hanlde
    VkCommandPool           m_commandPool;  // queue command pool
    VkSemaphore             m_semaphore;    // queue semaphore
    VkDevice                m_device;       // parent device
} * vkDeviceQueuep;

typedef class crVulkanRenderDevice : public crRenderDevice
{
public:
    crVulkanRenderDevice( void );
    crVulkanRenderDevice( const uint32_t in_ID, const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface );
    ~crVulkanRenderDevice( void );

    virtual bool				Create( const char** in_layers, const uint32_t in_numLayers, const char** in_enabledExtensions, const uint32_t in_numExtensions ) override;
	virtual void				Destroy( void ) override;
	virtual const char*         Name( void ) const override;
	virtual const properties_t	Properties( void ) const override;
	virtual const features_t	Features( void ) const override;
    virtual const uint32_t      Score( void ) const;

    /// @brief Find device memory type
    /// @param type_filter 
    /// @param properties 
    /// @return the index of the type 
    const uint32_t  FindMemoryType( const uint32_t in_filter, const VkMemoryPropertyFlags in_properties ) const;

    /// @brief check for extension if available in the device 
    /// @return 
    const bool      ExtensionAvailable( const idStr &in_ext ) const;
    const bool      SupportedPresentMode( const VkPresentModeKHR in_mode ) const;
    const bool      SupportedFormat( const VkSurfaceFormatKHR in_format ) const;
    const bool      FormatIsFilterable(const VkFormat in_format, const VkImageTiling tiling) const;
    const bool      SupportedDepthFormat( const VkFormat in_depthFormat ) const;
    const bool      SupportedDepthStencilFormat( const VkFormat in_depthStencilFormat ) const;

    /// Device ID Mask
    uint32_t Mask( void ) const { return m_id + 1; }

    ID_INLINE vkDeviceQueuep   PresentQueue( void ) const { return m_present; }
    ID_INLINE vkDeviceQueuep   GraphicQueue( void ) const { return m_graphic; }
    ID_INLINE vkDeviceQueuep   ComputeQueue( void ) const { return ( m_compute != nullptr ) ? m_compute : m_graphic; }
    ID_INLINE vkDeviceQueuep   TransferQueue( void ) const { return ( m_transfer != nullptr ) ? m_transfer : m_graphic; }

    ID_INLINE VkPipelineCache  PipelineCache( void ) const { return m_pipelineCache; }
    ID_INLINE VkPhysicalDevice PhysicDevice( void ) const { return m_phisicDevice; }
    ID_INLINE VkDevice LogicDevice( void ) const { return m_logicDevce; }
    ID_INLINE operator VkPipelineCache( void ) const { return m_pipelineCache; }
    ID_INLINE operator VkPhysicalDevice( void ) const { return m_phisicDevice; }
    ID_INLINE operator VkDevice( void ) const { return m_logicDevce; }
    
private: 
    bool                                            m_cacheLoaded;   // if cache is loaded, we don't save it again
    uint32_t                                        m_id;
    idStr                                           m_name;
    properties_t                                    m_internalProperties;
    features_t                                      m_internalFeatures;

    // device properties
    VkPhysicalDeviceProperties2                     m_propertiesv10;
    VkPhysicalDeviceVulkan11Properties              m_propertiesv11;
    VkPhysicalDeviceVulkan12Properties              m_propertiesv12;
    VkPhysicalDeviceVulkan13Properties              m_propertiesv13;
    
    // device enabled features
    VkPhysicalDeviceFeatures2                       m_featuresv10;
    VkPhysicalDeviceVulkan12Features                m_featuresv11;
    VkPhysicalDeviceVulkan12Features                m_featuresv12;
    VkPhysicalDeviceVulkan13Features                m_featuresv13;

    // device suface sapabilities 
    VkSurfaceCapabilities2KHR                       m_surfaceCapabilities;

    // device queues
    vkDeviceQueuep                                  m_present;
    vkDeviceQueuep                                  m_graphic;
    vkDeviceQueuep                                  m_compute;
    vkDeviceQueuep                                  m_transfer;
    VkPhysicalDevice                                m_phisicDevice;
    VkDevice                                        m_logicDevce;
    VkPipelineCache                                 m_pipelineCache;

    // device memory info
    VkPhysicalDeviceMemoryProperties2               m_memoryProperties;
    VkPhysicalDevice                                m_physical; // Physical Device handler 
    VkDevice                                        m_logic;    // Logic Device handler
    
    idList<VkQueueFamilyProperties2, TAG_VULKAN>    m_queueFamilyPropertiesList;
    idList<VkExtensionProperties, TAG_VULKAN>       m_deviceExtensions;
    idList<VkSurfaceFormat2KHR, TAG_VULKAN>         m_surfaceFormats;
    idList<VkPresentModeKHR, TAG_VULKAN>            m_presentModes;
    idList<queueInfo_t, TAG_VULKAN>                 m_queues;

    void SelectDeviceQueues( idList<VkDeviceQueueCreateInfo> &in_queueList );
    bool LoadCache( void );
    bool SaveCache( void );
} * crVulkanRenderDevicep;

typedef class crVulkanAPI : public crRenderAPI
{
public:
    crVulkanAPI( void );
    ~crVulkanAPI( void );

	virtual bool				StartUp( void ) override;
	virtual void				ShutDown( void ) override;
	virtual uint32_t			GetDevices( crRenderDevicep* in_deviceArray ) override;
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
} * crVulkanAPIp;

#endif //!__SYS_VULKAN_HPP__