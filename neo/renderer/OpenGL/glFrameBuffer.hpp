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

#ifndef __GL_FRAMEBUFFER_HPP__
#define __GL_FRAMEBUFFER_HPP__

class glFrameBuffer : public crFramebuffer
{
public:
    glFrameBuffer( void );
    ~glFrameBuffer( void );

    virtual bool Create(    const uint32_t in_bufferCount,
                            const uint32_t in_width,
                            const uint32_t in_height,
                            const Attachament_t* in_attachaments, 
                            const uint32_t in_numAttachaments );

    virtual bool    Resize( const uint32_t in_width, const uint32_t in_height );
    
    virtual void    Destroy( void );

    virtual void*   Handle( void ) const = 0;


private:
    GLuint  m_frameBuffer;
};

#endif //!__GL_FRAMEBUFFER_HPP__