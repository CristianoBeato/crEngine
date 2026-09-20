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
#ifndef __FILE_H__
#define __FILE_H__

/*
==============================================================

  File Streams.

==============================================================
*/

// mode parm for Seek
typedef enum
{
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

class idFileSystemLocal;

typedef class idFile* idFilep;
class idFile
{
public:
	virtual					~idFile( void ) {};
	
	/// @brief Get the name of the file.
	virtual const char* 	GetName( void ) const;
	
	/// @brief Get the full file path.
	virtual const char* 	GetFullPath( void ) const;
	
	/// @brief Read data from the file to the buffer.
	virtual intptr_t		Read( void* buffer, const size_t len );
	
	/// @brief Write data from the buffer to the file.
	virtual intptr_t		Write( const void* buffer,const size_t len );
	
	/// @brief Returns the length of the file.
	virtual size_t			Length( void ) const;
	
	/// @brief Return a time value for reload operations.
	virtual ID_TIME_T		Timestamp( void ) const;
	
	/// @brief Returns offset in file.
	virtual intptr_t		Tell( void ) const;
	
	/// @brief Forces flush on files being writting to.
	virtual void			ForceFlush( void );
	
	/// @brief Causes any buffered data to be written to the file.
	virtual void			Flush( void );
	
	/// @brief Seek on a file.
	virtual intptr_t		Seek( const intptr_t offset, const fsOrigin_t origin );
	
	/// @brief Go back to the beginning of the file.
	virtual void			Rewind( void );
	
	/// @brief Like fprintf.
	virtual intptr_t		Printf( VERIFY_FORMAT_STRING const char* fmt, ... );
	
	/// @brief Like fprintf but with argument pointer
	virtual intptr_t		VPrintf( const char* fmt, va_list arg );
	
	/// @brief Write a string with high precision floating point numbers to the file.
	virtual intptr_t		WriteFloatString( VERIFY_FORMAT_STRING const char* fmt, ... );
	
	/// @brief Endian portable alternatives to Read(...)
	virtual intptr_t		ReadInt( int& value );
	virtual intptr_t		ReadUnsignedInt( unsigned int& value );
	virtual intptr_t		ReadShort( short& value );
	virtual intptr_t		ReadUnsignedShort( unsigned short& value );
	virtual intptr_t		ReadChar( char& value );
	virtual intptr_t		ReadUnsignedChar( unsigned char& value );
	virtual intptr_t		ReadFloat( float& value );
	virtual intptr_t		ReadBool( bool& value );
	virtual intptr_t		ReadString( idStr& string );
	virtual intptr_t		ReadVec2( idVec2& vec );
	virtual intptr_t		ReadVec3( idVec3& vec );
	virtual intptr_t		ReadVec4( idVec4& vec );
	virtual intptr_t		ReadVec5( idVec5& vec );
	virtual intptr_t		ReadVec6( idVec6& vec );
	virtual intptr_t		ReadMat3( idMat3& mat );
	virtual intptr_t		ReadWinding( idWinding& winding );
	virtual intptr_t		ReadPlane( idPlane& plane );
	
	/// @brief Endian portable alternatives to Write(...)
	virtual intptr_t		WriteInt( const int value );
	virtual intptr_t		WriteUnsignedInt( const unsigned int value );
	virtual intptr_t		WriteShort( const short value );
	virtual intptr_t		WriteUnsignedShort( unsigned short value );
	virtual intptr_t		WriteChar( const char value );
	virtual intptr_t		WriteUnsignedChar( const unsigned char value );
	virtual intptr_t		WriteFloat( const float value );
	virtual intptr_t		WriteBool( const bool value );
	virtual intptr_t		WriteString( const char* string );
	virtual intptr_t		WriteVec2( const idVec2& vec );
	virtual intptr_t		WriteVec3( const idVec3& vec );
	virtual intptr_t		WriteVec4( const idVec4& vec );
	virtual intptr_t		WriteVec5( const idVec5& vec );
	virtual intptr_t		WriteVec6( const idVec6& vec );
	virtual intptr_t		WriteMat3( const idMat3& mat );
	virtual intptr_t		WriteWinding( const idWinding& winding );
	virtual intptr_t		WritePlane( const idPlane& plane );
	
	template<class type> ID_INLINE size_t ReadBig( type& c )
	{
		size_t r = Read( &c, sizeof( c ) );
		idSwap::Big( c );
		return r;
	}
	
	template<class type> ID_INLINE size_t ReadBigArray( type* c, int count )
	{
		size_t r = Read( c, sizeof( c[0] ) * count );
		idSwap::BigArray( c, count );
		return r;
	}
	
	template<class type> ID_INLINE size_t WriteBig( const type& c )
	{
		type b = c;
		idSwap::Big( b );
		return Write( &b, sizeof( b ) );
	}
	
	template<class type> ID_INLINE size_t WriteBigArray( const type* c, int count )
	{
		size_t r = 0;
		for( int i = 0; i < count; i++ )
		{
			r += WriteBig( c[i] );
		}
		return r;
	}
};

/*
================================================
idFile_Memory
================================================
*/
class idFile_Memory : public idFile
{
	friend class			idFileSystemLocal;
	
public:
	idFile_Memory();	// file for writing without name
	idFile_Memory( const char* name );	// file for writing
	idFile_Memory( const char* name, char* data, int length );	// file for writing
	idFile_Memory( const char* name, const char* data, int length );	// file for reading
	virtual					~idFile_Memory();
	
	virtual const char* 	GetName( void ) const
	{
		return name.c_str();
	}

	virtual const char* 	GetFullPath( void ) const
	{
		return name.c_str();
	}
	virtual intptr_t		Read( void* buffer, const size_t len );
	virtual intptr_t		Write( const void* buffer, const size_t len );
	virtual size_t			Length( void ) const;
	virtual void			SetLength( size_t len );
	virtual ID_TIME_T		Timestamp( void ) const;
	virtual intptr_t		Tell( void ) const;
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual intptr_t		Seek( long offset, fsOrigin_t origin );
	
	/// @brief Set the given length and don't allow the file to grow.
	void					SetMaxLength( size_t len );
	
	/// @brief changes memory file to read only
	void					MakeReadOnly( void );
	
	/// @brief Change the file to be writable
	void					MakeWritable( void );
	
	/// @brief clear the file
	virtual void			Clear( bool freeMemory = true );
	
	/// @brief set data for reading
	void					SetData( const char* data, size_t length );
	
	/// @brief returns const pointer to the memory buffer
	const char* 			GetDataPtr( void ) const
	{
		return filePtr;
	}

	/// @brief returns pointer to the memory buffer
	char* 					GetDataPtr( void )
	{
		return filePtr;
	}
	// set the file granularity
	void					SetGranularity( int g )
	{
		assert( g > 0 );
		granularity = g;
	}
	void					PreAllocate( size_t len );
	
	// Doesn't change how much is allocated, but allows you to set the size of the file to smaller than it should be.
	// Useful for stripping off a checksum at the end of the file
	void					TruncateData( size_t len );
	
	void					TakeDataOwnership();
	
	size_t					GetMaxLength()
	{
		return maxSize;
	}

	size_t					GetAllocated()
	{
		return allocated;
	}
	
protected:
	idStr					name;			// name of the file

private:
	int						mode;			// open mode
	size_t					maxSize;		// maximum size of file
	size_t					fileSize;		// size of the file
	size_t					allocated;		// allocated size
	int						granularity;	// file granularity
	char* 					filePtr;		// buffer holding the file data
	char* 					curPtr;			// current read/write pointer
};


class idFile_BitMsg : public idFile
{
	friend class			idFileSystemLocal;
	
public:
	idFile_BitMsg( idBitMsg& msg );
	idFile_BitMsg( const idBitMsg& msg );
	virtual					~idFile_BitMsg( void );
	
	virtual const char* 	GetName( void ) const
	{
		return name.c_str();
	}

	virtual const char* 	GetFullPath( void ) const
	{
		return name.c_str();
	}

	virtual intptr_t		Read( void* buffer, const size_t len );
	virtual intptr_t		Write( const void* buffer, const size_t len );
	virtual size_t			Length( void ) const;
	virtual ID_TIME_T		Timestamp( void ) const;
	virtual intptr_t		Tell( void ) const;
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual intptr_t		Seek( const intptr_t offset, const fsOrigin_t origin );
	
private:
	idStr					name;			// name of the file
	int						mode;			// open mode
	idBitMsg* 				msg;
};

typedef struct SDL_IOStream SDL_IOStream;
typedef SDL_IOStream* idFileHandle;
class idFile_Permanent : public idFile
{
	friend class			idFileSystemLocal;
	
public:
	idFile_Permanent();
	virtual					~idFile_Permanent( void );
	virtual intptr_t		Read( void* buffer, const size_t len );
	virtual intptr_t		Write( const void* buffer, const size_t len );
	virtual intptr_t		Tell( void ) const;
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual intptr_t		Seek( const intptr_t offset, const fsOrigin_t origin );
	
	virtual size_t			Length( void ) const
	{
		return fileSize;
	}

	virtual ID_TIME_T		Timestamp( void ) const
	{
		return ftimestamp;
	}

	virtual const char* 	GetName( void ) const
	{
		return name.c_str();
	}

	virtual const char* 	GetFullPath( void ) const
	{
		return fullPath.c_str();
	}

	// returns file pointer
	idFileHandle			GetFilePtr( void ) { return fhandle; }
	
private:
	idStr					name;			// relative path of the file - relative path
	idStr					fullPath;		// full file path - OS path
	int						mode;			// open mode
	size_t					fileSize;		// size of the file
	//idFileHandle			o;				// file handle
/// BEATO Begin: Use SDL iostream for file read/write portability
	idFileHandle			fhandle;
	ID_TIME_T				ftimestamp;
/// BEATO End
	bool					handleSync;		// true if written data is immediately flushed
};

class idFile_Cached : public idFile_Permanent
{
	friend class			idFileSystemLocal;
public:
	idFile_Cached();
	virtual					~idFile_Cached();
	
	void					CacheData( uintptr_t offset, size_t length );
	
	virtual intptr_t		Read( void* buffer, const size_t len );
	
	virtual intptr_t		Tell( void ) const;
	virtual intptr_t		Seek( const intptr_t offset, const fsOrigin_t origin );
	
private:
	uintptr_t			internalFilePos;
	uintptr_t			bufferedStartOffset;
	uintptr_t			bufferedEndOffset;
	byte* 				buffered;
};


class idFile_InZip : public idFile
{
	friend class			idFileSystemLocal;
	
public:
	idFile_InZip();
	virtual					~idFile_InZip();
	
	virtual const char* 	GetName() const
	{
		return name.c_str();
	}

	virtual const char* 	GetFullPath() const
	{
		return fullPath.c_str();
	}

	virtual intptr_t		Read( void* buffer, size_t len );
	virtual intptr_t		Write( const void* buffer, size_t len );
	virtual size_t			Length( void ) const;
	virtual ID_TIME_T		Timestamp( void ) const;
	virtual intptr_t		Tell( void ) const;
	virtual void			ForceFlush( void );
	virtual void			Flush( void );
	virtual intptr_t		Seek( const intptr_t offset, const fsOrigin_t origin );
	
private:
	idStr					name;			// name of the file in the pak
	idStr					fullPath;		// full file path including pak file name
	// DG: use ZPOS64_T, it's the type minizip uses and should also work with zip64 files > 2GB
	uint64_t				zipFilePos;		// zip file info position in pak
	// DG end
	size_t					fileSize;		// size of the file
	void* 					z;				// unzip info
};

class idFile_InnerResource : public idFile
{
	friend class			idFileSystemLocal;
	
public:
	idFile_InnerResource( const char* _name, idFile* rezFile, intptr_t _offset, size_t _len );
	virtual					~idFile_InnerResource();
	
	virtual const char* 	GetName() const
	{
		return name.c_str();
	}

	virtual const char* 	GetFullPath() const
	{
		return name.c_str();
	}
	virtual intptr_t		Read( void* buffer, size_t len );
	
	virtual intptr_t		Write( const void* buffer, size_t len )
	{
		assert( false );
		return 0;
	}

	virtual size_t			Length( void ) const
	{
		return length;
	}

	virtual ID_TIME_T		Timestamp( void ) const
	{
		return 0;
	}

	virtual intptr_t		Tell( void ) const;
	virtual intptr_t		Seek( const intptr_t offset, const fsOrigin_t origin );
	void					SetResourceBuffer( byte* buf )
	{
		resourceBuffer = buf;
		internalFilePos = 0;
	}
	
private:
	idStr				name;				// name of the file in the pak
	intptr_t			offset;				// offset in the resource file
	size_t				length;				// size
	idFile* 			resourceFile;		// actual file
	intptr_t			internalFilePos;	// seek offset
	byte* 				resourceBuffer;		// if using the temp save memory
};

/*
================================================
idFileLocal is a FileStream wrapper that automatically closes a file when the
class variable goes out of scope. Note that the pointer passed in to the constructor can be for
any type of File Stream that ultimately inherits from idFile, and that this is not actually a
SmartPointer, as it does not keep a reference count.
================================================
*/
class idFileLocal
{
public:
	// Constructor that accepts and stores the file pointer.
	idFileLocal( idFile* _file )	: file( _file )
	{
	}
	
	// Destructor that will destroy (close) the file when this wrapper class goes out of scope.
	~idFileLocal();
	
	// Cast to a file pointer.
	operator idFile* () const
	{
		return file;
	}
	
	// Member access operator for treating the wrapper as if it were the file, itself.
	idFile* operator -> () const
	{
		return file;
	}
	
protected:
	idFile* file;	// The managed file pointer.
};

#endif /* !__FILE_H__ */
