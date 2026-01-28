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

#ifndef __GL_TEXTURE_HPP__
#define __GL_TEXTURE_HPP__

class glSampler : public crSampler
{
public:
    glSampler( void );
    ~glSampler( void );
    virtual bool    Create( const filter_t in_filtering, const wrapping_t in_Swrap, const wrapping_t in_Twrap, const wrapping_t in_Rwrap );
    virtual void    Destroy( void );
    virtual void*   Handler( void ) const;

private:
    GLuint  m_sampler;
};

class glTexture : public crTexture
{
public:
    glTexture( void );
    ~glTexture( void );
    virtual bool    Create( const type_t in_type, const dimensions_t in_dimensions, const format_t in_format );
    virtual void    Destroy( void );
    virtual void*   Handler( void ) const;

    const GLuint    Texture( void ) const { return m_texture; }
    const GLenum    Target( void ) const { return m_target; }
    const GLenum    Internalformat( void ) const { return m_internalformat; }

private:
    GLenum  m_target;
    GLenum  m_internalformat;
    GLuint  m_texture;
};

#endif //!__GL_TEXTURE_HPP__