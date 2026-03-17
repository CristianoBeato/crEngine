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

#include "renderer_common.h"
#include "ImageManager.hpp"
#include "BTFTextureFile.hpp"

idCVar vk_uploadBufferSizeMB( "r_uploadBufferSizeMB", "64", CVAR_INTEGER | CVAR_INIT, "Size of gpu upload buffer." );
idCVar vk_dowloadBufferSizeMB( "r_dowloadBufferSizeMB", "64", CVAR_INTEGER | CVAR_INIT, "Size of gpu download buffer." );

constexpr size_t	MAX_TEXTURE_TRANSFER_BUFFER = 1024 * 1024 * 256; // 256mb for texture transfer 
constexpr size_t	MAX_TEXTURE_ACESS_BUFFER = 1024 * 64; // 64kb for copy from texture 

// do this with a pointer, in case we want to make the actual manager
// a private virtual subclass
idImageManager *idImageManager::Get( void )
{
	static idImageManagerLocal imageManager = idImageManagerLocal();
    return &imageManager;
}

/// TODO remove this
idImageManager*	idRenderSystem::GetGlobalImages( void )
{
	return idImageManager::Get();
}

idCVar preLoad_Images( "preLoad_Images", "1", CVAR_SYSTEM | CVAR_BOOL, "preload images during beginlevelload" );

/*
===============
R_ReloadImages_f

Regenerate all images that came directly from files that have changed, so
any saved changes will show up in place.

New r_texturesize/r_texturedepth variables will take effect on reload

reloadImages <all>
===============
*/
void R_ReloadImages_f( const idCmdArgs& args )
{
	bool all = false;
	auto imageManager = idImageManager::Get();

	if( args.Argc() == 2 )
	{
		if( !idStr::Icmp( args.Argv( 1 ), "all" ) )
		{
			all = true;
		}
		else
		{
			common->Printf( "USAGE: reloadImages <all>\n" );
			return;
		}
	}
	
	imageManager->ReloadImages( all );
}

typedef struct
{
	idImage*	image;
	int		size;
	int		index;
} sortedImage_t;

/*
=======================
R_QsortImageSizes

=======================
*/
static int R_QsortImageSizes( const void* a, const void* b )
{
	const sortedImage_t*	ea, *eb;
	
	ea = ( sortedImage_t* )a;
	eb = ( sortedImage_t* )b;
	
	if( ea->size > eb->size )
	{
		return -1;
	}
	if( ea->size < eb->size )
	{
		return 1;
	}
	return idStr::Icmp( ea->image->GetName(), eb->image->GetName() );
}

/*
=======================
R_QsortImageName

=======================
*/
static int R_QsortImageName( const void* a, const void* b )
{
	const sortedImage_t*	ea, *eb;
	
	ea = ( sortedImage_t* )a;
	eb = ( sortedImage_t* )b;
	
	return idStr::Icmp( ea->image->GetName(), eb->image->GetName() );
}

/*
===============
R_ListImages_f
===============
*/
void R_ListImages_f( const idCmdArgs& args )
{
	int		i, partialSize;
	idImage*	image;
	int		totalSize;
	int		count = 0;
	bool	uncompressedOnly = false;
	bool	unloaded = false;
	bool	failed = false;
	bool	sorted = false;
	bool	duplicated = false;
	bool	overSized = false;
	bool	sortByName = false;
	
	if( args.Argc() == 1 )
	{
	
	}
	else if( args.Argc() == 2 )
	{
		if( idStr::Icmp( args.Argv( 1 ), "uncompressed" ) == 0 )
		{
			uncompressedOnly = true;
		}
		else if( idStr::Icmp( args.Argv( 1 ), "sorted" ) == 0 )
		{
			sorted = true;
		}
		else if( idStr::Icmp( args.Argv( 1 ), "namesort" ) == 0 )
		{
			sortByName = true;
		}
		else if( idStr::Icmp( args.Argv( 1 ), "unloaded" ) == 0 )
		{
			unloaded = true;
		}
		else if( idStr::Icmp( args.Argv( 1 ), "duplicated" ) == 0 )
		{
			duplicated = true;
		}
		else if( idStr::Icmp( args.Argv( 1 ), "oversized" ) == 0 )
		{
			sorted = true;
			overSized = true;
		}
		else
		{
			failed = true;
		}
	}
	else
	{
		failed = true;
	}
	
	if( failed )
	{
		common->Printf( "usage: listImages [ sorted | namesort | unloaded | duplicated | showOverSized ]\n" );
		return;
	}
	
	const char* header = "       -w-- -h-- filt -fmt-- wrap  size --name-------\n";
	common->Printf( "\n%s", header );
	
	totalSize = 0;
	
	auto imageManager = static_cast<idImageManagerLocal*>( idImageManager::Get() );
	sortedImage_t*	sortedArray = ( sortedImage_t* )alloca( sizeof( sortedImage_t ) * imageManager->images.Num() );
	for( i = 0 ; i < imageManager->images.Num() ; i++ )
	{
		image = imageManager->images[ i ];
		if( uncompressedOnly )
		{
			if( image->IsCompressed() )
				continue;	
		}

		if( unloaded == image->IsLoaded() )
			continue;
		
		// only print duplicates (from mismatched wrap / clamp, etc)
		if( duplicated )
		{
			int j;
			for( j = i + 1 ; j < imageManager->images.Num() ; j++ )
			{
				if( idStr::Icmp( image->GetName(), imageManager->images[ j ]->GetName() ) == 0 )
					break;
			}

			if( j == imageManager->images.Num() )
				continue;
		}
		
		if( sorted || sortByName )
		{
			sortedArray[count].image = image;
			sortedArray[count].size = image->StorageSize();
			sortedArray[count].index = i;
		}
		else
		{
			common->Printf( "%4i:",	i );
			image->Print();
		}
		totalSize += image->StorageSize();
		count++;
	}
	
	if( sorted || sortByName )
	{
		if( sortByName )
			qsort( sortedArray, count, sizeof( sortedImage_t ), R_QsortImageName );
		else
			qsort( sortedArray, count, sizeof( sortedImage_t ), R_QsortImageSizes );
		
		partialSize = 0;
		for( i = 0 ; i < count ; i++ )
		{
			common->Printf( "%4i:",	sortedArray[i].index );
			sortedArray[i].image->Print();
			partialSize += sortedArray[i].image->StorageSize();
			if( ( ( i + 1 ) % 10 ) == 0 )
			{
				common->Printf( "-------- %5.1f of %5.1f megs --------\n",
								partialSize / ( 1024 * 1024.0 ), totalSize / ( 1024 * 1024.0 ) );
			}
		}
	}
	
	common->Printf( "%s", header );
	common->Printf( " %i images (%i total)\n", count, imageManager->images.Num() );
	common->Printf( " %5.1f total megabytes of images\n\n\n", totalSize / ( 1024 * 1024.0 ) );
}

/*
==============
AllocImage

Allocates an idImage, adds it to the list,
copies the name, and adds it to the hash chain.
==============
*/
idImage* idImageManagerLocal::AllocImage( const idStr &name )
{
	if( strlen( name ) >= MAX_IMAGE_NAME )
		common->Error( "idImageManagerLocal::AllocImage: \"%s\" is too long\n", name );
	
	int hash = idStr( name ).FileNameHash();
	
	idImage* image = new( TAG_IMAGE ) idImage( name );
	
	imageHash.Add( hash, images.Append( image ) );
	
	return image;
}

/*
==============
AllocStandaloneImage

Allocates an idImage,does not add it to the list or hash chain

==============
*/
idImage* idImageManagerLocal::AllocStandaloneImage( const idStr &name )
{
	if( strlen( name ) >= MAX_IMAGE_NAME )
		common->Error( "idImageManagerLocal::AllocImage: \"%s\" is too long\n", name );
	
	idImage* image = new( TAG_IMAGE ) idImage( name );

	return image;
}

/*
==================
ImageFromFunction

Images that are procedurally generated are allways specified
with a callback which must work at any time, allowing the OpenGL
system to be completely regenerated if needed.
==================
*/
idImage* idImageManagerLocal::ImageFromFunction( const idStr &_name, void ( *generatorFunction )( idImage* image ) )
{
	// strip any .tga file extensions from anywhere in the _name
	idStr name = _name;
	name.Replace( ".tga", "" );
	name.BackSlashesToSlashes();
	
	// see if the image already exists
	int hash = name.FileNameHash();
	for( int i = imageHash.First( hash ); i != -1; i = imageHash.Next( i ) )
	{
		idImage* image = images[i];
		if( name.Icmp( image->GetName() ) == 0 )
		{
			if( image->generatorFunction != generatorFunction )
				common->DPrintf( "WARNING: reused image %s with mixed generators\n", name.c_str() );
			
			return image;
		}
	}
	
	// create the image and issue the callback
	idImage*	 image = AllocImage( name );
	
	image->generatorFunction = generatorFunction;
	
	// check for precompressed, load is from the front end
	image->referencedOutsideLevelLoad = true;
	image->ActuallyLoadImage( false );
	
	return image;
}


/*
===============
GetImageWithParameters
==============
*/
idImage*	idImageManagerLocal::GetImageWithParameters( const idStr &_name, textureUsage_t usage, cubeFiles_t cubeMap ) const
{
	auto imageManager = idImageManagerLocal::Get();
	if( !_name || !_name[0] || idStr::Icmp( _name, "default" ) == 0 || idStr::Icmp( _name, "_default" ) == 0 )
	{
		declManager->MediaPrint( "DEFAULTED\n" );
		return imageManager->DefaultImage();
	}

	if( idStr::Icmpn( _name, "fonts", 5 ) == 0 || idStr::Icmpn( _name, "newfonts", 8 ) == 0 )
		usage = TD_FONT;

	if( idStr::Icmpn( _name, "lights", 6 ) == 0 )
		usage = TD_LIGHT;
		// filter = TF_LINEAR;	 // sikk - Added - no mipmaps for light textures
	

	if( idStr::Icmpn( _name, "savegame", 8 ) == 0 )
		usage = TD_HIGHQUALITY;
		// filter = TF_LINEAR;	 // no mipmaps for Savegame previews textures

	// strip any .tga file extensions from anywhere in the _name, including image program parameters
	idStrStatic< MAX_OSPATH > name = _name;
	name.Replace( ".tga", "" );
	name.BackSlashesToSlashes();
	int hash = name.FileNameHash();
	for( int i = imageHash.First( hash ); i != -1; i = imageHash.Next( i ) )
	{
		idImage*	 image = images[i];
		if( name.Icmp( image->GetName() ) == 0 )
		{
			// the built in's, like _white and _flat always match the other options
			if( name[0] == '_' )
				return image;
			
			if( image->cubeFiles != cubeMap )
				common->Error( "Image '%s' has been referenced with conflicting cube map states", _name );
			
			if( image->usage != usage )
				continue; // If an image is used differently then we need 2 copies of it because usage affects the way it's compressed and swizzled
			
			return image;
		}
	}
	return nullptr;
}
/*
===============
ImageFromFile

Finds or loads the given image, always returning a valid image pointer.
Loading of the image may be deferred for dynamic loading.
==============
*/
idImage*	idImageManagerLocal::ImageFromFile( const idStr &_name, const textureUsage_t _usage, const cubeFiles_t cubeMap )
{
	crBTFTextureFile	imageFile = crBTFTextureFile( _name );
	textureUsage_t usage = _usage;

	if( !_name || !_name[0] || idStr::Icmp( _name, "default" ) == 0 || idStr::Icmp( _name, "_default" ) == 0 )
	{
		declManager->MediaPrint( "DEFAULTED\n" );
		return defaultImage;
	}

	if( idStr::Icmpn( _name, "fonts", 5 ) == 0 || idStr::Icmpn( _name, "newfonts", 8 ) == 0 )
		usage = TD_FONT;

	if( idStr::Icmpn( _name, "lights", 6 ) == 0 )
		usage = TD_LIGHT;
		// filter = TF_LINEAR;	 // sikk - Added - no mipmaps for light textures

	if( idStr::Icmpn( _name, "savegame", 8 ) == 0 )
		usage = TD_HIGHQUALITY;
		// filter = TF_LINEAR;	 // no mipmaps for Savegame previews textures
		
	// strip any .tga file extensions from anywhere in the _name, including image program parameters
	idStrStatic< MAX_OSPATH > name = _name;
	name.Replace( ".tga", "" );
	name.BackSlashesToSlashes();
	
	//
	// see if the image is already loaded, unless we
	// are in a reloadImages call
	//
	int hash = name.FileNameHash();
	for( int i = imageHash.First( hash ); i != -1; i = imageHash.Next( i ) )
	{
		idImage*	 image = images[i];
		if( name.Icmp( image->GetName() ) == 0 )
		{
			// the built in's, like _white and _flat always match the other options
			if( name[0] == '_' )
				return image;
			
			if( image->cubeFiles != cubeMap )
				common->Error( "Image '%s' has been referenced with conflicting cube map states", _name );
	
			if( image->usage != usage )
				// If an image is used differently then we need 2 copies of it because usage affects the way it's compressed and swizzled
				continue;
			
			image->usage = usage;
			image->levelLoadReferenced = true;
			
			if( ( !insideLevelLoad  || preloadingMapImages ) && !image->IsLoaded() )
			{
				image->referencedOutsideLevelLoad = ( !insideLevelLoad && !preloadingMapImages );
				image->ActuallyLoadImage( false );	// load is from front end
				declManager->MediaPrint( "%ix%i %s (reload for mixed referneces)\n", image->GetUploadWidth(), image->GetUploadHeight(), image->GetName() );
			}
			return image;
		}
	}
	
	//
	// create a new image
	//
	idImage* image = AllocImage( name );
	image->cubeFiles = cubeMap;
	image->usage = usage;
	image->levelLoadReferenced = true;
	
	// load it if we aren't in a level preload
	if( !insideLevelLoad || preloadingMapImages )
	{
		image->referencedOutsideLevelLoad = ( !insideLevelLoad && !preloadingMapImages );
		if ( !(com_editors & EDITOR_DMAP) )
			image->ActuallyLoadImage( false );	// load is from front end
		
		declManager->MediaPrint( "%ix%i %s\n", image->GetUploadWidth(), image->GetUploadHeight(), image->GetName() );
	}
	else
	{
		declManager->MediaPrint( "%s\n", image->GetName() );
	}
	
	return image;
}

/*
========================
idImageManagerLocal::ScratchImage
========================
*/
idImage* idImageManagerLocal::ScratchImage( const idStr &_name, const idImageOpts* imgOpts, const textureUsage_t usage )
{
	if( !_name || !_name[0] )
		idLib::FatalError( "idImageManagerLocal::ScratchImage called with empty name" );
	
	if( imgOpts == nullptr )
		idLib::FatalError( "idImageManagerLocal::ScratchImage called with nullptr imgOpts" );
	
	idStr name = _name;
	
	//
	// see if the image is already loaded, unless we
	// are in a reloadImages call
	//
	int hash = name.FileNameHash();
	for( int i = imageHash.First( hash ); i != -1; i = imageHash.Next( i ) )
	{
		idImage*	 image = images[i];
		if( name.Icmp( image->GetName() ) == 0 )
		{
			// the built in's, like _white and _flat always match the other options
			if( name[0] == '_' )
				return image;
			
			if( image->usage != usage )
				// If an image is used differently then we need 2 copies of it because usage affects the way it's compressed and swizzled
				continue;
			
			image->usage = usage;
			image->levelLoadReferenced = true;
			image->referencedOutsideLevelLoad = true;
			return image;
		}
	}
	
	//
	// create a new image
	//
	idImage* newImage = AllocImage( name );
	if( newImage != nullptr )
		newImage->AllocImage( *imgOpts );
	
	return newImage;
}

/*
===============
idImageManagerLocal::GetImage
===============
*/
idImage* idImageManagerLocal::GetImage( const idStr &_name ) const
{
	auto imageManager = idImageManagerLocal::Get();
	if( !_name || !_name[0] || idStr::Icmp( _name, "default" ) == 0 || idStr::Icmp( _name, "_default" ) == 0 )
	{
		declManager->MediaPrint( "DEFAULTED\n" );
		return imageManager->DefaultImage();
	}
	
	// strip any .tga file extensions from anywhere in the _name, including image program parameters
	idStr name = _name;
	name.Replace( ".tga", "" );
	name.BackSlashesToSlashes();
	
	//
	// look in loaded images
	//
	int hash = name.FileNameHash();
	for( int i = imageHash.First( hash ); i != -1; i = imageHash.Next( i ) )
	{
		idImage* image = images[i];
		if( name.Icmp( image->GetName() ) == 0 )
			return image;
	}
	
	return nullptr;
}

/*
========================
idImage::SubImageUpload
========================
*/
#if 0
subImage_t idImageManagerLocal::SubImageUpload( int mipLevel, int destX, int destY, int destZ, int width, int height, const void *data, int pixelPitch ) const
{
	assert( x >= 0 && y >= 0 && mipLevel >= 0 && width >= 0 && height >= 0 && mipLevel < opts.numLevels );
	
	int compressedSize = 0;
	
	if( IsCompressed() )
	{
		assert( !( x & 3 ) && !( y & 3 ) );
		
		// compressed size may be larger than the dimensions due to padding to quads
		int quadW = ( width + 3 ) & ~3;
		int quadH = ( height + 3 ) & ~3;
		compressedSize = quadW * quadH * BitsForFormat( opts.format ) / 8;
		
		int padW = ( opts.width + 3 ) & ~3;
		int padH = ( opts.height + 3 ) & ~3;
		( void )padH;
		( void )padW;
		assert( x + width <= padW && y + height <= padH );
		// upload the non-aligned value, OpenGL understands that there
		// will be padding
		if( x + width > opts.width )
			width = opts.width - x;
		
		if( y + height > opts.height )
			height = opts.height - x;
		
	}
	else
	{
		assert( x + width <= opts.width && y + height <= opts.height );
	}
	
	int target;
	int uploadTarget;
	if( opts.textureType == TT_2D )
	{
		target = GL_TEXTURE_2D;
		uploadTarget = GL_TEXTURE_2D;
	}
	else if( opts.textureType == TT_CUBIC )
	{
		target = GL_TEXTURE_CUBE_MAP;
		uploadTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + z;
	}
	else
	{
		assert( !"invalid opts.textureType" );
		target = GL_TEXTURE_2D;
		uploadTarget = GL_TEXTURE_2D;
	}
	
	glBindTexture( target, texnum );
	
	if( pixelPitch != 0 )
	{
		glPixelStorei( GL_UNPACK_ROW_LENGTH, pixelPitch );
	}
	if( opts.format == FMT_RGB565 )
	{
		glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_TRUE );
	}
#ifdef DEBUG
	GL_CheckErrors();
#endif
	if( IsCompressed() )
	{
		glCompressedTexSubImage2D( uploadTarget, mipLevel, x, y, width, height, internalFormat, compressedSize, pic );
	}
	else
	{
	
		// make sure the pixel store alignment is correct so that lower mips get created
		// properly for odd shaped textures - this fixes the mip mapping issues with
		// fonts
		int unpackAlignment = width * BitsForFormat( ( textureFormat_t )opts.format ) / 8;
		if( ( unpackAlignment & 3 ) == 0 )
			glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
		else
			glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		
		
		glTexSubImage2D( uploadTarget, mipLevel, x, y, width, height, dataFormat, dataType, pic );
	}
#ifdef DEBUG
	GL_CheckErrors();
#endif
	if( opts.format == FMT_RGB565 )
		glPixelStorei( GL_UNPACK_SWAP_BYTES, GL_FALSE );
	
	if( pixelPitch != 0 )
		glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
}
#endif

uintptr_t idImageManagerLocal::UploadTexels( const void *in_buffer, const size_t in_size )
{
	// 16 ligned
	const size_t alignedSize = ( in_size + 15 ) & ~15; 
	const size_t bufferSize = m_pixelUnpack.buffer->Size();

	while ( true )
	{
		size_t freeSpace = 0;

		/// get current pointers 
		int currentHead = m_pixelUnpack.head.GetValue();
        int currentTail = m_pixelUnpack.tail.GetValue();

		/// Calculate free space considering the wrap-around.
        if ( currentHead >= currentTail ) 
			freeSpace = bufferSize - ( currentHead - currentTail ); // Space is what remains until the end + what was freed up at the beginning.
		else 
			freeSpace = currentTail - currentHead; // Tail is in front of Head (wrap-around occurred)
    
		/// Check if it fits (we leave a 1-byte margin to avoid confusing full with empty)
        if ( freeSpace <= alignedSize ) 
		{
            // Buffer full: Here you can either give a Yield or return 0.
            // idLib typically uses active waits or helper threads.
			Sys_Yield(); // TODO: we assume that we are using multithread to upload textures, but this will implemented in future
            continue; 
        }

		// Check wrap-around at the end of the physical buffer
        if ( currentHead + alignedSize > bufferSize ) 
		{
            // Option A: Reset Head to the beginning (requires Tail not to be at the beginning)
            if ( currentTail <= (int)alignedSize ) 
                continue; // No space at the beginning either.
            
			// Insert a "gap" and padding if necessary and start over from scratch.
            // (Note: You will need to update the Head to 0 and try again)
            m_pixelUnpack.head.SetValue( 0 );
			continue;
        }

		/// Try to reserve space atomically (Compare-and-Swap)
        if ( m_pixelUnpack.head.CompareExchange( currentHead, currentHead + alignedSize )) 
		{
        	// Copy the data to the staging buffer.
			m_pixelUnpack.buffer->Upload( in_buffer, currentHead, in_size );
            return (uintptr_t)currentHead;
        }
	}
	
    return 0;
}

#if 0
/*
===============
idImageManagerLocal::UploadSubImages
===============
*/
void idImageManagerLocal::UploadSubImages( const idImage *in_image, const uintptr_t in_baseOffset, const idList<crTexture::subImage_t> in_subimageList )
{
	// TODO: check if this is done by backend

	/// Upload texture texels
	m_transfer->CopyBufferToTexture( m_pixelPack, in_image->texnum, in_subimageList );

	/// Update buffer offset 
	if ( m_unpackTail.GetValue() < in_baseOffset )
		m_unpackTail.SetValue( in_baseOffset );
}
#endif

/*
===============
PurgeAllImages
===============
*/
void idImageManagerLocal::PurgeAllImages( void )
{
	int		i;
	idImage*	image;
	
	for( i = 0; i < images.Num() ; i++ )
	{
		image = images[i];
		image->PurgeImage();
	}
}

/*
===============
ReloadImages
===============
*/
void idImageManagerLocal::ReloadImages( bool all )
{
	for( int i = 0 ; i < images.Num() ; i++ )
	{
		images[ i ]->Reload( all );
	}
}

/*
===============
R_CombineCubeImages_f

Used to combine animations of six separate tga files into
a serials of 6x taller tga files, for preparation to roq compress
===============
*/
void R_CombineCubeImages_f( const idCmdArgs& args )
{
	if( args.Argc() != 2 )
	{
		common->Printf( "usage: combineCubeImages <baseName>\n" );
		common->Printf( " combines basename[1-6][0001-9999].tga to basenameCM[0001-9999].tga\n" );
		common->Printf( " 1: forward 2:right 3:back 4:left 5:up 6:down\n" );
		return;
	}
	
	idStr	baseName = args.Argv( 1 );
	common->SetRefreshOnPrint( true );
	
	for( int frameNum = 1 ; frameNum < 10000 ; frameNum++ )
	{
		char	filename[MAX_IMAGE_NAME];
		byte*	pics[6];
		int		width = 0, height = 0;
		int		side;
		int		orderRemap[6] = { 1, 3, 4, 2, 5, 6 };
		for( side = 0 ; side < 6 ; side++ )
		{
			sprintf( filename, "%s%i%04i.tga", baseName.c_str(), orderRemap[side], frameNum );
			
			common->Printf( "reading %s\n", filename );
			R_LoadImage( filename, &pics[side], &width, &height, nullptr, true );
			
			if( !pics[side] )
			{
				common->Printf( "not found.\n" );
				break;
			}
			
			// convert from "camera" images to native cube map images
			switch( side )
			{
				case 0:	// forward
					R_RotatePic( pics[side], width );
					break;
				case 1:	// back
					R_RotatePic( pics[side], width );
					R_HorizontalFlip( pics[side], width, height );
					R_VerticalFlip( pics[side], width, height );
					break;
				case 2:	// left
					R_VerticalFlip( pics[side], width, height );
					break;
				case 3:	// right
					R_HorizontalFlip( pics[side], width, height );
					break;
				case 4:	// up
					R_RotatePic( pics[side], width );
					break;
				case 5: // down
					R_RotatePic( pics[side], width );
					break;
			}
		}
		
		if( side != 6 )
		{
			for( int i = 0 ; i < side ; side++ )
			{
				Mem_Free( pics[side] );
			}
			break;
		}
		
		idTempArray<byte> buf( width * height * 6 * 4 );
		byte*	combined = ( byte* )buf.Ptr();
		for( side = 0 ; side < 6 ; side++ )
		{
			std::memcpy( combined + width * height * 4 * side, pics[side], width * height * 4 );
			Mem_Free( pics[side] );
		}
		sprintf( filename, "%sCM%04i.tga", baseName.c_str(), frameNum );
		
		common->Printf( "writing %s\n", filename );
		R_WriteTGA( filename, combined, width, height * 6 );
	}
	common->SetRefreshOnPrint( false );
}

/*
===============
UnbindAll
===============
*/
void idImageManagerLocal::UnbindAll( void )
{
#if 0
	int oldTMU = backEnd.trState.currenttmu;
	for( int i = 0; i < MAX_PROG_TEXTURE_PARMS; ++i )
	{
		backEnd.trState.currenttmu = i;
		BindNull();
	}
	backEnd.trState.currenttmu = oldTMU;
#endif 
}

/*
===============
BindNull
===============
*/
void idImageManagerLocal::BindNull( void )
{
	RENDERLOG_PRINTF( "BindNull()\n" );
	
#if 0 // TODO: Beato Remove
	// foresthale 2014-05-10: when using the tools code (which does not use shaders) we have to manage the texture unit enables
	if (com_editors)
	{
		//glActiveTexture(GL_TEXTURE0_ARB + backEnd.glState.currenttmu);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
#endif
}

/*
===============
Init
===============
*/
void idImageManagerLocal::Init()
{

	images.Resize( 1024, 1024 );
	imageHash.ResizeIndex( 1024 );
	
	CreateIntrinsicImages();
	
	cmdSystem->AddCommand( "reloadImages", R_ReloadImages_f, CMD_FL_RENDERER, "reloads images" );
	cmdSystem->AddCommand( "listImages", R_ListImages_f, CMD_FL_RENDERER, "lists images" );
	cmdSystem->AddCommand( "combineCubeImages", R_CombineCubeImages_f, CMD_FL_RENDERER, "combines six images for roq compression" );
	
// BEATO Begin:
	m_pixelPack.buffer = new vkBuffer();
	m_pixelUnpack.buffer = new vkBuffer();
	m_pixelPack.buffer->Create( vkBuffer::BUFFER_TYPE_PIXEL, vkBuffer::BUFFER_ACCESS_WRITE, MAX_TEXTURE_TRANSFER_BUFFER );
	m_pixelUnpack.buffer->Create( vkBuffer::BUFFER_TYPE_PIXEL, vkBuffer::BUFFER_ACCESS_READ, MAX_TEXTURE_ACESS_BUFFER );
// BEATO End

	// should forceLoadImages be here?
}

/*
===============
Shutdown
===============
*/
void idImageManagerLocal::Shutdown( void )
{
	images.DeleteContents( true );
	imageHash.Clear();

	if ( m_pixelUnpack.buffer )
	{
		m_pixelUnpack.buffer->Destroy();
		delete m_pixelUnpack.buffer;
		m_pixelUnpack.buffer = nullptr;
	}
	
	if ( m_pixelPack.buffer )
	{
		m_pixelPack.buffer->Destroy();
		delete m_pixelPack.buffer;
		m_pixelPack.buffer = nullptr;
	}
}

/*
====================
idImageManagerLocal::BeginLevelLoad
Frees all images used by the previous level
====================
*/
void idImageManagerLocal::BeginLevelLoad()
{
	insideLevelLoad = true;

	// foresthale 2014-05-28: Brian Harris suggested the editors should never purge assets, because of potential for crashes on improperly refcounted assets
	if ( com_editors )
		return;

	for( int i = 0 ; i < images.Num() ; i++ )
	{
		idImage*	image = images[ i ];
		
		// generator function images are always kept around
		if( image->generatorFunction )
		{
			continue;
		}
		
		if( !image->referencedOutsideLevelLoad && image->IsLoaded() )
		{
			image->PurgeImage();
			//idLib::Printf( "purging %s\n", image->GetName() );
		}
		else
		{
			//idLib::Printf( "not purging %s\n", image->GetName() );
		}
		
		image->levelLoadReferenced = false;
	}
}


/*
====================
idImageManagerLocal::ExcludePreloadImage
====================
*/
bool idImageManagerLocal::ExcludePreloadImage( const idStr &name )
{
	idStrStatic< MAX_OSPATH > imgName = name;
	imgName.ToLower();
	if( imgName.Find( "newfonts/", false ) >= 0 )
		return true;
	
	if( imgName.Find( "generated/swf/", false ) >= 0 )
		return true;
	
	if( imgName.Find( "/loadscreens/", false ) >= 0 )
		return true;
	
	return false;
}

/*
====================
idImageManagerLocal::Preload
====================
*/
void idImageManagerLocal::Preload( const idPreloadManifest& manifest, const bool& mapPreload )
{
	if( preLoad_Images.GetBool() && manifest.NumResources() > 0 )
	{
		// preload this levels images
		common->Printf( "Preloading images...\n" );
		preloadingMapImages = mapPreload;
		int	start = Sys_Milliseconds();
		int numLoaded = 0;
		
		//fileSystem->StartPreload( preloadImageFiles );
		for( int i = 0; i < manifest.NumResources(); i++ )
		{
			const preloadEntry_s& p = manifest.GetPreloadByIndex( i );
			if( p.resType == PRELOAD_IMAGE && !ExcludePreloadImage( p.resourceName ) )
			{
				ImageFromFile( p.resourceName, ( textureUsage_t )p.imgData.usage, ( cubeFiles_t )p.imgData.cubeMap );
				numLoaded++;
			}
		}
		//fileSystem->StopPreload();
		int	end = Sys_Milliseconds();
		common->Printf( "%05d images preloaded ( or were already loaded ) in %5.1f seconds\n", numLoaded, ( end - start ) * 0.001 );
		common->Printf( "----------------------------------------\n" );
		preloadingMapImages = false;
	}
}

/*
===============
idImageManagerLocal::LoadLevelImages
===============
*/
int idImageManagerLocal::LoadLevelImages( bool pacifier )
{
	int	loadCount = 0;
	int pProgress = 0;
	int imageCount = images.Num();
	for( int i = 0 ; i < images.Num() ; i++ )
	{
		if( pacifier )
		{
			pProgress  = (100 * i ) / imageCount;
			common->UpdateLevelLoadPacifier(true,pProgress);
		}
		
		idImage*	image = images[ i ];
		if( image->generatorFunction )
			continue;
		
		if( image->levelLoadReferenced && !image->IsLoaded() )
		{
			loadCount++;
			image->ActuallyLoadImage( false );
		}
	}

	return loadCount;
}

/*
===============
idImageManagerLocal::EndLevelLoad
===============
*/
void idImageManagerLocal::EndLevelLoad( void )
{
	insideLevelLoad = false;
	
	common->Printf( "----- idImageManagerLocal::EndLevelLoad -----\n" );
	int start = Sys_Milliseconds();
	int	loadCount = LoadLevelImages( true );
	
	int	end = Sys_Milliseconds();
	common->Printf( "%5i images loaded in %5.1f seconds\n", loadCount, ( end - start ) * 0.001 );
	common->Printf( "----------------------------------------\n" );
	//R_ListImages_f( idCmdArgs( "sorted sorted", false ) );
}

/*
===============
idImageManagerLocal::StartBuild
===============
*/
void idImageManagerLocal::StartBuild()
{
}

/*
===============
idImageManagerLocal::FinishBuild
===============
*/
void idImageManagerLocal::FinishBuild( bool removeDups )
{
}

/*
===============
idImageManagerLocal::PrintMemInfo
===============
*/
void idImageManagerLocal::PrintMemInfo( MemInfo_t* mi )
{
	int i, j, total = 0;
	int* sortIndex;
	idFile* f;
	
	f = fileSystem->OpenFileWrite( mi->filebase + "_images.txt" );
	if( !f )
		return;
	
	// sort first
	sortIndex = new( TAG_IMAGE ) int[images.Num()];
	
	for( i = 0; i < images.Num(); i++ )
	{
		sortIndex[i] = i;
	}
	
	for( i = 0; i < images.Num() - 1; i++ )
	{
		for( j = i + 1; j < images.Num(); j++ )
		{
			if( images[sortIndex[i]]->StorageSize() < images[sortIndex[j]]->StorageSize() )
			{
				int temp = sortIndex[i];
				sortIndex[i] = sortIndex[j];
				sortIndex[j] = temp;
			}
		}
	}
	
	// print next
	for( i = 0; i < images.Num(); i++ )
	{
		idImage* im = images[sortIndex[i]];
		int size;
		
		size = im->StorageSize();
		total += size;
		
		f->Printf( "%s %3i %s\n", idStr::FormatNumber( size ).c_str(), im->refCount, im->GetName() );
	}
	
	delete [] sortIndex;
	mi->imageAssetsTotal = total;
	
	f->Printf( "\nTotal image bytes allocated: %s\n", idStr::FormatNumber( total ).c_str() );
	fileSystem->CloseFile( f );
}
