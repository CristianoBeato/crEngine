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

typedef class crMemoryPool* crMemoryPoolp;
typedef class crVulkanRenderDevice : public crRenderDevice
{
public:
    crVulkanRenderDevice( void );
    crVulkanRenderDevice( const uint32_t in_ID, const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface );
    ~crVulkanRenderDevice( void );

    /// @brief Create the logic device
    /// @param in_layers the layer to be enabled
    /// @param in_numLayers layer names count
    /// @param in_enabledExtensions device extensions to be enable
    /// @param in_numExtensions 
    /// @return true on success
    virtual bool				Create( const char** in_layers, const uint32_t in_numLayers, const char** in_enabledExtensions, const uint32_t in_numExtensions ) override;
	virtual void				Destroy( void ) override;
	virtual const char*         Name( void ) const override;
	virtual const properties_t	Properties( void ) const override;
    virtual const int32_t       Score( void ) const override;
    virtual bool				ReloadCache( void ) override;

    /// @brief Allocate a memory page to be used by structures with the same configuration.
    crMemoryPoolp               Alloc( const size_t in_size, const size_t in_alignament, const uint32_t in_filter, const VkMemoryPropertyFlags in_properties );
    
    /// @brief Releasse a memory block
    void                        Free( crMemoryPoolp in_pool );
    
    /// @brief Defragment device memory (TODO: future)
    void                        Defrag( void );

    /// @brief check for extension if available in the device 
    /// @return 
    virtual const bool          ExtensionAvailable( const char* in_ext ) const override;

    const bool                  SupportedPresentMode( const VkPresentModeKHR in_mode ) const;
    const bool                  SupportedFormat( const VkSurfaceFormatKHR in_format ) const;
    const bool                  FormatIsFilterable(const VkFormat in_format, const VkImageTiling tiling) const;
    const bool                  SupportedDepthFormat( const VkFormat in_depthFormat ) const;
    const bool                  SupportedDepthStencilFormat( const VkFormat in_depthStencilFormat ) const;

    /// Device ID Mask
    ID_INLINE uint32_t          Mask( void ) const { return m_id + 1; }

    ID_INLINE vkDeviceQueuep    PresentQueue( void ) const { assert( m_present != nullptr ); return m_present; }
    ID_INLINE vkDeviceQueuep    GraphicQueue( void ) const { assert( m_graphic != nullptr ); return m_graphic; }
    ID_INLINE vkDeviceQueuep    ComputeQueue( void ) const { return ( m_compute != nullptr ) ? m_compute : m_graphic; }
    ID_INLINE vkDeviceQueuep    TransferQueue( void ) const { return ( m_transfer != nullptr ) ? m_transfer : m_graphic; }
    ID_INLINE VkPipelineCache   PipelineCache( void ) const { assert( m_pipelineCache != nullptr ); return m_pipelineCache; }
    ID_INLINE VkPhysicalDevice  PhysicDevice( void ) const { assert( m_physic != nullptr ); return m_physic; }
    ID_INLINE VkDevice          LogicDevice( void ) const { assert( m_logic != nullptr ); return m_logic; }
    ID_INLINE operator VkPipelineCache( void ) const { assert( m_pipelineCache != nullptr ); return m_pipelineCache; }
    ID_INLINE operator VkPhysicalDevice( void ) const { assert( m_physic != nullptr ); return m_physic; }
    ID_INLINE operator VkDevice( void ) const { assert( m_logic != nullptr ); return m_logic; }
    
private:
    struct memoryHeapInfo_t
    {
        size_t                  total = 0;  // total available heap
        size_t                  allocated = 0; // total used
        size_t                  free = 0;
        VkMemoryPropertyFlags   propertyFlags = 0;
    };

    struct memoryTypeInfo_t
    {
        uint32_t                            typeIndex = 0;
        uint32_t                            heapIndex = 0;
        VkMemoryPropertyFlags               propertyFlags = 0;
        idList<crMemoryPoolp, TAG_VULKAN>   pools;
    };

    bool                                            m_cacheLoaded;   // if cache is loaded, we don't save it again
    unsigned int                                    m_id;
    idStr                                           m_name;
    properties_t                                    m_internalProperties;

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
    VkPipelineCache                                 m_pipelineCache;

    // device memory info
    VkPhysicalDeviceMemoryProperties2               m_memoryProperties;
    VkPhysicalDevice                                m_physic;   // Physical Device handler 
    VkDevice                                        m_logic;    // Logic Device handler
    
    idList<VkQueueFamilyProperties2, TAG_VULKAN>    m_queueFamilyPropertiesList;
    idList<VkExtensionProperties, TAG_VULKAN>       m_deviceExtensions;
    idList<VkSurfaceFormat2KHR, TAG_VULKAN>         m_surfaceFormats;
    idList<VkPresentModeKHR, TAG_VULKAN>            m_presentModes;
    idList<queueInfo_t, TAG_VULKAN>                 m_queues;
    idList<memoryHeapInfo_t, TAG_VULKAN>            m_heaps;
    idList<memoryTypeInfo_t, TAG_VULKAN>            m_types;

    void SelectDeviceQueues( idList<VkDeviceQueueCreateInfo> &in_queueList );
    bool InitDeviceHeap( void );
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
    inline VkInstance           Instance( void ) const { assert( m_instance != nullptr ); return m_instance; }
    inline VkSurfaceKHR         Surface( void )  const { assert( m_surface != nullptr ); return m_surface; }

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
    bool    InitInstance( const idList<const char*> &in_requestedInstanceLayers, const idList<const char*> &in_requiredInstanceExtensions, const VkDebugUtilsMessengerCreateInfoEXT &in_debugUtilsMessengerCI );
    void    ReleaseInstance( void );
    bool    InitSurface( void );
    void    ReleaseSurface( void );
    bool    InitDebugUtilsMessenger( const VkDebugUtilsMessengerCreateInfoEXT &in_debugUtilsMessengerCI );
    void    ReleaseDebugUtilsMessenger( void );
    bool    IsExtensionAvailable( const idStr &in_ext ) const;
    bool    IsLayersAvailable( const idStr &in_layer ) const;
    void    GetInstanceProcs( void );
    void    LoadVulkanProcs( void );
} * crVulkanAPIp;

#endif //!__SYS_VULKAN_HPP__