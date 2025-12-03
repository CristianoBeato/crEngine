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

#ifndef __GL_SHADER_STORAGE_HPP__
#define __GL_SHADER_STORAGE_HPP__

class glBindlessTextureSlot : public crBindlessTextureSlot
{
public:
    glBindlessTextureSlot( const GLuint in_textureName, const GLuint in_samplerName ) : m_handle( 0 ) 
    {
        // create the shader acess handle and make resdent 
        m_handle = glGetTextureSamplerHandleARB( in_textureName, in_samplerName );
        glMakeTextureHandleResidentARB( m_handle );
    }

    ~glBindlessTextureSlot( void )
    {
        // release the shader
        glMakeTextureHandleNonResidentARB( m_handle );
    };

    GLuint64    GetHandle( void ) const { return m_handle; }

private:
    GLuint64    m_handle;
};

class glShaderStorage : public crShaderStorage
{
public:
    glShaderStorage( void );
    ~glShaderStorage( void );

    virtual void                    StartUp( void );
    virtual void                    ShutDown( void );
    virtual crBindlessTextureSlot*  BindTexture( const crTexture* in_texture, const crSampler* in_sampler );
    virtual void                    FreeSlot( crBindlessTextureSlot* &in_handle );

private:
    glBuffer*   m_TSSSBO;
};

#endif //!__GL_SHADER_STORAGE_HPP__ 