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

#include "precompiled.h"
#include "renderer/renderer_common.h"
#include "Vulkan/Vulkan.hpp"
#include "vkUtils.hpp"

// to don't suck whit game console  
#include <iostream> // std::cerr 

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

#define STR(r) case VK_ ##r: return idStr( #r )

idStr VulkanErrorString( VkResult errorCode )
{
    switch (errorCode)
    {
	    STR( NOT_READY ); 
        STR( TIMEOUT ); 
        STR( EVENT_SET ); 
        STR( EVENT_RESET ); 
        STR( INCOMPLETE ); 
        STR( ERROR_OUT_OF_HOST_MEMORY ); 
        STR( ERROR_OUT_OF_DEVICE_MEMORY ); 
        STR( ERROR_INITIALIZATION_FAILED ); 
        STR( ERROR_DEVICE_LOST ); 
        STR( ERROR_MEMORY_MAP_FAILED ); 
        STR( ERROR_LAYER_NOT_PRESENT ); 
        STR( ERROR_EXTENSION_NOT_PRESENT ); 
        STR( ERROR_FEATURE_NOT_PRESENT ); 
        STR( ERROR_INCOMPATIBLE_DRIVER ); 
        STR( ERROR_TOO_MANY_OBJECTS ); 
        STR( ERROR_FORMAT_NOT_SUPPORTED ); 
        STR( ERROR_FRAGMENTED_POOL ); 
        STR( ERROR_UNKNOWN ); 
        STR( ERROR_OUT_OF_POOL_MEMORY ); 
        STR( ERROR_INVALID_EXTERNAL_HANDLE ); 
        STR( ERROR_FRAGMENTATION ); 
        STR( ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS ); 
        STR( PIPELINE_COMPILE_REQUIRED ); 
        STR( ERROR_SURFACE_LOST_KHR ); 
        STR( ERROR_NATIVE_WINDOW_IN_USE_KHR ); 
        STR( SUBOPTIMAL_KHR ); 
        STR( ERROR_OUT_OF_DATE_KHR ); 
        STR( ERROR_INCOMPATIBLE_DISPLAY_KHR ); 
        STR( ERROR_VALIDATION_FAILED_EXT ); 
        STR( ERROR_INVALID_SHADER_NV ); 
    #ifdef VK_ENABLE_BETA_EXTENSIONS
        STR( ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR ); 
        STR( ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR ); 
        STR( ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR ); 
        STR( ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR ); 
        STR( ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR ); 
        STR( ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR ); 
    #endif
        STR( ERROR_NOT_PERMITTED_KHR ); 
        STR( ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT ); 
        STR( THREAD_IDLE_KHR ); 
        STR( THREAD_DONE_KHR ); 
        STR( OPERATION_DEFERRED_KHR ); 
        STR( OPERATION_NOT_DEFERRED_KHR ); 
        STR( ERROR_COMPRESSION_EXHAUSTED_EXT );
	default:
		return "UNKNOWN_ERROR";
    }
}
#undef STR
