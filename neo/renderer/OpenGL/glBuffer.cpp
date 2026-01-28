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
#include "glBuffer.hpp"

glBuffer::glBuffer( void ) : crBuffer(), m_buffer( 0 )
{
}

glBuffer::~glBuffer( void )
{
    Destroy();
}

bool glBuffer::Create(const type_t in_type, const access_t in_acess, const size_t in_size)
{
#if 1
    GLbitfield flags = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT; // don't need to flush  
#else
    GLbitfield flags = GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT; // need to flush 
#endif 

    Destroy();

    glCreateBuffers( 1, &m_buffer );
    if ( m_buffer == 0 )
        return false;    

    switch ( in_acess )
    {
        case BUFFER_ACCESS_WRITE:
            flags = GL_MAP_WRITE_BIT;
            break;
        case BUFFER_ACCESS_READ:
            flags = GL_MAP_READ_BIT;
            break;
        case BUFFER_ACCESS_NONE:
        {
            // TODO: prin a error
            return false;
        }; 
    }

    // create a constant mapped buffer, for easy and fast data acess
    glNamedBufferStorage( m_buffer, in_size, nullptr, flags );
    m_data = glMapNamedBufferRange( m_buffer, 0, in_size, flags );
    
    m_size = in_size;
    m_type = in_type;
    m_access = in_acess;
    return glIsBuffer( m_buffer ) == GL_TRUE;
}

bool glBuffer::Resize(const size_t in_newSize)
{
    GLuint newBuffer = 0;
#if 1
    m_flags = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT; // don't need to flush  
#else
    GLbitfield flags = GL_MAP_PERSISTENT_BIT | GL_MAP_UNSYNCHRONIZED_BIT; // need to flush 
#endif 

    // relase buffer map 
    glUnmapNamedBuffer( m_buffer );

    switch ( m_access )
    {
        case BUFFER_ACCESS_WRITE:
            m_flags |= GL_MAP_WRITE_BIT;
            break;
        case BUFFER_ACCESS_READ:
            m_flags |= GL_MAP_READ_BIT;
            break;
        case BUFFER_ACCESS_NONE:
        {
            // TODO: prin a error
            return false;
        }; 
    }

    glCreateBuffers( 1, &newBuffer );
    if ( newBuffer == 0 )
        return false;    

    // create a constant mapped buffer, for easy and fast data acess
    glNamedBufferStorage( newBuffer, in_newSize, nullptr, m_flags );
    if ( glIsBuffer( newBuffer ) == GL_TRUE )
    {
        // copy old buffer content
        size_t size = std::max( m_size, in_newSize );
        glCopyNamedBufferSubData( m_buffer, newBuffer, 0, 0, size );
    }
    else
    {
        // TODO: print error
        return false;
    }
    
    // destroy old buffer 
    glDeleteBuffers( 1, &m_buffer );

    // update buffer handle and size
    m_buffer = newBuffer;
    m_size = in_newSize;

    // remap buffer 
    m_data = glMapNamedBufferRange( m_buffer, 0, m_size, m_flags );

    return false;
}

void *glBuffer::Handle(void) const
{
    return const_cast<GLuint*>( &m_buffer );
}

void glBuffer::Flush(const uintptr_t in_offset, const size_t in_size) const
{
    assert( m_buffer != 0 );
    glFlushMappedNamedBufferRange( m_buffer, in_offset, in_size );
}

// Just to remember
// GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
// GL_ELEMENT_ARRAY_BARRIER_BIT;
// GL_UNIFORM_BARRIER_BIT
// GL_TEXTURE_FETCH_BARRIER_BIT
// GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
// GL_COMMAND_BARRIER_BIT
// GL_PIXEL_BUFFER_BARRIER_BIT
// GL_TEXTURE_UPDATE_BARRIER_BIT
// GL_BUFFER_UPDATE_BARRIER_BIT
// GL_FRAMEBUFFER_BARRIER_BIT
// GL_TRANSFORM_FEEDBACK_BARRIER_BIT
// GL_ATOMIC_COUNTER_BARRIER_BIT
// GL_SHADER_STORAGE_BARRIER_BIT

void glBuffer::StateTransition( const state_t in_state, const crCommandBuffer* in_commandBuffer )
{
    GLbitfield stateFlags = 0; // clear

    switch ( m_type )
    {
        case BUFFER_TYPE_UNDEFINED:
        {
            // TODO: print a warn 
        } break;
        case BUFFER_TYPE_INDEX:
        {
            stateFlags |= GL_ELEMENT_ARRAY_BARRIER_BIT;
        } break;
        case BUFFER_TYPE_VERTEX:
        {
            stateFlags |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
        } break;
        case BUFFER_TYPE_SHADER:
        {
            stateFlags |= GL_SHADER_STORAGE_BARRIER_BIT;
        } break;
        case BUFFER_TYPE_COMMANDS:
        {
            stateFlags |= GL_SHADER_STORAGE_BARRIER_BIT;
        } break;
        case BUFFER_TYPE_PIXEL:
        {
            stateFlags |= GL_PIXEL_BUFFER_BARRIER_BIT;
        } break;
    }

#if 0
    switch( in_state )
    {
        case RESOURCE_STATE_UNKNOW:
        {

        } break;
        case RESOURCE_STATE_COPY_DESTINATION:
        {

        } break;
        case RESOURCE_STATE_COPY_SOURCE:
        {

        } break;
        case RESOURCE_STATE_USE_RENDER:
        {

        } break;
        case RESOURCE_STATE_USE_COMPUTE:
        {

        } break;
        case RESOURCE_STATE_WRITE_COMPUTE:
        {

        } break;
        case RESOURCE_STATE_WRITE_RENDER:
        {

        } break;
    };
#endif

    if ( m_stateFlags != stateFlags )
    {
        // apply a memory barrier 
        glMemoryBarrier( stateFlags );
        m_stateFlags == stateFlags;
    }

    m_state = in_state;
}

void glBuffer::Destroy(void)
{
    if( m_buffer !=  0 )
    {
        glDeleteBuffers( 1, &m_buffer );
        m_buffer = 0;
    }
}

void glBuffer::CopyBuffer(const crBuffer *in_source, const uintptr_t in_srcOffset, const uintptr_t in_dstOffset, const size_t in_size) const
{
    const glBuffer *source = dynamic_cast<const glBuffer*>( in_source );
    glCopyNamedBufferSubData( source->m_buffer, m_buffer, in_srcOffset, in_dstOffset, in_size );
}
