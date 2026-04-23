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

#pragma hdrstop
#include "precompiled.h"
#include "renderer_common.h"
#include "BTFTextureFile.hpp"

static inline crInternalFormat ToInternalFormat( const BTF::format_t in_format, const bool sRGB )
{
    switch ( in_format )
    {
        case BTF::FORMAT_RED8U:
            return sRGB ? crInternalFormat::R8U_SRGB : crInternalFormat::R8U;

        case BTF::FORMAT_RED16U:
            return crInternalFormat::R16U;

        case BTF::FORMAT_RG8U:
            return sRGB ? crInternalFormat::RG8U_SRGB : crInternalFormat::RG8U;

        case BTF::FORMAT_RG16U:
            return crInternalFormat::RG16U;

        case BTF::FORMAT_RGBA8U:
            return sRGB ? crInternalFormat::RGBA8U_SRGB : crInternalFormat::RGBA8U;

        case BTF::FORMAT_RGBA16U:
            return crInternalFormat::RGBA16U;

        case BTF::FORMAT_RGBA16F:
            return crInternalFormat::RGBA16F;

        case BTF::FORMAT_RGBA32U:
            return crInternalFormat::RGBA32U;

        case BTF::FORMAT_RGBA32F:
            return crInternalFormat::RGBA32F;

        case BTF::FORMAT_BC1:
            return sRGB ? crInternalFormat::BC1_SRGB : crInternalFormat::BC1_RGB;

        case BTF::FORMAT_BC3:
            return sRGB ? crInternalFormat::BC3_SRGBA : crInternalFormat::BC3_RGBA;

        case BTF::FORMAT_BC5:
            return crInternalFormat::BC5_RG;

        case BTF::FORMAT_BC7:
            return sRGB ? crInternalFormat::BC7_SRGBA : crInternalFormat::BC7_RGBA;

        case BTF::FORMAT_BC6H:
            return crInternalFormat::BC6H_RGBA;

        case BTF::FORMAT_ETC2:
            return sRGB ? crInternalFormat::ETC2_SRGBA : crInternalFormat::ETC2_RGBA;

        case BTF::FORMAT_EAC:
            return crInternalFormat::RG_EAC_RG;
    }

    return crInternalFormat::NONE;
}  

/*
================================================================================================
	crBTFTextureFile: Beato Texture File Format 
================================================================================================
*/
crBTFTextureFile::crBTFTextureFile(void)
{
}

/*
========================
crBTFTextureFile::crBTFTextureFile
========================
*/

crBTFTextureFile::crBTFTextureFile(const idStr &in_name) : m_name(in_name)
{
}

/*
========================
crBTFTextureFile::~crBTFTextureFile
========================
*/
crBTFTextureFile::~crBTFTextureFile( void )
{
}

bool crBTFTextureFile::Open(void)
{
    // try load file
    idFileLocal bFile = fileSystem->OpenFileRead( m_name );
    if( bFile == nullptr )
    {
        common->Warning( "Can't load texture file: %s\n", m_name.c_str() );
        return false;
    }

    /// Read texture file header
    if( bFile->Read( &m_header, BTF::HEADER_SIZE ) <= 0 )
    {
        common->Warning( "Invalid texture file %s ( file size is < than %i bytes, BTF header size )\n", m_name.c_str(), BTF::HEADER_SIZE );
        return false;
    }

    if ( m_header.magic != BTF::MAGIC )
    {
        common->Warning( "Inavlid texture file %s ( invalid header magic %i got %i )", m_name.c_str(), BTF::MAGIC, m_header.magic );
        return false;
    }
    
    if ( m_header.version < BTF::VERSION || m_header.magic > BTF::VERSION )
    {
        common->Warning( "Invalid texture version: texture %s got %i suported %i\n", m_name.c_str(), m_header.version, BTF::VERSION );
        return false;
    }
    
    /// load images array
    m_images.Resize( m_header.layerCount * m_header.mipCount );

    /// move file pointer to the image array, and read images  
    bFile->Seek( m_header.subImageTableOffset, FS_SEEK_SET );
    bFile->Read( m_images.Ptr(), m_images.MemoryUsed() );

    /// allocate memory for the texel buffer, alingated
    m_texels = SDL_aligned_alloc( m_header.dataAlignment, m_header.pixelBufferSize );

    bFile->Seek( m_header.pixelDataOffset, FS_SEEK_SET );
    if( bFile->Read( m_texels, m_header.pixelBufferSize ) < m_header.pixelBufferSize )
    {
        SDL_aligned_free( m_texels );
        common->Warning( "Invalid texture file data: %s file pixel buffer size\n", m_name.c_str() );
    }

    // TODO: check file CRC    

    return true;
}

idImageOpts crBTFTextureFile::GetImageParameters(void) const
{
    idImageOpts opts{};

    opts.gammaMips = true;
    opts.readback = false;
    opts.sRGB = m_header.imageFlags & BTF::SRGB;    /// sRGB 
    
    if ( m_header.imageFlags & BTF::IMAGE1D )
    {
        opts.textureType = crTexture::IMAGE_1D;
        opts.width = m_images[0].width;
        opts.height = 0;
        opts.depth = 0;
    }
    else if ( m_header.imageFlags & BTF::IMAGE2D )
    {
        opts.textureType = crTexture::IMAGE_2D;
        opts.width = m_images[0].width;
        opts.height = m_images[0].height;
        opts.depth = 0;
    }
    else if ( m_header.imageFlags & BTF::IMAGE3D )
    {
        opts.textureType = crTexture::IMAGE_3D;
        opts.width = m_images[0].width;
        opts.height = m_images[0].height;
        opts.depth = m_images[0].depth;
    }
    else if ( m_header.imageFlags & BTF::CUBEMAP )
    {
        opts.textureType = crTexture::IMAGE_CUBEMAP;
        opts.width = m_images[0].width;
        opts.height = m_images[0].width;
    }
    
    /// get the internal pixel format
    opts.format = ToInternalFormat( static_cast<BTF::format_t>( m_header.pixelFormat ), opts.sRGB );
    
    /// layers 
    opts.numSides = m_header.layerCount;
    
    /// mipma count 
    opts.numLevels = m_header.mipCount;
    
    return opts;
}
