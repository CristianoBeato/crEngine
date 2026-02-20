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
#include "renderer_common.h"
#include "vkTexture.hpp"

// 1 : 1 format table, update all changes done in the crTexture::format_t
const VkFormat vulkanFormat[crTexture::TF_FORMAT_COUNT] =
{
    VK_FORMAT_UNDEFINED,                        // TF_NONE

    VK_FORMAT_R8_UNORM,                         // TF_R8I
    VK_FORMAT_R16_UNORM,                        // TF_R16I
    VK_FORMAT_R8G8_UNORM,                       // TF_RG8I
    VK_FORMAT_R16G16_UNORM,                     // TF_RG16I
    VK_FORMAT_R8G8B8A8_UNORM,                   // TF_RGBA8
    VK_FORMAT_R8G8B8A8_SRGB,                    // TF_SRGBA8
    VK_FORMAT_R16G16B16A16_UNORM,               // TF_RGBA16
    VK_FORMAT_R16G16B16A16_SFLOAT,              // TF_RGBA16F
    VK_FORMAT_R32G32B32A32_UINT,                // TF_RGBA32
    VK_FORMAT_R32G32B32A32_SFLOAT,              // TF_RGBA32F

    VK_FORMAT_R5G6B5_UNORM_PACK16,              // TF_RGB565

    VK_FORMAT_BC1_RGB_UNORM_BLOCK,              // TF_DXT1
    VK_FORMAT_BC1_RGB_SRGB_BLOCK,               // TF_DXT1_SRGB
    VK_FORMAT_BC3_UNORM_BLOCK,                  // TF_DXT5
    VK_FORMAT_BC3_SRGB_BLOCK,                   // TF_DXT5_SRGB

    VK_FORMAT_D16_UNORM,                       // TF_DEPTH16
    VK_FORMAT_D24_UNORM_S8_UINT,               // TF_DEPTH24
    VK_FORMAT_D32_SFLOAT,                      // TF_DEPTH32
    VK_FORMAT_D24_UNORM_S8_UINT,               // TF_DEPTH24_STENCIL8
    VK_FORMAT_D32_SFLOAT_S8_UINT               // TF_DEPTH32_STENCIL8
};

const uint8_t vulkanFormatSize[crTexture::TF_FORMAT_COUNT] =
{
    0, // TF_NONE

    1, // TF_R8I
    1, // TF_R16I
    2, // TF_RG8I
    4, // TF_RG8I
    4, // TF_RGBA8
    4, // TF_SRGBA8
    8, // TF_RGBA16
    8, // TF_RGBA16F
    16, // TF_RGBA32
    16, // TF_RGBA32F
    
    2, // TF_RGB565

    8,  // TF_DXT1 (para 4x4 block)
    8,  // TF_DXT1_SRGB (para 4x4 block)
    16, // TF_DXT5
    16, // TF_DXT5_SRGB

    2,  // TF_DEPTH16
    4,  // TF_DEPTH24
    4,  // TF_DEPTH32
    4,  // TF_DEPTH24_STENCIL8
    8,  // TF_DEPTH32_STENCIL8
};

const VkImageAspectFlags vulkanImageAspects[crTexture::TF_FORMAT_COUNT] =
{
    VK_IMAGE_ASPECT_NONE,       // TF_NONE

    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_R8I
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_R16I
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RG8I
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RG16I
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RGBA8
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_SRGBA8
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RGBA16
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RGBA16F
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RGBA32
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RGBA32F
    
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_RGB565
    
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_DXT1
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_DXT1_SRGB
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_DXT5
    VK_IMAGE_ASPECT_COLOR_BIT,  // TF_DXT5_SRGB
    
    VK_IMAGE_ASPECT_DEPTH_BIT,  // TF_DEPTH16
    VK_IMAGE_ASPECT_DEPTH_BIT,  // TF_DEPTH24
    VK_IMAGE_ASPECT_DEPTH_BIT,  // TF_DEPTH32
    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,    // TF_DEPTH24_STENCIL8
    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT     // TF_DEPTH32_STENCIL8
};

vkSampler::vkSampler( void ) : 
    m_device( nullptr ),
    m_sampler( nullptr )
{
}

vkSampler::~vkSampler( void )
{
}

bool vkSampler::Create(const filter_t in_filtering, const wrapping_t in_Swrap, const wrapping_t in_Twrap, const wrapping_t in_Rwrap)
{
    VkResult result = VK_SUCCESS;
    VkSamplerCreateInfo samplerCI{};
    
    auto device = tr.vkContext->Device();

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
        auto device = tr.vkContext->Device();
        vkDestroySampler( *device, m_sampler, k_allocationCallbacks );
        m_sampler = nullptr;
    }
}

void *vkSampler::Handler(void) const
{
    return const_cast<VkSampler*>( &m_sampler );
}

static VkImageAspectFlags GetAspect( const VkFormat fmt )
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

vkTexture::vkTexture( void ) : 
    m_image( nullptr ),
    m_view( nullptr ),
    m_memory( nullptr ),
    m_device( nullptr )
{
    m_state.layout = VK_IMAGE_LAYOUT_GENERAL;
    m_state.aspect = VK_IMAGE_ASPECT_NONE;
    m_state.stage = ;
    m_state.access = ;
}

vkTexture::~vkTexture( void )
{
}

bool vkTexture::Create(const type_t in_type, const dimensions_t in_dimensions, const format_t in_format)
{
    idList<uint32_t>    queues;
    VkResult result = VK_SUCCESS;
    VkFormat format = vulkanFormat[in_format];
    VkMemoryRequirements        req;
    VkImageCreateInfo           imageCI{};
    VkMemoryAllocateInfo        memoryAllocateInfo{};
    VkImageSubresourceRange     imageSubresource{};
    VkImageViewCreateInfo       imageViewCI{};

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

    switch ( in_type )
    {
        case TEXTURE_1D:
            imageCI.imageType = VK_IMAGE_TYPE_1D;
            break;
        case TEXTURE_2D:
            imageCI.imageType = VK_IMAGE_TYPE_2D;
            break;
        case TEXTURE_3D:
            imageCI.imageType = VK_IMAGE_TYPE_3D;
            break;
        case TEXTURE_CUBEMAP:
            imageCI.imageType = VK_IMAGE_TYPE_2D;
            break;
    default:
         // TODO: error call
        break;
    }
    
    result = vkCreateImage( m_device, &imageCI, k_allocationCallbacks, &m_image );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkCreateImage error %s\n", GetVulkanError( result ) );
        return false;
    };

    // get the image memory requirements
    vkGetImageMemoryRequirements( m_device, m_image, &req );
    
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.allocationSize = req.size;
    memoryAllocateInfo.memoryTypeIndex = tr.vkContext->FindMemoryType( req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
    
    result = vkAllocateMemory( m_device, &memoryAllocateInfo, k_allocationCallbacks, &m_memory );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkAllocateMemory error %s\n", GetVulkanError( result ) );
        return false;
    };

    result = vkBindImageMemory( m_device, m_image, m_memory, 0 );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkBindImageMemory error %s\n", GetVulkanError( result ) );
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
    case TEXTURE_1D:
        imageViewCI.viewType = ( in_dimensions.layers > 1 ) ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
        break;
    case TEXTURE_2D:
        imageViewCI.viewType = ( in_dimensions.layers > 1 ) ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
        break;
    case TEXTURE_3D:
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
        break;
    case TEXTURE_CUBEMAP:
        imageViewCI.viewType = ( in_dimensions.layers > 1 ) ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
        break;    
    default:
        break;
    }

    result = vkCreateImageView( m_device, &imageViewCI, k_allocationCallbacks, &m_view );
    if ( result != VK_SUCCESS )
    {
        common->Error( "vkCreateImageView error %s\n", GetVulkanError( result ) );
        return false;
    };

    m_dimensions = in_dimensions;
    return true;
}

void vkTexture::Destroy(void)
{
    if( m_view != nullptr )
    {
        vkDestroyImageView( m_device, m_view, k_allocationCallbacks );
        m_view = nullptr;
    }

    if( m_memory != nullptr )
    {
        vkFreeMemory( m_device, m_memory, k_allocationCallbacks );
        m_memory = nullptr;
    }

    if( m_image != nullptr )
    {
        vkDestroyImage( m_device, m_image, k_allocationCallbacks );
        m_image = nullptr;
    }

    m_device = nullptr;
}

void *vkTexture::Handler(void) const
{
    return m_image;
}

void vkTexture::SetState( const textureState_t &in_state, const VkCommandBuffer in_commandBuffer )
{
    // we update the whole image, since we map the current texture state change, we can't handle sections
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = in_state.aspect;    
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = m_dimensions.levels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = m_dimensions.layers;

    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.pNext = nullptr;
    barrier.srcStageMask = m_state.stage;
    barrier.srcAccessMask = m_state.access;
    barrier.dstStageMask = in_state.stage;
    barrier.dstAccessMask = in_state.access;
    barrier.oldLayout = m_state.layout;
    barrier.newLayout = in_state.layout;
    barrier.srcQueueFamilyIndex = m_state.queueFamily;
    barrier.dstQueueFamilyIndex = in_state.queueFamily;
    barrier.image = m_image;
    barrier.subresourceRange = subresourceRange;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;
    depInfo.dependencyFlags = 0;
    depInfo.memoryBarrierCount = 0;
    depInfo.pMemoryBarriers = nullptr;
    depInfo.bufferMemoryBarrierCount = 0;
    depInfo.pBufferMemoryBarriers = nullptr;
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &barrier;
    vkCmdPipelineBarrier2( in_commandBuffer, &depInfo );

    // update image state info
    m_state = in_state;
}
