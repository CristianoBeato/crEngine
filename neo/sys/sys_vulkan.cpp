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

#define NO_SDL_VULKAN_TYPEDEFS
#include <SDL3/SDL_vulkan.h>

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
crVulkanRenderDevice::crVulkanRenderDevice
==============
*/
crVulkanRenderDevice::crVulkanRenderDevice( void )
{
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

uint32_t crVulkanAPI::GetDevices( crRenderDevice **m_deviceArray )
{
    /// just get the device count 
    if( m_deviceArray == nullptr )
        return m_availablePhysicalDevices.Num();

    for ( uint32_t i = 0; i < m_availablePhysicalDevices.Num(); i++)
    {
        auto device 
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