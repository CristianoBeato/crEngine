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

#include "Context.hpp"
#include "Core.hpp"

#define NO_SDL_VULKAN_TYPEDEFS
#include <SDL3/SDL_vulkan.h>

idCVar vk_deviceID( "vk_deviceID", "-1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, " -1 select the device with the highest score, +0 select the device by index" );

static VKAPI_ATTR void* VKAPI_CALL vkAllocation( void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope );
static VKAPI_ATTR void* VKAPI_CALL vkReallocation( void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope);
static VKAPI_ATTR void  VKAPI_CALL vkFree( void* pUserData, void* pMemory );
static VKAPI_ATTR void  VKAPI_CALL vkInternalAllocation( void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope);
static VKAPI_ATTR void  VKAPI_CALL vkInternalFree( void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope );

// our onw allocation structure using SDL_malloc
static const VkAllocationCallbacks allocationCallbacksLocal = 
{
    nullptr,
    vkAllocation,
    vkReallocation,
    vkFree,
    vkInternalAllocation,
    vkInternalFree
};

#if 1 // TODO: a define to easy control
const VkAllocationCallbacks* k_allocationCallbacks = &allocationCallbacksLocal;
#else 
const VkAllocationCallbacks* k_allocationCallbacks = nullptr;
#endif 

/*
==============
vkContext::vkContext
==============
*/
vkContext::vkContext( void ) : 
    m_hasDebugUtils( false ),
    m_portabilityEnumerationAvailable( false ),
    m_instance( nullptr ),
    m_debugCallback( nullptr ),
    m_surface( nullptr )
{
}

vkContext::~vkContext(void)
{
}

/*
==============
vkContext::Init
==============
*/
bool vkContext::StarUp( void )
{    
    VkResult                            result = VK_SUCCESS;
    uint32_t                            instanceExtensionCount = 0;
    uint32_t                            instanceLayerCount = 0;
    uint32_t                            physicalDeviceCount = 0;
    uint32_t                            device = 0;
    Uint32                              SDL3ExtensionCount = 0;
    const char* const*                  SDL3Extensions = nullptr;
	idList<const char*, TAG_VULKAN>     requiredInstanceExtensions;
    idList<const char*, TAG_VULKAN>     requiredDeviceExtensions;
    idList<const char*, TAG_VULKAN>     requestedInstanceLayers;
    VkApplicationInfo                   applicationInfo{};
	VkInstanceCreateInfo                instanceCI{};
    VkDebugUtilsMessengerCreateInfoEXT  debugUtilsMessengerCI{};
    
    common->Printf( "Initializing Vulkan instance.\n");

    InitInstanceProcs();
    requiredInstanceExtensions.Append( VK_KHR_SURFACE_EXTENSION_NAME );
    requiredInstanceExtensions.Append( VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );
    requiredInstanceExtensions.Append( VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME );

    /// enumerate instance extensions
    result = vkEnumerateInstanceExtensionProperties( nullptr, &instanceExtensionCount, nullptr);
    if ( !ResultCheck( result, "vkEnumerateInstanceExtensionProperties" ) )
        return false;

    m_availableInstanceExtensions.Resize( instanceExtensionCount );
	result = vkEnumerateInstanceExtensionProperties( nullptr, &instanceExtensionCount, m_availableInstanceExtensions.Ptr() );
    if ( !ResultCheck( result, "vkEnumerateInstanceExtensionProperties" ) )
        return false;

    /// instance enumerate layers 
    result = vkEnumerateInstanceLayerProperties( &instanceLayerCount, nullptr );
    if ( !ResultCheck( result, "vkEnumerateInstanceExtensionProperties" ) )
        return false;

    m_supportedInstanceLayers.Resize( instanceLayerCount );
    result = vkEnumerateInstanceLayerProperties( &instanceLayerCount, m_supportedInstanceLayers.Ptr());
    if ( !ResultCheck( result, "vkEnumerateInstanceExtensionProperties" ) )
        return false;

	if ( ExtensionAvailable( VK_EXT_DEBUG_UTILS_EXTENSION_NAME ) )
    {
        m_hasDebugUtils = true;
		requiredInstanceExtensions.Append( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    }
    
    if( ExtensionAvailable( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME ) )
    {
        m_portabilityEnumerationAvailable = true;
        requiredInstanceExtensions.Append( VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME );
    }

    // copy and enable system extensios
    SDL3Extensions = SDL_Vulkan_GetInstanceExtensions( &SDL3ExtensionCount );
    printf( "SDL3 VkExt Found:\n" );    
    for ( uint32_t i = 0; i < SDL3ExtensionCount; i++)
    {
        requiredInstanceExtensions.Append( SDL3Extensions[i] );
        printf( " - %s\n", SDL3Extensions[i] );
    }

    /// apend layers 
    if( LayersAvailable( "VK_LAYER_KHRONOS_validation" ) )
        requestedInstanceLayers.Append( "VK_LAYER_KHRONOS_validation" );

    ///
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = GAME_NAME;
    applicationInfo.pEngineName = ENGINE_VERSION;
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    ///
	instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCI.pApplicationInfo = &applicationInfo;
	instanceCI.enabledLayerCount = requestedInstanceLayers.Num();
	instanceCI.ppEnabledLayerNames = requestedInstanceLayers.Ptr();
	instanceCI.enabledExtensionCount = requiredInstanceExtensions.Num();
	instanceCI.ppEnabledExtensionNames = requiredInstanceExtensions.Ptr();

    ///
    debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCI.pNext = nullptr;
    debugUtilsMessengerCI.flags = 0;

    /// if available, enable the debug output utils
    if ( m_hasDebugUtils )
	{
		debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugUtilsMessengerCI.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugUtilsMessengerCI.pfnUserCallback = DebugCallback;
        debugUtilsMessengerCI.pUserData = this;
        instanceCI.pNext = &debugUtilsMessengerCI;
	}

	if ( m_portabilityEnumerationAvailable )
        instanceCI.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	// Create the Vulkan instance
	result = vkCreateInstance(&instanceCI, k_allocationCallbacks, &m_instance );
    if ( !ResultCheck( result, "vkCreateInstance" ) )
        return false;
    
    LoadVulkanProcs();

    if ( m_hasDebugUtils )
    {
		result = vkCreateDebugUtilsMessengerEXT( m_instance, &debugUtilsMessengerCI, k_allocationCallbacks, &m_debugCallback );
        if ( !ResultCheck( result, "vkCreateDebugUtilsMessengerEXT" ) )
            m_hasDebugUtils = false;
    }

    auto video = sys->GetVideoSystem();
    if ( !SDL_Vulkan_CreateSurface( static_cast<SDL_Window*>( video->WindowHandler() ), m_instance, k_allocationCallbacks, &m_surface ) )
    {
        printf( "Failed to create window surface %s\n", SDL_GetError() );
        return false;
    }

    result = vkEnumeratePhysicalDevices( m_instance, &physicalDeviceCount, nullptr);
    if ( !ResultCheck( result, "vkEnumeratePhysicalDevices" ) )
        return false;
    
    if ( physicalDeviceCount < 1 )
    {
        printf( "No vulkan suported physical device found." );
        return false;
    }
    
    m_availablePhysicalDevices.Resize( physicalDeviceCount );
    m_devices.Resize( physicalDeviceCount );
    result = vkEnumeratePhysicalDevices( m_instance, &physicalDeviceCount, m_availablePhysicalDevices.Ptr() );    
    if ( !ResultCheck( result, "vkEnumeratePhysicalDevices" ) )
        return false;
    
    for ( uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        m_devices[i].Init( i, m_availablePhysicalDevices[i], m_surface ); 
    }
    
    printf( " %i Vulkan suported devices found\n", physicalDeviceCount );
    
    auto FallBack = [physicalDeviceCount, &device]( idList<vkRenderDevice> &devices, idList<const char*> layers, const idList<const char*> extensions )
    {   
        for ( uint32_t i = 0; i < physicalDeviceCount; i++)
        {
            if( devices[i].StartUp( layers, extensions ) )
            {
                device = i;
                break;
            }
        }

        /// no suitable device initialized
        device = UINT32_MAX;
    };

    /// We require a device whit swap chain suport, an whit the extension initialized
    requiredDeviceExtensions.Append( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

    if( vk_deviceID.GetInteger() > -1 )
    {
        device = std::clamp( static_cast<uint32_t>( vk_deviceID.GetInteger() ), 0u,  physicalDeviceCount -1 );
        // TODO: need create a fallback, to try initalize another device
        if( !m_devices[device].StartUp( requestedInstanceLayers, requiredDeviceExtensions ) )
        {   
            //TODO:
            // common->FatalError( "Can't initialize selected device ID: %i, name: %s\n", device, m_devices[device].Name() );
            throw idException( "Can't initialize selected device ID: %i, name: %s\n", device, m_devices[device].Name() );

        }
    }
    else
    {
        uint32_t bestScore = 0;
        uint32_t bestDevice = 0;
        /// find the best
        for ( uint32_t i = 0; i < physicalDeviceCount; i++)
        {
            uint32_t score = m_devices[i].Score();
            if ( bestScore < score )
            {
                bestScore = score;
                bestDevice = i;
            }
        }

        device = bestDevice;

        // TODO: need create a fallback, to try initalize another device
        if( !m_devices[device].StartUp( requestedInstanceLayers, requiredDeviceExtensions ) )
        {
            // common->FatalError( "Can't initialize selected device ID: %i, name: %s\n", device, m_devices[device].Name() );
            throw idException( "Can't initialize selected device ID: %i, name: %s\n", device, m_devices[device].Name() );
        }
    }

    m_currentDevice = device;

    return true;
}

/*
==============
vkContext::Shutdown
==============
*/
void vkContext::Shutdown( void )
{
    if ( m_debugCallback != nullptr )
    {
        vkDestroyDebugUtilsMessengerEXT( m_instance, m_debugCallback, k_allocationCallbacks );
        m_debugCallback = nullptr;
    }
    
    if ( m_surface != nullptr )
    {
        vkDestroySurfaceKHR( m_instance, m_surface, k_allocationCallbacks );
        m_surface = nullptr;
    }
    
    if ( m_instance != nullptr )
    {
        vkDestroyInstance( m_instance, k_allocationCallbacks );
        m_instance = nullptr;
    }
}

/*
==============
vkContext::ExtensionAvailable
==============
*/
bool vkContext::ExtensionAvailable(const idStr &in_ext) const
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
vkContext::LayersAvailable
==============
*/
bool vkContext::LayersAvailable(const idStr &in_layer) const
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
==============================================
vkAllocation
==============================================
*/
void *VKAPI_ATTR vkAllocation( void * in_userData, size_t in_size, size_t in_alignment, VkSystemAllocationScope in_allocationScope )
{
    void* memptr = nullptr;
#if 1
    void* original = SDL_malloc( in_size + in_alignment - 1 + sizeof(void*) );
    uintptr_t aligned = ( reinterpret_cast<uintptr_t>( original ) + sizeof(void*) + in_alignment - 1) & ~( in_alignment - 1 );
    (reinterpret_cast<void**>(aligned))[-1] = original;
    memptr = reinterpret_cast<void*>( aligned );
    // check if memory is aligned 
    assert( memptr && ( (uintptr_t)memptr % in_alignment ) == 0 );
#else
    memptr = SDL_aligned_alloc( in_alignment, in_size );
#endif
    return memptr;
}

/*
==============================================
vkReallocation
==============================================
*/
void* VKAPI_CALL vkReallocation( void* in_userData, void* in_original, size_t in_size, size_t in_alignment, VkSystemAllocationScope in_allocationScope )
{
    void* memptr = nullptr;
#if 1
    void* original = static_cast<void**>( in_original )[-1];
    if( original != nullptr )
        original = SDL_realloc( original, in_size + in_alignment - 1 + sizeof(void*) );
    else
        original = SDL_malloc( in_size + in_alignment - 1 + sizeof(void*) );

    uintptr_t aligned = ( reinterpret_cast<uintptr_t>( original ) + sizeof(void*) + in_alignment - 1) & ~( in_alignment - 1 );
    (reinterpret_cast<void**>(aligned))[-1] = original;
    memptr = reinterpret_cast<void*>( aligned );

    // check if memory is aligned 
    assert( memptr && ( (uintptr_t)memptr % in_alignment ) == 0 );
#else
    memptr = SDL_aligned_alloc( in_alignment, in_size );
    SDL_memcpy( memptr, in_original, sizeof( in_original ) );
    SDL_free( in_original );    
#endif
    return memptr;
}

/*
==============================================
vkFree
==============================================
*/
void VKAPI_CALL vkFree( void* in_userData, void* in_memory )
{
#if 1
    if ( in_memory ) 
    {
        void* original = static_cast<void**>( in_memory )[-1];
        SDL_free( original );
    }
#else
    SDL_aligned_free( in_memory );
#endif
}

/*
==============================================
vkInternalAllocation
==============================================
*/
void VKAPI_CALL vkInternalAllocation( void* in_userData, size_t in_size, VkInternalAllocationType in_allocationType, VkSystemAllocationScope in_allocationScope )
{
    //vkCtx.allocedMemory += size;
    //std::printf("[Vulkan] Internal allocation of %zu bytes, total %i\n", in_size, 0 );
}

/*
==============================================
vkInternalFree
==============================================
*/
void VKAPI_CALL vkInternalFree( void* in_userData, size_t in_size, VkInternalAllocationType in_allocationType, VkSystemAllocationScope in_allocationScope )
{
    // vkCtx.allocedMemory -= size; 
    //std::printf("[Vulkan] Internal free of %zu bytes, total %i\n", in_size, 0 );
}
