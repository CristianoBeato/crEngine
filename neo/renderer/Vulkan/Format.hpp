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

#ifndef __FORMAT_HPP__
#define __FORMAT_HPP__

/// pixel color format representation
typedef struct crInternalFormat
{
    enum format_t : uint8_t
    {
        NONE,

        // Red only
        R8U,            // 8  bits unsigned 1 component ( red only )
        R8U_SRGB,       // 8  bits unsigned 1 component ( red only ) sRGB color space
        R16U,           // 16 bits unsigned 1 component ( rend only )

        // Red green
        RG8U,           // 8  bits unsigned 2 components ( red green )
        RG8U_SRGB,      // 8  bits unsigned 2 components ( red green ) sRGB color space
        RG16U,       // 16 bits unsigned 2 components ( red green )   

        // RGBA colors
        RGBA8U,         // 8  bits unsigned 4 components ( RGB + Alpha )
        RGBA8U_SRGB,    // 8  bits unsigned 4 components ( RGB + Alpha ) sRGB color space
        RGBA16U,        // 16 bits unsigned 4 components ( RGB + Alpha )
        RGBA16F,        // 16 bits half float 4 components ( RGB + Alpha )
        RGBA32U,        // 32 bits unsigned 4 components ( RGB + Alpha )
        RGBA32F,        // 32 bits true float 4 components ( RGB + Alpha )
        
        // Depth component 
        DEPTH16,
        DEPTH24,
        DEPTH32,

        // Depth Stencil component 
        DEPTH24_STENCIL8,
        DEPTH32_STENCIL8,

        // Packed colors
        RGB565,  // r5 g6 b5 16 3 components

        // Compresed colors
        BC1_RGB,    // 4 bits per pixel (bpp), 4×4 block, RGB (no Alpha), 6:1 compression ratio ( DXT1 )
        BC1_SRGB,   // 4 bits per pixel (bpp), 4×4 block, RGB (no Alpha), 6:1 compression ratio  ( DXT1 ) sRGB color space
        BC3_RGBA,   // 8 bits per pixel (bpp), 4×4 block, RGBA, high quality Alpha (interpolated) ( DXT5 )
        BC3_SRGBA,  // 8 bits per pixel (bpp), 4×4 block, RGBA, high quality Alpha (interpolated) sRGB color space ( DXT5 )
        BC5_RG,     // 8 bits per pixel (bpp), 4×4 block, 2 components ( RG ), ideal for Normal Maps ( RGTC2 )
        BC7_RGBA,   // 8 bits per pixel (bpp), 4×4 block, high quality RGBA, reduces artifacts in gradients ( BPTC )
        BC7_SRGBA,  // 8 bits per pixel (bpp), 4×4 block, high quality RGBA, reduces artifacts in gradients sRGB color space ( BPTC )
        BC6H_RGBA,  // 8 bits per pixel (bpp), 4×4 block, RGB Half Float ( HDR support )
        ETC2_RGBA,  // 8 bits per pixel (bpp), 4×4 block, 4 components ( RGBA ) 
        ETC2_SRGBA, // 8 bits per pixel (bpp), 4×4 block, 4 components ( RGBA ), sRGB color space 
        RG_EAC_RG,  // 8 bits per pixel (bpp), 4×4 block, 2 components ( RG ), high precision (11 bits per channel)
        FORMAT_COUNT
    };

    crInternalFormat( void ) : format( NONE )
    {
    }

    crInternalFormat( const format_t &in_format ) : format( in_format )
    {
    }

// Just prevent missing enum outside of render library
#ifdef __RENDERER_LIB__
    crInternalFormat( const VkFormat &in_format );
    crInternalFormat    operator=( const VkFormat in_format );
    VkFormat            VKInternal( void ) const;
    operator            VkFormat( void ) const { return VKInternal(); }
#endif //__RENDERER_LIB__ 

    operator    format_t( void ) const { return format; }

    /// @brief  Return true if are a compressed format
    bool        Compressed( void ) const;

    /// @brief  number of bit per pixel  
    uint8_t     BitsPerPixel( void )const; 
    
    float       BytesPerPixel( void ) const;

    format_t format;
} crInternalFormat;


#endif //!__FORMAT_HPP__