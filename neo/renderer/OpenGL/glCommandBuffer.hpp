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

#ifndef __GL_COMMAND_BUFFER_HPP__
#define __GL_COMMAND_BUFFER_HPP__

// this is just a workarround, since OpenGL use a single driver internal command buffer
class glCommandBuffer : public crCommandBuffer
{
public:
    glCommandBuffer( void );
    ~glCommandBuffer( void );
    virtual void    Begin( void );
    virtual void    End( void );
    virtual void    Sumit( void );
    virtual void    LineWidth( const float in_lineWidth ) const;
    virtual void    BindFrameBuffer( const crFramebuffer* in_framebuffef );
    virtual void    BindIndexBuffer( const crBuffer* in_buffer );
    virtual void    BindVertexBuffers( const crBuffer* crPipelinein_buffer, uint32_t in_binding, const uintptr_t in_offsets, const size_t in_sizes, const size_t in_strides );
    virtual void    BindPipeline( const crPipeline* in_pipeline );
    virtual void    EndRenderPass( void ) const;
    virtual void    Draw(  const uint32_t in_vertexCount, const uint32_t in_instanceCount, const uint32_t in_firstVertex, const uint32_t in_firstInstance ) const;
    virtual void    DrawIndexed( const uint32_t in_indexCount, const uint32_t in_instanceCount, const uint32_t in_firstIndex, const int32_t in_vertexOffset, const uint32_t in_firstInstance ) const;
    virtual void    Dispatch(  const uint32_t in_groupCountX, const uint32_t in_groupCountY, const uint32_t in_groupCountZ ) const;
    virtual void    Clear( bool color, bool depth, bool stencil, byte stencilValue, float r, float g, float b, float a );
    virtual void    PolygonOffset( float scale, float bias );
    virtual void    DepthBoundsTest( const float zmin, const float zmax );
    virtual void    Scissor( int x /* left*/, int y /* bottom */, int w, int h ) const;
    virtual void    Viewport( int x /* left */, int y /* bottom */, int w, int h ) const;
private:
    GLuint  m_indexBuffer;
    GLuint  m_vertexBuffer;
    GLuint  m_vertexArray; // current vertex array
};

#endif //!__GL_COMMAND_BUFFER_HPP__