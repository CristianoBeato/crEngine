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

#ifndef __GL_SWAPCHAIN_HPP__
#define __GL_SWAPCHAIN_HPP__

class glSwapchain : public crSwapchain
{
public:
    glSwapchain( const uint32_t in_width, const uint32_t in_height );
    ~glSwapchain( void );
    virtual bool    Recreate( const uint32_t in_width, const uint32_t in_height );
    virtual void    AcquireImage( void );
    virtual void    PresentImage( void );
private:
    idStaticList<GLuint, SMP_FRAMES>  m_framebuffers;
    idStaticList<GLuint, SMP_FRAMES>  m_depthStencil;
    idStaticList<GLuint, SMP_FRAMES>  m_renderbuffers;
};

#endif //!__GL_SWAPCHAIN_HPP__