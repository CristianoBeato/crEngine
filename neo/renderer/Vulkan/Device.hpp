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

#ifndef __VK_DEVICE_HPP__
#define __VK_DEVICE_HPP__

#include <atomic>

struct queueInfo_t
{
    bool        present = false;    // is a present queue
    bool        graphic = false;    // is a graphic queue
    bool        transfer = false;   // is a transfer queue
    bool        compute = false;    // compute queue 
    uint32_t    index = 0;          // queue index 
    uint32_t    family = 0;         // quque family
};

typedef class vkDeviceQueue* vkDeviceQueuep;

/// @brief 
class vkDeviceQueue
{
public:
    vkDeviceQueue( const uint32_t in_family, const uint32_t in_index );
    ~vkDeviceQueue( void );
    bool            Init( const VkDevice in_device );
    bool            WaitSemaphore( const uint64_t in_value, const uint64_t in_timeout );
    uint64_t        IncrementTimeline( void );
    uint32_t        Index( void ) const { return m_index; }
    uint32_t        Family( void ) const { return m_index; }
    uint64_t        Timeline( void ) const { return m_timeline; }
    VkQueue         Queue( void ) const { return m_queue; }
    VkCommandPool   CommandPool( void ) const { return m_commandPool; }
    VkSemaphore     Semaphore( void ) const { return m_semaphore; }

private:
    uint32_t                m_index;        // index in the family 
    uint32_t                m_family;       // the family index
    std::atomic_uint64_t    m_timeline;     // events timeline counter
    VkQueue                 m_queue;        // queue hanlde
    VkCommandPool           m_commandPool;  // queue command pool
    VkSemaphore             m_semaphore;    // queue semaphore
    VkDevice                m_device;       // parent device
};

typedef class vkRenderDevice* vkRenderDevicep;

class vkRenderDevice
{
public:
    vkRenderDevice( void );
    ~vkRenderDevice( void );

    bool        Init( const uint32_t in_ID, const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface );
    bool        StartUp( const idList<const char*> &in_layers, const idList<const char*> &in_enabledExtensions );
    void        ShutDown( void );

    uint32_t    Score( void ) const;

    uint32_t    Mask( void ) const { return m_id + 1; }

    /// @brief check for extension if available in the device 
    /// @return 
    bool        ExtensionAvailable( const idStr &in_ext ) const;

    /// @brief Find device memory type
    /// @param type_filter 
    /// @param properties 
    /// @return the index of the type 
    uint32_t FindMemoryType( const uint32_t in_filter, const VkMemoryPropertyFlags in_properties ) const;

    const uint32_t  ShaderStorageBufferAlignament( void ) const;

    const bool                      SupportedFormat( const VkFormat in_format, const VkColorSpaceKHR in_colorSpace ) const;

    const bool                      SupportedPresentMode( const VkPresentModeKHR in_mode );

    const VkSurfaceFormatKHR        GetPresentFormat( const uint32_t in_formatID );
    
    /// @brief 
    /// @param depthFormat 
    /// @return 
    const bool                      SupportedDepthFormat( VkFormat *depthFormat ) const;

    /// @brief 
    /// @param depthStencilFormat 
    /// @return 
    const bool                      SupportedDepthStencilFormat( VkFormat *depthStencilFormat) const;

    /// @brief 
    /// @param in_format 
    /// @param tiling 
    /// @return 
    const bool                      FormatIsFilterable(const VkFormat in_format, const VkImageTiling tiling) const;

    /// @brief Device name
    inline const char*              Name( void ) const { return m_propertiesv10.properties.deviceName; }

    /// @brief the type of the gpu
    inline VkPhysicalDeviceType     Type( void ) const { return m_propertiesv10.properties.deviceType; }
    
    /// @brief current suported device version
    inline uint32_t                 ApiVersion( void ) const { return m_propertiesv10.properties.apiVersion; }

    /// @brief Max msaa samples suported
    uint32_t                        MaxSamples( void ) const;

    /// @brief logic device handler
    inline VkDevice                 Device( void ) const { return m_logic; }
    
    inline vkDeviceQueuep   PresentQueue( void ) const { return m_present; }
    inline vkDeviceQueuep   GraphicQueue( void ) const { return m_graphic; }
    inline vkDeviceQueuep   ComputeQueue( void ) const { return ( m_compute != nullptr ) ? m_compute : m_graphic; }
    inline vkDeviceQueuep   TransferQueue( void ) const { return ( m_transfer != nullptr ) ? m_transfer : m_graphic; }

    /// @brief Physic device handler
    inline VkPhysicalDevice PhysicalDevice( void ) const { return m_physical; }
        
    inline operator VkDevice( void ) const { return m_logic; }
    inline operator VkPhysicalDevice( void ) const { return m_physical; }

private:
    uint32_t                                        m_id;

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

    // device memory info
    VkPhysicalDeviceMemoryProperties2               m_memoryProperties;
    VkPhysicalDevice                                m_physical; // Physical Device handler 
    VkDevice                                        m_logic;    // Logic Device handler
    
    // device queues
    vkDeviceQueuep                                  m_present;
    vkDeviceQueuep                                  m_graphic;
    vkDeviceQueuep                                  m_compute;
    vkDeviceQueuep                                  m_transfer;

    idList<VkQueueFamilyProperties2, TAG_VULKAN>    m_queueFamilyPropertiesList;
    idList<VkExtensionProperties, TAG_VULKAN>       m_deviceExtensions;
    idList<VkSurfaceFormat2KHR, TAG_VULKAN>         m_surfaceFormats;
    idList<VkPresentModeKHR, TAG_VULKAN>            m_presentModes;
    idList<queueInfo_t, TAG_VULKAN>                 m_queues;

    void SelectDeviceQueues( idList<VkDeviceQueueCreateInfo> &in_queueList );
};

#endif //!__VK_DEVICE_HPP__