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

#pragma hdrstop
#include "precompiled.h"
#include "renderer/renderer_common.h"
#include "BTFTextureFile.hpp"
#include "Image.h"

/*
==============
idImage::idImage
==============
*/
idImage::idImage( const idStr &name ) : imgName( name )
{
	texnum = nullptr;
	generatorFunction = nullptr;

	usage = TD_DEFAULT;
	cubeFiles = CF_2D;
	
	referencedOutsideLevelLoad = false;
	levelLoadReferenced = false;
	defaulted = false;
	sourceFileTime = FILE_NOT_FOUND_TIMESTAMP;
	binaryFileTime = FILE_NOT_FOUND_TIMESTAMP;
	refCount = 0;
}

/*
==============
Bind

Automatically enables 2D mapping or cube mapping if needed
==============
*/
void idImage::Bind( void )
{
	RENDERLOG_PRINTF( "idImage::Bind( %s )\n", GetName() );
	
	// load the image if necessary (FIXME: not SMP safe!)
	if( !IsLoaded() )
	{
		// load the image on demand here, which isn't our normal game operating mode
		ActuallyLoadImage( true );
	}
	
	const int texUnit = backEnd.trState.currenttmu;
	
	tmu_t* tmu = &backEnd.trState.tmu[texUnit];
	// bind the texture
	if( opts.textureType == TT_2D )
	{
		if( tmu->current2DMap != texnum )
		{
			tmu->current2DMap = texnum;
			glBindTextureUnit( texUnit, texnum );
		}
	}
	else if( opts.textureType == TT_CUBIC )
	{
		if( tmu->currentCubeMap != texnum )
		{
			tmu->currentCubeMap = texnum;
			//glBindMultiTextureEXT( GL_TEXTURE0 + texUnit, GL_TEXTURE_CUBE_MAP, texnum );
			glBindTextureUnit( texUnit, texnum );
		}
	}
	else if( opts.textureType == TT_2D_ARRAY )
	{
		if( tmu->current2DArray != texnum )
		{
			tmu->current2DArray = texnum;
			
			// RB begin
			glBindTextureUnit( texUnit, texnum );
		}
	}
}

/*
================
GenerateImage
================
*/
void idImage::GenerateImage( const byte* pic, const uint32_t width, const uint32_t height, textureUsage_t usageParm )
{
	PurgeImage();
	
	usage = usageParm;
	cubeFiles = CF_2D;
	
	opts.textureType = crTexture::TEXTURE_2D;
	opts.width = width;
	opts.height = height;
	opts.numLevels = 0;
	DeriveOpts();
	
	// if we don't have a rendering context, just return after we
	// have filled in the parms.  We must have the values set, or
	// an image match from a shader before the render starts would miss
	// the generated texture
	if( !idRenderSystem::IsInitialized() )
		return;
	
	const bool toolUsage = IsToolUsage( usageParm );

	idBinaryImage im( GetName() );

	// foresthale 2014-05-30: give a nice progress display when binarizing
	commonLocal.LoadPacifierBinarizeFilename(GetName() , "generated image");
	if (opts.numLevels > 1)
		commonLocal.LoadPacifierBinarizeProgressTotal(opts.width * opts.height * 4 / 3);
	else
		commonLocal.LoadPacifierBinarizeProgressTotal(opts.width * opts.height);
	im.Load2DFromMemory( width, height, pic, opts.numLevels, opts.format, opts.colorFormat, opts.gammaMips, toolUsage );
	commonLocal.LoadPacifierBinarizeEnd();
	
	AllocImage();
	
	for( int i = 0; i < im.NumImages(); i++ )
	{
		const bimageImage_t& img = im.GetImageHeader( i );
		const byte* data = im.GetImageData( i );
		SubImageUpload( img.level, 0, 0, img.destZ, img.width, img.height, data );
	}
}

/*
====================
GenerateCubeImage

Non-square cube sides are not allowed
====================
*/
void idImage::GenerateCubeImage( const byte* pic[6], int size, textureUsage_t usageParm )
{
	PurgeImage();
	
	usage = usageParm;
	cubeFiles = CF_NATIVE;
	
	opts.textureType = TT_CUBIC;
	opts.width = size;
	opts.height = size;
	opts.numLevels = 0;
	opts.format = FMT_DXT5;
	DeriveOpts();
	
	// if we don't have a rendering context, just return after we
	// have filled in the parms.  We must have the values set, or
	// an image match from a shader before the render starts would miss
	// the generated texture
	if( !idRenderSystem::IsInitialized() )
		return;

	const bool toolUsage = IsToolUsage( usageParm );

	idBinaryImage im( GetName() );
	// foresthale 2014-05-30: give a nice progress display when binarizing
	commonLocal.LoadPacifierBinarizeFilename( GetName(), "generated cube image" );
	if (opts.numLevels > 1)
		commonLocal.LoadPacifierBinarizeProgressTotal(opts.width * opts.width * 6 * 4 / 3);
	else
		commonLocal.LoadPacifierBinarizeProgressTotal(opts.width * opts.width * 6);
	im.LoadCubeFromMemory( size, pic, opts.numLevels, opts.format, opts.colorFormat, opts.gammaMips, toolUsage, usage );
	commonLocal.LoadPacifierBinarizeEnd();
	
	AllocImage();
	
	for( int i = 0; i < im.NumImages(); i++ )
	{
		const bimageImage_t& img = im.GetImageHeader( i );
		const byte* data = im.GetImageData( i );
		SubImageUpload( img.level, 0, 0, img.destZ, img.width, img.height, data );
	}
}

// RB begin
void idImage::GenerateShadowArray(uint32_t width, uint32_t height, textureUsage_t usage)
{
	PurgeImage();
	
	cubeFiles = CF_2D_ARRAY;
	
	opts.textureType = IMAGE_2D;
	opts.width = width;
	opts.height = height;
	opts.numLevels = 0;
	DeriveOpts();
	
	// if we don't have a rendering context, just return after we
	// have filled in the parms.  We must have the values set, or
	// an image match from a shader before the render starts would miss
	// the generated texture
	if( !idRenderSystem::IsInitialized() )
		return;
	
	//idBinaryImage im( GetName() );
	//im.Load2DFromMemory( width, height, pic, opts.numLevels, opts.format, opts.colorFormat, opts.gammaMips );
	
	AllocImage();
	
	/*
	for( int i = 0; i < im.NumImages(); i++ )
	{
		const bimageImage_t& img = im.GetImageHeader( i );
		const byte* data = im.GetImageData( i );
		SubImageUpload( img.level, 0, 0, img.destZ, img.width, img.height, data );
	}
	*/
}
// RB end

/*
====================
CopyFramebuffer
====================
*/
void idImage::CopyFramebuffer( int32_t x, int32_t y, uint32_t imageWidth, uint32_t imageHeight )
{
#if 0
	// foresthale 2014-03-02: fixup incorrectly created render target textures - using a mipmapped texture as destination of a CopyFramebuffer yields a bogus mipmap chain, which works on NVIDIA and older AMD drivers (13.1 and below) but crashes on current AMD drivers (13.12 as of this writing).
	if ( opts.numLevels != 1 || filter != TF_LINEAR || repeat != TR_CLAMP )
	{
		common->Warning("idImage::CopyFramebuffer used on image \"%s\" which was not created for the purpose (should be TF_LINEAR, TR_CLAMP so that it has NO MIPMAPS) - this would render incorrectly (NVIDIA) or crash (AMD) in stock BFG Edition!", GetName());
		opts.numLevels = 1;
		filter = TF_LINEAR;
		repeat = TR_CLAMP;
		AllocImage();
	}

	glBindTexture( ( opts.textureType == TT_CUBIC ) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, texnum );

	// foresthale 2014-02-20: HDR view rendering - this seems to not be changed anywhere and conflicts with FBO rendering
	//glReadBuffer( GL_BACK );
	
	opts.width = imageWidth;
	opts.height = imageHeight;
	// foresthale 2014-02-20: HDR view rendering
	glCopyTexImage2D( GL_TEXTURE_2D, 0, opts.format == FMT_RGBA16F ? GL_RGBA16F : GL_RGBA8, x, y, imageWidth, imageHeight, 0 );
	
	// these shouldn't be necessary if the image was initialized properly
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
#else
#endif

	backEnd.pc.c_copyFrameBuffer++;
}

/*
====================
CopyDepthbuffer
====================
*/
void idImage::CopyDepthbuffer( int32_t x, int32_t y, uint32_t imageWidth, uint32_t imageHeight )
{
#if 0
	glBindTexture( ( opts.textureType == TT_CUBIC ) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, texnum );
	
	opts.width = imageWidth;
	opts.height = imageHeight;
	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, x, y, imageWidth, imageHeight, 0 );
#endif
	backEnd.pc.c_copyFrameBuffer++;
}

/*
=============
RB_UploadScratchImage

if rows = cols * 6, assume it is a cube map animation
=============
*/
void idImage::UploadScratch( const byte* data, int cols, int rows )
{
	// if rows = cols * 6, assume it is a cube map animation
	if( rows == cols * 6 )
	{
		rows /= 6;
		const byte* pic[6];
		for( int i = 0; i < 6; i++ )
		{
			pic[i] = data + cols * rows * 4 * i;
		}
		
		if( opts.textureType != TT_CUBIC || usage != TD_LOOKUP_TABLE_RGBA )
		{
			GenerateCubeImage( pic, cols, TF_LINEAR, TD_LOOKUP_TABLE_RGBA );
			return;
		}

		if( opts.width != cols || opts.height != rows )
		{
			opts.width = cols;
			opts.height = rows;
			AllocImage();
		}

		SetSamplerState( TF_LINEAR, TR_CLAMP );
		for( int i = 0; i < 6; i++ )
		{
			SubImageUpload( 0, 0, 0, i, opts.width, opts.height, pic[i] );
		}
		
	}
	else
	{
		if( opts.textureType != TT_2D || usage != TD_LOOKUP_TABLE_RGBA )
		{
			GenerateImage( data, cols, rows, TF_LINEAR, TR_REPEAT, TD_LOOKUP_TABLE_RGBA );
			return;
		}

		if( opts.width != cols || opts.height != rows )
		{
			opts.width = cols;
			opts.height = rows;
			AllocImage();
		}
		
		SetSamplerState( TF_LINEAR, TR_REPEAT );
		SubImageUpload( 0, 0, 0, 0, opts.width, opts.height, data );
	}
}

/*
==================
StorageSize
==================
*/
size_t idImage::StorageSize( void ) const
{
	if( !IsLoaded() )
		return 0;
	
	uint32_t baseSize = opts.width * opts.height;
	if( opts.numLevels > 1 )
	{
		baseSize *= 4;
		baseSize /= 3;
	}

	baseSize *= opts.format.BitsPerPixel();
	
	baseSize /= 8;
	
	return baseSize;
}

/*
==================
Print
==================
*/
void idImage::Print( void ) const
{
	if( generatorFunction )
		common->Printf( "F" );
	else
		common->Printf( " " );
	
	switch( opts.textureType )
	{
		case crTexture::TEXTURE_2D:
			common->Printf( " " );
			break;
		case crTexture::TEXTURE_CUBEMAP:
			common->Printf( "C" );
			break;
		default:
			common->Printf( "<BAD TYPE:%i>", opts.textureType );
			break;
	}
	
	common->Printf( "%4i %4i ",	opts.width, opts.height );
	
	switch( (crInternalFormat::format_t)opts.format )
	{
#define NAME_FORMAT( x ) case crInternalFormat::x: common->Printf( "%-6s ", #x ); break;
			NAME_FORMAT( NONE );
			NAME_FORMAT( R8U );
			NAME_FORMAT( R8U_SRGB );
			NAME_FORMAT( R16U );
			NAME_FORMAT( RG8U );
			NAME_FORMAT( RG8U_SRGB );
			NAME_FORMAT( RG16U );
			NAME_FORMAT( RGBA8U );
			NAME_FORMAT( RGBA8U_SRGB );
			NAME_FORMAT( RGBA16U );
			NAME_FORMAT( RGBA16F );
			NAME_FORMAT( RGBA32U );
			NAME_FORMAT( RGBA32F );
			NAME_FORMAT( DEPTH16 );
			NAME_FORMAT( DEPTH24 );
			NAME_FORMAT( DEPTH32 );
			NAME_FORMAT( DEPTH24_STENCIL8 );
			NAME_FORMAT( DEPTH32_STENCIL8 );
			NAME_FORMAT( RGB565 );
			NAME_FORMAT( BC1_RGB );
			NAME_FORMAT( BC1_SRGB );
			NAME_FORMAT( BC3_RGBA );
			NAME_FORMAT( BC3_SRGBA );
			NAME_FORMAT( BC5_RG );
			NAME_FORMAT( BC7_RGBA );
			NAME_FORMAT( BC7_SRGBA );
			NAME_FORMAT( BC6H_RGBA );
			NAME_FORMAT( ETC2_RGBA );
			NAME_FORMAT( ETC2_SRGBA );
			NAME_FORMAT( RG_EAC_RG );
		default:
			common->Printf( "<%3i>", opts.format );
			break;
	}
		
	common->Printf( "%4ik ", StorageSize() / 1024 );
	
	common->Printf( " %s\n", GetName() );
}

/*
===============
idImage::Reload
===============
*/
void idImage::Reload( const bool force )
{
	// always regenerate functional images
	if( generatorFunction )
	{
		common->DPrintf( "regenerating %s.\n", GetName() );
		if ( opts.textureType != TT_2D_ARRAY && cubeFiles != CF_2D_ARRAY ) // motorsep 12-18-2014; we don't need to regenerate shadowmaps (maybe even none of the functional images) since they aren't being modified
			generatorFunction( this ); 
		return;
	}
	
	// check file times
	if( !force )
	{
		ID_TIME_T current;
		if( cubeFiles != CF_2D )
		{
			R_LoadCubeImages( imgName, cubeFiles, nullptr, nullptr, &current );
		}
		else
		{
			// get the current values
			R_LoadImageProgram( imgName, nullptr, nullptr, nullptr, &current );
		}
		
		if( current <= sourceFileTime )
			return;
	}
	
	common->DPrintf( "reloading %s.\n", GetName() );
	
	PurgeImage();
	
	// Load is from the front end, so the back end must be synced
	ActuallyLoadImage( false );
}

/*
==================
idImage::MakeDefault

the default image will be grey with a white box outline
to allow you to see the mapping coordinates on a surface
==================
*/
void idImage::MakeDefault( void )
{
	int		x, y;
	byte	data[DEFAULT_SIZE][DEFAULT_SIZE][4];
	
	if( com_developer.GetBool() )
	{
		// grey center
		for( y = 0 ; y < DEFAULT_SIZE ; y++ )
		{
			for( x = 0 ; x < DEFAULT_SIZE ; x++ )
			{
				data[y][x][0] = 32;
				data[y][x][1] = 32;
				data[y][x][2] = 32;
				data[y][x][3] = 255;
			}
		}
		
		// white border
		for( x = 0 ; x < DEFAULT_SIZE ; x++ )
		{
			data[0][x][0] =
				data[0][x][1] =
					data[0][x][2] =
						data[0][x][3] = 255;
						
			data[x][0][0] =
				data[x][0][1] =
					data[x][0][2] =
						data[x][0][3] = 255;
						
			data[DEFAULT_SIZE - 1][x][0] =
				data[DEFAULT_SIZE - 1][x][1] =
					data[DEFAULT_SIZE - 1][x][2] =
						data[DEFAULT_SIZE - 1][x][3] = 255;
						
			data[x][DEFAULT_SIZE - 1][0] =
				data[x][DEFAULT_SIZE - 1][1] =
					data[x][DEFAULT_SIZE - 1][2] =
						data[x][DEFAULT_SIZE - 1][3] = 255;
		}
	}
	else
	{
		for( y = 0 ; y < DEFAULT_SIZE ; y++ )
		{
			for( x = 0 ; x < DEFAULT_SIZE ; x++ )
			{
				data[y][x][0] = 0;
				data[y][x][1] = 0;
				data[y][x][2] = 0;
				data[y][x][3] = 0;
			}
		}
	}
	
	GenerateImage( ( byte* )data, DEFAULT_SIZE, DEFAULT_SIZE, TD_DEFAULT );
				   
	defaulted = true;
}

/*
===============
ActuallyLoadImage

Absolutely every image goes through this path
On exit, the idImage will have a valid OpenGL texture number that can be bound
===============
*/
void idImage::ActuallyLoadImage( bool fromBackEnd )
{
	crBTFTextureFile	btf;

	// if we don't have a rendering context yet, just return
	if( !idRenderSystem::IsInitialized() )
		return;
	
	// this is the ONLY place generatorFunction will ever be called
	if( generatorFunction )
	{
		generatorFunction( this );
		return;
	}
	
	if( com_productionMode.GetInteger() != 0 )
	{
		sourceFileTime = FILE_NOT_FOUND_TIMESTAMP;
		if( cubeFiles != CF_2D )
			opts.textureType = crTexture::TEXTURE_CUBEMAP; //TT_CUBIC;
	}
	else
	{
		// RB begin
		if( cubeFiles == CF_2D_ARRAY )
			opts.textureType = crTexture::TEXTURE_2D; //TT_2D_ARRAY;
			
		// RB end
		else if( cubeFiles != CF_2D )
		{
			opts.textureType = crTexture::TEXTURE_CUBEMAP; // TT_CUBIC;
			R_LoadCubeImages( GetName(), cubeFiles, nullptr, nullptr, &sourceFileTime );
		}
		else
		{
			opts.textureType = TT_2D;
			R_LoadImageProgram( GetName(), nullptr, nullptr, nullptr, &sourceFileTime, &usage );
		}
	}
	
	const bool toolUsage = IsToolUsage( usage );

	
	/// BEATO Begin: We update old idBinaryImage to our offline baked texture format
	/// similar to what valve do on Source engine whit .VTF textures

	idStrStatic< MAX_OSPATH > generatedName = GetName();
	generatedName.StripFileExtension();
	generatedName.SetFileExtension( "btf" );
	
	// TODO: Multithread
	btf = crBTFTextureFile( generatedName );
	if ( !btf.Open() )
	{
		if( com_developer.GetBool() )
			idLib::Warning( "Couldn't load image: %s : %s", GetName(), generatedName.c_str() );
	}
	
	opts = btf.GetImageParameters();
	if( cvarSystem->GetCVarBool( "fs_buildresources" ) )
	{
		// for resource gathering write this image to the preload file for this map
		fileSystem->AddImagePreload( GetName(), usage, cubeFiles );
	}

	/// alloc backend image
	if ( fromBackEnd )
	{
		/// Create image Handle
		AllocImage();

		/// upload sub images
		DoUpload();
	}
}

/*
========================
idImage::AllocImage
========================
*/
void idImage::AllocImage( const idImageOpts& imgOpts )
{
	opts = imgOpts;
	DeriveOpts();
	AllocImage();
}

/*
========================
idImage::PurgeImage
========================
*/
void idImage::PurgeImage( void )
{    
	// clear all the current binding caches, so the next bind will do a real one
	for( int i = 0 ; i < MAX_MULTITEXTURE_UNITS ; i++ )
	{
		backEnd.trState.tmu[i].current2DMap = 0;
		backEnd.trState.tmu[i].current2DArray = 0;
		backEnd.trState.tmu[i].currentCubeMap = 0;
	}
}

/*
========================
idImage::SetPixel
========================
*/
void idImage::SetPixel( int mipLevel, int x, int y, const void* data, int dataSize )
{
	SubImageUpload( mipLevel, x, y, 0, 1, 1, data );
}

/*
========================
idImage::Resize
========================
*/
void idImage::Resize( const uint32_t width, const uint32_t height )
{
	if( opts.width == width && opts.height == height )
		return;
	
	opts.width = width;
	opts.height = height;
	AllocImage();
}

/*
========================
idImage::SetTexParameters
========================
*/
void idImage::SetTexParameters( void )
{
	int target = GL_TEXTURE_2D;
	switch( opts.textureType )
	{
		case TT_2D:
			target = GL_TEXTURE_2D;
			break;
		case TT_CUBIC:
			target = GL_TEXTURE_CUBE_MAP;
			break;
			// RB begin
		case TT_2D_ARRAY:
			target = GL_TEXTURE_2D_ARRAY;
			break;
			// RB end
		default:
			idLib::FatalError( "%s: bad texture type %d", GetName(), opts.textureType );
			return;
	}
	
	// ALPHA, LUMINANCE, LUMINANCE_ALPHA, and INTENSITY have been removed
	// in OpenGL 3.2. In order to mimic those modes, we use the swizzle operators
#if defined( USE_CORE_PROFILE )
	if( opts.colorFormat == CFM_GREEN_ALPHA )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	}
	else if( opts.format == FMT_LUM8 )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_ONE );
	}
	else if( opts.format == FMT_L8A8 )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	}
	else if( opts.format == FMT_ALPHA )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	}
	else if( opts.format == FMT_INT8 )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	}
	else
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_RED );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_GREEN );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_BLUE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_ALPHA );
	}
#else
	if( opts.colorFormat == CFM_GREEN_ALPHA )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_GREEN );
	}
	else if( opts.format == FMT_ALPHA )
	{
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_R, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_G, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_B, GL_ONE );
		glTexParameteri( target, GL_TEXTURE_SWIZZLE_A, GL_RED );
	}
#endif
}

/*
===============
BindAttachment
===============
*/
void idImage::BindAttachmentOnFBO(int attachmentType, int layer)
{
#if 0
	if (layer == -1)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, texnum, 0);
	}
	else
	{
		assert(opts.textureType == TT_2D_ARRAY);
		glFramebufferTextureLayer( GL_FRAMEBUFFER, attachmentType, texnum, 0, layer );
	}
#else
#endif
}

/*
========================
idImage::AllocImage

Every image will pass through this function. Allocates all the necessary MipMap levels for the
Image, but doesn't put anything in them.

This should not be done during normal game-play, if you can avoid it.
========================
*/
void idImage::AllocImage( void )
{
	crTexture::dimensions_t dimensions{}; 

	GL_CheckErrors();
	PurgeImage();
		
	// if we don't have a rendering context, just return after we
	// have filled in the parms.  We must have the values set, or
	// an image match from a shader before OpenGL starts would miss
	// the generated texture
	if( !idRenderSystem::IsInitialized() )
		return;
	
	// generate the texture number
	if ( glConfig.backend == BACKEND_VULKAN )
		texnum = new( TAG_RENDER ) vkTexture(); 
	else if( glConfig.backend == BACKEND_OPENGL )
		texnum = new( TAG_RENDER ) glTexture();
	
	assert( texnum != nullptr );

	dimensions.width = opts.width;
	dimensions.heigth = opts.height;
	dimensions.depth = opts.depth;
	dimensions.levels = opts.numLevels;
	dimensions.layers = opts.numSides;
	texnum->Create( opts.textureType, dimensions, opts.format );
	
	SetTexParameters();
	
	GL_CheckErrors();
}


/*
========================
idImage::DeriveOpts
========================
*/
ID_INLINE void idImage::DeriveOpts( void )
{	
	if( cubeFiles != CF_2D && usage != TD_SHADOW_ARRAY ) // foresthale 2014-10-05: we want to hit the TD_SHADOW_ARRAY case below
	{		
		// motorsep 05-17-2015; setting parameters for cubemap / skybox images

		if( usage == TD_HIGHQUALITY_CUBE ) 
		{		
			opts.format = crInternalFormat::RGBA8U; /// FMT_RGBA8;
			skyboxRGBswap = true;
		} 
		// motorsep 05-23-2015; due to necessity of having alpha channel in the skyboxes, I decided to drop YCoCg color space and YCoCg compressing altogether;
		// This means compresses skyboxes will be of lower quality than they could have been using YCoCgDXT5 comression. However, they will support alpha channel and will allow to have same effects as
		// HQ RGBA skyboxes. Since we have option to turn on HQ skyboxes, people can always use that to get true high quality vs half-HQ with YCoCgDXT5.
		if( usage == TD_LOWQUALITY_CUBE ) 
		{
			//opts.colorFormat = CFM_DEFAULT; // CFM_YCOCG_DXT5;
			opts.format = crInternalFormat::BC3_RGBA;/// FMT_DXT5;			
			skyboxRGBswap = false;
		}
	}

		switch( usage )
		{
			case TD_COVERAGE:
				opts.format = FMT_DXT1;
				opts.colorFormat = CFM_GREEN_ALPHA;
				break;
			case TD_DEPTH:
				opts.format = FMT_DEPTH;
				break;
				
			case TD_SHADOW_ARRAY:
				opts.format = FMT_SHADOW_ARRAY;
				break;
				
			case TD_DIFFUSE:
				// TD_DIFFUSE gets only set to when its a diffuse texture for an interaction
				opts.gammaMips = true;
				opts.format = FMT_DXT5;
				opts.colorFormat = CFM_YCOCG_DXT5;				
				break;
			case TD_SPECULAR:
				opts.gammaMips = true;
				opts.sRGB = true; // foresthale 2014-02-20: fixed r_useSRGB texture handling
				opts.format = FMT_DXT5;
				opts.colorFormat = CFM_DEFAULT;
				/*opts.gammaMips = true;
				opts.sRGB = true; // foresthale 2014-02-20: fixed r_useSRGB texture handling
				opts.format = FMT_RGBA8;
				opts.colorFormat = CFM_DEFAULT;*/
				break;
			case TD_GLOSS:
				// we want a one-channel image with very precise gradations, so use FMT_INT8 rather than FMT_DXT1				
				opts.format = FMT_INT8;
				break;
			case TD_DEFAULT:
				opts.gammaMips = true;
				opts.sRGB = true; // foresthale 2014-02-20: fixed r_useSRGB texture handling
				opts.format = FMT_DXT5;
				opts.colorFormat = CFM_DEFAULT;
				break;
			case TD_BUMP:
				opts.format = FMT_DXT5;
				opts.colorFormat = CFM_NORMAL_DXT5;
				break;
			case TD_FONT:
				opts.format = FMT_DXT1;
				opts.colorFormat = CFM_GREEN_ALPHA;
				opts.numLevels = 4; // We only support 4 levels because we align to 16 in the exporter
				opts.gammaMips = true;
				break;
			case TD_LIGHT:
				//opts.format = FMT_RGB565;
				//opts.gammaMips = true;
				opts.format = FMT_RGBA8;
				opts.gammaMips = false;				
				break;
			case TD_LOOKUP_TABLE_MONO:
				opts.format = FMT_INT8;
				break;
			case TD_LOOKUP_TABLE_ALPHA:
				opts.format = FMT_ALPHA;
				break;
			case TD_LOOKUP_TABLE_RGB1:
			case TD_LOOKUP_TABLE_RGBA:
				opts.format = FMT_RGBA8;
				break;
			// foresthale 2014-05-17: added TD_EDITOR* image types (uncompressed variants of TD_DEFAULT and such, which always read .tga, and do not write .bimage)
			case TD_EDITOR_DEFAULT:
				opts.colorFormat = CFM_DEFAULT;
				opts.format = FMT_DXT5;
				opts.gammaMips = true;			
				break;
			case TD_EDITOR_DIFFUSE:
				opts.colorFormat = CFM_YCOCG_DXT5;
				opts.format = FMT_DXT5;
				opts.gammaMips = true;
				break;
			case TD_EDITOR_BUMP:
				opts.colorFormat = CFM_NORMAL_DXT5;
				opts.format = FMT_DXT5;
				opts.gammaMips = true;
				break;
			case TD_EDITOR_COVERAGE:
				opts.colorFormat = CFM_GREEN_ALPHA;
				opts.format = FMT_DXT5;
				break;
// ---> sikk - Added - High Quality Texture Depth (full RGBA)
			case TD_HIGHQUALITY:
				opts.colorFormat = CFM_DEFAULT;
				opts.format = FMT_RGBA8;
				opts.gammaMips = true;				
				break;
// <--- sikk - Added - High Quality Texture Depth (full RGBA)
			// motorsep 05-17-2015; added this for uncompressed cubemap/skybox textures
			case TD_HIGHQUALITY_CUBE:
				opts.colorFormat = CFM_DEFAULT;
				opts.format = FMT_RGBA8;
				opts.gammaMips = true;				
				break;
			case TD_LOWQUALITY_CUBE:
				opts.colorFormat = CFM_DEFAULT; // CFM_YCOCG_DXT5;
				opts.format = FMT_DXT5;
				opts.gammaMips = true;
				break;
			// foresthale 2014-02-19: added TD_RGBA16F and TD_DEPTHSTENCIL for HDR view rendering
			case TD_RGBA16F:
				opts.format = FMT_RGBA16F;
				break;
			case TD_DEPTHSTENCIL:
				opts.format = FMT_DEPTHSTENCIL;
				break;			
			default:
				assert( false );
				opts.format = FMT_RGBA8;				
		}
}

/*
========================
idImage::DoUpload
========================
*/
void idImage::DoUpload( void )
{
	crBuffer* 					buffer = nullptr;
	crTransferCommandBuffer*	transfer = nullptr;
	// no subimage chain to upload, image is a generated, or framebuffer attachament
	if ( subimages.IsEmpty() )
		return;

	buffer = idRenderSystem::GetGlobalImages()->GetPixelUnpackBuffer();
	transfer = backEnd.GetTransferCMD();

	transfer->CopyBufferToTexture( buffer, texnum, subimages );
}

/*
========================
idImage::DoDownload
========================
*/
void idImage::DoDownload(void)
{
	crBuffer* 					buffer = nullptr;
	crTransferCommandBuffer*	transfer = nullptr;

	/// if no subimage we ingone
	if ( subimages.IsEmpty() )
		return;

	buffer = idRenderSystem::GetGlobalImages()->GetPixelPackBuffer();
	transfer = backEnd.GetTransferCMD();

	transfer->CopyTextureToBuffer( buffer, texnum, subimages );	
}
