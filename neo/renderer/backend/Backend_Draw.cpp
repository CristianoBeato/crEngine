
#include "Backend.hpp"

static VkDeviceSize vertexStride = sizeof( idDrawVert );

/*
==================
crBackend::CopyRender
==================
*/
void crBackend::CopyRender(const void *data)
{
}

/*
==================
crBackend::PostProcess
==================
*/
void crBackend::PostProcess( const void* data )
{
    
}

/*
==================
crBackend::DrawView
==================
*/
void crBackend::DrawView(const void *data, const int stereoEye)
{
}

static inline void vkCmdBindVertexBuffer( VkCommandBuffer in_commandBuffer, const VkBuffer in_buffers, const VkDeviceSize in_offsets )
{
	vkCmdBindVertexBuffers( in_commandBuffer, 0, 1, &in_buffers, &in_offsets );
}

/*
==================
crBackend::DrawElementsWithCounters
==================
*/
void crBackend::DrawElementsWithCounters( const drawSurf_t *surf )
{
	int32_t firstVertex = 0;
	uint32_t firstIndex = 0;
    crBufferp indexBuffer = nullptr;
    crBufferp vertexBuffer = nullptr; 
    auto backEnd = crBackend::Get();    
    auto cmdBuffer = tr.GraphicCommandBuffer();

    // get vertex buffer
    const vertCacheHandle_t vbHandle = surf->ambientCache;
	vertexBuffer = vertexCache.GetBuffer( vbHandle );
	const uintptr_t vertOffset = vbHandle.offset;

	// get index buffer
	const vertCacheHandle_t ibHandle = surf->indexCache;
    indexBuffer = vertexCache.GetBuffer( ibHandle );
	const uintptr_t indexOffset = ibHandle.offset;

	RENDERLOG_PRINTF( "Binding Buffers: %p:%i %p:%i\n", vertexBuffer, vertOffset, indexBuffer, indexOffset );

	/// Update index buffer
    if ( backEnd->trState.currentIndexBuffer != indexBuffer || !r_useStateCaching.GetBool() ) 
    {
		// qglBindBufferARB( GL_ELEMENT_ARRAY_BUFFER_ARB, (GLuint)indexBuffer->GetAPIObject() );
		vkCmdBindIndexBuffer( *cmdBuffer, *indexBuffer, 0, VK_INDEX_TYPE_UINT16 );
		backEnd->trState.currentIndexBuffer = indexBuffer;
	}

	/// Update vertex buffer
	if( ( backEnd->trState.currentVertexBuffer != vertexBuffer ) || !r_useStateCaching.GetBool() )
	{
		vkCmdBindVertexBuffer( *cmdBuffer, *vertexBuffer, 0 );
		backEnd->trState.currentVertexBuffer = vertexBuffer;
	}

	/// Get vertex and index position on buffers
	firstIndex = sizeof( uint16_t ) * surf->indexCache.offset;
	firstVertex = sizeof( idDrawVert ) * surf->ambientCache.offset;

    /// Issue the draw command
    vkCmdDrawIndexed( *cmdBuffer, r_singleTriangle.GetBool() ? 3 : surf->numIndexes, 1, firstIndex, firstVertex, 0 );
}