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
#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

/*
===============================================================================

	idCompressor is a layer ontop of idFile which provides lossless data
	compression. The compressor can be used as a regular file and multiple
	compressors can be stacked ontop of each other.

===============================================================================
*/

class idCompressor : public idFile
{
public:
	// compressor allocation
	static idCompressor* 	AllocNoCompression();
	static idCompressor* 	AllocBitStream();
	static idCompressor* 	AllocRunLength();
	static idCompressor* 	AllocRunLength_ZeroBased();
	static idCompressor* 	AllocHuffman();
	static idCompressor* 	AllocArithmetic();
	static idCompressor* 	AllocLZSS();
	static idCompressor* 	AllocLZSS_WordAligned();
	static idCompressor* 	AllocLZW();
	
	// initialization
	virtual void			Init( idFile* f, bool compress, int wordLength ) = 0;
	virtual void			FinishCompress( void ) = 0;
	virtual float			GetCompressionRatio( void ) const = 0;
	
	// common idFile interface
	virtual const char* 	GetName( void ) = 0;
	virtual const char* 	GetFullPath( void ) = 0;
	virtual intptr_t		Read( void* outData, const size_t outLength ) = 0;
	virtual intptr_t		Write( const void* inData, const size_t inLength ) = 0;
	virtual size_t			Length( void ) = 0;
	virtual ID_TIME_T		Timestamp( void ) = 0;
	virtual intptr_t		Tell( void ) = 0;
	virtual void			ForceFlush( void ) = 0;
	virtual void			Flush( void ) = 0;
	virtual intptr_t		Seek( const intptr_t offset, const fsOrigin_t origin ) = 0;
};

#endif /* !__COMPRESSOR_H__ */
