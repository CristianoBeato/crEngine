/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
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

#ifndef __IMAGE_MANAGER_HPP__
#define __IMAGE_MANAGER_HPP__

class crBuffer;
struct pixelBuffer_t
{
	idSysInterlockedInteger	head;
	idSysInterlockedInteger	tail;
	crBuffer*				buffer;
};

#include "Image.h"

class idImageManagerLocal : public idImageManager
{
public:

	idImageManagerLocal( void );
	~idImageManagerLocal( void );
	
	virtual void				Init( void );
	virtual void				Shutdown( void );
	
	// If the exact combination of parameters has been asked for already, an existing
	// image will be returned, otherwise a new image will be created.
	// Be careful not to use the same image file with different filter / repeat / etc parameters
	// if possible, because it will cause a second copy to be loaded.
	// If the load fails for any reason, the image will be filled in with the default
	// grid pattern.
	// Will automatically execute image programs if needed.
	idImage* 			ImageFromFile( const idStr &name, const cubeFiles_t cubeMap = CF_2D );
									   
	// look for a loaded image, whatever the parameters
	idImage* 			GetImage( const idStr &name ) const;
	
	// look for a loaded image, whatever the parameters
	idImage* 			GetImageWithParameters( const idStr &name, const cubeFiles_t cubeMap ) const;
	
	// The callback will be issued immediately, and later if images are reloaded or vid_restart
	// The callback function should call one of the idImage::Generate* functions to fill in the data
	idImage* 			ImageFromFunction( const idStr &name, void ( *generatorFunction )( idImage* image ) );
	
	// scratch images are for internal renderer use.  ScratchImage names should always begin with an underscore
	idImage* 			ScratchImage( const idStr &name, const idImageOpts* imgOpts );
	
	// purges all the images before a vid_restart
	void				PurgeAllImages( void );
	
	/// @brief reloads all apropriate images after a vid_restart
	void				ReloadImages( const bool all );
	
	/// @brief bind image and sampler to texture location buffer
	/// @return the index of the sampler in sampler buffer
	uint32_t			BindSampler( const idImage* in_image, const crSampler* in_sampler );

	// unbind all textures from all texture units
	void				UnbindAll( void );
	
	// disable the active texture unit
	void				BindNull( void );
	
	// Called only by renderSystem::BeginLevelLoad
	virtual void		BeginLevelLoad( void );
	
	// Called only by renderSystem::EndLevelLoad
	virtual void 		EndLevelLoad( void );
	
	virtual void		Preload( const idPreloadManifest& manifest, const bool& mapPreload );
	
	// Loads unloaded level images
	int					LoadLevelImages( const bool pacifier );
	
	// used to clear and then write the dds conversion batch file
	void				StartBuild( void );
	void				FinishBuild( const bool removeDups = false );
	
	void				PrintMemInfo( MemInfo_t* mi );

/// BEATO Begin:
	virtual idImage*	DefaultImage( void ) const { return defaultImage; }
/// BEATO End

	// built-in images
	void CreateIntrinsicImages( void );

	idImage* 			defaultImage;
	idImage* 			flatNormalMap;				// 128 128 255 in all pixels
	idImage* 			alphaNotchImage;			// 2x1 texture with just 1110 and 1111 with point sampling
	idImage* 			whiteImage;					// full of 0xff
	idImage* 			blackImage;					// full of 0x00
	idImage* 			glossImage;					// 15 15 15 in all pixels
	idImage* 			noFalloffImage;				// all 255, but zero clamped
	idImage* 			fogImage;					// increasing alpha is denser fog
	idImage* 			fogEnterImage;				// adjust fogImage alpha based on terminator plane
	idImage*			cinematicImage;
	// RB begin
	idImage*			shadowImage[5];
	idImage*			jitterImage1;				// shadow jitter
	idImage*			jitterImage4;
	idImage*			jitterImage16;
	idImage*			randomImage256;
	// RB end
	idImage* 			scratchImage;
	idImage* 			scratchImage2;
	idImage* 			accumImage;
	idImage* 			currentRenderImage;				// for SS_POST_PROCESS shaders
	idImage* 			currentDepthImage;				// for motion blur
	idImage* 			originalCurrentRenderImage;		// currentRenderImage before any changes for stereo rendering
	idImage* 			loadingIconImage;				// loading icon must exist always
	idImage* 			hellLoadingIconImage;				// loading icon must exist always
	// foresthale 2014-02-19: added an HDR framebuffer object, which needs images to back it
	idImage*			viewFramebufferRenderImage16;
	idImage*			viewFramebufferDepthImage;
	idImage*			ditherImage;
	idImage*			cameraImage;
	// foresthale 2014-04-07: r_glow
	// we blend a series of progressively smaller (and more blurred) images over the view
	// the first is full resolution, the later ones are progressively smaller
	idImage*			glowFramebufferImage8[4];
	idImage*			glowFramebufferImage16[4];
	
	//--------------------------------------------------------
	
	idImage* 			AllocImage( const idStr &name );
	idImage* 			AllocStandaloneImage( const idStr &name );
	
	bool				ExcludePreloadImage( const idStr &name );
	
	idList<idImage*, TAG_IDLIB_LIST_IMAGE>	images;
	idHashIndex			imageHash;
	
	bool				insideLevelLoad;			// don't actually load images now
	bool				preloadingMapImages;		// unless this is set

// BEATO Begin:
	virtual crBuffer*	GetPixelUnpackBuffer( void ) const override { return m_pixelUnpack.buffer; };
	virtual crBuffer*	GetPixelPackBuffer( void ) const override { return m_pixelPack.buffer; }
	
	// z is 0 for 2D textures, 0 - 5 for cube maps, and 0 - uploadDepth for 3D textures. Only
	// one plane at a time of 3D textures can be uploaded. The data is assumed to be correct for
	// the format, either bytes, halfFloats, floats, or DXT compressed. The data is assumed to
	// be in OpenGL RGBA format, the consoles may have to reorganize. pixelPitch is only needed
	// when updating from a source subrect. Width, height, and dest* are always in pixels, so
	// they must be a multiple of four for dxt data.
	//crTexture::subImage_t	SubImageUpload( int mipLevel, int destX, int destY, int destZ, int width, int height, const void* data, int pixelPitch = 0 ) const;
// BEATO End
protected:
	friend class idImage;

	/// @brief Upload texel buffer to transfer buffer
	/// @param in_buffer the source texels 
	/// @param in_size the size of the source
	/// @return 
	uintptr_t	UploadTexels( const void* in_buffer, const size_t in_size );

	/// @brief Copy the texels from source buffer to destination image
	/// @param in_image the destination image 
	/// @param in_subimageList subimage properties
	void		UploadSubImages( const idImage* in_image, const uintptr_t in_baseOffset, const idList<sub_image_t> in_subimageList );

private:
	pixelBuffer_t	m_pixelPack;		// source texture staging buffer
	pixelBuffer_t	m_pixelUnpack;		// destination texture staging buffer
};

int MakePowerOfTwo( int num );

/*
====================================================================

IMAGEPROCESS

FIXME: make an "imageBlock" type to hold byte*,width,height?
====================================================================
*/
byte* R_Dropsample( const byte* in, int inwidth, int inheight, int outwidth, int outheight );
byte* R_ResampleTexture( const byte* in, int inwidth, int inheight, int outwidth, int outheight );
byte* R_MipMapWithAlphaSpecularity( const byte* in, int width, int height );
byte* R_MipMapWithGamma( const byte* in, int width, int height );
byte* R_MipMap( const byte* in, int width, int height );

// these operate in-place on the provided pixels
void R_BlendOverTexture( byte* data, int pixelCount, const byte blend[4] );
void R_HorizontalFlip( byte* data, int width, int height );
void R_VerticalFlip( byte* data, int width, int height );
void R_RotatePic( byte* data, int width );
void R_ApplyCubeMapTransforms( int i, byte* data, int size );

/*
====================================================================

IMAGEFILES

====================================================================
*/
extern void R_WriteTGA( const char* filename, const byte* data, int width, int height, bool flipVertical, const char* basePath );

void R_LoadImage( const char* name, byte** pic, int* width, int* height, ID_TIME_T* timestamp, bool makePowerOf2 );
// pic is in top to bottom raster format
bool R_LoadCubeImages( const char* cname, cubeFiles_t extensions, byte* pic[6], int* size, ID_TIME_T* timestamp );

/*
====================================================================

IMAGEPROGRAM

====================================================================
*/

void R_LoadImageProgram( const char* name, byte** pic, int* width, int* height, ID_TIME_T* timestamp );
const char* R_ParsePastImageProgram( idLexer& src );

#endif //!__IMAGE_MANAGER_HPP__