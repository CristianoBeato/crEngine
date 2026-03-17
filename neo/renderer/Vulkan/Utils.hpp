
#ifndef __UTILS_HPP__
#define __UTILS_HPP__

extern idStr VulkanErrorString( VkResult errorCode );

inline void ReturnError( const VkResult in_errorCode, const char* in_fn, const char* in_file, const int in_line )
{
    auto error = VulkanErrorString( in_errorCode );
    printf( "VkResult is \"%s\" in \"%s\" at line %i\n", error.c_str(), in_file, in_line );
}

inline bool ResultCheck( const VkResult in_result, const char* in_fnName )
{
    /// no erro just pass
    if( in_result == VK_SUCCESS )
        return true;
    
    /// error found, what
    auto err = VulkanErrorString( in_result );
    printf( " Vulkan %s result: %s\n", in_fnName, err.c_str() );
    return false;
}

#define VK_CHECK(f)                                 \
{                                                   \
	VkResult res = (f);                             \
	if ( res != VK_SUCCESS )	                    \
	{					                            \
        ReturnError( res, #f, __FILE__, __LINE__ ); \
 	}											    \
}

#endif //!__UTILS_HPP__