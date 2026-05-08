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
#include "Core.hpp"
#include "Device.hpp"

#include <optional> // std::optional

idCVar vk_useComputQueues( "vk_useComputQueue", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 Enable vulkan find a compute queue, or 0 to use compute queue" );
idCVar vk_useTransferQueue( "vk_useTransferQueue", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 Enable vulkan transfer queues, or 0 to use just graphic queue" );

constexpr uint32_t k_PIPELINE_CACHE_FILE_MAGIC = 0x43462F30; // PCF/0;
constexpr uint32_t k_PIPELINE_CACHE_FILE_VERSION = 10; // 1.0
constexpr uint32_t k_PIPELINE_CAHCE_FILE_SEED = 0x1337;
static const float k_PRIORITY = 1.0f;

/// the cache will be stored in the user space path
static const char* k_CACHE_FILE = { "_pipeline_cache.pcf" };

#pragma pack( push, 1 )
struct cache_header_t
{
    // an arbitrary magic header to make sure this is actually our file
    uint32_t magic = 0;
    // version of the cache manager
    uint32_t version = 0;
    // this header length
    uint32_t length = 0;
    // equal to *pDataSize returned by vkGetPipelineCacheData   
    uint32_t dataSize = 0;
    // a hash of pipeline cache data, including the header
    uint32_t dataHash = 0;
    // equal to VkPhysicalDeviceProperties::vendorID
    uint32_t vendorID = 0;
    // equal to VkPhysicalDeviceProperties::deviceID
    uint32_t deviceID = 0;
    // VkPhysicalDeviceProperties::driverVersion
    uint32_t driverVersion = 0;
    // equal to sizeof(void*)
    uint32_t driverABI = 0;
    // equal to VkPhysicalDeviceProperties::pipelineCacheUUID 
    uint8_t uuid[VK_UUID_SIZE]; // Device driver info
};
#pragma pack( pop )


/*
==============
crVulkanRenderDevice::crVulkanRenderDevice
==============
*/
crVulkanRenderDevice::crVulkanRenderDevice( void ) : 
    m_physic( nullptr ),
    m_logic( nullptr ),
    m_present( nullptr ),
    m_graphic( nullptr ),
    m_compute( nullptr ),
    m_transfer( nullptr )
{
}

/*
==============
crVulkanRenderDevice::crVulkanRenderDevice
==============
*/
crVulkanRenderDevice::crVulkanRenderDevice(  const uint32_t in_ID, const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface ) :
    m_physic( nullptr ),
    m_logic( nullptr ),
    m_present( nullptr ),
    m_graphic( nullptr ),
    m_compute( nullptr ),
    m_transfer( nullptr )
{
    VkResult result = VK_SUCCESS;
    uint32_t i = 0;
    uint32_t queueFamilyCount = 0;
    uint32_t deviceExtensionCount = 0;
    uint32_t formatCount = 0;
    uint32_t presentModeCount = 0;
    
    m_id = in_ID;
    m_queueFamilyPropertiesList = idList<VkQueueFamilyProperties2, TAG_VULKAN>();
    m_deviceExtensions = idList<VkExtensionProperties>();
    m_surfaceFormats = idList<VkSurfaceFormat2KHR>();
    m_presentModes = idList<VkPresentModeKHR>();
    m_queues = idList<queueInfo_t>();
    m_physic = in_device;
    
    VkPhysicalDeviceSurfaceInfo2KHR deviceSurfaceInfo{}; 
    deviceSurfaceInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    deviceSurfaceInfo.surface = in_surface;
    deviceSurfaceInfo.pNext = nullptr;

    ///
    /// concatenate device properties
    ///
  
    // device properties Vulkan 1.2
    m_propertiesv13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
    m_propertiesv13.pNext = nullptr;
   
    // device properties Vulkan 1.2
    m_propertiesv12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
    m_propertiesv12.pNext = &m_propertiesv13;
   
    // device properties Vulkan 1.1
    m_propertiesv11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
    m_propertiesv11.pNext = &m_propertiesv12;

    // device properties Vulkan 1.0
    m_propertiesv10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    m_propertiesv10.pNext = &m_propertiesv11;

    // retrieve device properties
    vkGetPhysicalDeviceProperties2( m_physic, &m_propertiesv10 );

    ///
    /// concatenate device features
    ///

    /// Query 
    //m_deviceExtendedDynamicStateFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    //m_deviceExtendedDynamicStateFeaturesEXT.pNext = nullptr;

    /// aquire device vulkan 1.3 features 
    m_featuresv13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    m_featuresv13.pNext = nullptr; //&m_deviceExtendedDynamicStateFeaturesEXT;
    
    /// aquire device vulkan 1.2 features 
    m_featuresv12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    m_featuresv12.pNext = &m_featuresv13;

    /// aquire device vulkan 1.1 features 
    m_featuresv11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	m_featuresv11.pNext = &m_featuresv12; 

    /// aquire device vulkan 1.0 features 
    m_featuresv10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    m_featuresv10.pNext = &m_featuresv11;
    
    /// query features
	vkGetPhysicalDeviceFeatures2( m_physic, &m_featuresv10 );

    // query device memory properties
    m_memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    m_memoryProperties.pNext = nullptr;
    vkGetPhysicalDeviceMemoryProperties2( m_physic, &m_memoryProperties );

    // Find a queue family that supports graphics and presentation
	vkGetPhysicalDeviceQueueFamilyProperties2( m_physic, &queueFamilyCount, nullptr);
    m_queueFamilyPropertiesList.SetNum( queueFamilyCount );
    for ( i = 0; i < queueFamilyCount; ++i) 
    {
        m_queueFamilyPropertiesList[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        m_queueFamilyPropertiesList[i].pNext = nullptr; // Boa prática garantir que seja nulo
    }
    vkGetPhysicalDeviceQueueFamilyProperties2( m_physic, &queueFamilyCount, m_queueFamilyPropertiesList.Ptr() );

    // list the device available extensions 
    result = vkEnumerateDeviceExtensionProperties( m_physic, nullptr, &deviceExtensionCount, nullptr );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkEnumerateDeviceExtensionProperties failed!", VulkanErrorString( result ).c_str() );
        // TODO: exit ? fatal error ? throw a execption
    }
    
    m_deviceExtensions.SetNum( deviceExtensionCount);
	result = vkEnumerateDeviceExtensionProperties( m_physic, nullptr, &deviceExtensionCount, m_deviceExtensions.Ptr() );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkEnumerateDeviceExtensionProperties failed!", VulkanErrorString( result ).c_str() );
        // TODO: exit ? fatal error ? throw a execption
    }
    
    // query device suported surface formats
    result = vkGetPhysicalDeviceSurfaceFormats2KHR( m_physic, &deviceSurfaceInfo, &formatCount, nullptr );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfaceFormats2KHR failed!", VulkanErrorString( result ).c_str() );
        // TODO: exit ? fatal error ? throw a execption
    }
    
    m_surfaceFormats.SetNum( formatCount );
    for ( i = 0; i < formatCount; i++)
    {
        m_surfaceFormats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
        m_surfaceFormats[i].pNext = nullptr;
    }
    
    result = vkGetPhysicalDeviceSurfaceFormats2KHR( m_physic, &deviceSurfaceInfo, &formatCount, m_surfaceFormats.Ptr() );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfaceFormats2KHR failed!", VulkanErrorString( result ).c_str() );
        // TODO: exit ? fatal error ? throw a execption
    }
 
    // query device suported surface presenting mode
    result = vkGetPhysicalDeviceSurfacePresentModesKHR( m_physic, in_surface, &presentModeCount, nullptr );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfacePresentModesKHR failed!", VulkanErrorString( result ).c_str() );
        // TODO: exit ? fatal error ? throw a execption
    }

    m_presentModes.SetNum( presentModeCount );
    result = vkGetPhysicalDeviceSurfacePresentModesKHR( m_physic, in_surface, &presentModeCount, m_presentModes.Ptr() );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfacePresentModesKHR failed!", VulkanErrorString( result ).c_str() );
        // TODO: exit ? fatal error ? throw a execption
    }

    // query device surface  capabilities
    m_surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    m_surfaceCapabilities.pNext = nullptr;
    result = vkGetPhysicalDeviceSurfaceCapabilities2KHR( m_physic, &deviceSurfaceInfo, &m_surfaceCapabilities ); 
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfaceCapabilities2KHR failed!", VulkanErrorString( result ).c_str() );
        // TODO: exit ? fatal error ? throw a execption
    }

    for ( uint32_t family = 0; family < m_queueFamilyPropertiesList.Num(); family++ )
    {
        VkBool32 presentSupport = VK_FALSE;
        auto queueFamilyProperties = m_queueFamilyPropertiesList[family].queueFamilyProperties;
        vkGetPhysicalDeviceSurfaceSupportKHR( m_physic, family, in_surface, &presentSupport );
        queueInfo_t queue{};
        queue.family = family;
        queue.graphic = ( queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT );
        queue.compute = ( queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT ); 
        queue.transfer = ( queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT ); 
        queue.present = presentSupport == VK_TRUE;

        for ( uint32_t index = 0; index < queueFamilyProperties.queueCount; index++)
        {
            queue.index = index;
            m_queues.Append( queue );
        }
    }
    
    for ( i = 0; i < m_queues.Num(); i++)
    {
        auto queue = m_queues[i];
        common->Printf( "Queue %u: - index %u - family %u - present %s - graphic %s - compute %s - transfer %s\n", 
            i, queue.index, queue.family, queue.present ? "Yes" : "No", queue.graphic ? "Yes" : "No", queue.compute ? "Yes" : "No", queue.transfer ? "Yes" : "No" );
    }  
    
    m_name = m_propertiesv10.properties.deviceName;
    m_internalProperties.deviceID = m_propertiesv10.properties.deviceID;
    m_internalProperties.vendorID = m_propertiesv10.properties.vendorID;
    m_internalProperties.driverVersion = m_propertiesv10.properties.driverVersion;
    
    ///
    m_internalProperties.BCnTextureCompression = m_featuresv10.features.textureCompressionBC;
    m_internalProperties.ETC2TextureCompression = m_featuresv10.features.textureCompressionETC2;
    m_internalProperties.asotropicFiltering = m_featuresv10.features.samplerAnisotropy;
    m_internalProperties.maxSampleCount = m_propertiesv10.properties.limits.maxSamplerAllocationCount;
    m_internalProperties.maxAnisotropicFiltering = m_propertiesv10.properties.limits.maxSamplerAnisotropy;
    m_internalProperties.maxTextureLODBias = m_propertiesv10.properties.limits.maxSamplerLodBias;
    m_internalProperties.depthBoundsTestAvailable = m_featuresv10.features.depthBounds;
    m_internalProperties.occlusionQueryAvailable = m_featuresv10.features.occlusionQueryPrecise;
    m_internalProperties.timerQueryAvailable = m_propertiesv10.properties.limits.timestampComputeAndGraphics;
    m_internalProperties.timestampPeriod = m_propertiesv10.properties.limits.timestampPeriod;
}

/*
==============
crVulkanRenderDevice::~crVulkanRenderDevice
==============
*/
crVulkanRenderDevice::~crVulkanRenderDevice( void )
{
}

/*
==============
crVulkanRenderDevice::Create
==============
*/
bool crVulkanRenderDevice::Create( const char** in_layers, const uint32_t in_numLayers, const char** in_enabledExtensions, const uint32_t in_numExtensions )
{
    VkResult result = VK_SUCCESS;
    idList<VkDeviceQueueCreateInfo> queuesCI;

    common->Printf( "Initializing Vulkan Device %s\n", Name() );

    // find device queues
    SelectDeviceQueues( queuesCI );

    // Logic device create structure
    VkDeviceCreateInfo deviceCI{};
    deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCI.queueCreateInfoCount = queuesCI.Num();
    deviceCI.pQueueCreateInfos = queuesCI.Ptr();

    if ( m_featuresv12.timelineSemaphore != VK_TRUE )
    {
        common->Error( "unsuported device, Missing Vulkan 1.2 Timeline Semaphore feature!\n");
        return false;
    }

    if ( m_featuresv13.dynamicRendering != VK_TRUE )
    {
        common->Error( "unsuported device, Missing Vulkan 1.3 DynamicRendering features!\n");
        return false;
    }
    
    // configure device features
    // concatenate device features initialization 
    m_featuresv12.pNext = &m_featuresv13;   // initialize vulkan 1.3 device features
    m_featuresv11.pNext = &m_featuresv12;   // initialize vulkan 1.2 device features 
    m_featuresv10.pNext = &m_featuresv11;   // initialize vulkan 1.1 device features 
    deviceCI.pNext = &m_featuresv10;        // initialize vulkan 1.0 device features 

    /// check if all device extenions are available
    for ( uint32_t i = 0; i < in_numExtensions; i++)
    {
        auto ext = in_enabledExtensions[i];
        if( !ExtensionAvailable( ext ) )
            return false;
    }

    // Enable extensions
    deviceCI.enabledExtensionCount = in_numExtensions;
    deviceCI.ppEnabledExtensionNames = in_enabledExtensions;
    deviceCI.enabledLayerCount = in_numLayers;
    deviceCI.ppEnabledLayerNames = in_layers;

    // create the logic device handle
    result = vkCreateDevice( m_physic, &deviceCI, k_allocationCallbacks, &m_logic );
    if( !ResultCheck( result, "vkCreateDevice" ) )
        return false;

    // Initialize present and graphyc queue
    // this are required queues
    if ( !m_present || !m_graphic )
    {
        if( !m_present ) idLib::Error( "Missing Present Queue\n" );
        if( !m_graphic ) idLib::Error( "Missing Graphic Queue\n" );
        return false;
    }
    
    m_present->Init( m_logic );
    m_graphic->Init( m_logic );

    // if we found a compute queue, initialize
    // if no compute queue is found, we will use the graphic queue
    if ( m_compute != nullptr )
        m_compute->Init( m_logic );
    else
        idLib::Warning( "No compute queue found, using graphic!\n" );

    // If a transfer queue is found, initialize
    // if not, use a graphic queue copy
    if( m_transfer != nullptr )
        m_transfer->Init( m_logic );
    else
        idLib::Warning( "No transfer queue found, using graphic!\n" );

    if( !LoadCache() )
        idLib::Printf( "failed to load cache\n" );

    if( !InitDeviceHeap() )
    {
        idLib::Error( "FAILED TO INITIALIZE DEVICE HEAP!\n" );
        return false;
    }

    return true;
}

/*
==============
crVulkanRenderDevice::Destroy
==============
*/
void crVulkanRenderDevice::Destroy(void)
{
    ///
    if( !SaveCache() )
        idLib::Error( "Failed to load cache\n" );

    /// Release cache 
    if( m_pipelineCache != nullptr )
    {
        vkDestroyPipelineCache( m_logic, m_pipelineCache, k_allocationCallbacks );
        m_pipelineCache = nullptr;
        m_cacheLoaded = false;
    }

    /// release memory 
    for ( uint32_t i = 0; i < m_types.Num(); i++)
    {
        auto type = m_types[i];
        for ( uint32_t j = 0; j < type.pools.Num(); j++)
        {
            /// Well this is actually an error, if memory
            /// are not released at current point, we have a
            /// leack
            Free( type.pools[j] );
        }   
    }

    if( m_transfer != nullptr )
    {
        delete m_transfer;
        m_transfer = nullptr;
    }

    if( m_compute != nullptr )
    {
        delete m_compute;
        m_compute = nullptr;
    }

    if( m_graphic != nullptr )
    {
        delete m_graphic;
        m_graphic = nullptr;
    }

    if( m_present != nullptr )
    {
        delete m_present;
        m_present = nullptr;
    }

    if ( m_logic != nullptr )
    {
        vkDestroyDevice( m_logic, k_allocationCallbacks );
        m_logic = nullptr;
    }
}

/*
==============
crVulkanRenderDevice::Name
==============
*/
const char *crVulkanRenderDevice::Name( void ) const
{
    return m_name.c_str();
}

/*
==============
crVulkanRenderDevice::Properties
==============
*/
const crRenderDevice::properties_t crVulkanRenderDevice::Properties(void) const
{
    return m_internalProperties;
}

/*
==============
crVulkanRenderDevice::DeviceScore
==============
*/
const int32_t crVulkanRenderDevice::Score( void ) const
{
    int32_t score = 0;

    // missing timeline semaphores, that is required
    if ( !m_featuresv12.timelineSemaphore )
        return -1;

    // missing dynamic rendering, that is required
    if( !m_featuresv13.dynamicRendering )
        return -1;

    switch ( m_propertiesv10.properties.deviceType )
    {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        score += 10;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        score += 50;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 150;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 200;
    default:
        score += 1;
        break;
    };

    // TODO:
    for ( uint32_t i = 0; i < m_presentModes.Num(); i++)
    {
        switch ( m_presentModes[i] )
        {
        case VK_PRESENT_MODE_MAILBOX_KHR:
            score += 10;
            break;

        case VK_PRESENT_MODE_FIFO_KHR:
            score += 20;
            break;

        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            score += 50;
            break;

        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
            score += 50;
            break;

        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            score += 50;
            break;
        
        default:
            break;
        }
    };

    score += m_queueFamilyPropertiesList.Num() * 10;

    for ( uint32_t i = 0; i < m_queueFamilyPropertiesList.Num(); i++)
    {
        auto queueFamilyProperties = m_queueFamilyPropertiesList[i].queueFamilyProperties;
        if ( queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT )
            score += 100;

        if( queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT )
            score += 100;
    };

    return score;
}

/*
==============
crVulkanRenderDevice::ReloadCache
==============
*/
bool crVulkanRenderDevice::ReloadCache( void )
{
    if ( m_pipelineCache )
    {
        vkDestroyPipelineCache( m_logic, m_pipelineCache, k_allocationCallbacks );
        m_pipelineCache = nullptr;
    }
    
    return LoadCache();
}


/*
==============
crMemoryHeap::Alloc
==============
*/
crMemoryPool* crVulkanRenderDevice::Alloc( const size_t in_size, const size_t in_alignment, const uint32_t in_filter, const VkMemoryPropertyFlags in_properties )
{
    memoryTypeInfo_t*   type = nullptr; 
    size_t alignedSize = ( in_size + ( in_alignment - 1)) & ~( in_alignment - 1 );

    /// Find the suitabe memory type
    for ( uint32_t i = 0; i < m_types.Num(); i++)
    {
        memoryTypeInfo_t memoryType = m_types[i];    
		if ( in_filter & ( 1 << i ) && ( memoryType.propertyFlags & in_properties ) == in_properties )
				type = &m_types[i];
    }

    /// no suitable found
    if( type == nullptr )
    {
        idLib::Error( "Failed to find a suitabe memory type to alloc a device Memory Heap\n" );
        return nullptr;
    }

    /// check for available heap memory
    // TODO: But perhaps that's not the total amount of contiguous memory available. Something to think about in the future.
    if( m_heaps[type->heapIndex].free < alignedSize ) 
    {
        idLib::Error( "Failed to find a suitabe memory type to alloc a device Memory Heap\n" );
        return nullptr;
    }

    /// Alloc the memory
    crMemoryPool* memoryPage = new crMemoryPool();
    if( !memoryPage->Create( alignedSize, in_alignment, in_properties ) )
    {
        delete memoryPage;
        return nullptr;
    }

    /// update heap info
    m_heaps[type->heapIndex].free -= in_size;
    m_heaps[type->heapIndex].allocated += in_size;

    /// store the page structure for future management ( defragment )
    memoryPage->SetProperties( type->pools.Append( memoryPage ), type->typeIndex );
    return memoryPage;
}

/*
==============
crMemoryHeap::Free
==============
*/
void crVulkanRenderDevice::Free( crMemoryPool* in_pool )
{
    idassert( in_pool != nullptr );
    uint32_t index = in_pool->GetIndex();
    uint32_t type = in_pool->GetType();
    
    /// Remove from the list of used pools
    m_types[type].pools.RemoveIndex( index );

    /// "relase" memory sizes  
    m_heaps[m_types[type].heapIndex].allocated -= in_pool->Size();
    m_heaps[m_types[type].heapIndex].free += in_pool->Size();

    /// release Vulkan device Memory
    in_pool->Destroy();
    delete in_pool;
}

/*
==============
crMemoryHeap::Defrag
==============
*/
void crVulkanRenderDevice::Defrag( void )
{
    /// TODO :P 
    // TODO: Create a separathed thread to move memory, and write to a sub command buffer, and then send at frame beging
}

/*
==============
crVulkanRenderDevice::ExtensionAvailable
==============
*/
const bool crVulkanRenderDevice::ExtensionAvailable( const char* in_ext ) const
{
    for ( uint32_t i = 0; i < m_deviceExtensions.Num(); i++)
    {
        auto ext = m_deviceExtensions[i].extensionName;
#if 0
        if( strncmp( ext, in_ext, strlen( ext ) ) == 0 )
#else
        if( SDL_strncmp( ext, in_ext, strlen( ext ) ) == 0 )
#endif
            return true;
    }
    
    idLib::Warning( "%s not found!\n", in_ext );
    return false;
}

/*
==============
crVulkanRenderDevice::ExtensionAvailable
==============
*/
const bool crVulkanRenderDevice::SupportedFormat( const VkSurfaceFormatKHR in_format ) const
{
    for ( uint32_t i = 0; i < m_surfaceFormats.Num(); i++)
    {
        const VkSurfaceFormatKHR format = m_surfaceFormats[i].surfaceFormat;
        if ( in_format.colorSpace == format.colorSpace && in_format.format == format.format )
            return true;       
    }

    return false;
}

/*
==============
crVulkanRenderDevice::SupportedPresentMode
==============
*/
const bool crVulkanRenderDevice::SupportedPresentMode( const VkPresentModeKHR in_mode ) const
{
    for ( uint32_t i = 0; i < m_presentModes.Num(); i++)
    {
        auto mode = m_presentModes[i];
        if ( mode == in_mode )
            return true;        
    }
    
    return false;
}

/*
==============
crVulkanRenderDevice::SupportedDepthFormat
==============
*/
const bool crVulkanRenderDevice::SupportedDepthFormat( const VkFormat in_depthFormat ) const
{
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
	static VkFormat formatList[5] = 
    {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

    for ( uint32_t i = 0; i < 5; i++)
    {
        VkFormatProperties2 formatProps{};
        if ( formatList[i] != in_depthFormat )
            continue;

		vkGetPhysicalDeviceFormatProperties2( m_physic, formatList[i], &formatProps );
		if (formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			return true;
	}

	return false;
}

/*
==============
crVulkanRenderDevice::SupportedDepthStencilFormat
==============
*/
const bool crVulkanRenderDevice::SupportedDepthStencilFormat( const VkFormat in_depthStencilFormat ) const
{
    VkFormat formatList[3] = 
    {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
	};

    for ( uint32_t i = 0; i < 3; i++)
    {
        VkFormatProperties2 formatProps{};
        if ( formatList[i] != in_depthStencilFormat )
            continue;
        
        vkGetPhysicalDeviceFormatProperties2( m_physic, formatList[i], &formatProps);
        if ( formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
            return true;
    }

	return false;
}

/*
==============
crVulkanRenderDevice::FormatIsFilterable
==============
*/
const bool crVulkanRenderDevice::FormatIsFilterable(const VkFormat in_format, const VkImageTiling tiling) const
{
    VkFormatProperties2 formatProps;
    vkGetPhysicalDeviceFormatProperties2( m_physic, in_format, &formatProps);

	if (tiling == VK_IMAGE_TILING_OPTIMAL)
		return formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	if (tiling == VK_IMAGE_TILING_LINEAR)
		return formatProps.formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	return false;
}

#if 0
/*
==============
crVulkanRenderDevice::MaxSamples
==============
*/
uint32_t crVulkanRenderDevice::MaxSamples(void) const
{
    auto limits = m_propertiesv10.properties.limits;
    VkSampleCountFlags counts = limits.framebufferColorSampleCounts &  limits.framebufferDepthSampleCounts;

    if ( counts & VK_SAMPLE_COUNT_64_BIT ) return 64;
    if ( counts & VK_SAMPLE_COUNT_32_BIT ) return 32;
    if ( counts & VK_SAMPLE_COUNT_16_BIT ) return 16;
    if ( counts & VK_SAMPLE_COUNT_8_BIT ) return 8; 
    if ( counts & VK_SAMPLE_COUNT_4_BIT ) return 4; 
    if ( counts & VK_SAMPLE_COUNT_2_BIT ) return 2;

    // unsuported 
    return 1;
}
#endif

/*
==============
crVulkanRenderDevice::SelectDeviceQueues
@CristianoBeato: holly cow hard coded as fuck, need found a new algoritm, or easy way to do this 
==============
*/
void crVulkanRenderDevice::SelectDeviceQueues( idList<VkDeviceQueueCreateInfo> &in_queueList )
{
    std::optional<queueInfo_t> graphic;
    std::optional<queueInfo_t> present;
    std::optional<queueInfo_t> transfer;
    std::optional<queueInfo_t> compute;

    // find present (first queue with graphic == true)
    for (uint32_t i = 0; i < m_queues.Num(); ++i)
    {
        auto q = m_queues[i];
        if (!q.graphic) // skip non-graphic
            continue;

        graphic = q;
        break;
    }

    /// if not found, don't continue 
    if (!graphic.has_value())
        throw idException("No Graphic queue found in initialized device\n");

    // find present
    // prefer a present in a different family than present (dedicated)
    for (uint32_t i = 0; i < m_queues.Num(); ++i)
    {
        auto q = m_queues[i];
        if (!q.present)
            continue;

        if (q.family == graphic->family)
            continue; // prefer different family

        present = q;
        break;
    }

    // fallback: any present (including from graphic same family)
    if (!present.has_value())
    {
        for (uint32_t i = 0; i < m_queues.Num(); ++i)
        {
            auto q = m_queues[i];
            if (!q.present)
                continue;

            if ( graphic.value().family == q.family )
            {
                /// we can't use the same queue for both since family count 
                if( m_queueFamilyPropertiesList[q.family].queueFamilyProperties.queueCount < 2 )
                    continue; 
            }
            
            present = q;
            break;
        }
    }

    // if no present queue found, we don't need to proceed
    if (!present.has_value())
        throw idException("No present queue found in initialized device\n");

    // if enabled, find a transfer queue
    if (vk_useTransferQueue.GetBool())
    {
        // try a dedicated transfer family
        for (uint32_t i = 0; i < m_queues.Num(); ++i)
        {
            auto q = m_queues[i];
            
            // check if family support transfer commands
            if (!q.transfer)
                continue;

            // check for a family not used in presentation
            if (q.family == present->family)
                continue;

            // check for a family not used in graphics
            if (q.family == graphic->family)
                continue;

            transfer = q;
            break;
        }

        // fallback: accept transfer that doesn't 
        // share the same queue index as present or graphic
        if (!transfer.has_value())
        {
            for (uint32_t i = 0; i < m_queues.Num(); ++i)
            {
                auto q = m_queues[i];
                
                // ignore families that not suport transfer
                if (!q.transfer)
                    continue;
 
                if (q.index == present->index)
                    continue;

                if (q.index == graphic->index)
                    continue;

                transfer = q;
                break;
            }
        }
    }

    // if enabled, try find a compute queue
    if (vk_useComputQueues.GetBool())
    {
        // try a dedicated compute family
        for (uint32_t i = 0; i < m_queues.Num(); ++i)
        {
            auto q = m_queues[i];

            // check if family support compute commands
            if (!q.compute)
                continue;

            // ignore used in present
            if (q.family == present->family)
                continue;

            // ignore used in graphics 
            if (q.family == graphic->family)
                continue;

            // ignore used in transfer
            if (transfer.has_value() && q.family == transfer->family)
                continue;
                
            compute = q;
            break;
        }

        // fallback: accept compute that doesn't reuse present/graphic/transfer *index*
        if (!compute.has_value())
        {
            for (uint32_t i = 0; i < m_queues.Num(); ++i)
            {
                auto q = m_queues[i];
                if (!q.compute)
                    continue;
                if (q.index == present->index)
                    continue;
                if (q.index == graphic->index)
                    continue;
                if (transfer.has_value() && q.index == transfer->index)
                    continue;
                compute = q;
                break;
            }
        }
    }

    // helper to append or increment crQueueCreateInfo in in_queueList
    static const float k_PRIORITY_LIST[4] = { k_PRIORITY, k_PRIORITY, k_PRIORITY, k_PRIORITY }; // safe storage
    auto AppendQueue = [&](const queueInfo_t &q)
    {
        for (uint32_t i = 0; i < in_queueList.Num(); ++i)
        {
            if (in_queueList[i].queueFamilyIndex == q.family)
            {
                if (q.index >= in_queueList[i].queueCount)
                {
                    in_queueList[i].queueCount = q.index + 1;
                    return;
                }
            }
        }

        crQueueCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.queueFamilyIndex = q.family;
        ci.queueCount = q.index + 1;
        ci.pQueuePriorities = k_PRIORITY_LIST;
        in_queueList.Append(ci);
    };

    // append present and graphic first (guaranteed to exist)
    {
        auto p = present.value();
        AppendQueue(p);
        m_present = new crQueue(p.family, p.index);
    
        auto g = graphic.value();
        AppendQueue(g);
        m_graphic = new crQueue(g.family, g.index);
    }

    //  transfer, compute if present
    if ( transfer.has_value() )
    {
        auto t = transfer.value();
        AppendQueue(t);
        m_transfer = new crQueue(t.family, t.index);
    }
    else
    {
        m_transfer = nullptr;
    }

    if ( compute.has_value() )
    {
        auto c = compute.value();
        AppendQueue(c);
        m_compute = new crQueue(c.family, c.index);
    }
    else
    {
        m_compute = nullptr;
    }
}


/*
==============
crVulkanRenderDevice::InitDeviceHeap
==============
*/
bool crVulkanRenderDevice::InitDeviceHeap( void )
{
    uint32_t i = 0;

    auto props = m_memoryProperties.memoryProperties;
    if( props.memoryHeapCount < 1 || props.memoryTypeCount < 1 )
        return false;
    
    /// In the Vulkan ecosystem, the term "heap" refers to large, 
    /// contiguous blocks of physical memory available in the hardware 
    /// (such as the video card's VRAM or system RAM) that the 
    /// API manages to store resource data such as buffers and images.
    /// Memory Heaps (Device Memory)
    /// These represent the actual physical memory installed in 
    /// the computer or mobile device. When you query the device 
    /// memory properties (VkPhysicalDeviceMemoryProperties), 
    /// Vulkan returns a list of available heaps:

    /// VK_DEVICE_LOCAL_BIT: Indicates memory that is physically located on the GPU. 
    /// It is the fastest memory for graphics processing, 
    /// but is generally not directly accessible by the CPU.

    /// VK_HOST_VISIBLE_BIT: Usually associated with system RAM 
    /// or a portion of VRAM that the CPU can "see" and write to directly.
    
    /// get memory heap
    m_heaps.SetNum( props.memoryHeapCount );
    for ( i = 0; i < props.memoryHeapCount; i++)
    {
        /// aquire heap size
        m_heaps[i].total = props.memoryHeaps[i].size;
        m_heaps[i].free = m_heaps[i].total;
        m_heaps[i].propertyFlags = props.memoryHeaps[i].flags;
    }
    
    /// get types
    m_types.SetNum( props.memoryTypeCount );
    for ( i = 0; i < props.memoryTypeCount; i++)
    {
        m_types[i].typeIndex = i;
        m_types[i].heapIndex = props.memoryTypes[i].heapIndex;
        m_types[i].propertyFlags = props.memoryTypes[i].propertyFlags;
    }
    
    return true;
}


/*
==============
crVulkanRenderDevice::LoadCache
==============
*/
bool crVulkanRenderDevice::LoadCache( void )
{
    idFile* cacheFile = nullptr;
    cache_header_t header{};
    VkPipelineCacheCreateInfo pipelineCacheCI{};
    auto deviceProperties = m_propertiesv10.properties; 
    idStr cacheName = "generated/";
    cacheName += deviceProperties.deviceName;
    cacheName += k_CACHE_FILE;

    /// 
    pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    pipelineCacheCI.pNext = nullptr;
    pipelineCacheCI.flags = 0;
    
    // no cache available
    pipelineCacheCI.initialDataSize = 0;    
    pipelineCacheCI.pInitialData = nullptr;
    m_cacheLoaded = false;

    /// Try read cache file from save dir
    cacheFile =  fileSystem->OpenFileRead( cacheName.c_str() ); 
    if ( cacheFile != nullptr )
    {
        /// retrieve cache header
        cacheFile->Read( &header, sizeof( cache_header_t) );

        /// check device cache compatibility
        if( ( header.magic == k_PIPELINE_CACHE_FILE_MAGIC ) || 
            ( header.version == k_PIPELINE_CACHE_FILE_VERSION ) || 
            ( header.length == sizeof( cache_header_t ) ) )
        {
            /// validate cache compatibility
            if( ( header.driverABI != sizeof( void *) ) ||
                ( header.vendorID != deviceProperties.vendorID ) ||
                ( header.deviceID != deviceProperties.deviceID ) ||
                ( header.driverVersion != deviceProperties.driverVersion ) ||
                ( std::memcmp( header.uuid, deviceProperties.pipelineCacheUUID, sizeof(uint8_t) * VK_UUID_SIZE ) != 0 ) )
            {
                void* cacheData = Mem_Alloc( header.dataSize, TAG_VULKAN);
                /// read the cache 
                cacheFile->Read( cacheData, header.dataSize );

                auto hash = SDL_murmur3_32( cacheData, header.dataSize, k_PIPELINE_CAHCE_FILE_SEED );
                if ( hash == header.dataHash )
                {
                    /// A valid cache available
                    pipelineCacheCI.initialDataSize = header.dataSize;
                    pipelineCacheCI.pInitialData = cacheData;
                    m_cacheLoaded = true;
                }
                else
                {
                    Mem_Free( cacheData );
                    idLib::Error( "Pipeline cache invalid data hash!\n" );
                    m_cacheLoaded = false;
                }
            }
            else
            {
                idLib::Warning( "Pipeline Cache Out of date, rebuilt!\n" );
                m_cacheLoaded = false;
            }
        }
        else
        {
            idLib::Warning( "Unconpatible pipeline cache, engine may updated, rebuild!\n" );
            m_cacheLoaded = false;
        }

        fileSystem->CloseFile( cacheFile );
    }
    else
    {
        idLib::Warning( "Can't read cache file from disk, pipeline conpilation can be slow\n" );
        m_cacheLoaded = false;
    }
    
    /// create a cache from source, or allocate for new one
    auto result = vkCreatePipelineCache( m_logic, &pipelineCacheCI, k_allocationCallbacks, &m_pipelineCache );
    if ( result != VK_SUCCESS )
    {
        if( pipelineCacheCI.pInitialData != nullptr )
            Mem_Free( const_cast<void*>( pipelineCacheCI.pInitialData ) );

        /// fallback, some drivers can invalidade cache data and pass a vkCreatePipelineCache error
        /// so if we fail to create a cache from loaded data, try create a empty cache
        /// recomandation from 
        /// It’s not paranoia if they are really out to get you - "https://zeux.io/2019/07/17/serializing-pipeline-cache/" 
        m_cacheLoaded = false;
        pipelineCacheCI.initialDataSize = 0;    
        pipelineCacheCI.pInitialData = nullptr;
        result = vkCreatePipelineCache( m_logic, &pipelineCacheCI, k_allocationCallbacks, &m_pipelineCache );
        if( result )
        {
            idLib::Error( "crPipelineManager::StartUp::vkCreatePipelineCache %s\n", VulkanErrorString( result ).c_str() );
            return false;
        }
    }

    /// don't leak memory 
    if( pipelineCacheCI.pInitialData != nullptr )
        Mem_Free( const_cast<void*>( pipelineCacheCI.pInitialData ) );

    return true;
}

/*
==============
crVulkanRenderDevice::SaveCache
==============
*/
bool crVulkanRenderDevice::SaveCache(void)
{
    size_t cacheSize = 0;
    VkResult result = VK_SUCCESS;
    void* cacheData = nullptr;
    idFile* cacheFile = nullptr;
    auto deviceProperties = m_propertiesv10.properties; 
    idStr cacheName = "generated/";
    cacheName += deviceProperties.deviceName;
    cacheName += k_CACHE_FILE;

    if( m_cacheLoaded )
        return true;

    cache_header_t header{};
    header.magic = k_PIPELINE_CACHE_FILE_MAGIC;
    header.version = k_PIPELINE_CACHE_FILE_VERSION;
    header.length = sizeof( cache_header_t );
    header.dataSize = 0; 
    header.dataHash = 0;
    header.vendorID = deviceProperties.vendorID;
    header.deviceID = deviceProperties.deviceID;
    header.driverVersion = deviceProperties.driverVersion;
    header.driverABI = sizeof( void *);

    /// retrieve the cache data size 
    result = vkGetPipelineCacheData( m_logic, m_pipelineCache, &cacheSize, nullptr );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "Failed to retrive pipeline cache size\n%s\n", VulkanErrorString( result ).c_str() );
        return false;
    }
    
    /// alloc cache data
    cacheData = Mem_Alloc( cacheSize, TAG_TEMP );

    /// retrieve the cache source data 
    result = vkGetPipelineCacheData( m_logic, m_pipelineCache, &cacheSize, cacheData );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "Failed to retrive pipeline cache data\n%s\n", VulkanErrorString( result ).c_str() );
        return false;
    }

    /// store cache size
    header.dataSize = cacheSize;
    
    /// hash cache data to prevent file corruption
    header.dataHash =  SDL_murmur3_32( cacheData, cacheSize, k_PIPELINE_CAHCE_FILE_SEED );

    /// Try create cache file in the save directory
    cacheFile = fileSystem->OpenFileWrite( cacheName.c_str() ); 
    if ( !cacheFile )
    {
        Mem_Free( cacheData );
        idLib::Error( "Can't write cache file to disk\n" );
        return false;
    }

    /// write the header
    cacheFile->Write( &header, header.length );
    cacheFile->Write( cacheData, cacheSize );

    Mem_Free( cacheData );
    fileSystem->CloseFile( cacheFile );

    return true;
}