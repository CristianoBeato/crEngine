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

inline constexpr uint32_t k_MAX_DEBUG_LINES = 16384;
inline constexpr uint32_t k_MAX_DEBUG_POLYGONS = 8192;
inline constexpr uint32_t k_MAX_DEBUG_TEXT = 512;

/// @brief Fixed Pipeline WorkArround
class crDebugDraw
{
public:
    enum drawMode_t
    {
        DRAW_MODE_NONE = 0,
        DRAW_MODE_POINTS,
        DRAW_MODE_LINES,
        DRAW_MODE_LINE_LOOP,
        DRAW_MODE_TRIANGLES,
        DRAW_MODE_TRIANGLE_FAN
    };

    enum matrixMode_t
    {
        MATRIX_TEXTURE,
        MATRIX_PROJECTION,
        MATRIX_MODELVIEW,
        MATRIX_MODEL
    };

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

    struct debugLine_s 
    {
        idVec4		rgb;
        idVec3		start;
        idVec3		end;
        bool		depthTest;
        int			lifeTime;
    };

    struct debugPolygon_s 
    {
	    idVec4		rgb;
	    idWinding	winding;
	    bool		depthTest;
	    int			lifeTime;
    };

    struct debugText_s 
    {
	    idStr		text;
	    idVec3		origin;
	    float		scale;
	    idVec4		color;
	    idMat3		viewAxis;
	    int			align;
	    int			lifeTime;
	    bool		depthTest;
    };
    
    static void StartUp( void );
    static void ShutDown( void );

    static void Begin( const drawMode_t mode );
    static void End( void );
    static void DefaultState( void );

    static void LineWidth( const uint32_t width );
    
    // position
    static void Vertex2f( const float x, const float y );
    static void Vertex2fv( const float *v );
    static void Vertex3f( const float x, const float y, const float z );
    static void Vertex3fv(const float *v);
    static void Vertex4f( const float x, const float y, const float z, const float w );
    static void Vertex4fv( const float *v );

    // texture coordinate
    static void TexCoord1f( const float s );
    static void TexCoord1fv( const float *v );
    static void TexCoord2f( const float s, const float t );
    static void TexCoord2fv( const float *v);
    static void TexCoord3f( const float s, const float t, const float r );
    static void TexCoord3fv( const float *v);
    static void TexCoord4f( const float s,float t,float r,float q);
    static void TexCoord4fv(const float *v);
 
    // color
    static void Color3f( const float red, const float green, const float blue );
    static void Color3fv(const float *v );
    static void Color4f( const float red, const float green, const float blue, const float alpha );
    static void Color4fv( const float *v );
    static void Color3b( const int8_t red, const int8_t green, const int8_t blue );
    static void Color3bv( const int8_t *v );
    static void Color4b( const int8_t red, const int8_t green, const int8_t blue, const int8_t alpha );
    static void Color4bv( const int8_t *v );
    static void Color4ubv( const uint8_t *v );
    
    // normal 
    static void Normal3b ( const int8_t nx, const int8_t ny, const int8_t nz );
    static void Normal3bv( const int8_t *v);
    static void Normal3f( const float nx, const float ny, const float nz );
    static void Normal3fv( const float *v );

    //
    static void VertexPointer( const int size, const uint32_t type, const size_t stride, const void *pointer);
    static void TexCoordPointer( const int size, const uint32_t type, const size_t stride, const void *pointer);
    static void ColorPointer( const int size, const uint32_t type, const size_t stride, const void *pointer );
    static void NormalPointer( const uint32_t type, const size_t stride, const void *pointer );

    // projection matrix
    static void MatrixMode( const uint32_t mode );
    static void PopMatrix( void );
    static void PushMatrix( void );
    static void LoadIdentity( void );
    static void LoadMatrixf( const float *m );
    static void Ortho( const float left, const float right, const float bottom, const float top, const float zNear, const float zFar );

    /// Manage debug lines
    static void AddDebugLine( const idVec4 &color, const idVec3 &start, const idVec3 &end, const int lifeTime, const bool depthTest );
    static void ShowDebugLines( void );
    static void ClearDebugLines( const uint32_t time );

    /// Manage debug poligons
    static void AddDebugPolygon( const idVec4 &color, const idWinding &winding, const int lifeTime, const bool depthTest );
    static void ShowDebugPolygons( void );
    static void ClearDebugPolygons( const uint32_t time ); 

    /// Manage debug texts
    static float DrawTextLength( const char *text, float scale, int len );
    static void AddDebugText( const char *text, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align, const int lifetime, const bool depthTest ); 
    static void DrawText( const char *text, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align );
    static void ShowDebugText( void );
    static void ClearDebugText( const uint32_t time );

private:
    static uint32_t         m_first;        // first vertex of the current draw   
    static uint32_t         m_count;        // num vertex to draw
    static uint32_t         m_lineWidth;    // 
    static uint32_t         m_numDebugLines;
    static uint32_t         m_debugLineTime;
    static uint32_t			m_numDebugText;
    static uint32_t         m_debugTextTime;
    static uint32_t         m_numDebugPolygons;
    static uint32_t         m_debugPolygonTime;
    static drawMode_t       m_mode;         // draw mode
    static matrixMode_t     m_matrixMode;   //  
//    static GLenum           m_vertexType;
    static fixedVertex_t    m_vertex;
    static fixedVertex_t*   m_vertexes;

    static debugLine_s		m_debugLines[ k_MAX_DEBUG_LINES ];
    static debugPolygon_s	m_debugPolygons[ k_MAX_DEBUG_POLYGONS ];
    static debugText_s		m_debugText[ k_MAX_DEBUG_TEXT ];

    ///
    /// Vulkan Specific
    ///
    static VkCommandBuffer      m_debugCommandBuffer;
    static VkPipeline           m_debugPipeline;
    static VkBuffer             m_debugVertexBuffer;

    static void SimpleWorldSetup( void );
};

#endif //!__DEBUG_DRAW_HPP__