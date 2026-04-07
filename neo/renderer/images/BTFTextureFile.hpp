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

#ifndef __BRF_TEXTURE_FILE_HPP__
#define __BRF_TEXTURE_FILE_HPP__

/// @CristianoBeato: implement custom texture format, 
/// inspirate by valve .VTF ( use a custom texture format )
namespace BTF
{
	/// @brief image pixel depth format
	enum format_t : uint16_t
	{
		FORMAT_RED8U,   // 8  bits unsigned 1 component ( red only )
		FORMAT_RED16U,  // 16 bits unsigned 1 component ( rend only ) 
		FORMAT_RG8U,    // 8  bits unsigned 2 components ( red green ) 
		FORMAT_RG16U,   // 16 bits unsigned 2 components ( red green ) 
		FORMAT_RGBA8U,  // 8  bits unsigned 4 components ( RGB + Alpha )
		FORMAT_RGBA16U, // 16 bits unsigned 4 components ( RGB + Alpha )
		FORMAT_RGBA16F, // 16 bits half float 4 components ( RGB + Alpha )
		FORMAT_RGBA32U, // 32 bits unsigned 4 components ( RGB + Alpha )
		FORMAT_RGBA32F, // 32 bits true float 4 components ( RGB + Alpha )
		FORMAT_BC1,     // 8 bytes 4×4 block compression 3 components ( RGB )
		FORMAT_BC3,     // BC3 stores RGBA data, using BC1 for the RGB part and BC4 for the alpha part,
		FORMAT_BC5,     // 
		FORMAT_BC7,     //
		FORMAT_BC6H,    //
		FORMAT_ETC2,    //
		FORMAT_EAC      //
	};

	enum flags_t : uint32_t
	{
	    IMAGE1D = 0x0001,   /// Image 1D
	    IMAGE2D = 0x0002,   /// Image 2D
	    IMAGE3D = 0x0004,   /// Image 3D
	    CUBEMAP = 0x0008,   /// Cubemap texture
	    ARRAY   = 0x0010,   /// Array Image
	    SRGB    = 0x0020,   /// sRGB color range
	};

	typedef struct Header_s
	{
		uint32_t magic = 0;         // 'BTF\0'
		uint16_t version = 0;       // versionamento é obrigatório
		uint16_t headerSize = 0;    // 
		uint32_t imageFlags = 0;    //

		// the format suported
		uint16_t pixelFormat = 0;   //
		uint16_t pad0;              //

		uint16_t layerCount = 0;    //
		uint16_t mipCount = 0;      //
		
		uint32_t subImageTableOffset = 0;
		uint32_t pixelDataOffset = 0;
		uint32_t pixelBufferSize = 0;

		uint32_t dataAlignment = 0; // ex: 16 ou 256
	} Header_t;

	typedef struct alignas( 16 ) Image_s 
	{
	    uint32_t    width = 0;
	    uint32_t    height = 0;
	    uint32_t    depth = 0; 
	    uint32_t    padding0;   
	    uint32_t    offset = 0;
	    uint32_t    size = 0;
	} Image_t;

	typedef struct Footer_s
	{
	    uint64_t contentHash;   // CRC64 ou xxHash
	    uint32_t nameOffset;    // string opcional no fim do arquivo
	} Footer_t;

	inline constexpr uint32_t   MAGIC = { 'BTF\0' }; /// 0x00465442u; // "BTF\0"
	inline constexpr uint32_t   VERSION = 10; // 1.0 
	inline constexpr size_t     HEADER_SIZE = sizeof( Header_t );
	inline constexpr size_t     SUBIMAGE_SIZE = sizeof( Image_t );
};
/// Beato end


/// @CristianoBeato 
class crBTFTextureFile
{	
public:
	crBTFTextureFile( void );
	crBTFTextureFile( const idStr &in_name );
	~crBTFTextureFile( void );

	bool	Open( void );

	idImageOpts				GetImageParameters( void ) const;

	/// 
	ID_INLINE const char*	GetName( void ) const { return m_name.c_str(); }
	
	///
	ID_INLINE void			SetName( const char* in_name ) { m_name = in_name; }

	/// 
	ID_INLINE uint32_t		NumImages( void ) const { return m_images.Num(); }

    /// 
    ID_INLINE uint32_t      DataAlignment( void ) const { return m_header.dataAlignment; }

    ///
    ID_INLINE size_t        DataSize( void ) const { return m_header.pixelBufferSize; }

private:
	idStr										m_name;
	BTF::Header_t								m_header;
	idList<BTF::Image_t, TAG_IDLIB_LIST_IMAGE>	m_images;
    void*                                       m_texels;
};

#endif //!__BRF_TEXTURE_FILE_HPP__