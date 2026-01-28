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

#ifndef __GL_BUFFER_HPP__
#define __GL_BUFFER_HPP__

class glBuffer : public crBuffer
{
public:
    glBuffer( void );
    ~glBuffer( void );

    virtual bool    Create( const type_t in_type, const access_t in_acess, const size_t in_size );
    virtual bool    Resize( const size_t in_newSize );
    virtual void    Destroy( void );
    virtual void    CopyBuffer( const crBuffer* in_source, const uintptr_t in_srcOffset, const uintptr_t in_dstOffset, const size_t in_size ) const;
    virtual void    Flush( const uintptr_t in_offset, const size_t in_size ) const;
    virtual void    StateTransition( const state_t in_state, const crCommandBuffer* in_commandBuffer );
    virtual void*   Handle( void ) const;
    
    const GLenum    Target( void ) const { return m_target; }
    const GLuint    Buffer( void ) const { return m_buffer; }

private:
    GLbitfield  m_flags;
    GLbitfield  m_stateFlags;
    GLenum      m_target;
    GLuint      m_buffer;
};

#endif //__GL_BUFFER_HPP__