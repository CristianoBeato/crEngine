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

#ifndef __VK_UTILS_HPP__
#define __VK_UTILS_HPP__

extern VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT in_severity, VkDebugUtilsMessageTypeFlagsEXT in_types, const VkDebugUtilsMessengerCallbackDataEXT* in_data, void *in_user );
extern idStr VulkanErrorString( VkResult errorCode );
inline void ReturnError( const VkResult in_errorCode, const char* in_fn, const char* in_file, const int in_line )
{
    auto error = VulkanErrorString( in_errorCode );
    common->Error( "VkResult is \"%s\" in \"%s\" at line %i\n", VulkanErrorString( in_errorCode ), in_file, in_line );
}

inline bool ResultCheck( const VkResult in_result, const char* in_fnName )
{
    /// no erro just pass
    if( in_result == VK_SUCCESS )
        return true;
    
    /// error found, what
    auto err = VulkanErrorString( in_result );
    common->Error( " Vulkan %s result: %s\n", in_fnName, err );
    return false;
}

#if 1
#define VK_CHECK(f)                                 \
{                                                   \
	VkResult res = (f);                             \
	if ( res != VK_SUCCESS )	                    \
	{					                            \
        ReturnError( res, #f, __FILE__, __LINE__ ); \
 	}											    \
}
#else
    #define VK_CHECK(f) f
#endif


#endif //!__VK_UTILS_HPP__