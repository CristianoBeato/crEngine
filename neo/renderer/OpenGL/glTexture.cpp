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
#include "glTexture.hpp"

// 1 : 1 format table, update all changes done in the crTexture::format_t
const GLenum internalFormatTabe[crTexture::TF_FORMAT_COUNT] =
{
    GL_NONE,                            // TF_NONE
    
    // uncompressed
    GL_R8,                              // TF_R8I
    GL_R16,                             // TF_R16I
    GL_RG8,                             // TF_RG8I
    GL_RG16,                            // TF_RG16I
    GL_RGBA8,                           // TF_RGBA8
    GL_SRGB8_ALPHA8,                    // TF_SRGBA8
    GL_RGBA16,                          // TF_RGBA16
    GL_RGBA16F,                         // TF_RGBA16F
    GL_RGBA32UI,                        // TF_RGBA32
    GL_RGBA32F,                         // TF_RGBA32F

    // paked
    GL_RGB565,                          // TF_RGB565

    // compressed 
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT,        // TF_DXT1
    GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,       // TF_DXT1_SRGB
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,       // TF_DXT5
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, // TF_DXT5_SRGB

    // depth 
    GL_DEPTH_COMPONENT16,               // TF_DEPTH16
    GL_DEPTH_COMPONENT24,               // TF_DEPTH24
    GL_DEPTH_COMPONENT32F,              // TF_DEPTH32
   
    // stencil 
    GL_DEPTH24_STENCIL8,                // TF_DEPTH24_STENCIL8
    GL_DEPTH24_STENCIL8,                // TF_DEPTH32_STENCIL8
};

const GLenum formatTable[crTexture::TF_FORMAT_COUNT]
{
    GL_NONE,            // TF_NONE
    
    // unconpressed color
    GL_RED,             // TF_R8I
    GL_RED,             // TF_R16I
    GL_RG,              // TF_RG8I
    GL_RG,              // TF_RG16I
    GL_RGBA,            // TF_RGBA8
    GL_RGBA,            // TF_SRGBA8
    GL_RGBA,            // TF_RGBA16
    GL_RGBA,            // TF_RGBA16F
    GL_RGBA,            // TF_RGBA32
    GL_RGBA,            // TF_RGBA32F

    // paked
    GL_RGBA,            // TF_RGB565
    
    // compressed 
    GL_RGB,             // TF_DXT1
    GL_RGB,             // TF_DXT1_SRGB
    GL_RGBA,            // TF_DXT5
    GL_RGBA,            // TF_DXT5_SRGB
    
    // depth component 
    GL_DEPTH,           // TF_DEPTH16
    GL_DEPTH,           // TF_DEPTH24
    GL_DEPTH,           // TF_DEPTH32

    // stecil component 
    GL_DEPTH_STENCIL,   // TF_DEPTH24_STENCIL8
    GL_DEPTH_STENCIL    // TF_DEPTH32_STENCIL8
};

const GLenum dataType[crTexture::TF_FORMAT_COUNT]
{
    GL_NONE,                            // TF_NONE

    GL_UNSIGNED_BYTE,                   // TF_R8I
    GL_UNSIGNED_SHORT,                  // TF_R16I
    GL_UNSIGNED_BYTE,                   // TF_RG8I
    GL_UNSIGNED_SHORT,                  // TF_RG16I
    GL_UNSIGNED_BYTE,                   // TF_RGBA8
    GL_UNSIGNED_BYTE,                   // TF_SRGBA8
    GL_UNSIGNED_SHORT,                  // TF_RGBA16
    GL_HALF_FLOAT,                      // TF_RGBA16F
    GL_UNSIGNED_INT,                    // TF_RGBA32
    GL_FLOAT,                           // TF_RGBA32F
    
    GL_UNSIGNED_SHORT_5_6_5,            // TF_RGB565

    GL_NONE,                            // TF_DXT1 compressed, no data type
    GL_NONE,                            // TF_DXT1_SRGB compressed, no data type
    GL_NONE,                            // TF_DXT5 compressed, no data type
    GL_NONE,                            // TF_DXT5_SRGB compressed, no data type
    
    GL_UNSIGNED_SHORT,                  // TF_DEPTH16
    GL_UNSIGNED_INT,                    // TF_DEPTH24
    GL_FLOAT,                           // TF_DEPTH32
    GL_UNSIGNED_INT_24_8,               // TF_DEPTH24_STENCIL8
    GL_FLOAT_32_UNSIGNED_INT_24_8_REV,  // TF_DEPTH32_STENCIL8
};

// bytes per pixel
const uint8_t formatSize[crTexture::TF_FORMAT_COUNT] =
{
    0,  // TF_NONE

    1,  // TF_R8I
    2,  // TF_R16I
    2,  // TF_RG8I
    4,  // TF_RG16I
    4,  // TF_RGBA8
    4,  // TF_SRGBA8
    8,  // TF_RGBA16
    8,  // TF_RGBA16F
    16, // TF_RGBA32
    16, // TF_RGBA32F

    2,  // TF_RGB565

    8,  // TF_DXT1 (para 4x4 block)
    8,  // TF_DXT1 (para 4x4 block)
    16, // TF_DXT5
    16, // TF_DXT5

    2,  // TF_DEPTH16
    4,  // TF_DEPTH24
    4,  // TF_DEPTH32
    4,  // TF_DEPTH24_STENCIL8
    8,  // TF_DEPTH32_STENCIL8
};

glSampler::glSampler( void ) : m_sampler( 0 )
{
}

glSampler::~glSampler( void )
{
}

bool glSampler::Create( const filter_t in_filtering, const wrapping_t in_Swrap, const wrapping_t in_Twrap, const wrapping_t in_Rwrap )
{
    GLenum  filterMin = GL_LINEAR;
    GLenum  filterMag = GL_LINEAR;
    GLenum  wrapS = GL_CLAMP_TO_BORDER;
    GLenum  wrapT = GL_CLAMP_TO_BORDER;
    GLenum  wrapR = GL_CLAMP_TO_BORDER;
    GLfloat anisostropy = 0.0f;

    switch ( in_filtering )
    {
        case FILTER_NEARES:
            filterMin = GL_NEAREST;
            filterMag = GL_NEAREST;
            break;
        case FILTER_LINEAR:
            filterMin = GL_LINEAR;
            filterMag = GL_LINEAR;
            break;
        case FILTER_BILINEAR:
            filterMin = GL_LINEAR_MIPMAP_NEAREST;
            filterMag = GL_LINEAR;
            break;
        case FILTER_TRILINEAR:
            filterMin = GL_LINEAR_MIPMAP_LINEAR;
            filterMag = GL_LINEAR;
            break;
        case FILTER_ANISOTROPIC2X:
            filterMin = GL_LINEAR_MIPMAP_LINEAR;
            filterMag = GL_LINEAR;
            anisostropy = 2.0f;
            break;
        case FILTER_ANISOTROPIC4X:
            filterMin = GL_LINEAR_MIPMAP_LINEAR;
            filterMag = GL_LINEAR;
            anisostropy = 4.0f;
            break;
        case FILTER_ANISOTROPIC8X:
            filterMin = GL_LINEAR_MIPMAP_LINEAR;
            filterMag = GL_LINEAR;
            anisostropy = 8.0f;
            break;
        case FILTER_ANISOTROPIC16X:
            filterMin = GL_LINEAR_MIPMAP_LINEAR;
            filterMag = GL_LINEAR;
            anisostropy = 16.0f;
            break;
    };

    switch ( in_Swrap )
    {
        case WRAP_NONE:
            wrapS = GL_CLAMP_TO_BORDER;
            break;
        case WRAP_REPEAT:
            wrapS = GL_REPEAT;
            break;
        case WRAP_MIRRORED:
            wrapS = GL_MIRRORED_REPEAT;
            break;
        case WRAP_EDGE:
            wrapS = GL_CLAMP_TO_EDGE;
            break;
        case WRAP_BORDER:
            wrapS = GL_CLAMP_TO_BORDER;
            break;    
    }

    switch ( in_Twrap )
    {
        case WRAP_NONE:
            wrapT = GL_CLAMP_TO_BORDER;
            break;
        case WRAP_REPEAT:
            wrapT = GL_REPEAT;
            break;
        case WRAP_MIRRORED:
            wrapT = GL_MIRRORED_REPEAT;
            break;
        case WRAP_EDGE:
            wrapT = GL_CLAMP_TO_EDGE;
            break;
        case WRAP_BORDER:
            wrapT = GL_CLAMP_TO_BORDER;
            break;    
    }

    switch ( in_Rwrap )
    {
        case WRAP_NONE:
            wrapR = GL_CLAMP_TO_BORDER;
            break;
        case WRAP_REPEAT:
            wrapR = GL_REPEAT;
            break;
        case WRAP_MIRRORED:
            wrapR = GL_MIRRORED_REPEAT;
            break;
        case WRAP_EDGE:
            wrapR = GL_CLAMP_TO_EDGE;
            break;
        case WRAP_BORDER:
            wrapR = GL_CLAMP_TO_BORDER;
            break;    
    }

    // set texture filtering
    glSamplerParameteri( m_sampler, GL_TEXTURE_MIN_FILTER, filterMin );
    glSamplerParameteri( m_sampler, GL_TEXTURE_MAG_FILTER, filterMag );
    glSamplerParameterf( m_sampler, GL_TEXTURE_MAG_FILTER, anisostropy );

    // set sampler repeating
    glSamplerParameteri( m_sampler, GL_TEXTURE_WRAP_S, wrapS );
    glSamplerParameteri( m_sampler, GL_TEXTURE_WRAP_T, wrapT );
    glSamplerParameteri( m_sampler, GL_TEXTURE_WRAP_R, wrapR );

    return m_sampler != 0 && glIsSampler( m_sampler ) == GL_TRUE;
}

void glSampler::Destroy(void)
{
    if ( m_sampler != 0 )
    {
        glDeleteSamplers( 1, &m_sampler );
        m_sampler = 0;
    }
}

void *glSampler::Handler(void) const
{
    return const_cast<GLuint*>( &m_sampler );
}

glTexture::glTexture( void ) : crTexture(), 
    m_target( GL_NONE ),
    m_internalformat( GL_NONE ),
    m_texture( 0 )
{
}

glTexture::~glTexture( void )
{
}

bool glTexture::Create(const type_t in_type, const dimensions_t in_dimensions, const format_t in_format)
{
    switch ( in_type )
    {
    case TEXTURE_1D:
    {
        m_target = ( in_dimensions.layers > 1 ) ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
        glCreateTextures( m_target, 1, &m_texture );
        if ( in_dimensions.layers > 1 )
            glTextureStorage2D( m_texture, in_dimensions.levels, m_internalformat, in_dimensions.width, in_dimensions.layers );
        else
            glTextureStorage1D( m_texture, in_dimensions.levels, m_internalformat, in_dimensions.width );
    } break;
    case TEXTURE_2D:
    {
        m_target = ( in_dimensions.layers > 1 ) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
        glCreateTextures( m_target, 1, &m_texture );
        if ( in_dimensions.layers > 1 )
            glTextureStorage3D( m_texture, in_dimensions.levels, m_internalformat, in_dimensions.width, in_dimensions.heigth, in_dimensions.layers );
        else
            glTextureStorage2D( m_texture, in_dimensions.levels, m_internalformat, in_dimensions.width, in_dimensions.heigth );
        
    } break;
    case TEXTURE_3D:
    {
        glCreateTextures( GL_TEXTURE_3D, 1, &m_texture );
        glTextureStorage3D( m_texture, in_dimensions.layers, m_internalformat, in_dimensions.width, in_dimensions.heigth, in_dimensions.depth );
    } break;
    case TEXTURE_CUBEMAP:
    {
        m_target = ( in_dimensions.layers > 1 ) ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
        glCreateTextures( m_target, 1, &m_texture );

        if( in_dimensions.layers > 1 )
            glTextureStorage3D( m_texture, in_dimensions.levels, m_internalformat, in_dimensions.width, in_dimensions.width, 6 );
        else
            glTextureStorage3D( m_texture, in_dimensions.levels, m_internalformat, in_dimensions.width, in_dimensions.width, in_dimensions.layers * 6 );            
    } break;
    default:
        common->Error( "Invalid texture type");
        break;
    };

    return true;
}

void glTexture::Destroy(void)
{
    if ( m_texture != 0 )
    {
        glDeleteTextures( 1, &m_texture );
        m_texture = 0;
    }
}

void glTexture::SubImage( const uint32_t in_alignament, const idList<subImage_t> &in_subImages )
{
    GLenum format = formatTable[m_format];
    GLenum type = dataType[m_format];
    idImageManagerLocal* imgl = dynamic_cast<idImageManagerLocal*>( idRenderSystem::GetGlobalImages() );
    GLuint pubo = *static_cast<GLuint*>( imgl->GetPixelUnpackBuffer()->Handle() ); 
    

    // bind the source buffer 
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pubo );

    // byte to byte copy 
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    for ( uint32_t i = 0; i < in_subImages.Num(); i++)
    {
        const auto& sub = in_subImages[i];
        
        glPixelStorei(GL_UNPACK_ROW_LENGTH, sub.width);         // pixels por linha
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, sub.height);      // para 3D ou arrays 
     
        switch ( m_target )
        {
        case GL_TEXTURE_1D:
            glTextureSubImage1D( m_texture, sub.level, 0, sub.width, format, type, reinterpret_cast<void*>(sub.offset) );
            break;
        case GL_TEXTURE_1D_ARRAY:
            glTextureSubImage2D( m_texture, sub.level, 0, 0, sub.width, sub.layer, format, type, reinterpret_cast<void*>(sub.offset) );
            break;
        case GL_TEXTURE_2D:
            glTextureSubImage2D( m_texture, sub.level, 0, 0, sub.width, sub.height, format, type, reinterpret_cast<void*>(sub.offset) );
            break;
        case GL_TEXTURE_2D_ARRAY:
            glTextureSubImage3D( m_texture, sub.level, 0, 0, 0, sub.width, sub.height, sub.layer, format, type, reinterpret_cast<void*>(sub.offset) );
            break;
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            glTextureSubImage3D( m_texture, sub.level, 0, 0, 0, sub.width, sub.height, sub.depth, format, type, reinterpret_cast<void*>(sub.offset) );
            break;
        default:
            break;
        }
    }
}

void *glTexture::Handler(void) const
{
    return const_cast<GLuint*>( &m_texture );
}
