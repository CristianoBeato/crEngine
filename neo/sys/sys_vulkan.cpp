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

#include "sys_vulkan.hpp"
#include "idlib/precompiled.h"
#include "renderer/Vulkan/Core.hpp"
#include <optional> // std::optional

#define NO_SDL_VULKAN_TYPEDEFS
#include <SDL3/SDL_vulkan.h>

constexpr uint32_t k_PIPELINE_CACHE_FILE_MAGIC = 0x43462F30; // PCF/0;
constexpr uint32_t k_PIPELINE_CACHE_FILE_VERSION = 10; // 1.0
constexpr uint32_t k_PIPELINE_CAHCE_FILE_SEED = 0x1337;
static const float k_PRIORITY = 1.0f;

/// the cache will be stored in the user space path
static const char* k_CACHE_FILE = { "_pipeline_cache.pcf" };

idCVar vk_useComputQueues( "vk_useComputQueue", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 Enable vulkan find a compute queue, or 0 to use compute queue" );
idCVar vk_useTransferQueue( "vk_useTransferQueue", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 Enable vulkan transfer queues, or 0 to use just graphic queue" );

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

template< typename _type_ >
inline static void GetVkProc( _type_ &in_proc, const char* in_pName, VkInstance in_instance )
{
    in_proc = reinterpret_cast<_type_>( vkGetInstanceProcAddr( in_instance, in_pName ) );
}

#define GET_VK_PROC( P, I ) GetVkProc( P, #P, I )

crRenderAPI *crRenderAPI::Get(void)
{
    static crVulkanAPI gVulkanAPI = crVulkanAPI();
    return &gVulkanAPI;
}


/*
==============
vkDeviceQueue::vkDeviceQueue
==============
*/
vkDeviceQueue::vkDeviceQueue( const uint32_t in_family, const uint32_t in_index ) : 
    m_index( in_index ),
    m_family( in_family ),
    m_queue( nullptr ),
    m_commandPool( nullptr ),
    m_device( nullptr )
{
}

/*
==============
vkDeviceQueue::~vkDeviceQueue
==============
*/
vkDeviceQueue::~vkDeviceQueue( void )
{
    if ( m_semaphore )
    {
        vkDestroySemaphore( m_device, m_semaphore, k_allocationCallbacks );
        m_semaphore = nullptr;
    }

    if ( m_commandPool != nullptr )
    {
        vkDestroyCommandPool( m_device, m_commandPool, k_allocationCallbacks );
        m_commandPool = nullptr;
    }

    m_queue = nullptr;
    m_device = nullptr;
    m_family = 0;
    m_index = 0;
}

/*
==============
vkDeviceQueue::Init
==============
*/
bool vkDeviceQueue::Init( const VkDevice in_device )
{
    m_device = in_device;

    ///
    ///
    /// get device queue
    VkDeviceQueueInfo2 queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
    queueInfo.pNext = nullptr;
    queueInfo.flags = 0;
    queueInfo.queueFamilyIndex = m_family;
    queueInfo.queueIndex = m_index;
    vkGetDeviceQueue2( m_device, &queueInfo, &m_queue );

    ///
    ///
    /// Create queue command pool
    VkCommandPoolCreateInfo commandPoolCI{};
    commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCI.pNext = nullptr;
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCI.queueFamilyIndex = m_family;
    auto result = vkCreateCommandPool( m_device, &commandPoolCI, k_allocationCallbacks, &m_commandPool );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkCreateCommandPool failed! %s\n", VulkanErrorString( result) );
        return false;
    }
    
    return true;    
}

/*
==============
crVulkanRenderDevice::crVulkanRenderDevice
==============
*/
crVulkanRenderDevice::crVulkanRenderDevice( void ) : 
    m_physical( nullptr ),
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
    m_physical( nullptr ),
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
    m_physical = in_device;
    
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
    vkGetPhysicalDeviceProperties2( m_physical, &m_propertiesv10 );

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
	vkGetPhysicalDeviceFeatures2( m_physical, &m_featuresv10 );

    // query device memory properties
    m_memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    m_memoryProperties.pNext = nullptr;
    vkGetPhysicalDeviceMemoryProperties2( m_physical, &m_memoryProperties );

    // Find a queue family that supports graphics and presentation
	vkGetPhysicalDeviceQueueFamilyProperties2( m_physical, &queueFamilyCount, nullptr);
    m_queueFamilyPropertiesList.Resize( queueFamilyCount );
    for ( i = 0; i < queueFamilyCount; ++i) 
    {
        m_queueFamilyPropertiesList[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        m_queueFamilyPropertiesList[i].pNext = nullptr; // Boa prática garantir que seja nulo
    }
    vkGetPhysicalDeviceQueueFamilyProperties2( m_physical, &queueFamilyCount, m_queueFamilyPropertiesList.Ptr() );

    // list the device available extensions 
    result = vkEnumerateDeviceExtensionProperties( m_physical, nullptr, &deviceExtensionCount, nullptr );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkEnumerateDeviceExtensionProperties failed!", VulkanErrorString( result ) );
        // TODO: exit ? fatal error ? throw a execption
    }
    
    m_deviceExtensions.Resize( deviceExtensionCount);
	result = vkEnumerateDeviceExtensionProperties( m_physical, nullptr, &deviceExtensionCount, m_deviceExtensions.Ptr() );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkEnumerateDeviceExtensionProperties failed!", VulkanErrorString( result ) );
        // TODO: exit ? fatal error ? throw a execption
    }
    
    // query device suported surface formats
    result = vkGetPhysicalDeviceSurfaceFormats2KHR( m_physical, &deviceSurfaceInfo, &formatCount, nullptr );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfaceFormats2KHR failed!", VulkanErrorString( result ) );
        // TODO: exit ? fatal error ? throw a execption
    }
    
    m_surfaceFormats.Resize( formatCount );
    for ( i = 0; i < formatCount; i++)
    {
        m_surfaceFormats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
        m_surfaceFormats[i].pNext = nullptr;
    }
    
    result = vkGetPhysicalDeviceSurfaceFormats2KHR( m_physical, &deviceSurfaceInfo, &formatCount, m_surfaceFormats.Ptr() );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfaceFormats2KHR failed!", VulkanErrorString( result ) );
        // TODO: exit ? fatal error ? throw a execption
    }
 
    // query device suported surface presenting mode
    result = vkGetPhysicalDeviceSurfacePresentModesKHR( m_physical, in_surface, &presentModeCount, nullptr );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfacePresentModesKHR failed!", VulkanErrorString( result ) );
        // TODO: exit ? fatal error ? throw a execption
    }

    m_presentModes.Resize( presentModeCount );
    result = vkGetPhysicalDeviceSurfacePresentModesKHR( m_physical, in_surface, &presentModeCount, m_presentModes.Ptr() );
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfacePresentModesKHR failed!", VulkanErrorString( result ) );
        // TODO: exit ? fatal error ? throw a execption
    }

    // query device surface  capabilities
    m_surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    m_surfaceCapabilities.pNext = nullptr;
    result = vkGetPhysicalDeviceSurfaceCapabilities2KHR( m_physical, &deviceSurfaceInfo, &m_surfaceCapabilities ); 
    if ( result != VK_SUCCESS )
    {
        idLib::Error( "vkGetPhysicalDeviceSurfaceCapabilities2KHR failed!", VulkanErrorString( result ) );
        // TODO: exit ? fatal error ? throw a execption
    }

    for ( uint32_t family = 0; family < m_queueFamilyPropertiesList.Num(); family++ )
    {
        VkBool32 presentSupport = VK_FALSE;
        auto queueFamilyProperties = m_queueFamilyPropertiesList[family].queueFamilyProperties;
        vkGetPhysicalDeviceSurfaceSupportKHR( m_physical, family, in_surface, &presentSupport );
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
    result = vkCreateDevice( m_physical, &deviceCI, k_allocationCallbacks, &m_logic );
    if( !ResultCheck( result, "vkCreateDevice" ) )
        return false;

    // Initialize present and graphyc queue
    // this are required queues
    if ( !m_present || !m_graphic )
    {
        if( !m_present )
            idLib::Error( "Missing Present Queue\n" );
        if( !m_graphic )
            idLib::Error( "Missing Present Queue\n" );
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

    idLib::Printf( " -> succes\n" );

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

    if( m_pipelineCache != nullptr )
    {
        
    }

    if ( m_logic != nullptr )
    {
        vkDestroyDevice( m_logic, k_allocationCallbacks );
        m_logic = nullptr;
    }
}

const char *crVulkanRenderDevice::Name( void ) const
{
    return m_name.c_str();
}

const crRenderDevice::properties_t crVulkanRenderDevice::Properties(void) const
{
    return m_internalProperties;
}

const crRenderDevice::features_t crVulkanRenderDevice::Features(void) const
{
    return m_internalFeatures;
}

/*
==============
crVulkanRenderDevice::DeviceScore
==============
*/
const uint32_t crVulkanRenderDevice::Score( void ) const
{
    uint32_t score = 0;

    switch ( m_propertiesv10.properties.deviceType )
    {
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        score += 5;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        score += 10;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        score += 25;
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        score += 50;
    default:
        score += 1;
        break;
    };

    // TODO:
    //for ( uint32_t i = 0; i < m_presentModes.Num(); i++)
    //{
    //    switch ( m_presentModes[i] )
    //    {
    //    case VK_PRESENT_MODE_MAILBOX_KHR:
    //        break;
    //    case VK_PRESENT_MODE_FIFO_KHR:
    //        break;
    //    
    //    default:
    //        break;
    //    }
    //};

    score += m_queueFamilyPropertiesList.Num() * 5;

    for ( uint32_t i = 0; i < m_queueFamilyPropertiesList.Num(); i++)
    {
        auto queueFamilyProperties = m_queueFamilyPropertiesList[i].queueFamilyProperties;
        if ( queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT )
            score += 10;

        if( queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT )
            score += 10;
    };

    return score;
}

/*
==============
crVulkanRenderDevice::FindMemoryType
==============
*/
const uint32_t crVulkanRenderDevice::FindMemoryType( const uint32_t in_filter, const VkMemoryPropertyFlags properties ) const
{
    auto memoryProperties = m_memoryProperties.memoryProperties;
    for ( uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++ )
    {
        auto memoryType = memoryProperties.memoryTypes[i];    
        // Check if the current memory type is acceptable based on the type_filter,
		// the type_filter is a bitmask where each bit represents a memory type that is suitable
        // Check if the memory type has all the desired property flags
        // properties is a bitmask of the required memory properties
		if ( in_filter & (1 << i) && ( memoryType.propertyFlags & properties ) == properties )
				return i;
    }
    
    return UINT32_MAX;
}

/*
==============
crVulkanRenderDevice::ExtensionAvailable
==============
*/
const bool crVulkanRenderDevice::ExtensionAvailable(const idStr &in_ext) const
{
    for ( uint32_t i = 0; i < m_deviceExtensions.Num(); i++)
    {
        idStr ext = m_deviceExtensions[i].extensionName;
        if( ext == in_ext )
            return true;
    }
    
    idLib::Warning( "%s not found!\n", in_ext.c_str() );
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

		vkGetPhysicalDeviceFormatProperties2( m_physical, formatList[i], &formatProps );
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
        
        vkGetPhysicalDeviceFormatProperties2( m_physical, formatList[i], &formatProps);
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
    vkGetPhysicalDeviceFormatProperties2( m_physical, in_format, &formatProps);

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

    // helper to append or increment VkDeviceQueueCreateInfo in in_queueList
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

        VkDeviceQueueCreateInfo ci{};
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
        m_present = new vkDeviceQueue(p.family, p.index);
    
        auto g = graphic.value();
        AppendQueue(g);
        m_graphic = new vkDeviceQueue(g.family, g.index);
    }

    //  transfer, compute if present
    if ( transfer.has_value() )
    {
        auto t = transfer.value();
        AppendQueue(t);
        m_transfer = new vkDeviceQueue(t.family, t.index);
    }
    else
    {
        m_transfer = nullptr;
    }

    if ( compute.has_value() )
    {
        auto c = compute.value();
        AppendQueue(c);
        m_compute = new vkDeviceQueue(c.family, c.index);
    }
    else
    {
        m_compute = nullptr;
    }
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

/*
==============
crVulkanAPI::crVulkanAPI
==============
*/
crVulkanAPI::crVulkanAPI( void )
{
}

/*
==============
crVulkanAPI::~crVulkanAPI
==============
*/
crVulkanAPI::~crVulkanAPI( void )
{
}

/*
==============
crVulkanAPI::StartUp
==============
*/
bool crVulkanAPI::StartUp(void)
{
    VkResult                            result = VK_SUCCESS;
    uint32_t                            physicalDeviceCount = 0;
    Uint32                              SDL3ExtensionCount = 0;
    const char* const*                  SDL3Extensions = nullptr;
	idList<const char*, TAG_VULKAN>     requiredInstanceExtensions;
    idList<const char*, TAG_VULKAN>     requiredDeviceExtensions;
    idList<const char*, TAG_VULKAN>     requestedInstanceLayers;

    requiredInstanceExtensions.Append( VK_KHR_SURFACE_EXTENSION_NAME );
    requiredInstanceExtensions.Append( VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );
    requiredInstanceExtensions.Append( VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME );

    idLib::Printf( "|--- Initializing Vulkan instance. ---|\n");

    /// Load driver functions
    GetInstanceProcs();

    ListExtensionsAndLayers();

	if ( IsExtensionAvailable( VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) )
    {
        m_hasDebugUtils = true;
		requiredInstanceExtensions.Append( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    }
    
    if( IsExtensionAvailable( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME ) )
    {
        m_portabilityEnumerationAvailable = true;
        requiredInstanceExtensions.Append( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
    }

    // copy and enable system extensios
    SDL3Extensions = SDL_Vulkan_GetInstanceExtensions( &SDL3ExtensionCount );
    idLib::Printf( "SDL3 VulkanExt Found:\n" );    
    for ( uint32_t i = 0; i < SDL3ExtensionCount; i++)
    {
        requiredInstanceExtensions.Append( SDL3Extensions[i] );
       idLib::Printf( " - %s\n", SDL3Extensions[i] );
    }

    /// apend layers 
    if( IsLayersAvailable( "VK_LAYER_KHRONOS_validation" ) )
        requestedInstanceLayers.Append( "VK_LAYER_KHRONOS_validation" );    

    VkDebugUtilsMessengerCreateInfoEXT  debugUtilsMessengerCI{};
    debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCI.pNext = nullptr;
    debugUtilsMessengerCI.flags = 0;
    debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	debugUtilsMessengerCI.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugUtilsMessengerCI.pfnUserCallback = DebugCallback;
    debugUtilsMessengerCI.pUserData = this;
    
    /// Try initialize Vulkan instance
    if ( !InitInstance( requestedInstanceLayers, requiredDeviceExtensions, &debugUtilsMessengerCI  ) )
        return false;

    if( m_hasDebugUtils && !InitDebugUtilsMessenger( &debugUtilsMessengerCI ) )
        idLib::Error( "InitDebugUtilsMessenger failed!\n" );
    
    result = vkEnumeratePhysicalDevices( m_instance, &physicalDeviceCount, nullptr);
    if( result != VK_SUCCESS )
    {
        idLib::Error( "vkEnumeratePhysicalDevices failed! %s\n" );
        return false;
    }

    if ( physicalDeviceCount < 1 )
    {
        idLib::FatalError( "No vulkan suported physical device found." );
        return false;
    }
    
    m_availablePhysicalDevices.Resize( physicalDeviceCount );
    result = vkEnumeratePhysicalDevices( m_instance, &physicalDeviceCount, m_availablePhysicalDevices.Ptr() );    
    if( result != VK_SUCCESS )
    {
        idLib::Error( "vkEnumeratePhysicalDevices failed! %s\n" );
        return false;
    }

    return true;
}

void crVulkanAPI::ShutDown(void)
{
    ReleaseSurface();
    ReleaseDebugUtilsMessenger();
    ReleaseInstance();
}

uint32_t crVulkanAPI::GetDevices( crRenderDevice **in_deviceArray )
{
    /// just get the device count 
    if( in_deviceArray == nullptr )
        return m_availablePhysicalDevices.Num();

    for ( uint32_t i = 0; i < m_availablePhysicalDevices.Num(); i++)
    {
         auto device = in_deviceArray[i];
         device = new crVulkanRenderDevice( i, m_availablePhysicalDevices[i], m_surface );
    }
}

/*
==============
crVulkanAPI::ListExtensionsAndLayers
==============
*/
void crVulkanAPI::ListExtensionsAndLayers(void)
{
    VkResult    result = VK_SUCCESS;
    uint32_t    extensionCount = 0;
    uint32_t    layerCount = 0;

    /// enumerate instance extensions
    result = vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr);
    if ( result != VK_SUCCESS )
        idLib::Error( "vkEnumerateInstanceExtensionProperties failed! %s\n", VulkanErrorString( result ) );

    m_availableInstanceExtensions.Resize( extensionCount );
	result = vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, m_availableInstanceExtensions.Ptr() );
    if ( result != VK_SUCCESS )
        idLib::Error( "vkEnumerateInstanceExtensionProperties failed! %s\n", VulkanErrorString( result ) );

    /// instance enumerate layers 
    result = vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
    if ( result != VK_SUCCESS )
        idLib::Error( "vkEnumerateInstanceLayerProperties failed! %s\n", VulkanErrorString( result ) );

    m_supportedInstanceLayers.Resize( layerCount );
    result = vkEnumerateInstanceLayerProperties( &layerCount, m_supportedInstanceLayers.Ptr());
    if ( result != VK_SUCCESS )
        idLib::Error( "vkEnumerateInstanceLayerProperties failed! %s\n", VulkanErrorString( result ) );
}

/*
==============
crVulkanAPI::InitInstance
==============
*/
bool crVulkanAPI::InitInstance(const idList<const char *> in_requestedInstanceLayers,
                               const idList<const char *> &in_requiredInstanceExtensions,
                               const VkDebugUtilsMessengerCreateInfoEXT *in_debugUtilsMessengerCI)
{
    VkResult    result = VK_SUCCESS;
    /// Application Info
    ///
    ///
    VkApplicationInfo       application{};
    application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application.pApplicationName = GAME_NAME;
    application.pEngineName = ENGINE_VERSION;
    application.apiVersion = VK_API_VERSION_1_3;

    /// Create the vulkan API instance
	VkInstanceCreateInfo    instance{};
    instance.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance.flags = 0;
    instance.pNext = in_debugUtilsMessengerCI;
	instance.pApplicationInfo = &application;
	instance.enabledLayerCount = in_requestedInstanceLayers.Num();
	instance.ppEnabledLayerNames = in_requestedInstanceLayers.Ptr();
	instance.enabledExtensionCount = in_requiredInstanceExtensions.Num();
	instance.ppEnabledExtensionNames = in_requiredInstanceExtensions.Ptr();

    /// 
    if ( m_portabilityEnumerationAvailable )
        instance.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	result = vkCreateInstance( &instance, k_allocationCallbacks, &m_instance );
    if ( !result != VK_SUCCESS )
    {
        common->Error( "vkCreateInstance failed: %s\n", VulkanErrorString( result ) );
        return false;
    }

    return true;
}

/*
==============
crVulkanAPI::ReleaseInstance
==============
*/
void crVulkanAPI::ReleaseInstance(void)
{
    if( m_instance != nullptr )
    {
        vkDestroyInstance( m_instance, k_allocationCallbacks );
        m_instance = nullptr;
    }
}

/*
==============
crVulkanAPI::InitSurface
==============
*/
bool crVulkanAPI::InitSurface(void)
{
    auto video = crVideo::Get();
    if ( !SDL_Vulkan_CreateSurface( static_cast<SDL_Window*>( video->WindowHandler() ), m_instance, k_allocationCallbacks, &m_surface ) )
    {
        idLib::Printf( "Failed to create window surface %s\n", SDL_GetError() );
        return false;
    }

    return true;
}

/*
==============
crVulkanAPI::ReleaseSurface
==============
*/
void crVulkanAPI::ReleaseSurface(void)
{
    if( m_surface != nullptr )
    {
        vkDestroySurfaceKHR( m_instance, m_surface, k_allocationCallbacks );
        m_surface = nullptr;
    }
}

/*
==============
crVulkanAPI::InitDebugUtilsMessenger
==============
*/
bool crVulkanAPI::InitDebugUtilsMessenger( const VkDebugUtilsMessengerCreateInfoEXT *in_debugUtilsMessengerCI )
{
    VkResult    result = VK_SUCCESS;
    if ( !m_hasDebugUtils )
        return false;

    result = vkCreateDebugUtilsMessengerEXT( m_instance, in_debugUtilsMessengerCI, k_allocationCallbacks, &m_debugUtilsMessenger );
    if( result != VK_SUCCESS )
    {
        idLib::Error( "vkCreateDebugUtilsMessengerEXT failed! %s\n", VulkanErrorString( result ) );
        m_hasDebugUtils = false;
        return false;
    }

    return true;
}

/*
==============
crVulkanAPI::ReleaseDebugUtilsMessenger
==============
*/
void crVulkanAPI::ReleaseDebugUtilsMessenger(void)
{
    if( m_debugUtilsMessenger != nullptr )
    {
        vkDestroyDebugUtilsMessengerEXT( m_instance, m_debugUtilsMessenger, k_allocationCallbacks );
        m_debugUtilsMessenger = nullptr;
    }
}

/*
==============
crVulkanAPI::ExtensionAvailable
==============
*/
bool crVulkanAPI::IsExtensionAvailable(const idStr &in_ext) const
{
    for ( uint32_t i = 0; i < m_availableInstanceExtensions.Num(); i++)
    {
        idStr ext = m_availableInstanceExtensions[i].extensionName;
        if( ext == in_ext )
            return true;
    }
    
    printf( "%s not found!\n", in_ext.c_str() );
    return false;
}

/*
==============
crVulkanAPI::LayersAvailable
==============
*/
bool crVulkanAPI::IsLayersAvailable(const idStr &in_layer) const
{
    for ( uint32_t i = 0; i < m_supportedInstanceLayers.Num(); i++)
    {
        idStr layer = m_supportedInstanceLayers[i].layerName;
        if( layer == in_layer )
        {
            printf( "%s found\n", in_layer.c_str() );
            return true;
        }
    }
    
    printf( "%s not found\n", in_layer.c_str() );
    return false;
}

/*
==============
crVulkanAPI::GetInstanceProcs
==============
*/
void crVulkanAPI::GetInstanceProcs(void)
{
    /// get tne vkGetInstanceProcAddr from SDL_Vulkan_LoadLibrary, called before window creation 
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>( SDL_Vulkan_GetVkGetInstanceProcAddr() );
    assert( vkGetInstanceProcAddr != nullptr );

    //
    GET_VK_PROC( vkEnumerateInstanceLayerProperties, nullptr );
    assert( vkEnumerateInstanceLayerProperties != nullptr );

    GET_VK_PROC( vkEnumerateInstanceExtensionProperties, nullptr );
    assert( vkEnumerateInstanceExtensionProperties != nullptr );

    GET_VK_PROC( vkCreateInstance, nullptr );
    assert( vkCreateInstance != nullptr );
}

/*
==============
crVulkanAPI::LoadVulkanProcs
==============
*/
void crVulkanAPI::LoadVulkanProcs( void )
{
    GET_VK_PROC( vkCreateDebugUtilsMessengerEXT, m_instance );
    GET_VK_PROC( vkDestroyDebugUtilsMessengerEXT, m_instance );

    GET_VK_PROC( vkGetInstanceProcAddr, m_instance );
    GET_VK_PROC( vkEnumerateInstanceLayerProperties, m_instance );
    GET_VK_PROC( vkCreateInstance, m_instance );
    GET_VK_PROC( vkDestroyInstance, m_instance );
    GET_VK_PROC( vkEnumeratePhysicalDevices, m_instance );

    GET_VK_PROC( vkDestroySurfaceKHR, m_instance );

    // VkPhysicalDevice
    GET_VK_PROC( vkGetPhysicalDeviceMemoryProperties2, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceProperties2, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceFeatures2, m_instance );
    GET_VK_PROC( vkEnumerateDeviceExtensionProperties, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceQueueFamilyProperties2, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceSurfacePresentModesKHR, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceSurfaceCapabilities2KHR, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceSurfaceFormats2KHR, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceSurfaceSupportKHR, m_instance );
    GET_VK_PROC( vkGetPhysicalDeviceFormatProperties2, m_instance );

    // VkDevice
    GET_VK_PROC( vkCreateDevice, m_instance ) ;
    GET_VK_PROC( vkDestroyDevice, m_instance ) ;
    GET_VK_PROC( vkDeviceWaitIdle, m_instance ) ;
    GET_VK_PROC( vkGetDeviceProcAddr, m_instance ) ;

    // VkQueue
    GET_VK_PROC( vkGetDeviceQueue, m_instance );
    GET_VK_PROC( vkGetDeviceQueue2, m_instance );
    GET_VK_PROC( vkQueuePresentKHR, m_instance );
    GET_VK_PROC( vkQueueSubmit, m_instance );
    GET_VK_PROC( vkQueueSubmit2, m_instance );
    GET_VK_PROC( vkQueueWaitIdle, m_instance );
    GET_VK_PROC( vkCreateCommandPool, m_instance );
    GET_VK_PROC( vkDestroyCommandPool, m_instance );

    // VkFence
    GET_VK_PROC( vkCreateFence, m_instance );
    GET_VK_PROC( vkDestroyFence, m_instance );
    GET_VK_PROC( vkResetFences, m_instance );
    GET_VK_PROC( vkWaitForFences, m_instance );
    GET_VK_PROC( vkGetFenceStatus, m_instance );

    // VkSemaphore
    GET_VK_PROC( vkCreateSemaphore, m_instance );
    GET_VK_PROC( vkDestroySemaphore, m_instance );
    GET_VK_PROC( vkSignalSemaphore, m_instance );
    GET_VK_PROC( vkWaitSemaphores, m_instance );
    GET_VK_PROC( vkGetSemaphoreCounterValue, m_instance );

    // VkShaderModule
    GET_VK_PROC( vkCreateShaderModule, m_instance );
    GET_VK_PROC( vkDestroyShaderModule, m_instance );

    // VkSwapchainKHR
    GET_VK_PROC( vkCreateSwapchainKHR, m_instance );
    GET_VK_PROC( vkDestroySwapchainKHR, m_instance );
    GET_VK_PROC( vkGetSwapchainImagesKHR, m_instance );
    GET_VK_PROC( vkAcquireNextImage2KHR, m_instance );

    // VkDeviceMemory
    GET_VK_PROC( vkAllocateMemory, m_instance );
    GET_VK_PROC( vkFreeMemory, m_instance );
    GET_VK_PROC( vkMapMemory, m_instance );
    GET_VK_PROC( vkUnmapMemory, m_instance );
    GET_VK_PROC( vkFlushMappedMemoryRanges, m_instance );

    // VkBuffer
    GET_VK_PROC( vkCreateBuffer, m_instance );
    GET_VK_PROC( vkDestroyBuffer, m_instance );
    GET_VK_PROC( vkGetBufferMemoryRequirements, m_instance );
    GET_VK_PROC( vkBindBufferMemory, m_instance );
    GET_VK_PROC( vkCmdUpdateBuffer, m_instance );
    GET_VK_PROC( vkCmdFillBuffer, m_instance );

    // VkImage
    GET_VK_PROC( vkCreateImage,  m_instance );
    GET_VK_PROC( vkDestroyImage,  m_instance );
    GET_VK_PROC( vkGetImageMemoryRequirements,  m_instance );
    GET_VK_PROC( vkBindImageMemory,  m_instance );
    GET_VK_PROC( vkCmdClearColorImage,  m_instance );
    GET_VK_PROC( vkCmdClearDepthStencilImage,  m_instance );
    GET_VK_PROC( vkCmdResolveImage2,  m_instance );

    // VkImageView
    GET_VK_PROC( vkCreateImageView, m_instance );
    GET_VK_PROC( vkDestroyImageView, m_instance );

    // VkSampler
    GET_VK_PROC( vkCreateSampler, m_instance );
    GET_VK_PROC( vkDestroySampler, m_instance );

    // VkPipeline
    GET_VK_PROC( vkCreatePipelineLayout, m_instance );
    GET_VK_PROC( vkDestroyPipelineLayout, m_instance );
    GET_VK_PROC( vkCreateGraphicsPipelines, m_instance );
    GET_VK_PROC( vkDestroyPipeline, m_instance );

    GET_VK_PROC( vkCreatePipelineCache, m_instance );
    GET_VK_PROC( vkDestroyPipelineCache, m_instance );
    GET_VK_PROC( vkGetPipelineCacheData, m_instance );
    GET_VK_PROC( vkMergePipelineCaches, m_instance );

    // VkPipeline 
    GET_VK_PROC( vkCmdBindPipeline, m_instance );
    GET_VK_PROC( vkCmdSetViewport, m_instance );
    GET_VK_PROC( vkCmdSetScissor, m_instance );
    GET_VK_PROC( vkCmdSetLineWidth, m_instance );
    GET_VK_PROC( vkCmdSetDepthBias, m_instance );
    GET_VK_PROC( vkCmdSetBlendConstants, m_instance );
    GET_VK_PROC( vkCmdSetDepthBoundsTestEnable, m_instance );
    GET_VK_PROC( vkCmdSetDepthBounds, m_instance );
    GET_VK_PROC( vkCmdSetCullMode, m_instance );
    GET_VK_PROC( vkCmdSetStencilCompareMask, m_instance );
    GET_VK_PROC( vkCmdSetStencilWriteMask, m_instance );
    GET_VK_PROC( vkCmdSetStencilReference, m_instance );
    GET_VK_PROC( vkCmdPushConstants, m_instance );

    //
    GET_VK_PROC( vkCreateDescriptorSetLayout, m_instance );
    GET_VK_PROC( vkDestroyDescriptorSetLayout, m_instance );
    GET_VK_PROC( vkCreateDescriptorPool, m_instance );
    GET_VK_PROC( vkDestroyDescriptorPool, m_instance );
    GET_VK_PROC( vkResetDescriptorPool, m_instance );

    // VkDescriptorSet
    GET_VK_PROC( vkAllocateDescriptorSets, m_instance );
    GET_VK_PROC( vkFreeDescriptorSets, m_instance );
    GET_VK_PROC( vkCmdBindDescriptorSets, m_instance );
    GET_VK_PROC( vkUpdateDescriptorSets, m_instance );

    // VkRenderPass
    GET_VK_PROC( vkCreateRenderPass, m_instance );
    GET_VK_PROC( vkCreateRenderPass2, m_instance );
    GET_VK_PROC( vkDestroyRenderPass, m_instance );
    GET_VK_PROC( vkCmdBeginRenderPass2, m_instance );
    GET_VK_PROC( vkCmdEndRenderPass2, m_instance );

    // VkFramebuffer
    GET_VK_PROC( vkCreateFramebuffer, m_instance );
    GET_VK_PROC( vkDestroyFramebuffer, m_instance );

    // VkCommandBuffer
    GET_VK_PROC( vkAllocateCommandBuffers, m_instance );
    GET_VK_PROC( vkFreeCommandBuffers, m_instance );
    GET_VK_PROC( vkResetCommandBuffer, m_instance );
    GET_VK_PROC( vkBeginCommandBuffer, m_instance );
    GET_VK_PROC( vkEndCommandBuffer, m_instance );
    GET_VK_PROC( vkCmdExecuteCommands, m_instance );
    GET_VK_PROC( vkCmdBindIndexBuffer, m_instance );
    GET_VK_PROC( vkCmdBindVertexBuffers2, m_instance );
    GET_VK_PROC( vkCmdDraw, m_instance );
    GET_VK_PROC( vkCmdDrawIndexed, m_instance );
    GET_VK_PROC( vkCmdDrawIndirect, m_instance );
    GET_VK_PROC( vkCmdDrawIndexedIndirect, m_instance );
    GET_VK_PROC( vkCmdDispatch, m_instance );
    GET_VK_PROC( vkCmdDispatchIndirect, m_instance );
    GET_VK_PROC( vkCmdPipelineBarrier2, m_instance );

    // VkEvent
    GET_VK_PROC( vkCmdSetEvent2, m_instance );
    GET_VK_PROC( vkCmdResetEvent2, m_instance );
    GET_VK_PROC( vkCmdWaitEvents2, m_instance );

    // VkQueryPool
    GET_VK_PROC( vkCmdBeginQuery, m_instance );
    GET_VK_PROC( vkCmdResetQueryPool, m_instance );
    GET_VK_PROC( vkCmdEndQuery, m_instance );
    GET_VK_PROC( vkCmdCopyQueryPoolResults, m_instance );

    // VK_KHR_copy_commands2
    GET_VK_PROC( vkCmdBlitImage2, m_instance );
    GET_VK_PROC( vkCmdCopyBuffer2, m_instance );
    GET_VK_PROC( vkCmdCopyBufferToImage2, m_instance );
    GET_VK_PROC( vkCmdCopyImage2, m_instance );
    GET_VK_PROC( vkCmdCopyImageToBuffer2, m_instance );

    // VK_KHR_dynamic_rendering
    GET_VK_PROC( vkCmdBeginRendering, m_instance );
    GET_VK_PROC( vkCmdEndRendering, m_instance );
    GET_VK_PROC( vkCmdClearAttachments, m_instance );

    GET_VK_PROC( vkCreateDebugUtilsMessengerEXT,  m_instance );
    GET_VK_PROC( vkDestroyDebugUtilsMessengerEXT,  m_instance );
}