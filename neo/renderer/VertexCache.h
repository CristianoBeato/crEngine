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
#ifndef __VERTEX_CACHE_H__
#define __VERTEX_CACHE_H__

#include <atomic>

/// TODO:
/// Two transfer buffers ( vertex ad indexes ) 
/// 1 sub Command buffer to register copy operatios
/// 1 Timeline semaphore to control copy
/// End Level Load submition

constexpr size_t VERTCACHE_INDEX_MEMORY_PER_FRAME = 1024 * 1024 * 64; /// 64mb per frame
constexpr size_t VERTCACHE_VERTEX_MEMORY_PER_FRAME = 1024 * 1024 * 64; /// 64mb per frame

// there are a lot more static indexes than vertexes, because interactions are just new
// index lists that reference existing vertexes
// Beato: But indexes are 16 bits integers only
constexpr size_t STATIC_INDEX_MEMORY = 1024 * 1024 * 256; 	// 255 mb
constexpr size_t STATIC_VERTEX_MEMORY = 1024 * 1024 * 512;	// 

constexpr size_t VERTCACHE_STAGING_MEMORY_PER_FRAME = VERTCACHE_INDEX_MEMORY_PER_FRAME + VERTCACHE_VERTEX_MEMORY_PER_FRAME;

constexpr uint32_t VERTEX_CACHE_ALIGN	= 32;
constexpr uint32_t INDEX_CACHE_ALIGN	= 16;
constexpr uint32_t JOINT_CACHE_ALIGN	= 16;

enum cache_flags_e : uint8_t
{
	CACHE_STATIC 	= 1 << 0, // 0000 0001 
	CACHE_INDEX 	= 1 << 1, // 0000 0010
};

typedef struct vertCacheHandle_s
{
	vertCacheHandle_s( void ) : 
		flags( 0 ),
		frame( 0 ),
		size( 0 ),
		offset( 0 )
	{
	}

	vertCacheHandle_s( const vertCacheHandle_s& r ) : 
		flags( r.flags ),
		frame( r.frame ),
		size( r.size ),
		offset( r.offset )
	{
	}

	uint8_t		flags;	//
	uint8_t 	frame;	//
	size_t 		size;	//
	uintptr_t	offset;	//

	inline bool operator ==( const vertCacheHandle_s& r ) const { return ( offset == r.offset ) && ( size == r.size ); }
	inline operator bool( void ) const { return ( size != 0 ); }

} vertCacheHandle_t;

static size_t chs = sizeof( vertCacheHandle_t );

typedef class crBuffer* crBufferp;
typedef class crMemoryPool* crMemoryPoolp;
typedef class crCommandBuffer* crCommandBufferp;
typedef class crSemaphoreTimeline* crSemaphoreTimelinep;
class idVertexCache
{
public:
	void			Init( const uint32_t in_frames, const bool in_restart = false );
	void			Shutdown( void );
	void			PurgeAll( void );
	void			BeginMapLoad( void );
	void			EndMapLoad( void );

	// call on loading a new map
	void			FreeStaticData( void );
	
	// this data is only valid for one frame of rendering
	ID_INLINE vertCacheHandle_t	AllocVertex( const void* data, const size_t bytes )
	{
		return ActuallyAlloc( data, bytes, CACHE_VERTEX_DYNAMIC );
	}

	ID_INLINE vertCacheHandle_t	AllocIndex( const void* data, const size_t bytes )
	{
		return ActuallyAlloc( data, bytes, CACHE_INDEX_DYNAMIC );
	}
	
	// this data is valid until the next map load
	ID_INLINE  vertCacheHandle_t	AllocStaticVertex( const void* data, const size_t bytes )
	{
		if( m_caches[CACHE_VERTEX_STATIC].memoryUsed.load() + bytes > STATIC_VERTEX_MEMORY )
			idLib::FatalError( "AllocStaticVertex failed, increase STATIC_VERTEX_MEMORY" );
		
		return ActuallyAlloc( data, bytes, CACHE_VERTEX_STATIC );
	}

	ID_INLINE vertCacheHandle_t	AllocStaticIndex( const void* data, const size_t bytes )
	{
		if( m_caches[CACHE_INDEX_STATIC].memoryUsed.load() + bytes > STATIC_INDEX_MEMORY )
			idLib::FatalError( "AllocStaticIndex failed, increase STATIC_INDEX_MEMORY" );

		return ActuallyAlloc( data, bytes, CACHE_INDEX_STATIC );
	}
	
	ID_INLINE void* MappedVertexBuffer( const vertCacheHandle_t handle ) const
	{
		return static_cast<void*>( static_cast<byte*>( m_stagingMap ) + handle.offset );
	}

	// Returns false if it's been purged
	// This can only be called by the front end, the back end should only be looking at
	// vertCacheHandle_t that are already validated.
	ID_INLINE bool CacheIsCurrent( const vertCacheHandle_t handle ) const
	{
		if( handle.flags & CACHE_STATIC )
			return true;
		
		if( handle.frame != currentFrame )
			return false;
		
		return true;
	}
	
	static ID_INLINE bool CacheIsStatic( const vertCacheHandle_t &handle )
	{
		return ( handle.flags & CACHE_STATIC ) != 0;
	}
	
	crBufferp	GetBuffer( const vertCacheHandle_t &in_handle ) const;

	// vb/ib is a temporary reference -- don't store it
	bool			GetVertexBuffer( vertCacheHandle_t handle, idVertexBuffer* vb );
	bool			GetIndexBuffer( vertCacheHandle_t handle, idIndexBuffer* ib );
	void			BeginBackEnd( void );
	void			EndBackEnd( void );
public:
	enum cache_type_t
	{
		/// static buffer
		CACHE_INDEX_STATIC,
		CACHE_VERTEX_STATIC,
		/// dynamic buffers
		CACHE_INDEX_DYNAMIC,
		CACHE_VERTEX_DYNAMIC,
		CACHE_BUFFERS_COUNT
	};

	struct cache_info_t
	{
		uint32_t 				allocations;
		std::atomic<uintptr_t>	memoryUsedFrame;		
		std::atomic<uintptr_t>	memoryUsed;

		inline void Clear( void )
		{
			allocations = 0;
			memoryUsed.store( 0 );
		}
	};

	uint8_t			currentFrame;	// for determining the active buffers
	uint32_t		listNum;		// currentFrame % VERTCACHE_NUM_FRAMES
	uint32_t		drawListNum;	// ( currentFrame - 1) % VERTCACHE_NUM_FRAMES
	uint32_t		allocations;	// number of index and vertex allocations combined
	
	// High water marks for the per-frame buffers
	uint32_t		mostUsedVertex;
	uint32_t		mostUsedIndex;
	
	// Try to make room for <bytes> bytes
	vertCacheHandle_t	ActuallyAlloc( const void* data, const size_t bytes, const cache_type_t type );

	/// BEATO Begin:
	std::atomic<uintptr_t>							m_stagingOffset;
	crCommandBufferp								m_copyCommands;
	crSemaphoreTimelinep							m_copySync;
	crMemoryPoolp									m_renderBuffersPool;
	crMemoryPoolp									m_stagingMemoryPool;
	crBuffer*										m_staging;
	void*											m_stagingMap;
	idStaticList<cache_info_t, CACHE_BUFFERS_COUNT>	m_caches;
	idStaticList<crBufferp, CACHE_BUFFERS_COUNT>	m_buffers;
	
	/// Upload from staging buffer, to render buffer
	void		UploadBuffer( const cache_type_t in_type, const size_t in_size, const uintptr_t in_srcOffset, const uintptr_t in_dstOffset );
	
	/// Copy to staging buffer
	uintptr_t	UploadStage( const void* in_data, const size_t in_size );

	/// BEATO End

};

// platform specific code to std::memcpy into vertex buffers efficiently
// 16 byte alignment is guaranteed
void CopyBuffer( byte* dst, const byte* src, int numBytes );

extern	idVertexCache	vertexCache;

#endif // __VERTEX_CACHE_H__

