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

#include "idlib/precompiled.h"
#include "renderer_common.h"
#include "Format.hpp"

/// TODO implement in opengl context to query this extensions 
/// GL_EXT_texture_sRGB_R8
/// GL_EXT_texture_sRGB_RG8

/// GL Table
static const GLenum k_OPENGL_FORMAT_TABLE[crInternalFormat::FORMAT_COUNT] = 
{
    GL_NONE,                                // NONE
    GL_R8,                                  // R8U
    GL_SR8_EXT,                             // R8U_SRGB
    GL_R16,                                 // R16U
    GL_RG8,                                 // RG8U
    GL_SRG8_EXT,                            // RG8U_SRGB
    GL_RG16,                                // RG16U
    GL_RGBA8,                               // RGBA8U
    GL_SRGB8_ALPHA8,                        // RGBA8U_SRGB
    GL_RGBA16,                              // RGBA16U
    GL_RGBA16F,                             // RGBA16F
    GL_RGBA32UI,                            // RGBA32U
    GL_RGBA32F,                             // RGBA32F
    GL_DEPTH_COMPONENT16,                   // DEPTH16
    GL_DEPTH_COMPONENT24,                   // DEPTH24
    GL_DEPTH_COMPONENT32F,                  // DEPTH32
    GL_DEPTH24_STENCIL8,                    // DEPTH24_STENCIL8
    GL_DEPTH32F_STENCIL8,                   // DEPTH32_STENCIL8
    GL_RGB565,                              // RGB565
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,        // BC1
    GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,       // BC1_SRGB
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,       // BC3
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, // BC3_SRGB
    GL_COMPRESSED_RG_RGTC2,                 // BC5
    GL_COMPRESSED_RGBA_BPTC_UNORM,          // BC7
    GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,    // BC7_SRGB
    GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,    // BC6H
    GL_COMPRESSED_RGBA8_ETC2_EAC,           // ETC2
    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,    // ETC2_sRGB
    GL_COMPRESSED_RG11_EAC,                 // EAC
};

static const GLenum k_OPENGL_COMPONENTS[crInternalFormat::FORMAT_COUNT] =
{
    GL_NONE,// NONE
    GL_RED,// R8U
    GL_RED,// R8U_SRGB
    GL_RED,// R16U
    GL_RG,// RG8U
    GL_RG,// RG8U_SRGB
    GL_RG,// RG16U
    GL_RGBA,// RGBA8U
    GL_RGBA,// RGBA8U_SRGB
    GL_RGBA,// RGBA16U
    GL_RGBA,// RGBA16F
    GL_RGBA,// RGBA32U
    GL_RGBA,// RGBA32F
    GL_DEPTH_COMPONENT,// DEPTH16
    GL_DEPTH_COMPONENT,// DEPTH24
    GL_DEPTH_COMPONENT,// DEPTH32
    GL_DEPTH_STENCIL,// DEPTH24_STENCIL8
    GL_DEPTH_STENCIL,// DEPTH32_STENCIL8,
    GL_RGB,// RGB565,
    GL_RGB,// BC1_RGB
    GL_RGB,// BC1_SRGB
    GL_RGBA,// BC3_RGBA
    GL_RGBA,// BC3_SRGBA
    GL_RG,// BC5_RG
    GL_RGBA,// BC7_RGBA
    GL_RGBA,// BC7_SRGBA
    GL_RGBA,// BC6H_RGBA
    GL_RGBA,// ETC2_RGBA
    GL_RGBA,// ETC2_SRGBA
    GL_RG// RG_EAC_RG
};

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

/// VK Table

GLenum crInternalFormat::GLInternal(void) const
{
    return k_OPENGL_FORMAT_TABLE[static_cast<uint32_t>( format )];
}

GLenum crInternalFormat::GLFormat(void) const
{
    return k_OPENGL_COMPONENTS[static_cast<uint32_t>( format )];
}

VkFormat crInternalFormat::VKInternal(void) const
{
    return K_VULKAN_FORMAT_TABLE[static_cast<uint32_t>( format )];
}

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
}
