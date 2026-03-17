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

#include "Texture.hpp"
#include "Core.hpp"

static VkImageType k_IMAGE_TYPE_TABLE[IMAGE_TYPE_COUNT] = 
{
    VK_IMAGE_TYPE_MAX_ENUM, // IMAGE_NONE ( VK_IMAGE_TYPE_MAX_ENUM Will cause a error )
    VK_IMAGE_TYPE_1D,       // IMAGE_1D
    VK_IMAGE_TYPE_2D,       // IMAGE_2D
    VK_IMAGE_TYPE_3D,       // IMAGE_3D
    VK_IMAGE_TYPE_3D,       // IMAGE_CUBEMAP
};

static inline VkImageAspectFlags GetAspect( const VkFormat fmt )
{
    switch (fmt)
    {
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
        return VK_IMAGE_ASPECT_DEPTH_BIT;

    default:
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

bool vkTexture::Create( const image_type_t in_type, const dimensions_t in_dimensions, const crInternalFormat in_format )
{    
    idList<uint32_t>    queues;
    VkResult result = VK_SUCCESS;
    VkFormat format = in_format;
    VkMemoryRequirements        req;
    VkImageCreateInfo           imageCI{};
    VkMemoryAllocateInfo        memoryAllocateInfo{};
    VkImageSubresourceRange     imageSubresource{};
    VkImageViewCreateInfo       imageViewCI{};
    auto device = tr.GetRenderDevice();

    m_type = in_type;
    m_format = in_format;
    m_dimensions = in_dimensions;

    ///
    ///
    ///
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.pNext = nullptr;
    imageCI.flags = 0;
    imageCI.imageType = VK_IMAGE_TYPE_1D;
    imageCI.format = format;
    imageCI.extent = { in_dimensions.width, in_dimensions.heigth, in_dimensions.depth };
    imageCI.mipLevels = in_dimensions.levels;
    imageCI.arrayLayers = in_dimensions.layers;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT; //TODO:
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageCI.sharingMode = VK_SHARING_MODE_CONCURRENT; // can be used for graphic and compute queues
    imageCI.queueFamilyIndexCount = queues.Num();
    imageCI.pQueueFamilyIndices = queues.Ptr();
    imageCI.initialLayout = m_state.layout;
    imageCI.imageType = k_IMAGE_TYPE_TABLE[in_type];
    
    result = vkCreateImage( *device, &imageCI, k_allocationCallbacks, &m_image );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkCreateImage error %s\n", VulkanErrorString( result ) );
        return false;
    };

    // get the image memory requirements
    vkGetImageMemoryRequirements( *device, m_image, &req );
    
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.allocationSize = req.size;
    memoryAllocateInfo.memoryTypeIndex = device->FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
    
    result = vkAllocateMemory( *device, &memoryAllocateInfo, k_allocationCallbacks, &m_memory );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkAllocateMemory error %s\n", VulkanErrorString( result ) );
        return false;
    };

    result = vkBindImageMemory( *device, m_image, m_memory, 0 );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkBindImageMemory error %s\n", VulkanErrorString( result ) );
        return false;
    };

    imageSubresource.levelCount = in_dimensions.levels;
    imageSubresource.layerCount = in_dimensions.layers;
    imageSubresource.baseMipLevel = 0;
    imageSubresource.baseArrayLayer = 0;
    imageSubresource.aspectMask = GetAspect( format );

    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.pNext = nullptr;
    imageViewCI.flags = 0;
    imageViewCI.image = m_image;
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D;
    imageViewCI.format = format;
    imageViewCI.subresourceRange = imageSubresource;
    //imageViewCI.components = ;

    switch ( m_type )
    {
    case IMAGE_1D:
        imageViewCI.viewType = ( in_dimensions.layers > 1 ) ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
        break;
    case IMAGE_2D:
        imageViewCI.viewType = ( in_dimensions.layers > 1 ) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        break;
    case IMAGE_3D:
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case IMAGE_CUBEMAP:
        imageViewCI.viewType = ( in_dimensions.layers > 1 ) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        break;    
    default:
        idLib::Error( "INVALID IMAGE TYPE %i\n", m_type );
        break;
    }

    result = vkCreateImageView( *device, &imageViewCI, k_allocationCallbacks, &m_view );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkCreateImageView error %s\n", VulkanErrorString( result ) );
        return false;
    };

    m_dimensions = in_dimensions;
    return true;
}

void vkTexture::Destroy(void)
{
    auto device = tr.GetRenderDevice();
    
    if( m_view != nullptr )
    {
        vkDestroyImageView( *device, m_view, k_allocationCallbacks );
        m_view = nullptr;
    }

    if( m_memory != nullptr )
    {
        vkFreeMemory( *device, m_memory, k_allocationCallbacks );
        m_memory = nullptr;
    }

    if( m_image != nullptr )
    {
        vkDestroyImage( *device, m_image, k_allocationCallbacks );
        m_image = nullptr;
    }
}

vkSampler::vkSampler( void ) : m_sampler( nullptr )
{
}

vkSampler::~vkSampler( void )
{
    Destroy();
}

bool vkSampler::Create( const filter_t in_filtering, const wrapping_t in_Swrap, const wrapping_t in_Twrap, const wrapping_t in_Rwrap )
{
    VkResult result = VK_SUCCESS;
    VkSamplerCreateInfo samplerCI{};
    
    auto device = tr.GetRenderDevice();

    samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCI.pNext = nullptr;
    samplerCI.flags = 0;
    samplerCI.magFilter = VK_FILTER_NEAREST;
    samplerCI.minFilter = VK_FILTER_NEAREST;
    samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerCI.mipLodBias = 0.0;
    samplerCI.anisotropyEnable = VK_FALSE;
    samplerCI.maxAnisotropy = 0.0f;
    samplerCI.compareEnable = VK_FALSE;
    samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCI.minLod = 0.0f;
    samplerCI.maxLod = 0.0f;
    samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCI.unnormalizedCoordinates = VK_FALSE;

    switch ( in_filtering )
    {
        case FILTER_NEAREST:    
            samplerCI.magFilter = VK_FILTER_NEAREST;
            samplerCI.minFilter = VK_FILTER_NEAREST;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case FILTER_LINEAR:
            samplerCI.magFilter = VK_FILTER_LINEAR;
            samplerCI.minFilter = VK_FILTER_LINEAR;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case FILTER_BILINEAR:
            samplerCI.magFilter = VK_FILTER_LINEAR;
            samplerCI.minFilter = VK_FILTER_LINEAR;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            break;
        case FILTER_TRILINEAR:
            samplerCI.magFilter = VK_FILTER_LINEAR;
            samplerCI.minFilter = VK_FILTER_LINEAR;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            break;
        case FILTER_ANISOTROPIC2X:
            samplerCI.magFilter = VK_FILTER_LINEAR;
            samplerCI.minFilter = VK_FILTER_LINEAR;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCI.anisotropyEnable = VK_TRUE;
            samplerCI.maxAnisotropy = 2.0f;
            break;
        case FILTER_ANISOTROPIC4X:
            samplerCI.magFilter = VK_FILTER_LINEAR;
            samplerCI.minFilter = VK_FILTER_LINEAR;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCI.anisotropyEnable = VK_TRUE;
            samplerCI.maxAnisotropy = 4.0f;
            break;
        case FILTER_ANISOTROPIC8X:
            samplerCI.magFilter = VK_FILTER_LINEAR;
            samplerCI.minFilter = VK_FILTER_LINEAR;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCI.anisotropyEnable = VK_TRUE;
            samplerCI.maxAnisotropy = 8.0f;
            break;
        case FILTER_ANISOTROPIC16X:
            samplerCI.magFilter = VK_FILTER_LINEAR;
            samplerCI.minFilter = VK_FILTER_LINEAR;
            samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerCI.anisotropyEnable = VK_TRUE;
            samplerCI.maxAnisotropy = 16.0f;
            break;
    };

    switch ( in_Swrap )
    {
        case WRAP_NONE:
            samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;
        case WRAP_REPEAT:
            samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            break;
        case WRAP_MIRRORED:
            samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            break;
        case WRAP_EDGE:
            samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case WRAP_BORDER:
            samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;    
    }

    switch ( in_Twrap )
    {
        case WRAP_NONE:
            samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;
        case WRAP_REPEAT:
            samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            break;
        case WRAP_MIRRORED:
            samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            break;
        case WRAP_EDGE:
            samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case WRAP_BORDER:
            samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;    
    }

    switch ( in_Rwrap )
    {
        case WRAP_NONE:
            samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;
        case WRAP_REPEAT:
            samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            break;
        case WRAP_MIRRORED:
            samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            break;
        case WRAP_EDGE:
            samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case WRAP_BORDER:
            samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;    
    }

    result = vkCreateSampler( *device, &samplerCI, k_allocationCallbacks, &m_sampler );
    if( result != VK_SUCCESS )
    {
        common->Error( "vkSampler::Create::vkCreateSampler failed" );
        return false;
    }

    return true;
}

void vkSampler::Destroy(void)
{
    if( m_sampler != nullptr )
    {
        auto device = tr.GetRenderDevice();
        vkDestroySampler( *device, m_sampler, k_allocationCallbacks );
        m_sampler = nullptr;
    }
}
