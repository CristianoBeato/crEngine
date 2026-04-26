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
	uint32_t i = 0;
	vkDeviceQueuep queue = nullptr;
	auto device = tr.GetRenderDevice();

	currentFrame = 0;
	listNum = 0;
	
	mostUsedVertex = 0;
	mostUsedIndex = 0;
	
/// BEATO Begin
	if ( glConfig.isTransferQueueAvailable )
		queue = dynamic_cast<crVulkanRenderDevicep>( tr.GetRenderDevice() )->TransferQueue();
	else
		queue = dynamic_cast<crVulkanRenderDevicep>( tr.GetRenderDevice() )->GraphicQueue();

	///
	/// Create Buffers
	m_buffers.SetNum( CACHE_BUFFERS_COUNT );
	
	VkDeviceSize minAlign = 0;
	VkDeviceSize currentOffset = 0;
    uint32_t memoryTypeBits = 0xFFFFFFFF;
	for ( i = 0; i < CACHE_BUFFERS_COUNT; i++)
	{
		VkMemoryRequirements req{};
		size_t size = 0;
		crBuffer::type_t type;
		switch ( i )
		{
			case CACHE_INDEX_STATIC:
			{
				size = STATIC_INDEX_MEMORY;
				type = crBuffer::BUFFER_TYPE_INDEX;
			} break;
			case CACHE_VERTEX_STATIC:
			{
				size = STATIC_INDEX_MEMORY;
				type = crBuffer::BUFFER_TYPE_VERTEX;
			} break;
			case CACHE_INDEX_DYNAMIC:
			{
				size = in_frames * VERTCACHE_INDEX_MEMORY_PER_FRAME;
				type = crBuffer::BUFFER_TYPE_INDEX;
			} break;
			case CACHE_VERTEX_DYNAMIC:
			{
				size = in_frames * VERTCACHE_VERTEX_MEMORY_PER_FRAME;
				type = crBuffer::BUFFER_TYPE_VERTEX;
			} break;
		}

		m_buffers[i] = new crBuffer();
		if( !m_buffers[i]->Create( type, crBuffer::BUFFER_ACCESS_WRITE, size ) )
			throw idException( "Failed to create vertex cache buffers" );

		req = m_buffers[i]->MemoryRequirements();

		minAlign = std::max( minAlign, req.alignment );

		// Aligns the current offset according to the buffer requirement.
        currentOffset = __align( currentOffset, req.alignment );
    
        // Add the buffer size to the offset for the next
        currentOffset += req.size;

        // Combines the supported memory type bits (bit-by-bit AND).
        memoryTypeBits &= req.memoryTypeBits;
	}

	/// Allocate the the memory pool for the rende buffers
    m_renderBuffersPool = device->Alloc( currentOffset, minAlign, memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if( m_renderBuffersPool )
        idLib::FatalError( "Vertex Cache Buffers memory pool allocation failed, no suitable memory four or no available memory size\n" );
	
	/// now allocate buffer memory
	for ( i = 0; i < CACHE_BUFFERS_COUNT; i++)
	{
		if( !m_buffers[i]->Storage( m_renderBuffersPool ) )
			throw idException("");
	}
	
	///
	///________________________________________________________________________
	/// Create the staging buffer
	///
	m_staging = new crBuffer();
	if( !m_staging->Create( crBuffer::BUFFER_TYPE_SOURCE, crBuffer::BUFFER_ACCESS_WRITE, VERTCACHE_STAGING_MEMORY_PER_FRAME ) );
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
	for ( uint32_t i = 0; i < CACHE_BUFFERS_COUNT; i++)
	{
		VK_SAFE_DESTROY( m_buffers[i] );
	}

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

/*
==============
idVertexCache::EndMapLoad
==============
*/
void idVertexCache::EndMapLoad(void)
{
}

/*
 ==============
 idVertexCache::FreeStaticData
 call on loading a new map
 ==============
 */
void idVertexCache::FreeStaticData( void )
{
	m_caches[CACHE_INDEX_STATIC].Clear();
	m_caches[CACHE_VERTEX_STATIC].Clear();
	mostUsedVertex = 0;
	mostUsedIndex = 0;
}

/*
==============
idVertexCache::ActuallyAlloc
==============
*/
vertCacheHandle_t idVertexCache::ActuallyAlloc( const void* data, const size_t bytes, const cache_type_t  type )
{
	uintptr_t endPos = 0;
	vertCacheHandle_t handle{};
	
	if( bytes == 0 )
		return vertCacheHandle_t();
	
	// RB: changed UINT_PTR to uintptr_t
	assert( ( ( ( uintptr_t )( data ) ) & 15 ) == 0 );
	// RB end
	
	assert( ( bytes & 15 ) == 0 );
	
	// thread safe interlocked adds
	endPos = m_caches[type].memoryUsed + bytes;
	if( type == CACHE_VERTEX_STATIC )
	{
		if( endPos > m_buffers[type]->Size() )
			idLib::Error( "Out of index cache" );
	} 
	else if( type == CACHE_INDEX_STATIC )
	{
		if( endPos > m_buffers[type]->Size() )
			idLib::Error( "Out of vertex cache" );
	}
	else if( type == CACHE_VERTEX_DYNAMIC ) 
	{
		//if( ( m_caches[type].memoryUsedFrame + bytes ) > VERTCACHE_INDEX_MEMORY_PER_FRAME )
	}

	m_caches[type].allocations++;
	
	uintptr_t offset = endPos - bytes;
	
	// Actually perform the data transfer
	if( data != nullptr )
	{
		/// copy data to staging buffer
		uintptr_t staging = UploadStage( data, bytes );

		/// Register command to copy from staging to vertex buffer
		UploadBuffer( type, bytes, staging, offset );
	}
	
	handle.frame = currentFrame;
	handle.offset = offset;
	handle.size = bytes;

	if( type == CACHE_INDEX_STATIC || type == CACHE_VERTEX_DYNAMIC )
		handle.flags |= CACHE_STATIC;

	if( type == CACHE_INDEX_STATIC || type == CACHE_INDEX_DYNAMIC )
		handle.flags |= CACHE_INDEX;

	return handle;
}

/*
==============
idVertexCache::UploadBuffer
==============
*/
void idVertexCache::UploadBuffer( const cache_type_t in_type, const size_t in_size, const uintptr_t in_srcOffset, const uintptr_t in_dstOffset )
{
	uint32_t queueFamily = VK_QUEUE_FAMILY_IGNORED;
	if( glConfig.isTransferQueueAvailable )
		queueFamily = tr.GetRenderDevice()->TransferQueue()->Family();
	else
		queueFamily = tr.GetRenderDevice()->GraphicQueue()->Family();

	///
	/// update source buffer state
	crBuffer::state_t sourceState{};
	sourceState.queueFamily = queueFamily;
	sourceState.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	sourceState.access = VK_ACCESS_2_TRANSFER_READ_BIT;
	m_buffers[in_type]->SetState( m_copyCommands, sourceState );

	///
	/// update staging buffer state
	crBuffer::state_t stagingState{};
	stagingState.queueFamily = queueFamily;
	stagingState.stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	stagingState.access = VK_ACCESS_2_TRANSFER_READ_BIT;
	m_staging->SetState( m_copyCommands, stagingState );

	///
	/// copy content
	VkBufferCopy2 bufferCopy{};
	bufferCopy.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
	bufferCopy.pNext = nullptr;
	bufferCopy.srcOffset = static_cast<VkDeviceSize>( in_srcOffset );
	bufferCopy.dstOffset = static_cast<VkDeviceSize>( in_dstOffset );
	bufferCopy.size = static_cast<VkDeviceSize>( in_size );

	VkCopyBufferInfo2 copyBuffer{};
	copyBuffer.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
	copyBuffer.pNext = nullptr;
	copyBuffer.regionCount = 1;
	copyBuffer.pRegions = &bufferCopy;
	copyBuffer.srcBuffer = *m_staging;
	copyBuffer.dstBuffer = *m_buffers[in_type];
	
	/// Upload buffer
	vkCmdCopyBuffer2( *m_copyCommands, &copyBuffer );
}

/*
==============
idVertexCache::UploadStage
==============
*/
uintptr_t idVertexCache::UploadStage( const void *in_data, const size_t in_size )
{
	uintptr_t offset = 0;
	
	if( ( m_stagingOffset + in_size ) > VERTCACHE_STAGING_MEMORY_PER_FRAME )
		m_stagingOffset.store( 0 );

	offset = m_stagingOffset.load();

	/// copy to stagin buffer memory
	std::memcpy( static_cast<byte*>( m_stagingMap ) + offset, in_data, in_size );

	/// move offset
	m_stagingOffset.fetch_add( in_size );
    return offset;
}

/*
==============
idVertexCache::GetBuffer
==============
*/
crBufferp idVertexCache::GetBuffer(const vertCacheHandle_t &in_handle) const
{
	crBufferp buffer = nullptr;
	if( in_handle.flags &CACHE_STATIC )
	{
		if( in_handle.flags &CACHE_INDEX )
			buffer = const_cast<crBufferp>( m_buffers[CACHE_INDEX_STATIC] );
		else
			buffer = const_cast<crBufferp>( m_buffers[CACHE_VERTEX_STATIC] );
	}
	else
	{
		const uint32_t frameNum = in_handle.frame;
		if ( frameNum != ( vertexCache.currentFrame - 1 ) ) 
        {
			idLib::Warning( "crBackend::DrawElementsWithCounters, vertexBuffer == NULL" );
			return nullptr;
		}
	
		if( in_handle.flags &CACHE_INDEX )
			buffer = const_cast<crBufferp>( m_buffers[CACHE_INDEX_DYNAMIC] );
		else
			buffer = const_cast<crBufferp>( m_buffers[CACHE_VERTEX_DYNAMIC] );
	}

	return buffer;
}

/*
==============
idVertexCache::BeginBackEnd
==============
*/
void idVertexCache::BeginBackEnd( void )
{
	uint32_t allocations = 0;
	mostUsedVertex = std::max<uint32_t>( mostUsedVertex, m_caches[CACHE_VERTEX_DYNAMIC].memoryUsed.load() );
	mostUsedIndex = std::max<uint32_t>( mostUsedIndex, m_caches[CACHE_INDEX_DYNAMIC].memoryUsed.load() );
	allocations += m_caches[CACHE_VERTEX_DYNAMIC].allocations;
	allocations += m_caches[CACHE_INDEX_DYNAMIC].allocations;

	if( r_showVertexCache.GetBool() )
	{
		idLib::Printf( "%08d: %d allocations, %dkB vertex, %dkB index: %dkB vertex, %dkB index, %kB joint\n",
			currentFrame, allocations,
			m_caches[listNum].memoryUsed.load() / 1024,
			m_caches[listNum].memoryUsed.load() / 1024,
			mostUsedVertex / 1024,
			mostUsedIndex / 1024 );
	}

	m_copyCommands->BeginSubCommand();
		
	m_caches[CACHE_INDEX_DYNAMIC].memoryUsedFrame.store( 0 );
	m_caches[CACHE_VERTEX_DYNAMIC].memoryUsedFrame.store( 0 );

#if 0
	// unmap the current frame so the GPU can read it
	const int startUnmap = Sys_Milliseconds();
	UnmapGeoBufferSet( frameData[listNum] );
	UnmapGeoBufferSet( staticData );
	const int endUnmap = Sys_Milliseconds();
	if( endUnmap - startUnmap > 1 )
		idLib::PrintfIf( r_showVertexCacheTimings.GetBool(), "idVertexCache::unmap took %i msec\n", endUnmap - startUnmap );
	
	drawListNum = listNum;
	
	// prepare the next frame for writing to by the CPU
	currentFrame++;
	
	listNum = currentFrame % VERTCACHE_NUM_FRAMES;
	const int startMap = Sys_Milliseconds();
	MapGeoBufferSet( frameData[listNum] );
	const int endMap = Sys_Milliseconds();
	if( endMap - startMap > 1 )
	{
		idLib::PrintfIf( r_showVertexCacheTimings.GetBool(), "idVertexCache::map took %i msec\n", endMap - startMap );
	}
	
	ClearGeoBufferSet( frameData[listNum] );
	
	const int startBind = Sys_Milliseconds();
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, ( GLuint )frameData[drawListNum].vertexBuffer.GetAPIObject() );
	glBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, ( GLuint )frameData[drawListNum].indexBuffer.GetAPIObject() );
	const int endBind = Sys_Milliseconds();
	if( endBind - startBind > 1 )
	{
		idLib::Printf( "idVertexCache::bind took %i msec\n", endBind - startBind );
	}
#endif
}

void idVertexCache::EndBackEnd(void)
{
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
