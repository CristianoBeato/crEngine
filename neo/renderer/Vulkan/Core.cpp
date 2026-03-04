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
#include <iostream> // std::cerr 

#define NO_SDL_VULKAN_TYPEDEFS
#include <SDL3/SDL_vulkan.h>

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
PFN_vkGetPhysicalDeviceFormatProperties2         vkGetPhysicalDeviceFormatProperties2;


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

//
PFN_vkCmdBindPipeline                            vkCmdBindPipeline = nullptr;
PFN_vkCmdSetViewport                             vkCmdSetViewport = nullptr;
PFN_vkCmdSetScissor                              vkCmdSetScissor = nullptr;
PFN_vkCmdSetLineWidth                            vkCmdSetLineWidth = nullptr;
PFN_vkCmdSetDepthBias                            vkCmdSetDepthBias = nullptr;
PFN_vkCmdSetBlendConstants                       vkCmdSetBlendConstants = nullptr;
PFN_vkCmdSetDepthBoundsTestEnable                vkCmdSetDepthBoundsTestEnable = nullptr;
PFN_vkCmdSetDepthBounds                          vkCmdSetDepthBounds = nullptr;
PFN_vkCmdSetCullMode                             vkCmdSetCullMode = nullptr;
PFN_vkCmdSetStencilCompareMask                   vkCmdSetStencilCompareMask = nullptr;
PFN_vkCmdSetStencilWriteMask                     vkCmdSetStencilWriteMask = nullptr;
PFN_vkCmdSetStencilReference                     vkCmdSetStencilReference = nullptr;
PFN_vkCmdPushConstants                           vkCmdPushConstants = nullptr;

//
PFN_vkCreateDescriptorSetLayout                  vkCreateDescriptorSetLayout = nullptr;
PFN_vkDestroyDescriptorSetLayout                 vkDestroyDescriptorSetLayout = nullptr;
PFN_vkCreateDescriptorPool                       vkCreateDescriptorPool = nullptr;
PFN_vkDestroyDescriptorPool                      vkDestroyDescriptorPool = nullptr;
PFN_vkResetDescriptorPool                        vkResetDescriptorPool = nullptr;

// VkDescriptorSet
PFN_vkAllocateDescriptorSets                     vkAllocateDescriptorSets = nullptr;
PFN_vkFreeDescriptorSets                         vkFreeDescriptorSets = nullptr;
PFN_vkUpdateDescriptorSets                       vkUpdateDescriptorSets = nullptr;

// VkEvent
PFN_vkCmdSetEvent2                               vkCmdSetEvent2 = nullptr;
PFN_vkCmdResetEvent2                             vkCmdResetEvent2 = nullptr;
PFN_vkCmdWaitEvents2                             vkCmdWaitEvents2 = nullptr;

// VkQueryPool
PFN_vkCmdBeginQuery                              vkCmdBeginQuery = nullptr;
PFN_vkCmdResetQueryPool                          vkCmdResetQueryPool = nullptr;
PFN_vkCmdEndQuery                                vkCmdEndQuery = nullptr;
PFN_vkCmdCopyQueryPoolResults                    vkCmdCopyQueryPoolResults = nullptr;

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
PFN_vkCreateDebugUtilsMessengerEXT               vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT              vkDestroyDebugUtilsMessengerEXT = nullptr;

template< typename _type_ >
inline static void GetVkProc( _type_ &in_proc, const char* in_pName, VkInstance in_instance )
{
    in_proc = reinterpret_cast<_type_>( vkGetInstanceProcAddr( in_instance, in_pName ) );
}

#define GET_VK_PROC( P, I ) GetVkProc( P, #P, I )

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

    return VK_FALSE;
}

void vkContext::InitInstanceProcs( void )
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

void vkContext::LoadVulkanProcs( void )
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

void VkImageStateTransition( vkImageHandle_t* in_image,
                                const VkCommandBuffer in_commandBuffer, 
                                const VkImageLayout in_newLayout, 
                                const VkPipelineStageFlags2 in_stageMask,
                                const VkAccessFlags2 in_accessMask )
{
    assert( in_image != nullptr );

    VkImageMemoryBarrier2 destinationBarrier{};
    destinationBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    destinationBarrier.pNext = nullptr;
    destinationBarrier.srcStageMask = in_image->stage;
    destinationBarrier.srcAccessMask = in_image->access;
    destinationBarrier.dstStageMask = in_stageMask;
    destinationBarrier.dstAccessMask = in_accessMask;
    destinationBarrier.oldLayout = in_image->layout;
    destinationBarrier.newLayout = in_newLayout;
    destinationBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //m_graphicQueue->Family();
    destinationBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; //m_presentQueue->Family();
    destinationBarrier.image = in_image->image;
    destinationBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    destinationBarrier.subresourceRange.baseMipLevel = 0;
    destinationBarrier.subresourceRange.levelCount = 1;
    destinationBarrier.subresourceRange.baseArrayLayer = 0;
    destinationBarrier.subresourceRange.layerCount = 1;
        
    /// perform a state transition to destination
    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;
    dependencyInfo.dependencyFlags = 0;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.pMemoryBarriers = nullptr;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.pBufferMemoryBarriers = nullptr;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &destinationBarrier;
    vkCmdPipelineBarrier2( in_commandBuffer, &dependencyInfo );

    /// keep track of texture state
    in_image->layout = in_newLayout;
    in_image->stage = in_stageMask;
    in_image->access = in_accessMask;
}