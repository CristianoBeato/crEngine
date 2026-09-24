
#include "precompiled.h"
#include "ToolsDialogRenderer.hpp"

#include "OpenGL.hpp"

constexpr uint32_t k_QUAD_VERTEX_COUNT = 4;
constexpr uint32_t k_QUAD_INDEX_COUNT  = 6;
constexpr uint32_t k_MAX_ELEMENTS_COUNT = 16 * 1024;
constexpr uint32_t k_MAX_VERTICES_COUNT = k_MAX_ELEMENTS_COUNT * 4;
constexpr size_t   k_VERTEX_SIZE = sizeof( float ) * 4;

crToolsDialogRenderer::crToolsDialogRenderer( void )
{
}

crToolsDialogRenderer::~crToolsDialogRenderer( void )
{
}

void crToolsDialogRenderer::Init(void)
{
    //TODO: Font ?
    InitShaders();
	InitBuffers();
	CreateSamplers();
	CreateVertexArray();
}

void crToolsDialogRenderer::Release(void)
{
}

void crToolsDialogRenderer::Begin(void)
{
    gl::BindVertexArray( m_vertexArray );
    gl::UseProgram( m_program );
    gl::BindBufferRange( GL_UNIFORM_BUFFER, 0, m_uniformBuffer, 0, sizeof( float ) * 20 );

	gl::SetState( GL_BLEND, GL_TRUE );
    gl::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	gl::SetState( GL_DEPTH_TEST, GL_FALSE );
	
	// clear defalt frame buffer
	gl::ClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	gl::Viewport( 0, 0, m_width, m_heigth );
	gl::Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void crToolsDialogRenderer::End( void )
{
    Flush();
    gl::UseProgram( 0 );
    gl::BindVertexArray( 0 );
}

void crToolsDialogRenderer::StartClip(void)
{
    Flush();

    Gwen::Rect rect = ClipRegion();
	rect.y = m_heigth - ( rect.y + rect.h );
	
	gl::Scissor( rect.x * Scale(), rect.y * Scale(), rect.w * Scale(), rect.h * Scale() );

    // Enable clipping
    gl::SetState( GL_SCISSOR_TEST, GL_TRUE );
}

void crToolsDialogRenderer::EndClip(void)
{
    // Disable clipping
    gl::SetState( GL_SCISSOR_TEST, GL_FALSE );
    Flush();
}

void crToolsDialogRenderer::SetDrawColor(Gwen::Color color)
{
}

void crToolsDialogRenderer::DrawPixel(int x, int y)
{
}

void crToolsDialogRenderer::DrawLinedRect(Gwen::Rect rect)
{
}

void crToolsDialogRenderer::DrawFilledRect(Gwen::Rect rect)
{
}

void crToolsDialogRenderer::DrawTexturedRect(Gwen::Texture *pTexture, Gwen::Rect pTargetRect, float u1, float v1, float u2, float v2)
{
}

void crToolsDialogRenderer::RenderText(Gwen::Font *pFont, Gwen::Point pos, const Gwen::UnicodeString &text)
{
}

void crToolsDialogRenderer::LoadFont(Gwen::Font *pFont)
{
}

void crToolsDialogRenderer::FreeFont(Gwen::Font *pFont)
{
}

void crToolsDialogRenderer::LoadTexture(Gwen::Texture *pTexture)
{
}

void crToolsDialogRenderer::FreeTexture(Gwen::Texture *pTexture)
{
}

void crToolsDialogRenderer::CreateFrameBuffer(Gwen::FrameBuffer *pFrameBuffer)
{
}

void crToolsDialogRenderer::FreeFrameBuffer(Gwen::FrameBuffer *pFrameBuffer)
{
}

void crToolsDialogRenderer::BindFrameBuffer(Gwen::FrameBuffer *pFrameBuffer, Gwen::Rect renderRect)
{
}

void crToolsDialogRenderer::DrawFrameBuffer(Gwen::FrameBuffer *pFrameBuffer, Gwen::Rect targetRect)
{
}

Gwen::Color crToolsDialogRenderer::PixelColour(Gwen::Texture *pTexture, unsigned int x, unsigned int y, const Gwen::Color &col_default)
{
    return Gwen::Color();
}

Gwen::Point crToolsDialogRenderer::MeasureText(Gwen::Font *pFont, const Gwen::UnicodeString &text)
{
    return Gwen::Point();
}

void crToolsDialogRenderer::AddQuad(const bounds_t pos, const bounds_t uv)
{
    if ( ( m_vhead + 4) > k_MAX_VERTICES_COUNT || ( m_ihead + 6 ) > k_MAX_ELEMENTS_COUNT )
	{
		Flush();
		m_ihead = m_itail = 0;
		m_vhead = m_vtail = 0;
	}

	// TL ____ TR
	//   |   /|
	//	 |T1/ |
	//   | /T2|
	// BL|/___|BR

	const float vertexes[16]
	{
		pos.left, pos.top, uv.left, uv.top,			// 0 TL
		pos.right, pos.top, uv.right, uv.top,		// 1 TR
		pos.left, pos.bottom, uv.left, uv.bottom,	// 2 BL
		pos.right, pos.bottom, uv.right, uv.bottom,	// 3 B   R
	};

	// copy vertexes to vertex buffer array 
	uint16_t baseVertex = m_vhead - m_vtail; 
	const uint16_t indexes[6]
	{
		// T1
		baseVertex + ( ( uint16_t ) 0u ), // TL
		baseVertex + ( ( uint16_t ) 1u ), // TR
		baseVertex + ( ( uint16_t ) 2u ), // BL
		// T2
		baseVertex + ( ( uint16_t ) 1u ), // TR
		baseVertex + ( ( uint16_t ) 2u ), // BL
		baseVertex + ( ( uint16_t ) 3u ), // BR
	};

	// upload to buffers 
	std::memcpy( &m_vertexes[m_vhead * 4], vertexes, k_VERTEX_SIZE * 4);
	std::memcpy( &m_elements[m_ihead], indexes, sizeof(uint16_t) * 6 );

	// move position on buffer
	m_vhead += 4;
	m_ihead += 6;
}

void crToolsDialogRenderer::Flush(void)
{
    /// Current no surface to draw 
    if ( !( m_ihead > m_itail ) )
		return;
	
    /// Current draw mode is line rect
	if ( m_currentMode == RECT_LINE )
		gl::DrawArrays( GL_LINE_LOOP, m_vtail, m_vhead - m_vtail );
	else /// current draw mode is triangle
		gl::DrawElementsBaseVertex( GL_TRIANGLES, m_ihead - m_itail, GL_UNSIGNED_SHORT, reinterpret_cast<void*>( sizeof( GLushort ) * m_itail ), m_vtail );

	m_itail = m_ihead;
	m_vtail = m_vhead;   
}

static const char* s_vertexShaderSouce = 
{
    ""
};

static const char* s_fragmentShaderSource = 
{
    ""
};

void crToolsDialogRenderer::InitShaders(void)
{
    GLuint vs = 0;
    GLuint fs = 0;

    ///
    /// =========================================
    /// Create Vertex shader 
    vs = gl::CreateShaderStage( GL_VERTEX_SHADER, s_vertexShaderSouce );
    if( !vs ) 
        throw idException( "Failed To initialize Tool Shader");

    ///
    /// =========================================
    /// Create Fragment shader 
    fs = gl::CreateShaderStage( GL_FRAGMENT_SHADER, s_fragmentShaderSource );
    if( !fs ) 
        throw idException( "Failed To initialize Tool Shader");

    /// Create Shader Program
    GLuint shaders[2] = { vs, fs };
    m_program = gl::CreateShaderProgram( shaders, 2 );
    if( !m_program ) 
        throw idException( "Failed To Create Tool Program");

    /// Get rid of the shader objects 
    gl::DeleteShader( fs );
    gl::DeleteShader( vs );
}

void crToolsDialogRenderer::InitBuffers(void)
{
    GLuint buffers[3]{ 0 };
    GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    /// Create buffers handlers
    gl::CreateBuffers( 3, buffers );

    /// Allocate memory for the vertex buffer 
    gl::NamedBufferStorage( buffers[0], k_VERTEX_SIZE * k_MAX_VERTICES_COUNT, nullptr, flags );

    /// Retrieve the persistent pointer of the map
    m_vertexes = static_cast<float*>( gl::MapNamedBufferRange( buffers[0], 0,  k_VERTEX_SIZE * k_MAX_VERTICES_COUNT, flags ) );
    m_vertexBuffer = buffers[0];

    /// Allocate memory for the index buffer 
    gl::NamedBufferStorage( buffers[1], k_VERTEX_SIZE * k_MAX_VERTICES_COUNT, nullptr, flags );

    /// retrieve the persistent pointer of the elements map 
    m_elements = static_cast<uint16_t*>( gl::MapNamedBufferRange(( buffers[1], 0, sizeof( uint16_t ) * k_MAX_ELEMENTS_COUNT, flags ) ) );
    m_elementBuffer = buffers[1];

    /// Allocate memory for the uniform buffer
    //gl::( buffers[2], sizeof( float ) * 20, nullptr, GL_DYNAMIC_STORAGE_BIT );

}

void crToolsDialogRenderer::CreateSamplers( void )
{
    GLuint samples[2]{ 0 };
    gl::CreateSamplers( 2, samples );

    /// Base image samples 
    m_sample = samples[0];
    gl::SamplerParameteri( m_fontSample, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    gl::SamplerParameteri( m_fontSample, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // we don't filter fonts 
    m_fontSample = samples[1];
    gl::SamplerParameteri( m_fontSample, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    gl::SamplerParameteri( m_fontSample, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void crToolsDialogRenderer::CreateVertexArray(void)
{
    gl::CreateVertexArrays( 1, &m_vertexArray );
    
    /// Configure Vertex position attrib
    gl::EnableVertexArrayAttrib( m_vertexArray, 0 );
    gl::VertexArrayAttribBinding( m_vertexArray, 0, 0 );
    gl::VertexArrayAttribFormat( m_vertexArray, 0, 2, GL_FLOAT, GL_FALSE, 0 );

    /// Configure Vertex texture coodinate
    gl::EnableVertexArrayAttrib( m_vertexArray, 1 );
    gl::VertexArrayAttribBinding( m_vertexArray, 1, 0 );
    gl::VertexArrayAttribFormat( m_vertexArray, 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 2 );

    /// Bind vertex buffer to the vertex array
    gl::VertexArrayVertexBuffer( m_vertexArray, 0, m_vertexBuffer, 0, k_VERTEX_SIZE );

    /// Bind the index array buffer to the vertex array
    gl::VertexArrayElementBuffer( m_elementBuffer, m_elementBuffer );
}
