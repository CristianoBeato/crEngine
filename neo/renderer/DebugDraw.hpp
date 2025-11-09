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

#ifndef __DEBUG_DRAW_HPP__
#define __DEBUG_DRAW_HPP__

/// @brief Fixed Pipeline WorkArround
class glDebugDraw
{
public:
    struct alignas( 16 ) fixedVertex_t
    {
        // vertex positions
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 0.0f; // only do a padding 

        // texture coordinate
        float s = 0.0f;
        float t = 0.0f;
        float u = 0.0f; //  u and v cause we already use R for color ( and keep alphabetic order ) 
        float v = 0.0f;

        // vertex color
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;

        // normal value
        float n = 0.0f;
        float m = 0.0f;
        float o = 0.0f;
        float p = 0.0f; // just padding
    };

    struct matrix_t
    {
        float   m[16];
    };
    

    static void StartUp( void );
    static void ShutDown( void );

    static void Begin( const GLenum mode );

    static void End( void );

    // position
    static void Vertex2f( const GLfloat x, const GLfloat y );
    static void Vertex2fv( const GLfloat *v );
    static void Vertex3f( const GLfloat x, const GLfloat y, const GLfloat z );
    static void Vertex3fv(const GLfloat *v);
    static void Vertex4f( const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w );
    static void Vertex4fv( const GLfloat *v );

    // texture coordinate
    static void TexCoord1f( const GLfloat s );
    static void TexCoord1fv( const GLfloat *v );
    static void TexCoord2f( const GLfloat s, const GLfloat t );
    static void TexCoord2fv( const GLfloat *v);
    static void TexCoord3f( const GLfloat s, const GLfloat t, const GLfloat r );
    static void TexCoord3fv( const GLfloat *v);
    static void TexCoord4f( const GLfloat s,GLfloat t,GLfloat r,GLfloat q);
    static void TexCoord4fv(const GLfloat *v);
 
    // color
    static void Color3f( const GLfloat red, const GLfloat green, const GLfloat blue );
    static void Color3fv(const GLfloat *v );
    static void Color4f( const GLfloat red, const GLfloat green, const GLfloat blue, const GLfloat alpha );
    static void Color4fv( const GLfloat *v );
    static void Color3b( const GLbyte red, const GLbyte green, const GLbyte blue );
    static void Color3bv( const GLbyte *v );
    static void Color4b( const GLbyte red, const GLbyte green, const GLbyte blue, const GLbyte alpha );
    static void Color4bv( const GLbyte *v );
    static void Color4ubv( const GLubyte *v );
    
    // normal 
    static void Normal3b ( const GLbyte nx, const GLbyte ny, const GLbyte nz );
    static void Normal3bv( const GLbyte *v);
    static void Normal3f( const GLfloat nx, const GLfloat ny, const GLfloat nz );
    static void Normal3fv( const GLfloat *v );

    //
    static void VertexPointer( const GLint size, const GLenum type, const GLsizei stride, const GLvoid *pointer);
    static void TexCoordPointer( const GLint size, const GLenum type, const GLsizei stride, const GLvoid *pointer);
    static void ColorPointer( const GLint size, const GLenum type, const GLsizei stride, const GLvoid *pointer );
    static void NormalPointer( const GLenum type, const GLsizei stride, const GLvoid *pointer );

    // projection matrix
    static void MatrixMode( const GLenum mode );
    static void PopMatrix( void );
    static void PushMatrix( void );
    static void LoadIdentity( void );
    static void LoadMatrixf( const GLfloat *m );
    static void Ortho( const GLfloat left, const GLfloat right, const GLfloat bottom, const GLfloat top, const GLfloat zNear, const GLfloat zFar );

private:
    static GLint            m_first;        // first vertex of the current draw   
    static GLsizei          m_count;        // num vertex to draw
    static GLenum           m_mode;         // draw mode
    static GLenum           m_matrixMode;   //  
    static GLuint           m_vertexArray;  // vertex array object
    static GLuint           m_program;      // shader program object
    static GLenum           m_vertexType;
    static GLuint           m_uniformBuffer;//
    static fixedVertex_t    m_vertex;
    static fixedVertex_t*   m_vertexes;
};

#endif //!__DEBUG_DRAW_HPP__