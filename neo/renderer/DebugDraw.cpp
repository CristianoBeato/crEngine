/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos

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
along with crEngine Source Code. If not, see <http://www.gnu.org/licenses/>.

In addition, the crEngine Source Code is also subject to certain additional terms. 
You should have received a copy of these additional terms immediately following the 
terms and conditions of the GNU General Public License which accompanied the crEngine
Source Code.

===========================================================================
*/

#include "precompiled.h"
#include "tr_local.h"
#include "DebugDraw.hpp"

constexpr GLuint k_VERTEX_POSITION_ATTRIB = 0;
constexpr GLuint k_VERTEX_TEXTCOORD_ATTRIB = 0;
constexpr GLuint k_VERTEX_COLOR_ATTRIB = 0;
constexpr GLuint k_VERTEX_NORMAL_ATTRIB = 0;

constexpr size_t k_FLOAT_SIZE = sizeof( float );
constexpr size_t k_VEC2F_SIZE = sizeof( float ) * 2;
constexpr size_t k_VEC3F_SIZE = sizeof( float ) * 3;
constexpr size_t k_VEC4F_SIZE = sizeof( float ) * 4;

// uniform buffers sizes
constexpr size_t k_UNIFORMS_BUFFER_MODELVIEW_LOCATION = 0;
constexpr size_t k_UNIFORMS_BUFFER_MODELVIEW_SIZE = k_VEC4F_SIZE * 4;

constexpr size_t k_UNIFORMS_BUFFER_PROJECTION_LOCATION = k_UNIFORMS_BUFFER_MODELVIEW_LOCATION + k_UNIFORMS_BUFFER_MODELVIEW_SIZE;
constexpr size_t k_UNIFORMS_BUFFER_PROJECTION_SIZE = k_VEC4F_SIZE * 4;

constexpr size_t k_UNIFORMS_BUFFER_TEXTURE_LOCATION = k_UNIFORMS_BUFFER_MODELVIEW_LOCATION + k_UNIFORMS_BUFFER_MODELVIEW_SIZE;
constexpr size_t k_UNIFORMS_BUFFER_TEXTURE_SIZE = k_VEC4F_SIZE * 4;

constexpr size_t k_UNIFORMS_BUFFER_SIZE = k_UNIFORMS_BUFFER_TEXTURE_LOCATION * k_UNIFORMS_BUFFER_TEXTURE_SIZE;

//
GLint                       glDebugDraw::m_first = 0;
GLsizei                     glDebugDraw::m_count = 0;
GLenum                      glDebugDraw::m_mode = 0;
GLenum                      glDebugDraw::m_matrixMode = 0;
GLuint                      glDebugDraw::m_vertexArray = 0;
GLuint                      glDebugDraw::m_program = 0;
glDebugDraw::fixedVertex_t  glDebugDraw::m_vertex{};
glDebugDraw::fixedVertex_t* glDebugDraw::m_vertexes;

void glDebugDraw::StartUp( void )
{
    glCreateVertexArrays( 1, &m_vertexArray );

    // vertex position
    glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_POSITION_ATTRIB, 0 );
    glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_POSITION_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, x ) );

    // texture coordinate
    glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_TEXTCOORD_ATTRIB, 0 );
    glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_TEXTCOORD_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, s ) );

    // vertex color
    glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 0 );
    glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, s ) );

    // vertex normal
    glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 0 );
    glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, s ) );
}

void glDebugDraw::ShutDown( void )
{
    if ( m_program != 0 )
    {
        glDeleteProgram( m_program );
        m_program = 0;
    }

    if ( m_vertexArray )
    {
        glDeleteVertexArrays( 1, &m_vertexArray );
        m_vertexArray = 0;
    }
    
}

void glDebugDraw::Begin(const GLenum mode)
{
    m_mode = mode;
}

void glDebugDraw::End(void)
{
    GLint currentVao = 0;

    // check current VAO
    glGetIntegerv( GL_VERTEX_ARRAY_BINDING, &currentVao );
    
    // check current program 
    // glGetIntegerv( , &currentVao );

    glUseProgram( m_program );
    glBindVertexArray( m_vertexArray );
    glDrawArrays( m_mode, m_first, m_count );

    m_first += m_count;
    m_count = 0;

    glUseProgram( 0 );
    glBindVertexArray( 0 );
}

void glDebugDraw::Vertex2f( const GLfloat x, const GLfloat y )
{
    m_vertex.x = x;
    m_vertex.y = y;
    m_vertexes[m_count++] = m_vertex;
}

void glDebugDraw::Vertex2fv( const GLfloat *v)
{
    std::memcpy( &m_vertex.x, v, k_VEC2F_SIZE );
    m_vertexes[m_count++] = m_vertex;
}

void glDebugDraw::Vertex3f(const GLfloat x, const GLfloat y, const GLfloat z)
{
    m_vertex.x = x;
    m_vertex.y = y;
    m_vertex.z = z;
    m_vertexes[m_count++] = m_vertex;
}

void glDebugDraw::Vertex3fv(const GLfloat *v)
{
    std::memcpy( &m_vertex.x, v, k_VEC3F_SIZE );
    m_vertexes[m_count++] = m_vertex;
}

void glDebugDraw::Vertex4f( const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w )
{
    m_vertex.x = x;
    m_vertex.y = y;
    m_vertex.z = z;
    m_vertex.w = w;
    m_vertexes[m_count++] = m_vertex;
}

void glDebugDraw::Vertex4fv( const GLfloat *v )
{
    std::memcpy( &m_vertex.x, v, k_VEC4F_SIZE );
    m_vertexes[m_count++] = m_vertex;
}

void glDebugDraw::TexCoord1f(const GLfloat s)
{
    m_vertex.s = s;
}

void glDebugDraw::TexCoord1fv(const GLfloat *v)
{
    m_vertex.s = *v;
}

void glDebugDraw::TexCoord2f(const GLfloat s, const GLfloat t)
{
    m_vertex.s = s;
    m_vertex.t = t;
}

void glDebugDraw::TexCoord2fv(const GLfloat *v)
{
    std::memcpy( &m_vertex.s, v, k_VEC2F_SIZE );
}

void glDebugDraw::TexCoord3f( const GLfloat s, const GLfloat t, const GLfloat r )
{
    m_vertex.s = s;
    m_vertex.t = t;
    m_vertex.u = r;
}

void glDebugDraw::TexCoord3fv(const GLfloat *v)
{
    std::memcpy( &m_vertex.s, v, k_VEC3F_SIZE );
}

void glDebugDraw::TexCoord4f(const GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    m_vertex.s = s;
    m_vertex.t = t;
    m_vertex.u = r;
    m_vertex.v = q;
}

void glDebugDraw::TexCoord4fv(const GLfloat *v)
{
    std::memcpy( &m_vertex.s, v, k_VEC4F_SIZE );
}

void glDebugDraw::Color3f(const GLfloat red, const GLfloat green, const GLfloat blue)
{
    m_vertex.r = red;
    m_vertex.g = green;
    m_vertex.b = blue;
    m_vertex.a = 1.0f;
}

void glDebugDraw::Color3fv(const GLfloat *v)
{
    std::memcpy( &m_vertex.r, v, k_VEC3F_SIZE );
    m_vertex.a = 1.0f;
}

void glDebugDraw::Color4f(const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha)
{
    m_vertex.r = red;
    m_vertex.g = green;
    m_vertex.b = blue;
    m_vertex.a = alpha;
}

void glDebugDraw::Color4fv( const GLfloat *v )
{
    std::memcpy( &m_vertex.r, v, k_VEC4F_SIZE );
}

void glDebugDraw::Color3b( const GLbyte red, const GLbyte green, const GLbyte blue )
{
    m_vertex.r = static_cast<float>( red ) / 127.0f;
    m_vertex.g = static_cast<float>( green ) / 127.0f;
    m_vertex.b = static_cast<float>( blue ) / 127.0f;
    m_vertex.a = 1.0f;
}

void glDebugDraw::Color3bv( const GLbyte *v )
{
    m_vertex.r = static_cast<float>( v[0] ) / 127.0f;
    m_vertex.g = static_cast<float>( v[1] ) / 127.0f;
    m_vertex.b = static_cast<float>( v[2] ) / 127.0f;
    m_vertex.a = 1.0f;
}

void glDebugDraw::Color4b( const GLbyte red, const GLbyte green, const GLbyte blue, const GLbyte alpha )
{
    m_vertex.r = static_cast<float>( red ) / 127.0f;
    m_vertex.g = static_cast<float>( green ) / 127.0f;
    m_vertex.b = static_cast<float>( blue ) / 127.0f;
    m_vertex.a = static_cast<float>( alpha ) / 127.0f;
}

void glDebugDraw::Color4bv( const GLbyte *v )
{
    m_vertex.r = static_cast<float>( v[0] ) / 127.0f;
    m_vertex.g = static_cast<float>( v[1] ) / 127.0f;
    m_vertex.b = static_cast<float>( v[2] ) / 127.0f;
    m_vertex.a = static_cast<float>( v[3] ) / 127.0f;
}

void glDebugDraw::Color4ubv(const GLubyte *v )
{
    m_vertex.r = static_cast<float>( v[0] ) / 255.0f;
    m_vertex.g = static_cast<float>( v[1] ) / 255.0f;
    m_vertex.b = static_cast<float>( v[2] ) / 255.0f;
    m_vertex.a = static_cast<float>( v[3] ) / 255.0f;   
}

void glDebugDraw::Normal3b( const GLbyte nx, const GLbyte ny, const GLbyte nz )
{
    m_vertex.n = static_cast<float>( nx ) / 127.0f;
    m_vertex.n = static_cast<float>( ny ) / 127.0f;
    m_vertex.n = static_cast<float>( nz ) / 127.0f;
    m_vertex.n = 1.0f;
}

void glDebugDraw::Normal3bv(const GLbyte *v)
{
    m_vertex.n = static_cast<float>( v[0] ) / 127.0f;
    m_vertex.n = static_cast<float>( v[1] ) / 127.0f;
    m_vertex.n = static_cast<float>( v[2] ) / 127.0f;
    m_vertex.n = 1.0f;
}

void glDebugDraw::Normal3f(const GLfloat nx, const GLfloat ny, const GLfloat nz)
{
    m_vertex.n = nx;
    m_vertex.n = ny;
    m_vertex.n = nz;
    m_vertex.n = 1.0f;
}

void glDebugDraw::Normal3fv( const GLfloat *v)
{
    std::memcpy( &m_vertex.n, v, k_VEC4F_SIZE );
}

void glDebugDraw::MatrixMode( const GLenum mode )
{
    m_matrixMode = mode;
}

void glDebugDraw::PopMatrix(void)
{
}

void glDebugDraw::PushMatrix(void)
{
}

void glDebugDraw::LoadIdentity(void)
{
    float   matrix[16]
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f 
    };

    switch ( m_matrixMode )
    {
    case GL_MODELVIEW:
        glBufferSubData( GL_UNIFORM_BUFFER, k_UNIFORMS_BUFFER_MODELVIEW_LOCATION, k_UNIFORMS_BUFFER_MODELVIEW_SIZE, matrix );
        break;
    case GL_PROJECTION:
        glBufferSubData( GL_UNIFORM_BUFFER, k_UNIFORMS_BUFFER_PROJECTION_LOCATION, k_UNIFORMS_BUFFER_PROJECTION_SIZE, matrix );
        break;
    case GL_TEXTURE:
        glBufferSubData( GL_UNIFORM_BUFFER, k_UNIFORMS_BUFFER_MODELVIEW_LOCATION, k_UNIFORMS_BUFFER_TEXTURE_SIZE, matrix );
        break;

    default:
        // todo: trow a error 
        break;
    };
}

void glDebugDraw::LoadMatrixf(const GLfloat *m)
{
    switch ( m_matrixMode )
    {
    case GL_MODELVIEW:
        glBufferSubData( GL_UNIFORM_BUFFER, k_UNIFORMS_BUFFER_MODELVIEW_LOCATION, k_UNIFORMS_BUFFER_MODELVIEW_SIZE, m );
        break;
    case GL_PROJECTION:
        glBufferSubData( GL_UNIFORM_BUFFER, k_UNIFORMS_BUFFER_PROJECTION_LOCATION, k_UNIFORMS_BUFFER_PROJECTION_SIZE, m );
        break;
    case GL_TEXTURE:
        glBufferSubData( GL_UNIFORM_BUFFER, k_UNIFORMS_BUFFER_MODELVIEW_LOCATION, k_UNIFORMS_BUFFER_TEXTURE_SIZE, m );
        break;

    default:
        // todo: trow a error 
        break;
    }
}

void glDebugDraw::Ortho(const GLfloat left, const GLfloat right, const GLfloat bottom, const GLfloat top, const GLfloat zNear, const GLfloat zFar)
{
    float   m[16];

    float rl = right - left;
    float tb = top - bottom;
    float fn = zFar - zNear;

    m[0]  =  2.0f / rl;
    m[1]  =  0.0f;
    m[2]  =  0.0f;
    m[3]  =  0.0f;

    m[4]  =  0.0f;
    m[5]  =  2.0f / tb;
    m[6]  =  0.0f;
    m[7]  =  0.0f;

    m[8]  =  0.0f;
    m[9]  =  0.0f;
    m[10] = -2.0f / fn;
    m[11] =  0.0f;

    m[12] = -(right + left) / rl;
    m[13] = -(top + bottom) / tb;
    m[14] = -(zFar + zNear) / fn;
    m[15] =  1.0f;

    glBufferSubData( GL_UNIFORM_BUFFER, k_UNIFORMS_BUFFER_PROJECTION_LOCATION, k_UNIFORMS_BUFFER_PROJECTION_SIZE, m );
}
