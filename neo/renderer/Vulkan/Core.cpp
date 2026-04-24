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

#include "Core.hpp"

// to don't suck whit game console  
// #include <iostream> // std::cerr 

PFN_vkEnumerateInstanceLayerProperties           vkEnumerateInstanceLayerProperties = nullptr;
PFN_vkEnumerateInstanceExtensionProperties       vkEnumerateInstanceExtensionProperties = nullptr;

// VkInstance
PFN_vkCreateInstance                             vkCreateInstance = nullptr;
PFN_vkDestroyInstance                            vkDestroyInstance = nullptr;
PFN_vkGetInstanceProcAddr                        vkGetInstanceProcAddr = nullptr;
PFN_vkEnumeratePhysicalDevices                   vkEnumeratePhysicalDevices = nullptr;

PFN_vkDestroySurfaceKHR                          vkDestroySurfaceKHR = nullptr;

// VkPhysicalDevice
PFN_vkGetPhysicalDeviceMemoryProperties2         vkGetPhysicalDeviceMemoryProperties2 = nullptr;
PFN_vkGetPhysicalDeviceProperties2               vkGetPhysicalDeviceProperties2 = nullptr;
PFN_vkGetPhysicalDeviceFeatures2                 vkGetPhysicalDeviceFeatures2 = nullptr;
PFN_vkEnumerateDeviceExtensionProperties         vkEnumerateDeviceExtensionProperties = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2    vkGetPhysicalDeviceQueueFamilyProperties2 = nullptr;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR    vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR   vkGetPhysicalDeviceSurfaceCapabilities2KHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormats2KHR        vkGetPhysicalDeviceSurfaceFormats2KHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR         vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
PFN_vkGetPhysicalDeviceFormatProperties2         vkGetPhysicalDeviceFormatProperties2 = nullptr;


// VkDevice
PFN_vkCreateDevice                               vkCreateDevice = nullptr;
PFN_vkDestroyDevice                              vkDestroyDevice = nullptr;
PFN_vkDeviceWaitIdle                             vkDeviceWaitIdle = nullptr;
PFN_vkGetDeviceProcAddr                          vkGetDeviceProcAddr = nullptr;

// VkQueue
PFN_vkGetDeviceQueue                             vkGetDeviceQueue = nullptr;
PFN_vkGetDeviceQueue2                            vkGetDeviceQueue2 = nullptr;
PFN_vkQueuePresentKHR                            vkQueuePresentKHR = nullptr;
PFN_vkQueueSubmit                                vkQueueSubmit = nullptr;
PFN_vkQueueSubmit2                               vkQueueSubmit2 = nullptr;
PFN_vkQueueWaitIdle                              vkQueueWaitIdle = nullptr;
PFN_vkCreateCommandPool                          vkCreateCommandPool = nullptr;
PFN_vkDestroyCommandPool                         vkDestroyCommandPool = nullptr;

// VkFence
PFN_vkCreateFence                                vkCreateFence = nullptr;
PFN_vkDestroyFence                               vkDestroyFence = nullptr;
PFN_vkResetFences                                vkResetFences = nullptr;
PFN_vkWaitForFences                              vkWaitForFences = nullptr;
PFN_vkGetFenceStatus                             vkGetFenceStatus = nullptr;

// VkSemaphore
PFN_vkCreateSemaphore                            vkCreateSemaphore = nullptr;
PFN_vkDestroySemaphore                           vkDestroySemaphore = nullptr;
PFN_vkSignalSemaphore                            vkSignalSemaphore = nullptr;
PFN_vkWaitSemaphores                             vkWaitSemaphores = nullptr;
PFN_vkGetSemaphoreCounterValue                   vkGetSemaphoreCounterValue = nullptr;

// VkShaderModule
PFN_vkCreateShaderModule                         vkCreateShaderModule = nullptr;
PFN_vkDestroyShaderModule                        vkDestroyShaderModule = nullptr;

// VkSwapchainKHR
PFN_vkCreateSwapchainKHR                         vkCreateSwapchainKHR = nullptr;
PFN_vkDestroySwapchainKHR                        vkDestroySwapchainKHR = nullptr;
PFN_vkGetSwapchainImagesKHR                      vkGetSwapchainImagesKHR = nullptr;
PFN_vkAcquireNextImage2KHR                       vkAcquireNextImage2KHR = nullptr;

// VkDeviceMemory
PFN_vkAllocateMemory                             vkAllocateMemory = nullptr;
PFN_vkFreeMemory                                 vkFreeMemory = nullptr;
PFN_vkMapMemory                                  vkMapMemory = nullptr;
PFN_vkUnmapMemory                                vkUnmapMemory = nullptr;
PFN_vkFlushMappedMemoryRanges                    vkFlushMappedMemoryRanges = nullptr;

// VkBuffer
PFN_vkCreateBuffer                               vkCreateBuffer = nullptr;
PFN_vkDestroyBuffer                              vkDestroyBuffer = nullptr;
PFN_vkGetBufferMemoryRequirements                vkGetBufferMemoryRequirements = nullptr;
PFN_vkBindBufferMemory                           vkBindBufferMemory = nullptr;
PFN_vkCmdUpdateBuffer                            vkCmdUpdateBuffer = nullptr;
PFN_vkCmdFillBuffer                              vkCmdFillBuffer = nullptr;

// VkImage
PFN_vkCreateImage                                vkCreateImage = nullptr;
PFN_vkDestroyImage                               vkDestroyImage = nullptr;
PFN_vkGetImageMemoryRequirements                 vkGetImageMemoryRequirements = nullptr;
PFN_vkBindImageMemory                            vkBindImageMemory = nullptr;
PFN_vkCmdClearColorImage                         vkCmdClearColorImage = nullptr;
PFN_vkCmdClearDepthStencilImage                  vkCmdClearDepthStencilImage = nullptr;
PFN_vkCmdResolveImage2                           vkCmdResolveImage2 = nullptr;

// VkImageView
PFN_vkCreateImageView                            vkCreateImageView = nullptr;
PFN_vkDestroyImageView                           vkDestroyImageView = nullptr;

// VkSampler
PFN_vkCreateSampler                              vkCreateSampler = nullptr;
PFN_vkDestroySampler                             vkDestroySampler = nullptr;

// VkPipeline 
PFN_vkCreatePipelineLayout                      vkCreatePipelineLayout = nullptr;
PFN_vkDestroyPipelineLayout                     vkDestroyPipelineLayout = nullptr;
PFN_vkCreateGraphicsPipelines                   vkCreateGraphicsPipelines = nullptr;
PFN_vkDestroyPipeline                           vkDestroyPipeline = nullptr;                  

// VkRenderPass
PFN_vkCreateRenderPass                           vkCreateRenderPass = nullptr;
PFN_vkCreateRenderPass2                          vkCreateRenderPass2 = nullptr;
PFN_vkDestroyRenderPass                          vkDestroyRenderPass = nullptr;
PFN_vkCmdBeginRenderPass2                        vkCmdBeginRenderPass2 = nullptr;
PFN_vkCmdEndRenderPass2                          vkCmdEndRenderPass2 = nullptr;

// VkFramebuffer
PFN_vkCreateFramebuffer                          vkCreateFramebuffer = nullptr;
PFN_vkDestroyFramebuffer                         vkDestroyFramebuffer = nullptr;

// VkCommandBuffer
PFN_vkAllocateCommandBuffers                     vkAllocateCommandBuffers = nullptr;
PFN_vkFreeCommandBuffers                         vkFreeCommandBuffers = nullptr;
PFN_vkResetCommandBuffer                         vkResetCommandBuffer = nullptr;
PFN_vkBeginCommandBuffer                         vkBeginCommandBuffer = nullptr;
PFN_vkEndCommandBuffer                           vkEndCommandBuffer = nullptr;
PFN_vkCmdExecuteCommands                         vkCmdExecuteCommands = nullptr;
PFN_vkCmdBindIndexBuffer                         vkCmdBindIndexBuffer = nullptr;
PFN_vkCmdBindVertexBuffers2                      vkCmdBindVertexBuffers2 = nullptr;
PFN_vkCmdDraw                                    vkCmdDraw = nullptr;
PFN_vkCmdDrawIndexed                             vkCmdDrawIndexed = nullptr;
PFN_vkCmdDrawIndirect                            vkCmdDrawIndirect = nullptr;
PFN_vkCmdDrawIndexedIndirect                     vkCmdDrawIndexedIndirect = nullptr;
PFN_vkCmdDispatch                                vkCmdDispatch = nullptr;
PFN_vkCmdDispatchIndirect                        vkCmdDispatchIndirect = nullptr;
PFN_vkCmdPipelineBarrier2                        vkCmdPipelineBarrier2 = nullptr;

// pipeline cache
PFN_vkCreatePipelineCache                       vkCreatePipelineCache = nullptr;
PFN_vkDestroyPipelineCache                      vkDestroyPipelineCache = nullptr;
PFN_vkGetPipelineCacheData                      vkGetPipelineCacheData = nullptr;
PFN_vkMergePipelineCaches                       vkMergePipelineCaches = nullptr;

//
PFN_vkCmdBindPipeline                           vkCmdBindPipeline = nullptr;
PFN_vkCmdSetViewport                            vkCmdSetViewport = nullptr;
PFN_vkCmdSetScissor                             vkCmdSetScissor = nullptr;
PFN_vkCmdSetLineWidth                           vkCmdSetLineWidth = nullptr;
PFN_vkCmdSetPrimitiveTopology                   vkCmdSetPrimitiveTopology = nullptr;
PFN_vkCmdSetBlendConstants                      vkCmdSetBlendConstants = nullptr;
PFN_vkCmdSetDepthBoundsTestEnable               vkCmdSetDepthBoundsTestEnable = nullptr;
PFN_vkCmdSetDepthBounds                         vkCmdSetDepthBounds = nullptr;
PFN_vkCmdSetCullMode                            vkCmdSetCullMode = nullptr;
PFN_vkCmdSetStencilCompareMask                  vkCmdSetStencilCompareMask = nullptr;
PFN_vkCmdSetStencilWriteMask                    vkCmdSetStencilWriteMask = nullptr;
PFN_vkCmdSetStencilReference                    vkCmdSetStencilReference = nullptr;
PFN_vkCmdPushConstants                          vkCmdPushConstants = nullptr;

PFN_vkCmdSetDepthTestEnable                     vkCmdSetDepthTestEnable = nullptr;

PFN_vkCmdSetDepthBiasEnable                     vkCmdSetDepthBiasEnable = nullptr;
PFN_vkCmdSetDepthBias                           vkCmdSetDepthBias = nullptr;

//
PFN_vkCreateDescriptorSetLayout                  vkCreateDescriptorSetLayout = nullptr;
PFN_vkDestroyDescriptorSetLayout                 vkDestroyDescriptorSetLayout = nullptr;
PFN_vkCreateDescriptorPool                       vkCreateDescriptorPool = nullptr;
PFN_vkDestroyDescriptorPool                      vkDestroyDescriptorPool = nullptr;
PFN_vkResetDescriptorPool                        vkResetDescriptorPool = nullptr;

// VkDescriptorSet
PFN_vkAllocateDescriptorSets                    vkAllocateDescriptorSets = nullptr;
PFN_vkFreeDescriptorSets                        vkFreeDescriptorSets = nullptr;
PFN_vkCmdBindDescriptorSets                     vkCmdBindDescriptorSets = nullptr;
PFN_vkUpdateDescriptorSets                      vkUpdateDescriptorSets = nullptr;

// VkEvent
PFN_vkCmdSetEvent2                               vkCmdSetEvent2 = nullptr;
PFN_vkCmdResetEvent2                             vkCmdResetEvent2 = nullptr;
PFN_vkCmdWaitEvents2                             vkCmdWaitEvents2 = nullptr;

// VkQueryPool
PFN_vkCreateQueryPool                           vkCreateQueryPool = nullptr;
PFN_vkDestroyQueryPool                          vkDestroyQueryPool = nullptr;
PFN_vkResetQueryPool                            vkResetQueryPool = nullptr;
PFN_vkGetQueryPoolResults                       vkGetQueryPoolResults = nullptr;

PFN_vkCmdBeginQuery                             vkCmdBeginQuery = nullptr;
PFN_vkCmdResetQueryPool                         vkCmdResetQueryPool = nullptr;
PFN_vkCmdEndQuery                               vkCmdEndQuery = nullptr;
PFN_vkCmdCopyQueryPoolResults                   vkCmdCopyQueryPoolResults = nullptr;
PFN_vkCmdWriteTimestamp                         vkCmdWriteTimestamp = nullptr;

// VK_KHR_copy_commands2
PFN_vkCmdBlitImage2                             vkCmdBlitImage2 = nullptr;
PFN_vkCmdCopyBuffer2                            vkCmdCopyBuffer2 = nullptr;
PFN_vkCmdCopyBufferToImage2                     vkCmdCopyBufferToImage2 = nullptr;
PFN_vkCmdCopyImage2                             vkCmdCopyImage2 = nullptr;
PFN_vkCmdCopyImageToBuffer2                     vkCmdCopyImageToBuffer2 = nullptr;

// VK_KHR_dynamic_rendering
PFN_vkCmdBeginRendering                         vkCmdBeginRendering = nullptr;
PFN_vkCmdEndRendering                           vkCmdEndRendering = nullptr;
PFN_vkCmdClearAttachments                       vkCmdClearAttachments = nullptr;

// VK_EXT_debug_utils
PFN_vkCreateDebugUtilsMessengerEXT              vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT             vkDestroyDebugUtilsMessengerEXT = nullptr;

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
==============================================
crvkContext::DebugCallback
==============================================
*/
VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT in_messageSeverity, VkDebugUtilsMessageTypeFlagsEXT in_messageType, const VkDebugUtilsMessengerCallbackDataEXT* in_callbackData, void* pUserData )
{
    //    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    //        return VK_FALSE;

    const char* color = "";
    const char* severityStr = "";
    switch ( in_messageSeverity) 
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: 
        color = "\033[90m"; // GRAY 
        severityStr = "VERBOSE"; 
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    
        color = "\033[36m"; 
        severityStr = "INFO";    
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: 
        color = "\033[33m"; 
        severityStr = "WARNING"; 
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   
        color = "\033[31m"; // RED
        severityStr = "ERROR";   
        break;
    default:
        break;
    }

    const char* typeStr = "";
    switch ( in_messageType ) 
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:     
            typeStr = "GENERAL"; 
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:  
            typeStr = "VALIDATION"; 
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: 
            typeStr = "PERFORMANCE"; 
            break;
    }

#if 0
    std::cerr << color << "[VULKAN][" << severityStr << "][" << typeStr << "] " << in_callbackData->pMessage << "\033[0m" << std::endl;

    if ( in_callbackData->objectCount > 0) 
    {
        std::cerr << "  Objects involved:" << std::endl;
        for (uint32_t i = 0; i < in_callbackData->objectCount; ++i) 
        {
            std::cerr << "    - [" << in_callbackData->pObjects[i].objectType << "] "
                      << ( in_callbackData->pObjects[i].pObjectName ? in_callbackData->pObjects[i].pObjectName : "Unnamed")
                      << std::endl;
        }
    }
#endif
    return VK_FALSE;
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
