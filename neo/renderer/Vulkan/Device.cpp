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

#include "Device.hpp"
#include "Core.hpp"

static const float k_PRIORITY = 1.0f;

#include <optional> // std::optional

idCVar vk_useComputQueues( "vk_useComputQueue", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 Enable vulkan find a compute queue, or 0 to use compute queue" );
idCVar vk_useTransferQueue( "vk_useTransferQueue", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "1 Enable vulkan transfer queues, or 0 to use just graphic queue" );

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
    if ( !ResultCheck( result, "vkCreateCommandPool" ) )
        return false;

    VkSemaphoreTypeCreateInfo typeInfo{};
    typeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    typeInfo.initialValue = 0;

    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semInfo.pNext = &typeInfo;

    result = vkCreateSemaphore( m_device, &semInfo, k_allocationCallbacks, &m_semaphore );
    if ( !ResultCheck( result, "vkCreateSemaphore" ) )
        return false;

    return true;
    
}

bool vkDeviceQueue::WaitSemaphore( const uint64_t in_value, const uint64_t in_timeout )
{
    VkSemaphoreWaitInfo waitInfo{};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.pNext = nullptr;
    waitInfo.flags = 0;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &m_semaphore;
    waitInfo.pValues = &in_value;

    auto result = vkWaitSemaphores( m_device, &waitInfo, UINT64_MAX );
    if ( result == VK_TIMEOUT )
    {
        // todo: say somethin
    }

    return true;
}

uint64_t vkDeviceQueue::IncrementTimeline(void)
{
    return m_timeline.fetch_add( 1 );
}

/*
==============
vkRenderDevice::vkRenderDevice
==============
*/
vkRenderDevice::vkRenderDevice( void ) : 
    m_physical( nullptr ),
    m_logic( nullptr ),
    m_present( nullptr ),
    m_graphic( nullptr ),
    m_compute( nullptr ),
    m_transfer( nullptr )
{
}

bool vkRenderDevice::Init( const uint32_t in_ID, const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface )
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
    if( !ResultCheck( result, "vkEnumerateDeviceExtensionProperties" ) )
        return false;

    m_deviceExtensions.Resize( deviceExtensionCount);
	result = vkEnumerateDeviceExtensionProperties( m_physical, nullptr, &deviceExtensionCount, m_deviceExtensions.Ptr() );
    if( !ResultCheck( result, "vkEnumerateDeviceExtensionProperties" ) )
        return false;
 
    // query device suported surface formats
    result = vkGetPhysicalDeviceSurfaceFormats2KHR( m_physical, &deviceSurfaceInfo, &formatCount, nullptr );
    if( !ResultCheck( result, "vkGetPhysicalDeviceSurfaceFormats2KHR" ) )
        return false;
 
    m_surfaceFormats.Resize( formatCount );
    for ( i = 0; i < formatCount; i++)
    {
        m_surfaceFormats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
        m_surfaceFormats[i].pNext = nullptr;
    }
    
    result = vkGetPhysicalDeviceSurfaceFormats2KHR( m_physical, &deviceSurfaceInfo, &formatCount, m_surfaceFormats.Ptr() );
    if( !ResultCheck( result, "vkGetPhysicalDeviceSurfaceFormats2KHR" ) )
        return false;
 
    // query device suported surface presenting mode
    result = vkGetPhysicalDeviceSurfacePresentModesKHR( m_physical, in_surface, &presentModeCount, nullptr );
    if( !ResultCheck( result, "vkGetPhysicalDeviceSurfacePresentModesKHR" ) )
        return false;
 
    m_presentModes.Resize( presentModeCount );
    result = vkGetPhysicalDeviceSurfacePresentModesKHR( m_physical, in_surface, &presentModeCount, m_presentModes.Ptr() );
    if( !ResultCheck( result, "vkGetPhysicalDeviceSurfacePresentModesKHR" ) )
        return false;

    // query device surface  capabilities
    m_surfaceCapabilities.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
    m_surfaceCapabilities.pNext = nullptr;
    result = vkGetPhysicalDeviceSurfaceCapabilities2KHR( m_physical, &deviceSurfaceInfo, &m_surfaceCapabilities ); 
    if( !ResultCheck( result, "vkGetPhysicalDeviceSurfaceCapabilities2KHR" ) )
        return false;

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
    

    return true;
}

/*
==============
vkRenderDevice::StartUp
==============
*/
bool vkRenderDevice::StartUp( const idList<const char*> &in_layers, const idList<const char *> &in_enabledExtensions )
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
    for ( uint32_t i = 0; i < in_enabledExtensions.Num(); i++)
    {
        auto ext = in_enabledExtensions[i];
        if( !ExtensionAvailable( ext ) )
        {
            return false;
        }
    }

    // Enable extensions
    deviceCI.enabledExtensionCount = in_enabledExtensions.Num();
    deviceCI.ppEnabledExtensionNames = in_enabledExtensions.Ptr();
    deviceCI.enabledLayerCount = in_layers.Num();
    deviceCI.ppEnabledLayerNames = in_layers.Ptr();

    // create the logic device handle
    result = vkCreateDevice( m_physical, &deviceCI, k_allocationCallbacks, &m_logic );
    if( !ResultCheck( result, "vkCreateDevice" ) )
        return false;

    // Initialize present and graphyc queue
    // this are required queues
    if ( !m_present || !m_graphic )
    {
        if( !m_present )
            common->Error( "Missing Present Queue\n" );
        if( !m_graphic )
            common->Error( "Missing Present Queue\n" );
        return false;
    }
    
    m_present->Init( m_logic );
    m_graphic->Init( m_logic );

    // if we found a compute queue, initialize
    // if no compute queue is found, we will use the graphic queue
    if ( m_compute != nullptr )
        m_compute->Init( m_logic );
    else
        common->Warning( "No compute queue found, using graphic!\n" );

    // If a transfer queue is found, initialize
    // if not, use a graphic queue copy
    if( m_transfer != nullptr )
        m_transfer->Init( m_logic );
    else
        common->Warning( "No transfer queue found, using graphic!\n" );

    common->Printf( " -> succes\n" );

    return true;
}

/*
==============
vkRenderDevice::ShutDown
==============
*/
void vkRenderDevice::ShutDown(void)
{
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
vkRenderDevice::DeviceScore
==============
*/
uint32_t vkRenderDevice::Score(void) const
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

bool vkRenderDevice::ExtensionAvailable(const idStr &in_ext) const
{
    for ( uint32_t i = 0; i < m_deviceExtensions.Num(); i++)
    {
        idStr ext = m_deviceExtensions[i].extensionName;
        if( ext == in_ext )
            return true;
    }
    
    printf( "%s not found!\n", in_ext.c_str() );
    return false;
}

/*
==============
vkRenderDevice::FindMemoryType
==============
*/
uint32_t vkRenderDevice::FindMemoryType(const uint32_t in_filter, const VkMemoryPropertyFlags properties) const
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

const uint32_t vkRenderDevice::ShaderStorageBufferAlignament(void) const
{
    return m_propertiesv10.properties.limits.minStorageBufferOffsetAlignment;
}

const bool vkRenderDevice::SupportedFormat(const VkFormat in_format, const VkColorSpaceKHR in_colorSpace) const
{
    for ( uint32_t i = 0; i < m_surfaceFormats.Num(); i++)
    {
        auto format = m_surfaceFormats[i].surfaceFormat;
        if ( format.format == in_format && format.colorSpace == in_colorSpace )
            return true;       
    }

    return false;
}

const bool vkRenderDevice::SupportedPresentMode( const VkPresentModeKHR in_mode )
{
    for ( uint32_t i = 0; i < m_presentModes.Num(); i++)
    {
        auto mode = m_presentModes[i];
        if ( mode == in_mode )
            return true;        
    }
    
    return false;
}

const VkSurfaceFormatKHR vkRenderDevice::GetPresentFormat(const uint32_t in_formatID)
{
    // todo get a better format list
    if ( in_formatID >= m_surfaceFormats.Num() )
        return m_surfaceFormats[0].surfaceFormat;

    return m_surfaceFormats[in_formatID].surfaceFormat;
}

/*
==============
vkRenderDevice::SupportedDepthFormat
==============
*/
const bool vkRenderDevice::SupportedDepthFormat(VkFormat *depthFormat) const
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
        auto format = formatList[i];
		vkGetPhysicalDeviceFormatProperties2( m_physical, format, &formatProps );
		if (formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthFormat = format;
			return true;
		}
	}

	return false;
}

/*
==============
vkRenderDevice::SupportedDepthStencilFormat
==============
*/
const bool vkRenderDevice::SupportedDepthStencilFormat(VkFormat *depthStencilFormat) const
{
    VkFormat formatList[3] = 
    {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
	};

	for (auto& format : formatList)
	{
		VkFormatProperties2 formatProps;
		vkGetPhysicalDeviceFormatProperties2( m_physical, format, &formatProps);
		if (formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthStencilFormat = format;
			return true;
		}
	}

	return false;
}

/*
==============
vkRenderDevice::FormatIsFilterable
==============
*/
const bool vkRenderDevice::FormatIsFilterable(const VkFormat in_format, const VkImageTiling tiling) const
{
    VkFormatProperties2 formatProps;
    vkGetPhysicalDeviceFormatProperties2( m_physical, in_format, &formatProps);

	if (tiling == VK_IMAGE_TILING_OPTIMAL)
		return formatProps.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	if (tiling == VK_IMAGE_TILING_LINEAR)
		return formatProps.formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	return false;
}

/*
==============
vkRenderDevice::MaxSamples
==============
*/
uint32_t vkRenderDevice::MaxSamples(void) const
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


/*
==============
vkRenderDevice::SelectDeviceQueues
@CristianoBeato: holly cow hard coded as fuck, need found a new algoritm, or easy way to do this 
==============
*/
void vkRenderDevice::SelectDeviceQueues( idList<VkDeviceQueueCreateInfo> &in_queueList )
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
