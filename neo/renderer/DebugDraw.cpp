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
#include "renderer_common.h"
#include "DebugDraw.hpp"

constexpr uint32_t k_VERTEX_POSITION_ATTRIB = 0;
constexpr uint32_t k_VERTEX_TEXTCOORD_ATTRIB = 0;
constexpr uint32_t k_VERTEX_COLOR_ATTRIB = 0;
constexpr uint32_t k_VERTEX_NORMAL_ATTRIB = 0;

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
uint32_t                    crDebugDraw::m_first = 0;
uint32_t                    crDebugDraw::m_count = 0;
crDebugDraw::drawMode_t     crDebugDraw::m_mode = crDebugDraw::DRAW_MODE_NONE;
crDebugDraw::matrixMode_t   crDebugDraw::m_matrixMode = crDebugDraw::MATRIX_TEXTURE;
crDebugDraw::fixedVertex_t  crDebugDraw::m_vertex{};
crDebugDraw::fixedVertex_t* crDebugDraw::m_vertexes;

void crDebugDraw::StartUp( void )
{
    // glCreateVertexArrays( 1, &m_vertexArray );

    // vertex position
    // glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_POSITION_ATTRIB, 0 );
    // glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_POSITION_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, x ) );

    // texture coordinate
    // glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_TEXTCOORD_ATTRIB, 0 );
    // glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_TEXTCOORD_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, s ) );

    // vertex color
    // glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 0 );
    // glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, s ) );

    // vertex normal
    // glVertexArrayAttribBinding( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 0 );
    // glVertexArrayAttribFormat( m_vertexArray, k_VERTEX_COLOR_ATTRIB, 4, GL_FLOAT, GL_FALSE, offsetof( fixedVertex_t, s ) );
}

void crDebugDraw::ShutDown( void )
{
#if 0
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
#endif
}

void crDebugDraw::Begin( const drawMode_t mode )
{
    m_mode = mode;
}

void crDebugDraw::End(void)
{
    /*
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
    */
}

void crDebugDraw::Vertex2f( const float x, const float y )
{
    m_vertex.x = x;
    m_vertex.y = y;
    m_vertexes[m_count++] = m_vertex;
}

void crDebugDraw::Vertex2fv( const float *v)
{
    std::memcpy( &m_vertex.x, v, k_VEC2F_SIZE );
    m_vertexes[m_count++] = m_vertex;
}

void crDebugDraw::Vertex3f(const float x, const float y, const float z)
{
    m_vertex.x = x;
    m_vertex.y = y;
    m_vertex.z = z;
    m_vertexes[m_count++] = m_vertex;
}

void crDebugDraw::Vertex3fv(const float *v)
{
    std::memcpy( &m_vertex.x, v, k_VEC3F_SIZE );
    m_vertexes[m_count++] = m_vertex;
}

void crDebugDraw::Vertex4f( const float x, const float y, const float z, const float w )
{
    m_vertex.x = x;
    m_vertex.y = y;
    m_vertex.z = z;
    m_vertex.w = w;
    m_vertexes[m_count++] = m_vertex;
}

void crDebugDraw::Vertex4fv( const float *v )
{
    std::memcpy( &m_vertex.x, v, k_VEC4F_SIZE );
    m_vertexes[m_count++] = m_vertex;
}

void crDebugDraw::TexCoord1f(const float s)
{
    m_vertex.s = s;
}

void crDebugDraw::TexCoord1fv(const float *v)
{
    m_vertex.s = *v;
}

void crDebugDraw::TexCoord2f(const float s, const float t)
{
    m_vertex.s = s;
    m_vertex.t = t;
}

void crDebugDraw::TexCoord2fv(const float *v)
{
    std::memcpy( &m_vertex.s, v, k_VEC2F_SIZE );
}

void crDebugDraw::TexCoord3f( const float s, const float t, const float r )
{
    m_vertex.s = s;
    m_vertex.t = t;
    m_vertex.u = r;
}

void crDebugDraw::TexCoord3fv(const float *v)
{
    std::memcpy( &m_vertex.s, v, k_VEC3F_SIZE );
}

void crDebugDraw::TexCoord4f(const float s, float t, float r, float q)
{
    m_vertex.s = s;
    m_vertex.t = t;
    m_vertex.u = r;
    m_vertex.v = q;
}

void crDebugDraw::TexCoord4fv(const float *v)
{
    std::memcpy( &m_vertex.s, v, k_VEC4F_SIZE );
}

void crDebugDraw::Color3f(const float red, const float green, const float blue)
{
    m_vertex.r = red;
    m_vertex.g = green;
    m_vertex.b = blue;
    m_vertex.a = 1.0f;
}

void crDebugDraw::Color3fv(const float *v)
{
    std::memcpy( &m_vertex.r, v, k_VEC3F_SIZE );
    m_vertex.a = 1.0f;
}

void crDebugDraw::Color4f(const float red, const float green, const float blue, const float alpha)
{
    m_vertex.r = red;
    m_vertex.g = green;
    m_vertex.b = blue;
    m_vertex.a = alpha;
}

void crDebugDraw::Color4fv( const float *v )
{
    std::memcpy( &m_vertex.r, v, k_VEC4F_SIZE );
}

void crDebugDraw::Color3b( const int8_t red, const int8_t green, const int8_t blue )
{
    m_vertex.r = static_cast<float>( red ) / 127.0f;
    m_vertex.g = static_cast<float>( green ) / 127.0f;
    m_vertex.b = static_cast<float>( blue ) / 127.0f;
    m_vertex.a = 1.0f;
}

void crDebugDraw::Color3bv( const int8_t *v )
{
    m_vertex.r = static_cast<float>( v[0] ) / 127.0f;
    m_vertex.g = static_cast<float>( v[1] ) / 127.0f;
    m_vertex.b = static_cast<float>( v[2] ) / 127.0f;
    m_vertex.a = 1.0f;
}

void crDebugDraw::Color4b( const int8_t red, const int8_t green, const int8_t blue, const int8_t alpha )
{
    m_vertex.r = static_cast<float>( red ) / 127.0f;
    m_vertex.g = static_cast<float>( green ) / 127.0f;
    m_vertex.b = static_cast<float>( blue ) / 127.0f;
    m_vertex.a = static_cast<float>( alpha ) / 127.0f;
}

void crDebugDraw::Color4bv( const int8_t *v )
{
    m_vertex.r = static_cast<float>( v[0] ) / 127.0f;
    m_vertex.g = static_cast<float>( v[1] ) / 127.0f;
    m_vertex.b = static_cast<float>( v[2] ) / 127.0f;
    m_vertex.a = static_cast<float>( v[3] ) / 127.0f;
}

void crDebugDraw::Color4ubv( const uint8_t *v )
{
    m_vertex.r = static_cast<float>( v[0] ) / 255.0f;
    m_vertex.g = static_cast<float>( v[1] ) / 255.0f;
    m_vertex.b = static_cast<float>( v[2] ) / 255.0f;
    m_vertex.a = static_cast<float>( v[3] ) / 255.0f;   
}

void crDebugDraw::Normal3b( const int8_t nx, const int8_t ny, const int8_t nz )
{
    m_vertex.n = static_cast<float>( nx ) / 127.0f;
    m_vertex.n = static_cast<float>( ny ) / 127.0f;
    m_vertex.n = static_cast<float>( nz ) / 127.0f;
    m_vertex.n = 1.0f;
}

void crDebugDraw::Normal3bv(const int8_t *v)
{
    m_vertex.n = static_cast<float>( v[0] ) / 127.0f;
    m_vertex.n = static_cast<float>( v[1] ) / 127.0f;
    m_vertex.n = static_cast<float>( v[2] ) / 127.0f;
    m_vertex.n = 1.0f;
}

void crDebugDraw::Normal3f(const float nx, const float ny, const float nz)
{
    m_vertex.n = nx;
    m_vertex.n = ny;
    m_vertex.n = nz;
    m_vertex.n = 1.0f;
}

void crDebugDraw::Normal3fv( const float *v)
{
    std::memcpy( &m_vertex.n, v, k_VEC4F_SIZE );
}

void crDebugDraw::MatrixMode( const uint32_t mode )
{
    //m_matrixMode = mode;
}

void crDebugDraw::PopMatrix(void)
{
}

void crDebugDraw::PushMatrix(void)
{
}

void crDebugDraw::LoadIdentity( void )
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
    case MATRIX_MODELVIEW:
        break;
    case MATRIX_PROJECTION:
        break;
    case MATRIX_MODEL:
        break;
    case MATRIX_TEXTURE:
        break;
    };
}

void crDebugDraw::LoadMatrixf(const float *m)
{
    switch ( m_matrixMode )
    {
    case MATRIX_MODELVIEW:
        break;
    case MATRIX_PROJECTION:
        break;
    case MATRIX_MODEL:
        break;
    case MATRIX_TEXTURE:
        break;
    };
}

void crDebugDraw::Ortho(const float left, const float right, const float bottom, const float top, const float zNear, const float zFar)
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

}
