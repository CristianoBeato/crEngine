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

#ifndef __VULKAN_DEVICE_HPP__
#define __VULKAN_DEVICE_HPP__

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

    ID_INLINE crQueuep    PresentQueue( void ) const { assert( m_present != nullptr ); return m_present; }
    ID_INLINE crQueuep    GraphicQueue( void ) const { assert( m_graphic != nullptr ); return m_graphic; }
    ID_INLINE crQueuep    ComputeQueue( void ) const { return ( m_compute != nullptr ) ? m_compute : m_graphic; }
    ID_INLINE crQueuep    TransferQueue( void ) const { return ( m_transfer != nullptr ) ? m_transfer : m_graphic; }
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
    crQueuep                                  m_present;
    crQueuep                                  m_graphic;
    crQueuep                                  m_compute;
    crQueuep                                  m_transfer;
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

#endif //!__VULKAN_DEVICE_HPP__