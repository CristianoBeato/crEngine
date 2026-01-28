/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos.

This file is part of the crEngine GPL Source Code ("crEngine Source Code").

crEngine Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

crEngine Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with crEngine Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#include "precompiled.h"
#include "renderer_common.h"
#include "glCommandBuffer.hpp"

glCommandBuffer::glCommandBuffer( void ) : crCommandBuffer()
{
}

glCommandBuffer::~glCommandBuffer( void )
{
}

void glCommandBuffer::Begin(void)
{
}

void glCommandBuffer::End(void)
{
}

void glCommandBuffer::Sumit(void)
{
}

void glCommandBuffer::LineWidth( const float in_lineWidth ) const
{
	glLineWidth( in_lineWidth );
}

void glCommandBuffer::BindFrameBuffer( const crFramebuffer *in_framebuffef )
{
	assert( in_framebuffef != nullptr );
	auto fbHandle = static_cast<GLuint*>( in_framebuffef->Handle() );
	tr.glContext->BindFrameBuffer( *fbHandle );

	// TODO: clear frame buffer 
}

void glCommandBuffer::BindIndexBuffer( const crBuffer *in_buffer )
{
	assert( in_buffer != nullptr );
	m_indexBuffer = *static_cast<GLuint*>( in_buffer->Handle() );
	glVertexArrayElementBuffer( m_vertexArray, m_indexBuffer );
}

void glCommandBuffer::BindVertexBuffers(const crBuffer *in_buffer, uint32_t in_binding, const uintptr_t in_offset, const size_t in_size, const size_t in_stride)
{
	assert( in_buffer != nullptr );
	m_vertexBuffer = *static_cast<GLuint*>( in_buffer->Handle() );
	glVertexArrayVertexBuffer( m_vertexArray, in_binding, m_vertexBuffer, in_offset, in_stride );
}

void glCommandBuffer::BindPipeline( const crPipeline *in_pipeline )
{
	assert( in_pipeline != nullptr );
	auto pipeline = dynamic_cast<const glPipeline*>( in_pipeline );

	// TODO:
	// glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	// glDepthRange(0.0, 1.0);

	//
	m_vertexArray = pipeline->m_vertexArray;
	tr.glContext->BindVertexArray( m_vertexArray );

	// update face culling
	tr.glContext->FaceCull( pipeline->m_cullFace, pipeline->m_cullFaceMode );

	// update depth test
	tr.glContext->DepthTest( pipeline->m_depthTest, pipeline->m_depthFunc );	

	// update blending
	tr.glContext->Blending(	pipeline->m_blendEnable, 
							pipeline->m_blendSRCFactor, 
							pipeline->m_blendSRCAlphaFactor,
							pipeline->m_blendDSTFactor,
							pipeline->m_blendDSTAlphaFactor,
							pipeline->m_blendOp );
	
	tr.glContext->StencilTest(	pipeline->m_stencilEnable, pipeline->m_stencilFace, pipeline->m_stencilPass, pipeline->m_stencilFail, pipeline->m_stencilZfail );
}

void glCommandBuffer::EndRenderPass(void) const
{
}

void glCommandBuffer::Draw(const uint32_t in_vertexCount, const uint32_t in_instanceCount, const uint32_t in_firstVertex, const uint32_t in_firstInstance) const
{
	glDrawArraysInstanced( GL_TRIANGLES, in_firstInstance, in_vertexCount, in_instanceCount  );
}

void glCommandBuffer::DrawIndexed(const uint32_t in_indexCount, const uint32_t in_instanceCount, const uint32_t in_firstIndex, const int32_t in_vertexOffset, const uint32_t in_firstInstance) const
{
	glDrawElementsInstancedBaseVertex( GL_TRIANGLES, in_indexCount, GL_INDEX_TYPE,  (void*)(sizeof(triIndex_t) * in_firstIndex), in_instanceCount, in_vertexOffset );
}

void glCommandBuffer::Dispatch(const uint32_t in_groupCountX, const uint32_t in_groupCountY, const uint32_t in_groupCountZ) const
{
	glDispatchCompute( in_groupCountX, in_groupCountY, in_groupCountZ );
}

void glCommandBuffer::Clear(const bool in_color, const bool in_depth, const bool in_stencil, const byte in_stencilValue, const float in_red, const float in_green, const float in_blue, const float in_alpha)
{
    GLbitfield clearFlags = 0;
	if( in_color )
	{
		glClearColor( in_red, in_green, in_blue, in_alpha );
		clearFlags |= GL_COLOR_BUFFER_BIT;
	}

	if( in_depth )
	{
		clearFlags |= GL_DEPTH_BUFFER_BIT;
	}
    
	if( in_stencil )
	{
		glClearStencil( in_stencilValue );
		clearFlags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear( clearFlags );
}

void glCommandBuffer::PolygonOffset( const float in_scale, const float in_bias, const bool enable )
{
	if ( enable )
	{
		glEnable( GL_POLYGON_OFFSET_FILL );
		glEnable( GL_POLYGON_OFFSET_LINE );
		glPolygonOffset( in_scale, in_bias );
	}
	else
	{
		glDisable( GL_POLYGON_OFFSET_FILL );
		glDisable( GL_POLYGON_OFFSET_LINE );
	}
	
}

void glCommandBuffer::DepthBoundsTest(const float zmin, const float zmax, const bool enable )
{
	// TODO by shader
	if ( enable )
	{
		glEnable( GL_DEPTH_BOUNDS_TEST_EXT );
		glDepthBoundsEXT( zmin, zmax );
	}
	else
		glDisable( GL_DEPTH_BOUNDS_TEST_EXT );	
}

void glCommandBuffer::Scissor(int x, int y, int w, int h) const
{
    glScissor( x, y, w, h );
}

void glCommandBuffer::Viewport(int x, int y, int w, int h) const
{
	glViewport( x, y, w, h );
}

void glCommandBuffer::CopyTexture(const crTexture *in_src, const crTexture *in_dst, const idList<crTexture::subImage_t> in_subImages)
{
	assert( !in_src || !in_dst );
	const glTexture* srcImage = dynamic_cast<const glTexture*>( in_src );
	const glTexture* dstImage = dynamic_cast<const glTexture*>( in_dst );
	
	// Make sure any previous writing is finished.
	glMemoryBarrier( GL_TEXTURE_UPDATE_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

	for ( uint32_t i = 0; i < in_subImages.Num(); i++)
	{
		crTexture::subImage_t image = in_subImages[i];

		// perform the copy 
		glCopyImageSubData(	srcImage->Texture(),
							srcImage->Target(),
							image.level,
							0, 0, 0,
							dstImage->Texture(),
							dstImage->Target(),
							image.level,
							0, 0, 0,
							image.width,
							image.height,
							image.depth );		
	}

	// prepare image for use
	glMemoryBarrier( GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT );	
}

void glCommandBuffer::CopyBuffer( const crBuffer *in_srcBuffer, const crBuffer *in_dstBuffer, const uintptr_t in_offset, const size_t in_size )
{
	const glBuffer* src = dynamic_cast<const glBuffer*>( in_srcBuffer );
	const glBuffer* dst = dynamic_cast<const glBuffer*>( in_dstBuffer );

	///
	//glMemoryBarrier( GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_TRANSFORM_FEEDBACK_BARRIER_BIT );

	glCopyNamedBufferSubData( src->Buffer(), dst->Buffer(), in_offset, in_offset, in_size );
}
