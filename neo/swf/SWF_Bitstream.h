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
#ifndef __SWF_BITSTREAM_H__
#define __SWF_BITSTREAM_H__

class idSWFBitStream
{
public:
	idSWFBitStream();
	idSWFBitStream( const byte* data, uint32_t len, bool copy )
	{
		free = false;
		Load( data, len, copy );
	}
	~idSWFBitStream()
	{
		Free();
	}
	
	idSWFBitStream& operator=( idSWFBitStream& other );
	
	void			Load( const byte* data, uint32_t len, bool copy );
	void			Free();
	const byte* 	Ptr()
	{
		return startp;
	}
	
	uint32_t			Length() const
	{
		return ( uint32_t )( endp - startp );
	}
	uint32_t			Tell() const
	{
		return ( uint32_t )( readp - startp );
	}
	void			Seek( int32_t offset )
	{
		readp += offset;
	}
	void			Rewind()
	{
		readp = startp;
	}
	
	void			ResetBits();
	
	int				ReadS( unsigned int numBits );
	unsigned int	ReadU( unsigned int numBits );
	bool			ReadBool();
	
	const byte* 	ReadData( int size );
	
	template< typename T >
	void			ReadLittle( T& val );
	
	uint8_t			ReadU8();
	uint16_t			ReadU16();
	uint32_t			ReadU32();
	int16_t			ReadS16();
	int32_t			ReadS32();
	uint32_t			ReadEncodedU32();
	float			ReadFixed8();
	float			ReadFixed16();
	float			ReadFloat();
	double			ReadDouble();
	const char* 	ReadString();
	
	void			ReadRect( swfRect_t& rect );
	void			ReadMatrix( swfMatrix_t& matrix );
	void			ReadColorXFormRGBA( swfColorXform_t& cxf );
	void			ReadColorRGB( swfColorRGB_t& color );
	void			ReadColorRGBA( swfColorRGBA_t& color );
	void			ReadGradient( swfGradient_t& grad, bool rgba );
	void			ReadMorphGradient( swfGradient_t& grad );
	
private:
	bool			free;
	
	const byte* 	startp;
	const byte* 	endp;
	const byte* 	readp;
	
	uint64_t			currentBit;
	uint64_t			currentByte;
	
	int				ReadInternalS( uint64_t& regCurrentBit, uint64_t& regCurrentByte, unsigned int numBits );
	unsigned int	ReadInternalU( uint64_t& regCurrentBit, uint64_t& regCurrentByte, unsigned int numBits );
};

/*
========================
idSWFBitStream::ResetBits
========================
*/
ID_INLINE void idSWFBitStream::ResetBits()
{
	currentBit = 0;
	currentByte = 0;
}

/*
========================
idSWFBitStream::ReadLittle
========================
*/
template< typename T >
void idSWFBitStream::ReadLittle( T& val )
{
	val = *( T* )ReadData( sizeof( val ) );
	idSwap::Little( val );
}

/*
========================
Wrappers for the most basic types
========================
*/
ID_INLINE bool   idSWFBitStream::ReadBool()
{
	return ( ReadU( 1 ) != 0 );
}
ID_INLINE uint8_t  idSWFBitStream::ReadU8()
{
	ResetBits();
	return *readp++;
}
ID_INLINE uint16_t idSWFBitStream::ReadU16()
{
	ResetBits();
	readp += 2;
	return ( readp[-2] | ( readp[-1] << 8 ) );
}
ID_INLINE uint32_t idSWFBitStream::ReadU32()
{
	ResetBits();
	readp += 4;
	return ( readp[-4] | ( readp[-3] << 8 ) | ( readp[-2] << 16 ) | ( readp[-1] << 24 ) );
}
ID_INLINE int16_t  idSWFBitStream::ReadS16()
{
	ResetBits();
	readp += 2;
	return ( readp[-2] | ( readp[-1] << 8 ) );
}
ID_INLINE int32_t  idSWFBitStream::ReadS32()
{
	ResetBits();
	readp += 4;
	return ( readp[-4] | ( readp[-3] << 8 ) | ( readp[-2] << 16 ) | ( readp[-1] << 24 ) );
}
ID_INLINE float  idSWFBitStream::ReadFixed8()
{
	ResetBits();
	readp += 2;
	return SWFFIXED8( ( readp[-2] | ( readp[-1] << 8 ) ) );
}
ID_INLINE float  idSWFBitStream::ReadFixed16()
{
	ResetBits();
	readp += 4;
	return SWFFIXED16( ( readp[-4] | ( readp[-3] << 8 ) | ( readp[-2] << 16 ) | ( readp[-1] << 24 ) ) );
}
ID_INLINE float  idSWFBitStream::ReadFloat()
{
	ResetBits();
	readp += 4;
	uint32_t i = ( readp[-4] | ( readp[-3] << 8 ) | ( readp[-2] << 16 ) | ( readp[-1] << 24 ) );
	return ( float& )i;
}

ID_INLINE double idSWFBitStream::ReadDouble()
{
	const byte* swfIsRetarded = ReadData( 8 );
	byte buffer[8];
	buffer[0] = swfIsRetarded[4];
	buffer[1] = swfIsRetarded[5];
	buffer[2] = swfIsRetarded[6];
	buffer[3] = swfIsRetarded[7];
	buffer[4] = swfIsRetarded[0];
	buffer[5] = swfIsRetarded[1];
	buffer[6] = swfIsRetarded[2];
	buffer[7] = swfIsRetarded[3];
	double d = *( double* )buffer;
	idSwap::Little( d );
	return d;
}

#endif // !__SWF_BITSTREAM_H__
