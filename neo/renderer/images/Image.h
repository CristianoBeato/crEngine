/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Softwa              re LLC, a ZeniMax Media company.
Copyright (C) 2014-2016 Robert Beckebans
Copyright (C) 2014-2016 Kot in Action Creative Artel

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "renderer/Vulkan/Core.hpp"

/*
====================================================================

IMAGE

idImage have a one to one correspondance with Vulkan textures.

No texture is ever used that does not have a corresponding idImage.

====================================================================
*/
inline constexpr int MAX_TEXTURE_LEVELS = 14;
inline constexpr int DEFAULT_SIZE = 16;

inline bool IsToolUsage( textureUsage_t usage )
{
	return usage == TD_EDITOR_DEFAULT || 
		usage == TD_EDITOR_DIFFUSE || 
		usage == TD_EDITOR_BUMP || 
		usage == TD_EDITOR_COVERAGE;
}

textureUsage_t CheckEditorUsage( textureUsage_t usage );

struct sub_image_t
{
    uint16_t    level;  // mipmap level
    uint16_t    layer;  // layer of the multi texture
    uint32_t    width;  // face width 
    uint32_t    height; // face height
    uint32_t    depth;  // face depth
    uintptr_t   offset; // offset in texture buffer
    size_t      size;   // pixel legenth
};

/*
================================================
idImageOpts hold parameters for texture operations.
================================================
*/
class idImageOpts
{
public:
	idImageOpts( void ) : 
		format( crInternalFormat::NONE ),
		width( 0 ),
		height( 0 ),
		depth( 0 ),
		numSides( 0 ),
		numLevels( 0 ),
		textureType( IMAGE_2D ),
		gammaMips( false ),
		readback( false ),
		sRGB( false ) // foresthale 2014-02-20: fixed r_useSRGB texture handling
	{
	}
	
	ID_INLINE bool	operator==( const idImageOpts& opts )
	{
		return ( std::memcmp( this, &opts, sizeof( *this ) ) == 0 );
	}
	
	//---------------------------------------------------
	// these determine the physical memory size and layout
	//---------------------------------------------------
	bool				gammaMips;		// if true, mips will be generated with gamma correction
	bool				readback;		// 360 specific - cpu reads back from this texture, so allocate with cached memory
	bool				sRGB; 			// foresthale 2014-02-20: fixed r_useSRGB texture handling
	image_type_t		textureType;
	crInternalFormat	format;
	int16_t				numSides; 		// 6 cubemap / n shadow array
	int16_t				numLevels;		// if 0, will be 1 for NEAREST / LINEAR filters, otherwise based on size
	uint32_t			width;
	uint32_t			height;			// not needed for cube maps
	uint32_t			depth;
};

#define	MAX_IMAGE_NAME	256

class vkTexture;
class idImage
{
public:
	idImage( const idStr &name );
	
	ID_INLINE const char* 	GetName( void ) const
	{
		return imgName;
	}
	
	/// @brief Makes this image active on the current GL texture unit.
	/// automatically enables or disables cube mapping
	/// May perform file loading if the image was not preloaded.
	void		Bind( void );
		
	/// @brief used by callback functions to specify the actual data
	/// data goes from the bottom to the top line of the image, as OpenGL expects it
	/// These perform an implicit Bind() on the current texture unit
	/// FIXME: should we implement cinematics this way, instead of with explicit calls?
	void		GenerateImage( const byte* pic, const uint32_t width, const uint32_t height, textureUsage_t usage );
	void		GenerateCubeImage( const byte* pic[6], int size, textureUsage_t usage );
								   
	// RB begin
	void		GenerateShadowArray( uint32_t width, uint32_t height, textureUsage_t usage );
	// RB end
	
	void		CopyFramebuffer( int32_t x, int32_t y, uint32_t width, uint32_t height );
	void		CopyDepthbuffer( int32_t x, int32_t y, uint32_t width, uint32_t height );
	
	void		UploadScratch( const byte* pic, int width, int height );
	
	/// @brief estimates size of the image based on dimensions and storage type
	size_t		StorageSize( void ) const;
	
	/// @brief print a one line summary of the image
	void		Print( void ) const;
	
	/// @brief check for changed timestamp on disk and reload if necessary
	void		Reload( const bool force );
	
	ID_INLINE void	AddReference( void ) { refCount++; };
	
	/// @brief fill with a grid pattern
	void		MakeDefault( void );	
	
	void		ActuallyLoadImage( bool fromBackEnd );

	//---------------------------------------------
	// Platform specific implementations
	//---------------------------------------------
	void		AllocImage( const idImageOpts& imgOpts );
	
	/// @brief Deletes the texture object, but leaves the structure so it can be reloaded
	/// or resized.
	void		PurgeImage( void );
								
	/// @brief SetPixel is assumed to be a fast memory write on consoles, degenerating to a
	/// SubImageUpload on PCs.  Used to update the page mapping images.
	/// We could remove this now, because the consoles don't use the intermediate page mapping
	/// textures now that they can pack everything into the virtual page table images.
	void		SetPixel( int mipLevel, int x, int y, const void* data, int dataSize );
	
	/// @brief some scratch images are dynamically resized based on the display window size.  This
	/// simply purges the image and recreates it if the sizes are different, so it should not be
	/// done under any normal circumstances, and probably not at all on consoles.
	void		Resize( const uint32_t width, const u_int32_t height );

	void		SetTexParameters( void );	// update aniso and trilinear

	void		BindAttachmentOnFBO(int attachmentType, int layer = -1);
		
	ID_INLINE bool IsCompressed( void ) const
	{
		return opts.format.Compressed();
	}
	
	ID_INLINE bool IsLoaded( void ) const
	{
		return texnum != nullptr;
	}

	ID_INLINE const idImageOpts& GetOpts( void ) const
	{
		return opts;
	}

	ID_INLINE int GetUploadWidth( void ) const
	{
		return opts.width;
	}

	ID_INLINE int GetUploadHeight( void ) const
	{
		return opts.height;
	}

	ID_INLINE vkTexture* GetTexnum( void ) const
	{
		return texnum;
	}
	
	ID_INLINE void SetReferencedOutsideLevelLoad( void )
	{
		referencedOutsideLevelLoad = true;
	}
	
	ID_INLINE void SetReferencedInsideLevelLoad( void )
	{
		levelLoadReferenced = true;
	}
	
protected:
	friend class idImageManagerLocal;
	void				AllocImage( void );
	void				DeriveOpts( void );
	idList<sub_image_t>	GetSubImages( void ) const { return subimages; }

private:
	bool				referencedOutsideLevelLoad;
	bool				levelLoadReferenced;	// for determining if it needs to be purged
	bool				defaulted;				// true if the default image was generated because a file couldn't be loaded
	int					refCount;				// overall ref count
	ID_TIME_T			sourceFileTime;			// the most recent of all images used in creation, for reloadImages command
	ID_TIME_T			binaryFileTime;			// the time stamp of the binary file

	
	///
	/// parameters that define this image
	///
	
	// If this is a cube map, and if so, what kind
	cubeFiles_t						cubeFiles;				
	// Used to determine the type of compression to use
	textureUsage_t					usage;					
	
	// Parameters that determine the storage method
	idImageOpts						opts;					

	// game path, including extension (except for cube maps), may be an image program
	idStr							imgName;
	vkTexture*						texnum;
	idList<sub_image_t>				subimages;
	
	// nullptr for files
	void	( *generatorFunction )( idImage* image );	

// BEATO Begin: Upload/Dowload images to image memory

	/// @brief 
	void	DoUpload( void );

	/// @brief 
	void	DoDownload( void );
	
// BEATO End

};

#endif //!__IMAGE_H__