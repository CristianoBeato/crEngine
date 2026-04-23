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

#include "Format.hpp"
#include "Core.hpp" 

static const VkFormat K_VULKAN_FORMAT_TABLE[crInternalFormat::FORMAT_COUNT] = 
{
    VK_FORMAT_UNDEFINED,                    // NONE
    VK_FORMAT_R8_UNORM,                     // R8U
    VK_FORMAT_R8_SRGB,                      // R8U_sRGB
    VK_FORMAT_R16_UNORM,                    // R16U
    VK_FORMAT_R8G8_UNORM,                   // RG8U
    VK_FORMAT_R8G8_SRGB,                    // RG8U_sRGB
    VK_FORMAT_R16G16_UNORM,                 // RG16U
    VK_FORMAT_R8G8B8A8_UNORM,               // RGBA8U
    VK_FORMAT_R8G8B8A8_SRGB,                // RGBA8U_sRGB
    VK_FORMAT_R16G16B16A16_UNORM,           // RGBA16U
    VK_FORMAT_R16G16B16A16_SFLOAT,          // RGBA16F
    VK_FORMAT_R32G32B32A32_UINT,            // RGBA32U
    VK_FORMAT_R32G32B32A32_SFLOAT,          // RGBA32F
    VK_FORMAT_D16_UNORM,                    // DEPTH16
    VK_FORMAT_D32_SFLOAT,                   // DEPTH24
    VK_FORMAT_D32_SFLOAT,                   // DEPTH32
    VK_FORMAT_D24_UNORM_S8_UINT,            // DEPTH24_STENCIL8
    VK_FORMAT_D32_SFLOAT_S8_UINT,           // DEPTH32_STENCIL8
    VK_FORMAT_R5G6B5_UNORM_PACK16,          // RGB565
    VK_FORMAT_BC1_RGB_UNORM_BLOCK,          // BC1
    VK_FORMAT_BC1_RGB_SRGB_BLOCK,           // BC1_sRGB
    VK_FORMAT_BC3_UNORM_BLOCK,              // BC3
    VK_FORMAT_BC3_SRGB_BLOCK,               // BC3_sRGB
    VK_FORMAT_BC5_UNORM_BLOCK,              // BC5
    VK_FORMAT_BC7_UNORM_BLOCK,              // BC7
    VK_FORMAT_BC7_SRGB_BLOCK,               // BC7_sRGB
    VK_FORMAT_BC6H_SFLOAT_BLOCK,            // BC6H
    VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,    // ETC2
    VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,     // ETC2_sRGB
    VK_FORMAT_EAC_R11G11_UNORM_BLOCK,   // FORMAT_EAC
};

#ifdef __RENDERER_LIB__

static inline crInternalFormat::format_t FromVulkan( const VkFormat &in_fmt )
{
    crInternalFormat::format_t format;
    switch ( in_fmt )
    {
        case VK_FORMAT_R8_UNORM:
            format = crInternalFormat::R8U;
            break;
        
        case VK_FORMAT_R8_SRGB:
            format = crInternalFormat::R8U_SRGB;
            break;
        
        case VK_FORMAT_R16_UNORM:
            format = crInternalFormat::R16U;
            break;
        
        case VK_FORMAT_R8G8_UNORM:
            format = crInternalFormat::RG8U;
            break;
        
        case VK_FORMAT_R8G8_SRGB:
            format = crInternalFormat::RG8U_SRGB;
            break;
        
        case VK_FORMAT_R16G16_UNORM:
            format = crInternalFormat::RG16U;
            break;
        
        case VK_FORMAT_R8G8B8A8_UNORM:
            format = crInternalFormat::RGBA8U;
            break;
        
        case VK_FORMAT_R8G8B8A8_SRGB:
            format = crInternalFormat::RGBA8U_SRGB;
            break;
        
        case VK_FORMAT_R16G16B16A16_UNORM:
            format = crInternalFormat::RGBA16U;
            break;
        
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            format = crInternalFormat::RGBA16F;
            break;
        
        case VK_FORMAT_R32G32B32A32_UINT:
            format = crInternalFormat::RGBA32U;
            break;
        
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            format = crInternalFormat::RGBA32F;
            break;
        
        case VK_FORMAT_D16_UNORM:
            format = crInternalFormat::DEPTH16;
            break;
        
        case VK_FORMAT_D32_SFLOAT:
            format = crInternalFormat::DEPTH32;
            break;
        
        case VK_FORMAT_D24_UNORM_S8_UINT:
            format = crInternalFormat::DEPTH24_STENCIL8;
            break;
        
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            format = crInternalFormat::DEPTH32_STENCIL8;
            break;
        
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            format = crInternalFormat::RGB565;
            break;
        
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            format = crInternalFormat::BC1_RGB;
            break;
        
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            format = crInternalFormat::BC1_SRGB;
            break;
        
        case VK_FORMAT_BC3_UNORM_BLOCK:
            format = crInternalFormat::BC3_RGBA;
            break;
        
        case VK_FORMAT_BC3_SRGB_BLOCK:
            format = crInternalFormat::BC3_SRGBA;
            break;
        
        case VK_FORMAT_BC5_UNORM_BLOCK:
            format = crInternalFormat::BC5_RG;
            break;
        
        case VK_FORMAT_BC7_UNORM_BLOCK:
            format = crInternalFormat::BC7_RGBA;
            break;
        
        case VK_FORMAT_BC7_SRGB_BLOCK:
            format = crInternalFormat::BC7_SRGBA;
            break;
        
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            format = crInternalFormat::BC6H_RGBA;
            break;
        
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            format = crInternalFormat::ETC2_RGBA;
            break;
        
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            format = crInternalFormat::ETC2_SRGBA;
            break;

        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            format = crInternalFormat::RG_EAC_RG;
            break;
    
    default:
        format = crInternalFormat::NONE;
        break;
    }

    return format;
}

crInternalFormat::crInternalFormat( const VkFormat &in_format )
{
    format = FromVulkan( in_format );
}

crInternalFormat crInternalFormat::operator=( const VkFormat in_format )
{
    format = FromVulkan( in_format );
    return *this;
}

VkFormat crInternalFormat::VKInternal(void) const
{
    return K_VULKAN_FORMAT_TABLE[static_cast<uint32_t>( format )];
}
#endif //!__RENDERER__

bool crInternalFormat::Compressed(void) const
{
    switch ( format )
    {
        case BC1_RGB:
        case BC1_SRGB:
        case BC3_RGBA:
        case BC3_SRGBA:
        case BC5_RG:
        case BC7_RGBA:
        case BC7_SRGBA:
        case BC6H_RGBA:
        case ETC2_RGBA:
        case ETC2_SRGBA:
        case RG_EAC_RG:
            return true;
    }

    return false;
}

uint8_t crInternalFormat::BitsPerPixel(void) const
{
    switch ( format )
    {
        case R8U:
            return 8;
        case R8U_SRGB:
            return 8;
        case R16U:
            return 16;
        case RG8U:
            return 16;
        case RG8U_SRGB:
            return 16;
        case RG16U:
            return 32;
        case RGBA8U:
            return 32;
        case RGBA8U_SRGB:
            return 32;
        case RGBA16U:
            return 64;
        case RGBA16F:
            return 64;
        case RGBA32U:
            return 128;
        case RGBA32F:
            return 128;
        case DEPTH16:
            return 16;
        case DEPTH24:   // vulkan don't suport, so we round to 32
            return 32;
        case DEPTH32:
            return 32;
        case DEPTH24_STENCIL8:
            return 32;
        case DEPTH32_STENCIL8:
            return 40;
        case RGB565:
            return 16;
        case BC1_RGB:
            return 4;
        case BC1_SRGB:
            return 4;
        case BC3_RGBA:
            return 8;
        case BC3_SRGBA:
            return 8;
        case BC5_RG:
            return 8;
        case BC7_RGBA:
            return 8;
        case BC7_SRGBA:
            return 8;
        case BC6H_RGBA:
            return 8;
        case ETC2_RGBA:
            return 8;
        case ETC2_SRGBA:
            return 8;
        case RG_EAC_RG:
            return 8;
    }

    return 0;
}

float crInternalFormat::BytesPerPixel(void) const
{
    switch ( format )
    {
        case R8U:
            return 1.0f;
        case R8U_SRGB:
            return 1.0f;
        case R16U:
            return 2.0f;
        case RG8U:
            return 2.0f;
        case RG8U_SRGB:
            return 2.0f;
        case RG16U:
            return 4.0f;
        case RGBA8U:
            return 4.0f;
        case RGBA8U_SRGB:
            return 4.0f;
        case RGBA16U:
            return 8.0f;
        case RGBA16F:
            return 8.0f;
        case RGBA32U:
            return 16.0f;
        case RGBA32F:
            return 16.0f;
        case DEPTH16:
            return 2.0f;
        case DEPTH24:   // vulkan don't suport, so we round to 32
            return 4.0f;
        case DEPTH32:
            return 4.0f;
        case DEPTH24_STENCIL8:
            return 4.0f;
        case DEPTH32_STENCIL8:
            return 5.0f;
        case RGB565:
            return 2.0f;
        case BC1_RGB:
            return 0.5f;
        case BC1_SRGB:
            return 0.5f;
        case BC3_RGBA:
            return 1.0f;
        case BC3_SRGBA:
            return 1.0f;
        case BC5_RG:
            return 1.0f;
        case BC7_RGBA:
            return 1.0f;
        case BC7_SRGBA:
            return 1.0f;
        case BC6H_RGBA:
            return 1.0f;
        case ETC2_RGBA:
            return 1.0f;
        case ETC2_SRGBA:
            return 1.0f;
        case RG_EAC_RG:
            return 1.0f;
    }

    return 0.0f;
}
