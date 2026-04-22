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

#include "simplex.h"

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

extern idCVar r_debugLineDepthTest;

//
uint32_t                    crDebugDraw::m_first = 0;
uint32_t                    crDebugDraw::m_count = 0;
uint32_t                    crDebugDraw::m_lineWidth = 1;
uint32_t                    crDebugDraw::m_numDebugLines = 0;
uint32_t                    crDebugDraw::m_debugLineTime = 0;
uint32_t			        crDebugDraw::m_numDebugText = 0;
uint32_t                    crDebugDraw::m_debugTextTime = 0;
uint32_t                    crDebugDraw::m_numDebugPolygons = 0;
uint32_t                    crDebugDraw::m_debugPolygonTime = 0;
crDebugDraw::drawMode_t     crDebugDraw::m_mode = crDebugDraw::DRAW_MODE_NONE;
crDebugDraw::matrixMode_t   crDebugDraw::m_matrixMode = crDebugDraw::MATRIX_TEXTURE;
crDebugDraw::fixedVertex_t  crDebugDraw::m_vertex{};
crDebugDraw::fixedVertex_t* crDebugDraw::m_vertexes;

crDebugDraw::debugLine_s    crDebugDraw::m_debugLines[ k_MAX_DEBUG_LINES ]{};
crDebugDraw::debugPolygon_s crDebugDraw::m_debugPolygons[ k_MAX_DEBUG_POLYGONS ]{};
crDebugDraw::debugText_s    crDebugDraw::m_debugText[ k_MAX_DEBUG_TEXT ]{};

VkCommandBuffer             crDebugDraw::m_debugCommandBuffer = nullptr;
VkPipeline                  crDebugDraw::m_debugPipeline = nullptr;
VkBuffer                    crDebugDraw::m_debugVertexBuffer = nullptr; // VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT

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
    VkPrimitiveTopology topo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    m_mode = mode;

    switch ( m_mode )
    {
        case DRAW_MODE_POINTS:          topo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        case DRAW_MODE_LINES:           topo = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case DRAW_MODE_LINE_LOOP:       topo = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
        case DRAW_MODE_TRIANGLES:       topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        case DRAW_MODE_TRIANGLE_FAN:    topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN; break;
    
    default:
        // todo: call a error
        break;
    }

    //vkCmdBindVertexBuffers( m_debugCommandBuffer, 0, 1, &debugBuffer, &currentFrameOffset );
    vkCmdSetPrimitiveTopology( m_debugCommandBuffer, topo );

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

    glUseProgram( 0 );
    glBindVertexArray( 0 );
    */

    vkCmdDraw( m_debugCommandBuffer, m_count, 1, m_first, 0 );

    m_first += m_count;
    m_count = 0;
}

void crDebugDraw::DefaultState(void)
{
    m_lineWidth = 1;

    /// reset line to 1 pixel
    vkCmdSetLineWidth( m_debugCommandBuffer, 1.0f );

    /// Disable depth test
    vkCmdSetDepthTestEnable( m_debugCommandBuffer, VK_FALSE );

    /// Disable Depth Bias
    vkCmdSetDepthBiasEnable( m_debugCommandBuffer, VK_FALSE );
}

void crDebugDraw::LineWidth( const uint32_t width )
{
    if( m_lineWidth == width )
        return;

    vkCmdSetLineWidth( m_debugCommandBuffer, static_cast<float>( width ) );
    m_lineWidth = width;
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

void crDebugDraw::PopMatrix( void )
{
}

void crDebugDraw::PushMatrix( void )
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

/*
================
crDebugDraw::AddDebugLine
================
*/
void crDebugDraw::AddDebugLine( const idVec4 &color, const idVec3 &start, const idVec3 &end, const int lifeTime, const bool depthTest ) 
{
	debugLine_s *line = nullptr;

	if ( m_numDebugLines < k_MAX_DEBUG_LINES ) 
    {
		line = &m_debugLines[ m_numDebugLines++ ];
		line->rgb		= color;
		line->start		= start;
		line->end		= end;
		line->depthTest = depthTest;
		line->lifeTime	= m_debugLineTime + lifeTime;
	}
}

/*
================
crDebugDraw::SimpleWorldSetup
================
*/
void crDebugDraw::SimpleWorldSetup( void ) 
{
	//backEnd.currentSpace = &backEnd.viewDef->worldSpace;

    // TODO: set model view matrix
    //qglLoadMatrixf( backEnd.viewDef->worldSpace.modelViewMatrix );

	//GL_Scissor( backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
	//			backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
	//			backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
	//			backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1 );
	//backEnd.currentScissor = backEnd.viewDef->scissor;

    /// TODO:
    VkRect2D scissor{};
    vkCmdSetScissor( m_debugCommandBuffer, 0, 1, &scissor );
}

/*
================
crDebugDraw::ShowDebugLines
================
*/
void crDebugDraw::ShowDebugLines( void ) 
{
    int			i = 0;
	int			width = 0;
	debugLine_s	*line = nullptr;
    
	if ( !m_numDebugLines ) 
    return;
    
	// all lines are expressed in world coordinates
	SimpleWorldSetup();

    auto globalImages = static_cast<idImageManagerLocal*>( idImageManager::Get() );
	globalImages->BindNull();


	width = r_debugLineWidth.GetInteger();
	if ( width < 1 )
		width = 1;
    else if ( width > 10 ) 
		width = 10;

	// draw lines
	LineWidth( width );

	//if ( !r_debugLineDepthTest.GetBool() ) 
    //{
	//	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHFUNC_ALWAYS );
	//} 
    //else 
    //{
	//	GL_State( GLS_POLYMODE_LINE );
	//}

	Begin( DRAW_MODE_LINES );

	line = m_debugLines;
	for ( i = 0; i < m_numDebugLines; i++, line++ ) 
    {
		if ( !line->depthTest ) 
        {
			Color3fv( line->rgb.ToFloatPtr() );
			Vertex3fv( line->start.ToFloatPtr() );
			Vertex3fv( line->end.ToFloatPtr() );
		}
	}
	End();

//	if ( !r_debugLineDepthTest.GetBool() ) 
//    {
//		GL_State( GLS_POLYMODE_LINE );
//	}

	Begin( DRAW_MODE_LINES );

	line = m_debugLines;
	for ( i = 0; i < m_numDebugLines; i++, line++ ) 
    {
		if ( line->depthTest ) 
        {
			Color4fv( line->rgb.ToFloatPtr() );
			Vertex3fv( line->start.ToFloatPtr() );
			Vertex3fv( line->end.ToFloatPtr() );
		}
	}

	End();

    DefaultState();
}

/*
================
crDebugDraw::ClearDebugLines
================
*/
void crDebugDraw::ClearDebugLines( const uint32_t time ) 
{
	int			i;
	int			num;
	debugLine_s	*line;

	m_debugLineTime = time;

	if ( !time ) 
    {
		m_numDebugLines = 0;
		return;
	}

	// copy any lines that still need to be drawn
	num	= 0;
	line = m_debugLines;
	for ( i = 0; i < m_numDebugLines; i++, line++ ) 
    {
		if ( line->lifeTime > time ) 
        {
			if ( num != i ) 
            {
				m_debugLines[ num ] = *line;
			}
			num++;
		}
	}
	m_numDebugLines = num;
}


/*
================
crDebugDraw::AddDebugPolygon
================
*/
void crDebugDraw::AddDebugPolygon( const idVec4 &color, const idWinding &winding, const int lifeTime, const bool depthTest ) 
{
	debugPolygon_s *poly = nullptr;

	if ( m_numDebugPolygons < k_MAX_DEBUG_POLYGONS ) {
		poly = &m_debugPolygons[ m_numDebugPolygons++ ];
		poly->rgb		= color;
		poly->winding	= winding;
		poly->depthTest = depthTest;
		poly->lifeTime	= m_debugPolygonTime + lifeTime;
	}
}

/*
================
crDebugDraw::ShowDebugPolygons
================
*/
void crDebugDraw::ShowDebugPolygons( void ) 
{
	int				i = 0, j = 0;
	debugPolygon_s	*poly = nullptr;

	if ( !m_numDebugPolygons ) 
		return;

	// all lines are expressed in world coordinates
	SimpleWorldSetup();

    auto globalImages = static_cast<idImageManagerLocal*>( idImageManager::Get() );
	globalImages->BindNull();

	if ( r_debugPolygonFilled.GetBool() ) 
    {
		// GL_State( GLS_POLYGON_OFFSET | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA | GLS_DEPTHMASK );
		// glPolygonOffset( -1, -2 );
        vkCmdSetDepthBiasEnable( m_debugCommandBuffer, VK_TRUE );
        vkCmdSetDepthBias( m_debugCommandBuffer, -2, 0.0f, -1 );
	} 
    else 
    {
		// GL_State( GLS_POLYGON_OFFSET | GLS_POLYMODE_LINE );
    	// glPolygonOffset( -1, -2 );
        vkCmdSetDepthBiasEnable( m_debugCommandBuffer, VK_TRUE );
        vkCmdSetDepthBias( m_debugCommandBuffer, -2, 0.0f, -1 );

	}

	poly = m_debugPolygons;
	for ( i = 0; i < m_numDebugPolygons; i++, poly++ ) 
    {
//		if ( !poly->depthTest ) {

			Color4fv( poly->rgb.ToFloatPtr() );

			Begin( DRAW_MODE_TRIANGLE_FAN );

			for ( j = 0; j < poly->winding.GetNumPoints(); j++) 
            {
				Vertex3fv( poly->winding[j].ToFloatPtr() );
			}

			End();
//		}
	}

	DefaultState();
}

/*
================
crDebugDraw::ClearDebugPolygons
================
*/
void crDebugDraw::ClearDebugPolygons( const uint32_t time ) 
{
	int				i = 0;
	int				num = 0;
	debugPolygon_s	*poly = nullptr;

	m_debugPolygonTime = time;

	if ( !time ) 
    {
		m_numDebugPolygons = 0;
		return;
	}

	// copy any polygons that still need to be drawn
	num	= 0;

	poly = m_debugPolygons;
	for ( i = 0; i < m_numDebugPolygons; i++, poly++ ) 
    {
		if ( poly->lifeTime > time ) 
        {
			if ( num != i ) 
            {
				m_debugPolygons[ num ] = *poly;
			}
			num++;
		}
	}

	m_numDebugPolygons = num;
}

/*
================
crDebugDraw::DrawTextLength
returns the length of the given text
================
*/
float crDebugDraw::DrawTextLength( const char *text, float scale, int len ) 
{
	int i, num, index, charIndex;
	float spacing, textLen = 0.0f;

	if ( text && *text ) 
    {
		if ( !len ) 
        {
			len = strlen(text);
		}
		for ( i = 0; i < len; i++ ) 
        {
			charIndex = text[i] - 32;
			if ( charIndex < 0 || charIndex > NUM_SIMPLEX_CHARS ) 
				continue;
			
			num = simplex[charIndex][0] * 2;
			spacing = simplex[charIndex][1];
			index = 2;

			while( index - 2 < num ) 
            {
				if ( simplex[charIndex][index] < 0) 
                {
					index++;
					continue; 
				}

				index += 2;
				if ( simplex[charIndex][index] < 0) 
                {  
					index++;
					continue; 
				} 
			}
			textLen += spacing * scale;  
		}
	}
	return textLen;
}

/*
================
crDebugDraw::AddDebugText
================
*/
void crDebugDraw::AddDebugText( const char *text, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align, const int lifetime, const bool depthTest ) 
{
	debugText_s *debugText = nullptr;

	if ( m_numDebugText < k_MAX_DEBUG_TEXT ) 
    {
		debugText = &m_debugText[ m_numDebugText++ ];
		debugText->text			= text;
		debugText->origin		= origin;
		debugText->scale		= scale;
		debugText->color		= color;
		debugText->viewAxis		= viewAxis;
		debugText->align		= align;
		debugText->lifeTime		= m_debugTextTime + lifetime;
		debugText->depthTest	= depthTest;
	}
}

/*
================
crDebugDraw::DrawText

  oriented on the viewaxis
  align can be 0-left, 1-center (default), 2-right
================
*/
void crDebugDraw::DrawText( const char *text, const idVec3 &origin, float scale, const idVec4 &color, const idMat3 &viewAxis, const int align ) 
{
	int i, j, len, num, index, charIndex, line;
	float textLen = 1.0f, spacing = 1.0f;
	idVec3 org, p1, p2;

	if ( text && *text ) 
    {
		Begin( DRAW_MODE_LINES );
		Color3fv( color.ToFloatPtr() );

		if ( text[0] == '\n' ) 
			line = 1;
        else 
			line = 0;

		len = strlen( text );
		for ( i = 0; i < len; i++ ) 
        {
			if ( i == 0 || text[i] == '\n' ) 
            {
				org = origin - viewAxis[2] * ( line * 36.0f * scale );
				if ( align != 0 ) {
					for ( j = 1; i+j <= len; j++ ) 
                    {
						if ( i+j == len || text[i+j] == '\n' ) 
                        {
							textLen = DrawTextLength( text+i, scale, j );
							break;
						}
					}

					if ( align == 2 ) 
                    {
						// right
						org += viewAxis[1] * textLen;
					} 
                    else 
                    {
						// center
						org += viewAxis[1] * ( textLen * 0.5f );
					}
				}
				line++;
			}

			charIndex = text[i] - 32;
			if ( charIndex < 0 || charIndex > NUM_SIMPLEX_CHARS ) 
				continue;

			num = simplex[charIndex][0] * 2;
			spacing = simplex[charIndex][1];
			index = 2;

			while( index - 2 < num ) 
            {
				if ( simplex[charIndex][index] < 0) 
                {  
					index++;
					continue; 
				}

				p1 = org + scale * simplex[charIndex][index] * -viewAxis[1] + scale * simplex[charIndex][index+1] * viewAxis[2];
				index += 2;
				if ( simplex[charIndex][index] < 0) 
                {
					index++;
					continue;
				}
				p2 = org + scale * simplex[charIndex][index] * -viewAxis[1] + scale * simplex[charIndex][index+1] * viewAxis[2];

				Vertex3fv( p1.ToFloatPtr() );
				Vertex3fv( p2.ToFloatPtr() );
			}
			org -= viewAxis[1] * ( spacing * scale );
		}

		End();
	}
}

/*
================
crDebugDraw::ShowDebugText
================
*/
void crDebugDraw::ShowDebugText( void ) 
{
	int			i = 0;
	int			width = 0;
	debugText_s	*text = nullptr;

    
	if ( !m_numDebugText ) 
    return;
    
	// all lines are expressed in world coordinates
	SimpleWorldSetup();
    
    auto globalImages = static_cast<idImageManagerLocal*>( idImageManager::Get() );
	globalImages->BindNull();

	width = r_debugLineWidth.GetInteger();
	if ( width < 1 ) 
		width = 1;
	else if ( width > 10 ) 
		width = 10;

	// draw lines
	LineWidth( width );

    // TODO:
	//if ( !r_debugLineDepthTest.GetBool() )
	//	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHFUNC_ALWAYS );
    //else 
	//	GL_State( GLS_POLYMODE_LINE );

	text = m_debugText;
	for ( i = 0; i < m_numDebugText; i++, text++ ) 
    {
		if ( !text->depthTest ) 
			DrawText( text->text, text->origin, text->scale, text->color, text->viewAxis, text->align );
	}

    //if ( !r_debugLineDepthTest.GetBool() ) 
    //{
	//	GL_State( GLS_POLYMODE_LINE );
	//}

	text = m_debugText;
	for ( i = 0; i < m_numDebugText; i++, text++ ) 
    {
		if ( text->depthTest )
			DrawText( text->text, text->origin, text->scale, text->color, text->viewAxis, text->align );
	}

	LineWidth( 1 );
}

/*
================
crDebugDraw::ClearDebugText
================
*/
void crDebugDraw::ClearDebugText( const uint32_t time ) 
{
	int			i;
	int			num;
	debugText_s	*text;

	m_debugTextTime = time;

	if ( !time ) 
    {
		// free up our strings
		text = m_debugText;
		for ( i = 0; i < k_MAX_DEBUG_TEXT; i++, text++ ) 
        {
			text->text.Clear();
		}

		m_numDebugText = 0;
		return;
	}

	// copy any text that still needs to be drawn
	num	= 0;
	text = m_debugText;
	for ( i = 0; i < m_numDebugText; i++, text++ ) 
    {
		if ( text->lifeTime > time ) 
        {
			if ( num != i )
				m_debugText[ num ] = *text;
			
			num++;
		}
	}
	m_numDebugText = num;
}