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

#include "precompiled.h"
#include "renderer_common.h"
#include "VertexCache.h"

idVertexCache vertexCache;

idCVar r_showVertexCache( "r_showVertexCache", "0", CVAR_RENDERER | CVAR_BOOL, "Print stats about the vertex cache every frame" );
idCVar r_showVertexCacheTimings( "r_showVertexCacheTimings", "0", CVAR_RENDERER | CVAR_BOOL, "Print stats about the vertex cache every frame" );

/*
==============
idVertexCache::Init
==============
*/
void idVertexCache::Init( const uint32_t in_frames, const bool in_restart )
{
	uint32_t i = 0, j = 0;
    uint32_t memoryTypeBits = 0xFFFFFFFF;
	size_t indexBufferSize = 0;
	size_t vertexBufferSize = 0;
	VkDeviceSize minAlign = 0;
	VkDeviceSize currentOffset = 0;
	crQueuep queue = nullptr;
	auto device = tr.GetRenderDevice();

	frame = 0;
	listNum = 0;
	m_mostUsedVertex = 0;
	m_mostUsedIndex = 0;

	if ( glConfig.isTransferQueueAvailable )
		queue = dynamic_cast<crVulkanRenderDevicep>( tr.GetRenderDevice() )->TransferQueue();
	else
		queue = dynamic_cast<crVulkanRenderDevicep>( tr.GetRenderDevice() )->GraphicQueue();

	///
	/// Configure dynamic buffer regions
	m_drawBuffer.SetNum( CACHE_BUFFER_COUNT );
	m_dynamicBuffers.SetNum( CACHE_BUFFER_COUNT );
	for ( i = 0; i < CACHE_BUFFER_COUNT; i++)
	{
		m_dynamicBuffers[i].SetNum( in_frames );
		for( j = 0; j < in_frames; j++ )
		{
			m_dynamicBuffers[i][j].count.store( 0u );
			m_dynamicBuffers[i][j].used.store( 0ull );
		
			if( i == CACHE_INDEX_BUFFER )
			{
				m_dynamicBuffers[i][j].size = VERTCACHE_INDEX_MEMORY_PER_FRAME;
				m_dynamicBuffers[i][j].offset = indexBufferSize;
				indexBufferSize += VERTCACHE_INDEX_MEMORY_PER_FRAME;
			}
			else if( i == CACHE_VERTEX_BUFFER )
			{
				m_dynamicBuffers[i][j].size = VERTCACHE_VERTEX_MEMORY_PER_FRAME;
				m_dynamicBuffers[i][j].offset = vertexBufferSize;
				vertexBufferSize += VERTCACHE_VERTEX_MEMORY_PER_FRAME;
			}			
		}
	}

	///
	/// configure static buffer regions
	m_staticBuffers.SetNum( CACHE_BUFFER_COUNT );
	for ( i = 0; i < CACHE_BUFFER_COUNT; i++)
	{
		m_staticBuffers[i].count.store( 0u );
		m_staticBuffers[i].used.store( 0ull );

		if( i == CACHE_INDEX_BUFFER )
		{
			m_staticBuffers[i].size = STATIC_INDEX_MEMORY;
			m_staticBuffers[i].offset = indexBufferSize;
			indexBufferSize += STATIC_INDEX_MEMORY;
		}
		else if( i == CACHE_VERTEX_BUFFER )
		{
			m_staticBuffers[i].size = STATIC_INDEX_MEMORY;
			m_staticBuffers[i].offset = vertexBufferSize;
			vertexBufferSize += STATIC_INDEX_MEMORY;
		}
	}

	///
	/// Create the index buffer 
	m_drawBuffer[CACHE_INDEX_BUFFER] = new crBuffer();
	if( !m_drawBuffer[CACHE_INDEX_BUFFER]->Create( crBuffer::BUFFER_TYPE_INDEX, indexBufferSize ) )
			throw idException( "Failed to create index cache buffers" );

	// Combines the supported memory type bits (bit-by-bit AND).
	VkMemoryRequirements iBuffReq = m_drawBuffer[CACHE_INDEX_BUFFER]->MemoryRequirements();
	memoryTypeBits &= iBuffReq.memoryTypeBits;
	
	/// 
	/// Create vertex buffer
	m_drawBuffer[CACHE_VERTEX_BUFFER] = new crBuffer();
	if( !m_drawBuffer[CACHE_VERTEX_BUFFER]->Create( crBuffer::BUFFER_TYPE_VERTEX, vertexBufferSize ) )
			throw idException( "Failed to create vertex cache buffers" );

	// Combines the supported memory type bits (bit-by-bit AND).
	VkMemoryRequirements vBuffReq = m_drawBuffer[CACHE_INDEX_BUFFER]->MemoryRequirements();
	memoryTypeBits &= vBuffReq.memoryTypeBits;

	///
	/// Get the minimum aligment
	minAlign = std::max( iBuffReq.alignment, vBuffReq.alignment );

	/// 
	/// Allocate the the memory pool for the rende buffers
    m_renderBuffersPool = device->Alloc( iBuffReq.size + vBuffReq.size, minAlign, memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
    if( m_renderBuffersPool )
        idLib::FatalError( "Vertex Cache Buffers memory pool allocation failed, no suitable memory or no available memory size\n" );
	
	/// now allocate buffer memory
	if( !m_drawBuffer[CACHE_INDEX_BUFFER]->Storage( m_renderBuffersPool ) )
		throw idException("Failed to allocate index buffer memory!");

	if( !m_drawBuffer[CACHE_VERTEX_BUFFER]->Storage( m_renderBuffersPool ) )
		throw idException("Failed to allocate vertex buffer memory!");

	///
	///________________________________________________________________________
	/// Create the staging buffer
	m_staging = new crBuffer();
	if( !m_staging->Create( crBuffer::BUFFER_TYPE_SOURCE, VERTCACHE_STAGING_MEMORY_PER_FRAME ) );
		throw idException( "Failed to create vertex cahce staging buffer" );

	auto stagingMemoryRequirements = m_staging->MemoryRequirements();

	m_stagingMemoryPool = device->Alloc( currentOffset, stagingMemoryRequirements.alignment, stagingMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    if( m_stagingMemoryPool )
        idLib::FatalError( "Shader storage buffers memory pool allocation failed, no suitable memory four or no available memory size\n" );

	if( !m_staging->Storage( m_stagingMemoryPool ) )
		throw idException( "" );

	m_stagingMap = m_staging->GetMemoryPage()->Map();	

	///
	///________________________________________________________________________
	/// Create copy command buffer
	///
	m_copyCommands = new crCommandBuffer();
	if( !m_copyCommands->Create( r_bufferCount.GetInteger(), queue, false ) )
		throw idException( "Failed to create vertex cache copy command buffer!\n" );

	///
	///________________________________________________________________________
	/// Create transfer semaphore
	///
	m_copySync = new crSemaphoreTimeline();
	if( !m_copySync->Create() )
		throw idException( "Failed to create vertex cahce staging buffer" );
}

/*
==============
idVertexCache::Shutdown
==============
*/
void idVertexCache::Shutdown( void )
{
	auto device = tr.GetRenderDevice();
	
	VK_SAFE_DESTROY( m_copySync );
	VK_SAFE_DESTROY( m_copyCommands );

	/// Release staging buffer
	VK_SAFE_DESTROY( m_staging );

	/// Release staging memory
	device->Free( m_stagingMemoryPool );

	/// Release render buffers 
	//VK_SAFE_DESTROY( m_drawVertexBuffer );
	//VK_SAFE_DESTROY( m_drawIndexBuffer );
	
	/// Release render buffers memory
	device->Free( m_renderBuffersPool );
}

/*
==============
idVertexCache::PurgeAll
==============
*/
void idVertexCache::PurgeAll( void )
{
	Shutdown();
	Init( true );
}

void idVertexCache::BeginMapLoad(void)
{
}

/*
==============
idVertexCache::EndMapLoad
==============
*/
void idVertexCache::EndMapLoad(void)
{
	VkBufferMemoryBarrier2 destinationBarrier{};
	idList<VkBufferMemoryBarrier2> bariers;
	auto gQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	auto tQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	/// Get Queues
	if( glConfig.isTransferQueueAvailable )
	{
		auto device = tr.GetRenderDevice();
		gQueueFamilyIndex = device->GraphicQueue()->Family();
		tQueueFamilyIndex = device->TransferQueue()->Family();
	}

	///
	/// Change index buffer state to render
	{
    	destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    	destinationBarrier.pNext = nullptr;

		/// from copy destination
    	destinationBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    	destinationBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    	
		/// to index input buffer
		destinationBarrier.dstStageMask = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
    	destinationBarrier.dstAccessMask = VK_ACCESS_2_INDEX_READ_BIT;
		
		/// 
    	destinationBarrier.srcQueueFamilyIndex = tQueueFamilyIndex;
    	destinationBarrier.dstQueueFamilyIndex = gQueueFamilyIndex;

		/// current frame region
		destinationBarrier.offset = static_cast<VkDeviceSize>( m_staticBuffers[CACHE_INDEX_BUFFER].offset );
		destinationBarrier.size = static_cast<VkDeviceSize>( m_staticBuffers[CACHE_INDEX_BUFFER].size );
    	destinationBarrier.buffer = *m_drawBuffer[CACHE_INDEX_BUFFER];
		bariers.Append( destinationBarrier );
	}

	///
	/// Change vertex buffer state to render
	{
    	destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    	destinationBarrier.pNext = nullptr;

		/// from copy destination
    	destinationBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    	destinationBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    	
		/// to index input buffer
		destinationBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    	destinationBarrier.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
		
		/// 
    	destinationBarrier.srcQueueFamilyIndex = tQueueFamilyIndex;
    	destinationBarrier.dstQueueFamilyIndex = gQueueFamilyIndex;

		/// current frame region
		destinationBarrier.offset = static_cast<VkDeviceSize>( m_staticBuffers[CACHE_VERTEX_BUFFER].offset );
		destinationBarrier.size = static_cast<VkDeviceSize>( m_staticBuffers[CACHE_VERTEX_BUFFER].size );
    	destinationBarrier.buffer = *m_drawBuffer[CACHE_VERTEX_BUFFER];
		bariers.Append( destinationBarrier );
	}

	m_copyCommands->BufferMemoryBarriers( bariers.Ptr(), bariers.Num() );

	/// Submit copy operations
	if( glConfig.isTransferQueueAvailable )
	{
		auto transfer = tr.TransferCommandBuffer();
		transfer->Execute( m_copyCommands );
	}
	else
	{
		auto graphic = tr.GraphicCommandBuffer();
		graphic->Execute( m_copyCommands );
	}
}

/*
 ==============
 idVertexCache::FreeStaticData
 call on loading a new map
 ==============
 */
void idVertexCache::FreeStaticData( void )
{
	/// reset
	m_staticBuffers[CACHE_INDEX_BUFFER].used.store( 0 );
	m_staticBuffers[CACHE_VERTEX_BUFFER].used.store( 0 );
	m_mostUsedVertex = 0;
	m_mostUsedIndex = 0;
}

/*
==============
idVertexCache::AllocVertex
==============
*/
vertCacheHandle_t idVertexCache::AllocVertex( const void* in_data, const size_t in_bytes )
{
	vertCacheHandle_t cache{};
	if( in_bytes == 0 )
		return vertCacheHandle_t();

	cache.size = __align( in_bytes, 16u );
	cache.offset = AllocDynamic( CACHE_VERTEX_BUFFER, cache.size );
	if( cache.offset == UINTPTR_MAX )
		idLib::FatalError( "AllocStaticIndex failed, increase VERTCACHE_VERTEX_MEMORY_PER_FRAME" );

	/// if we have data upload
	if( in_data != nullptr )
	{
		assert( ( ( reinterpret_cast<uintptr_t>( in_data ) ) & 15 ) == 0 );

		/// copy data to staging buffer
		uintptr_t staging = UploadStage( in_data, cache.size );

		/// Store the transfer coomand from the staging buffer
		VkBufferCopy2 bufferCopy{};
		bufferCopy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		bufferCopy.pNext = nullptr;
		bufferCopy.srcOffset = static_cast<VkDeviceSize>( staging );
		bufferCopy.dstOffset = static_cast<VkDeviceSize>( cache.offset  );
		bufferCopy.size = static_cast<VkDeviceSize>( cache.size );
		m_copyListIndex.Append( bufferCopy );
	}

	cache.flags = 0;
	cache.frame = frame;
	return cache;
}

/*
==============
idVertexCache::AllocIndex
==============
*/
vertCacheHandle_t idVertexCache::AllocIndex( const void* const in_data, const size_t in_bytes )
{
	vertCacheHandle_t cache{};
	if( in_bytes == 0 )
		return vertCacheHandle_t();

	cache.size = __align( in_bytes, 16u );
	cache.offset = AllocDynamic( CACHE_INDEX_BUFFER, cache.size );
	if( cache.offset == UINTPTR_MAX )
		idLib::FatalError( "AllocStaticIndex failed, increase VERTCACHE_INDEX_MEMORY_PER_FRAME" );

	/// if we have data upload
	if( in_data != nullptr )
	{
		assert( ( ( reinterpret_cast<uintptr_t>( in_data ) ) & 15 ) == 0 );

		/// copy data to staging buffer
		uintptr_t staging = UploadStage( in_data, cache.size );

		/// Store the transfer coomand from the staging buffer
		VkBufferCopy2 bufferCopy{};
		bufferCopy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		bufferCopy.pNext = nullptr;
		bufferCopy.srcOffset = static_cast<VkDeviceSize>( staging );
		bufferCopy.dstOffset = static_cast<VkDeviceSize>( cache.offset  );
		bufferCopy.size = static_cast<VkDeviceSize>( cache.size );
		m_copyListIndex.Append( bufferCopy );
	}

	cache.flags = CACHE_INDEX;
	cache.frame = frame;
	return cache;
}

/*
==============
idVertexCache::AllocStaticIndex
==============
*/
vertCacheHandle_t idVertexCache::AllocStaticIndex( const void* in_data, const size_t in_bytes )
{
	vertCacheHandle_t cache{};
	if( in_bytes == 0 )
		return vertCacheHandle_t();
	
	//assert( ( bytes & 15 ) == 0 );

	cache.size = __align( in_bytes, 16u );
	cache.offset = AllocStatic( CACHE_INDEX_BUFFER, cache.size );
	if( cache.offset == UINTPTR_MAX )
		idLib::FatalError( "AllocStaticIndex failed, increase STATIC_INDEX_MEMORY" );

	/// if we have data upload
	if( in_data != nullptr )
	{
		assert( ( ( reinterpret_cast<uintptr_t>( in_data ) ) & 15 ) == 0 );

		/// copy data to staging buffer
		uintptr_t staging = UploadStage( in_data, cache.size );

		/// Store the transfer coomand from the staging buffer
		VkBufferCopy2 bufferCopy{};
		bufferCopy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		bufferCopy.pNext = nullptr;
		bufferCopy.srcOffset = static_cast<VkDeviceSize>( staging );
		bufferCopy.dstOffset = static_cast<VkDeviceSize>( cache.offset  );
		bufferCopy.size = static_cast<VkDeviceSize>( cache.size );
		m_copyListIndex.Append( bufferCopy );
	}

	cache.flags = CACHE_STATIC | CACHE_INDEX;
	cache.frame = frame;
	return cache;
}

/*
==============
idVertexCache::AllocStaticVertex
==============
*/
vertCacheHandle_t idVertexCache::AllocStaticVertex( const void* in_data, const size_t in_bytes )
{
	vertCacheHandle_t cache{};
	if( in_bytes == 0 )
		return vertCacheHandle_t();

	//assert( ( bytes & 15 ) == 0 );

	cache.size = __align( in_bytes, 16u );
	cache.offset = AllocStatic( CACHE_INDEX_BUFFER, cache.size );
	if( cache.offset == UINTPTR_MAX )
		idLib::FatalError( "AllocStaticVertex failed, increase STATIC_VERTEX_MEMORY" );

	/// if we have data upload
	if( in_data != nullptr )
	{
		assert( ( ( reinterpret_cast<uintptr_t>( in_data ) ) & 15 ) == 0 );

		/// copy data to staging buffer
		uintptr_t staging = UploadStage( in_data, cache.size );
		
		/// Store the transfer coomand from the staging buffer
		VkBufferCopy2 bufferCopy{};
		bufferCopy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
		bufferCopy.pNext = nullptr;
		bufferCopy.srcOffset = static_cast<VkDeviceSize>( staging );
		bufferCopy.dstOffset = static_cast<VkDeviceSize>( cache.offset  );
		bufferCopy.size = static_cast<VkDeviceSize>( cache.size );
		m_copyListVertex.Append( bufferCopy );
	}

	cache.flags = CACHE_STATIC;
	cache.frame = frame;
	return cache;
}

void *idVertexCache::MappedIndexBuffer(const vertCacheHandle_t handle) const
{
    uintptr_t offset = const_cast<idVertexCache*>( this )->AllocStaging( handle.size );
	return static_cast<void*>( static_cast<byte*>( m_stagingMap ) + offset );
}

/*
==============
idVertexCache::AllocStaticVertex
==============
*/
void *idVertexCache::MappedVertexBuffer(const vertCacheHandle_t handle) const
{
	uintptr_t offset = const_cast<idVertexCache*>( this )->AllocStaging( handle.size );
	return static_cast<void*>( static_cast<byte*>( m_stagingMap ) + offset );
}

/*
==============
idVertexCache::UploadStage
==============
*/
uintptr_t idVertexCache::UploadStage( const void *in_data, const size_t in_size )
{
	uintptr_t offset = 0;
	
	/// reached the buffer end, reset the pointer
	if( ( m_stagingOffset + in_size ) > VERTCACHE_STAGING_MEMORY_PER_FRAME )
		m_stagingOffset.store( 0 );

	/// 
	offset = m_stagingOffset.fetch_add( in_size );

	/// copy to stagin buffer memory
	std::memcpy( static_cast<byte*>( m_stagingMap ) + offset, in_data, in_size );

    return offset;
}

/*
==============
idVertexCache::BeginBackEnd
==============
*/
void idVertexCache::BeginBackEnd( void )
{
	uint32_t allocations = 0;
	m_mostUsedVertex = std::max<uint32_t>( m_mostUsedVertex, m_dynamicBuffers[CACHE_INDEX_BUFFER][frame].used.load() );
	m_mostUsedIndex = std::max<uint32_t>( m_mostUsedIndex, m_dynamicBuffers[CACHE_VERTEX_BUFFER][frame].used.load() );
	allocations += m_dynamicBuffers[CACHE_INDEX_BUFFER][frame].count.load();
	allocations += m_dynamicBuffers[CACHE_VERTEX_BUFFER][frame].count.load();

	if( r_showVertexCache.GetBool() )
	{
		idLib::Printf( "%08d: %d allocations, %dkB vertex, %dkB index: %dkB vertex, %dkB index\n",
			frame, allocations,
			m_dynamicBuffers[CACHE_VERTEX_BUFFER][frame].used.load() / 1024,
			m_dynamicBuffers[CACHE_INDEX_BUFFER][frame].used.load() / 1024,
			m_mostUsedVertex / 1024,
			m_mostUsedIndex / 1024 );
	}

	m_copyCommands->BeginSubCommand();
	
	// reset the counters and offsets
	for ( auto i = 0; i < CACHE_BUFFER_COUNT; i++)
	{
		m_dynamicBuffers[i][frame].used.store( 0 );
		m_dynamicBuffers[i][frame].count.store( 0 );
	}
}

/*
==============
idVertexCache::EndBackEnd
==============
*/
void idVertexCache::EndBackEnd( void )
{
	VkBufferMemoryBarrier2 destinationBarrier{};
	idList<VkBufferMemoryBarrier2> bariers( 4 );
	bariers.Resize( 4 );
	auto gQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	auto tQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	/// Get Queues
	if( glConfig.isTransferQueueAvailable )
	{
		auto device = tr.GetRenderDevice();
		gQueueFamilyIndex = device->GraphicQueue()->Family();
		tQueueFamilyIndex = device->TransferQueue()->Family();
	}

	FlushTransferLists();

	///
	/// Change index buffer state to render
	{
    	destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    	destinationBarrier.pNext = nullptr;

		/// from copy destination
    	destinationBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    	destinationBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    	
		/// to index input buffer
		destinationBarrier.dstStageMask = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
    	destinationBarrier.dstAccessMask = VK_ACCESS_2_INDEX_READ_BIT;
		
		/// 
    	destinationBarrier.srcQueueFamilyIndex = tQueueFamilyIndex;
    	destinationBarrier.dstQueueFamilyIndex = gQueueFamilyIndex;

		/// current frame region
		destinationBarrier.offset = m_dynamicBuffers[frame][CACHE_INDEX_BUFFER].offset;
		destinationBarrier.size = m_dynamicBuffers[frame][CACHE_INDEX_BUFFER].size;
    	destinationBarrier.buffer = *m_drawBuffer[CACHE_INDEX_BUFFER];
		bariers.Append( destinationBarrier );
	}

	///
	/// Change vertex buffer state to render
	{
    	destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    	destinationBarrier.pNext = nullptr;

		/// from copy destination
    	destinationBarrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    	destinationBarrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    	
		/// to index input buffer
		destinationBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    	destinationBarrier.dstAccessMask = VK_ACCESS_2_UNIFORM_READ_BIT;
		
		/// 
    	destinationBarrier.srcQueueFamilyIndex = tQueueFamilyIndex;
    	destinationBarrier.dstQueueFamilyIndex = gQueueFamilyIndex;

		/// current frame region
		destinationBarrier.offset = m_dynamicBuffers[frame][CACHE_VERTEX_BUFFER].offset;
		destinationBarrier.size = m_dynamicBuffers[frame][CACHE_VERTEX_BUFFER].size;
    	destinationBarrier.buffer = *m_drawBuffer[CACHE_VERTEX_BUFFER];
		bariers.Append( destinationBarrier );
	}

	/// swap dynamic region
	auto numFrames = r_bufferCount.GetInteger();
	frame = ( frame + 1 ) % numFrames;

	///
	/// Change index buffer state to copy
	{
    	destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    	destinationBarrier.pNext = nullptr;

		/// From index input buffer 
		destinationBarrier.srcStageMask = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
    	destinationBarrier.srcAccessMask = VK_ACCESS_2_INDEX_READ_BIT;

		/// To copy destination
		destinationBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    	destinationBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

		/// 
    	destinationBarrier.srcQueueFamilyIndex = gQueueFamilyIndex;
    	destinationBarrier.dstQueueFamilyIndex = tQueueFamilyIndex;

		/// current frame region
		destinationBarrier.offset = m_dynamicBuffers[frame][CACHE_INDEX_BUFFER].offset;
		destinationBarrier.size = m_dynamicBuffers[frame][CACHE_INDEX_BUFFER].size;
    	destinationBarrier.buffer = *m_drawBuffer[CACHE_INDEX_BUFFER];
		bariers.Append( destinationBarrier );
	}

	///
	/// Change vertex buffer state to copy
	{
    	destinationBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    	destinationBarrier.pNext = nullptr;

		/// From index input buffer 
		destinationBarrier.srcStageMask = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
    	destinationBarrier.srcAccessMask = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;

		/// To copy destination
		destinationBarrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    	destinationBarrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;

		/// 
    	destinationBarrier.srcQueueFamilyIndex = gQueueFamilyIndex;
    	destinationBarrier.dstQueueFamilyIndex = tQueueFamilyIndex;

		/// current frame region
		destinationBarrier.offset = m_dynamicBuffers[frame][CACHE_VERTEX_BUFFER].offset;
		destinationBarrier.size = m_dynamicBuffers[frame][CACHE_VERTEX_BUFFER].size;
    	destinationBarrier.buffer = *m_drawBuffer[CACHE_VERTEX_BUFFER];
		bariers.Append( destinationBarrier );
	}
	
	m_copyCommands->BufferMemoryBarriers( bariers.Ptr(), bariers.Num() );

	/// Submit copy operations
	if( glConfig.isTransferQueueAvailable )
	{
		auto transfer = tr.TransferCommandBuffer();
		transfer->Execute( m_copyCommands );
	}
	else
	{
		auto graphic = tr.GraphicCommandBuffer();
		graphic->Execute( m_copyCommands );
	}
}

/*
==============
idVertexCache::AllocIndexStatic
==============
*/
uintptr_t idVertexCache::AllocStatic( const cache_type_t in_buffer, const size_t in_bytes )
{
	if( ( m_staticBuffers[in_buffer].used + in_bytes ) > m_staticBuffers[in_buffer].size )
	{
		/// TODO: Change output name 
		idLib::Error( "Out of static cache range" );
		return UINTPTR_MAX;
	}

	m_staticBuffers[in_buffer].count.fetch_add( 1 );
	
	/// return the global offset in the buffer 
	return m_staticBuffers[in_buffer].used.fetch_add( in_bytes ) + m_staticBuffers[in_buffer].offset;
}

/*
==============
idVertexCache::AllocDynamic
==============
*/
uintptr_t idVertexCache::AllocDynamic(const cache_type_t in_buffer, const size_t in_bytes )
{
	if( ( m_dynamicBuffers[in_buffer][frame].used + in_bytes ) > m_dynamicBuffers[in_buffer][frame].size )
	{
		idLib::Error( "Out of frame index cache" );
		return UINTPTR_MAX;
	}

	auto local_offset = m_dynamicBuffers[in_buffer][frame].used.fetch_add( in_bytes );
    return local_offset = m_dynamicBuffers[in_buffer][frame].offset + local_offset;
}

/*
==============
idVertexCache::FlushTransferList
==============
*/
void idVertexCache::FlushTransferLists( void )
{
	VkCopyBufferInfo2 copyBuffer{};
	
	///
	/// Submit index copy 
	copyBuffer.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
	copyBuffer.pNext = nullptr;
	copyBuffer.regionCount = m_copyListIndex.Num();
	copyBuffer.pRegions = m_copyListIndex.Ptr();
	copyBuffer.srcBuffer = *m_staging;
	copyBuffer.dstBuffer = *m_drawBuffer[CACHE_INDEX_BUFFER];
	
	/// Upload index buffer
	vkCmdCopyBuffer2( *m_copyCommands, &copyBuffer );

	///
	/// sumit vertex copy
	copyBuffer.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
	copyBuffer.pNext = nullptr;
	copyBuffer.regionCount = m_copyListVertex.Num();
	copyBuffer.pRegions = m_copyListVertex.Ptr();
	copyBuffer.srcBuffer = *m_staging;
	copyBuffer.dstBuffer = *m_drawBuffer[CACHE_VERTEX_BUFFER];
	
	/// Upload buffer
	vkCmdCopyBuffer2( *m_copyCommands, &copyBuffer );
}
