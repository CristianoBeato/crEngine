
#include "Gwen/Renderers/OpenGL.h"
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "Gwen/WindowProvider.h"
#include "FontData.h"

#include <iostream>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "OpenGL.h"

constexpr uint32_t k_QUAD_VERTEX_COUNT = 4;
constexpr uint32_t k_QUAD_INDEX_COUNT  = 6;
constexpr uint32_t k_MAX_ELEMENTS_COUNT = 16 * 1024;
constexpr uint32_t k_MAX_VERTICES_COUNT = k_MAX_ELEMENTS_COUNT * 4;
constexpr size_t   k_VERTEX_SIZE = sizeof( float ) * 4;

struct glyph_t 
{
	int advance = 0;
	glm::ivec2  size;
	glm::ivec2	bearings;
	glm::vec4	tcoord;
};

struct  renderFont_t
{
    gl::Texture   texture;
	int			h = 0;	
	glyph_t     glyphs[256];
};

struct renderFrameBuffer_t
{
	gl::FrameBuffer		frameBuffer;
	gl::Texture			renderImage;
};

static int ReadShaderSource( const char* path, char** source )
{
    int len = 0;
    FILE* srcf = nullptr;

    srcf = fopen( path, "r" );
    if ( !srcf )
        return -1;

    fseek( srcf, 0, SEEK_END );
    len = ftell( srcf );
    fseek( srcf, 0, SEEK_SET );

    *source = static_cast<char*>( std::malloc( len ) ); 

    fread( *source, 1, len, srcf );

    // close source file 
    fclose( srcf );

    return len;
}

Gwen::Renderer::OpenGL::OpenGL( void ) :
	m_mode( RECT_LINE ),
	m_vhead( 0 ),
	m_vtail( 0 ),
	m_ihead( 0 ),
	m_itail( 0 ),
	m_elements( nullptr ),
	m_vertexes( nullptr  ),
	m_pContext( nullptr )
{
}

Gwen::Renderer::OpenGL::OpenGL(gl::Context *in_pContext):
	m_mode( RECT_LINE ),
	m_vhead( 0 ),
	m_vtail( 0 ),
	m_ihead( 0 ),
	m_itail( 0 ),
	m_elements( nullptr ),
	m_vertexes( nullptr  ),
	m_pContext( in_pContext )
{
}

Gwen::Renderer::OpenGL::~OpenGL( void )
{
}

void Gwen::Renderer::OpenGL::Init( void )
{
	// todo throw a error
	if( !TTF_Init() )
		throw std::runtime_error( SDL_GetError() );

	InitShaders();
	InitBuffers();
	CreateSamplers();
	CreateVertexArray();
}

void Gwen::Renderer::OpenGL::Release(void)
{
	if ( m_fontSample )
		m_fontSample.Destroy();
	
	if( m_sample )
		m_sample.Destroy();

	if( m_white )
		m_white.Destroy();

	if ( m_uniformBuffer )
		m_uniformBuffer.Destroy();	

	if( m_elementBuffer )
		m_elementBuffer.Destroy();

	if( m_vertexBuffer )
		m_vertexBuffer.Destroy();

	if ( m_program )
		m_program.Destroy();
	
	if( m_vertexArray )
		m_vertexArray.Destroy();

	TTF_Quit();
}

void Gwen::Renderer::OpenGL::Begin( void )
{
	m_pContext->BindVertexArray( m_vertexArray );
	m_pContext->BindProgram( m_program );

	GLuint ubo = m_uniformBuffer;
	GLintptr uboOffset = 0;
	GLsizeiptr uboSizes = sizeof( float ) * 20;
	m_pContext->BindUniformBuffers( &ubo, &uboOffset, &uboSizes, 0, 1 );

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	
	// clear defalt frame buffer
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	glViewport( 0, 0, m_width, m_heigth );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Gwen::Renderer::OpenGL::End( void )
{
	Flush();
	m_pContext->BindProgram( 0 );
	m_pContext->BindVertexArray( 0 );
}

void Gwen::Renderer::OpenGL::SetDrawColor( Gwen::Color color )
{
	// convert color and upload to uniform buffer
	glm::vec4 rgba;
	rgba.r = static_cast<float>( color.r ) / 255.0f;
	rgba.g = static_cast<float>( color.g ) / 255.0f;
	rgba.b = static_cast<float>( color.b ) / 255.0f;
	rgba.a = static_cast<float>( color.a ) / 255.0f;
	m_uniformBuffer.Upload( glm::value_ptr( rgba ), 0, sizeof( glm::vec4 ) );
}

void Gwen::Renderer::OpenGL::StartClip( void )
{
	Flush();
	Gwen::Rect rect = ClipRegion();
	rect.y = m_heigth - ( rect.y + rect.h );
	
	glScissor( rect.x * Scale(), rect.y * Scale(), rect.w * Scale(), rect.h * Scale() );
	glEnable( GL_SCISSOR_TEST );
};

void Gwen::Renderer::OpenGL::EndClip( void )
{
	Flush();
 	glDisable( GL_SCISSOR_TEST );
};

void Gwen::Renderer::OpenGL::DrawPixel(int x, int y)
{
	bounds_t pos{};
	if ( m_mode != RECT_FILL )
	{
		Flush();

		GLuint white = m_white;
		GLuint sample = m_sample;
		m_pContext->BindTextures( &white, &sample, 0, 1 );
		m_mode = RECT_FILL;
	}

	Translate( x, y );

	pos.left = static_cast<float>( x );
	pos.top = static_cast<float>( y );
	pos.right = static_cast<float>( x + 1 );
	pos.bottom = static_cast<float>( y + 1 );	

	AddQuad( pos, { 0.0f, 0.0f, 0.0f, 0.0f} );
}

void Gwen::Renderer::OpenGL::DrawLinedRect(Gwen::Rect rect)
{
	bounds_t pos{};
	if ( m_mode != RECT_LINE )
	{
		Flush();

		GLuint white = m_white;
		GLuint sample = m_sample;
		m_pContext->BindTextures( &white, &sample, 0, 1 );
		m_mode = RECT_LINE;
	}

	Translate( rect );

	pos.left = static_cast<float>( rect.x );
	pos.top = static_cast<float>( rect.y );
	pos.right = static_cast<float>( rect.x + rect.w );
	pos.bottom = static_cast<float>( rect.y + rect.h );	

	AddQuad( pos, { 0.0f, 0.0f, 0.0f, 0.0f} );
}

void Gwen::Renderer::OpenGL::DrawFilledRect(Gwen::Rect rect)
{
	bounds_t pos{};
	if ( m_mode != RECT_FILL )
	{
		Flush();

		GLuint white = m_white;
		GLuint sample = m_sample;
		m_pContext->BindTextures( &white, &sample, 0, 1 );
		m_mode = RECT_FILL;
	}

	Translate( rect );

	pos.left = static_cast<float>( rect.x );
	pos.top = static_cast<float>( rect.y );
	pos.right = static_cast<float>( rect.x + rect.w );
	pos.bottom = static_cast<float>( rect.y + rect.h );	

	AddQuad( pos, { 0.0f, 0.0f, 0.0f, 0.0f} );
}

void Gwen::Renderer::OpenGL::DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect rect, float u1, float v1, float u2, float v2 )
{
	bounds_t pos{};
	bounds_t uv{};

	if( pTexture == nullptr ) 
	{
		DrawMissingImage( rect );
		return;
	}

	if ( pTexture->data == nullptr )
	{
		DrawMissingImage( rect );	
		return;
	}

	gl::Texture* tex = static_cast<gl::Texture*>( pTexture->data );
	if ( m_mode != RECT_TEXTURED )
	{
		Flush();
		GLuint texture = *tex;
		GLuint sample = m_sample;
		m_pContext->BindTextures( &texture, &sample, 0, 1 );
		m_mode = RECT_TEXTURED;
	}
	else
	{
		auto state = m_pContext->CurrentState();
		if ( state.textures.textures[0] != *tex )
		{
			Flush();

			GLuint texture = *tex;
			GLuint sample = m_sample;
			m_pContext->BindTextures( &texture, &sample, 0, 1 );
		}
	}

	// Missing image, not loaded properly?
	if ( !tex )
		return DrawMissingImage( rect );

	Translate( rect );

	pos.left = rect.x;
	pos.top = rect.y;
	pos.right = rect.x + rect.w;
	pos.bottom = rect.y + rect.h;	

	uv.left = u1;
	uv.right = u2;
	uv.top = v1;
	uv.bottom = v2;

	AddQuad( pos, uv );
}

void Gwen::Renderer::OpenGL::RenderText(Gwen::Font *pFont, Gwen::Point pos, const Gwen::UnicodeString &text)
{
	int offset = 0;
	if ( !pFont || !pFont->data )
		return;

	if ( !text.length() )
		return;

	// tranlate text star position
	Translate( pos.x, pos.y );
	
	renderFont_t* font = static_cast<renderFont_t*>( pFont->data );
	if ( m_mode != RECT_FONT )
	{
		GLuint texture = font->texture;
		GLuint sample = m_fontSample;
		m_pContext->BindTextures( &texture, &sample, 0, 1 );
		m_mode = RECT_FONT;
	}
	else
	{
		auto state = m_pContext->CurrentState();
		if ( state.textures.textures[0] != font->texture )
		{
			Flush();
			GLuint texture = font->texture;
			GLuint sample = m_sample;
			m_pContext->BindTextures( &texture, &sample, 0, 1 );
		}
	}

	for( wchar_t ch : text )
	{
		glyph_t g{};
	
		if ( ch < 0 || ch > 255 )
			g = font->glyphs[' '];
		else
			g = font->glyphs[ch];

		///
		/// 
		///

		int h = font->h; // max gltph height, to help alinhate characters to top lef render origin to bottom left

		Rect r;
		r.x = pos.x - g.bearings.x + offset;
		r.y = pos.y + h - g.size.y;
		r.w = pos.x + g.size.x + offset;
		r.h = pos.y + h - g.bearings.y;
		
		//Translate( r );

		AddQuad( { (float)r.x, (float)r.y, (float)r.w , (float)r.h }, { g.tcoord.x, g.tcoord.y, g.tcoord.z, g.tcoord.w } );

		// avança pro próximo caractere
        offset += g.advance;
	}
}

void Gwen::Renderer::OpenGL::LoadFont( Gwen::Font *pFont )
{
	TTF_Font*       font = nullptr;
    SDL_Surface*    atlas = nullptr;
    glyph_t         glyphs[256]{};

	if ( !pFont || pFont->facename.empty() )
		return;

	Gwen::String fontname = Gwen::Utility::UnicodeToString( pFont->facename );

	font = TTF_OpenFont( fontname.c_str(), pFont->size );
	if ( !font )
	{
		std::cout << "Failed to open font " << pFont->facename.c_str() << std::endl;
		pFont->data = nullptr;
        return;
	}

    /// create a gliph attlas surface
    atlas = SDL_CreateSurface( 512, 512, SDL_PIXELFORMAT_RGBA32 );
    if( !atlas )
	{
		pFont->data = nullptr;
		TTF_CloseFont( font );
        return;
	}

    // create a white transparent background
    SDL_PixelFormatDetails formatDetails{};
    formatDetails.format = SDL_PIXELFORMAT_RGBA32;
    SDL_FillSurfaceRect( atlas, nullptr, SDL_MapRGBA( &formatDetails, nullptr, 0, 0, 0, 0 ) );

    int x = 0, y = 0, rowHeight = 0;

    for ( int ch = 0; ch < 256; ++ch ) 
    {
		int minX, minY, maxX, maxY;
        SDL_Surface* glyphSurface = nullptr;
        glyphSurface = TTF_RenderGlyph_Blended( font, ch, { 255, 255, 255, 255 } );
        if (!glyphSurface) 
            continue;

        if (x + glyphSurface->w >= 512 ) 
        {
            x = 0;
            y += rowHeight + 1;
            rowHeight = 0;
        }

        SDL_Rect dst = { x, y, glyphSurface->w, glyphSurface->h };
        SDL_BlitSurface( glyphSurface, nullptr, atlas, &dst);

        rowHeight = std::max(rowHeight, glyphSurface->h);

        glyph_t& g = glyphs[ch];
		g.tcoord.x = float( x ) / 512;
        g.tcoord.y = float( y ) / 512;
        g.tcoord.z = float( x + glyphSurface->w ) / 512;
        g.tcoord.w = float( y + glyphSurface->h ) / 512;

        TTF_GetGlyphMetrics( font, ch, &minX, &maxX, &minY, &maxY, &g.advance );

#if 1
		g.size.x = glyphSurface->w;
		g.size.y = glyphSurface->h;
#else
		g.size.x = maxX - minX;
		g.size.y = maxY - minY;
#endif

		g.bearings.x = minX;
		g.bearings.y = minY;

		x += glyphSurface->w + 1;
        
		SDL_DestroySurface( glyphSurface );
    }

#if 0 // just to help degub
    IMG_SavePNG( m_atlas, "font_data.png" );
#endif

	pFont->data = std::malloc( sizeof( renderFont_t ) );

	// create and upload texture
	static_cast<renderFont_t*>( pFont->data )->texture = gl::Texture();
	
	// alloc font atlas image
	gl::Texture::createInfo_t fontAtlas{};
	fontAtlas.target = GL_TEXTURE_2D;
	fontAtlas.levels = 1;
	fontAtlas.layers = 1;
	fontAtlas.samples = 1;
	fontAtlas.format = GL_RGBA8;
	fontAtlas.dimensions.width = 512;
	fontAtlas.dimensions.height = 512;
	fontAtlas.dimensions.depth = 1;
	static_cast<renderFont_t*>( pFont->data )->texture.Create( &fontAtlas );
	
	gl::Texture::subImage_t	fontSubimage{};
	fontSubimage.level = 0;
	fontSubimage.layer = 0;
	fontSubimage.imageSize = 512 * 512 * 4;
	fontSubimage.offsets = { 0, 0, 0 };
	fontSubimage.dimension = { 512, 512, 1 };
	static_cast<renderFont_t*>( pFont->data )->texture.SubImage( &fontSubimage, atlas->pixels );
	
	static_cast<renderFont_t*>( pFont->data )->h = TTF_GetFontHeight( font );

	// copy the glyphs
    std::memcpy( static_cast<renderFont_t*>( pFont->data )->glyphs, glyphs, sizeof( glyph_t ) * 256 );

    if ( atlas )
    {
        SDL_DestroySurface( atlas );
        atlas = nullptr;
    }

    if ( font != nullptr )
    {
        TTF_CloseFont( font );
        font = nullptr;
    }
}

void Gwen::Renderer::OpenGL::FreeFont(Gwen::Font *pFont)
{
	if ( !pFont || pFont->data == nullptr )
		return;

	static_cast<renderFont_t*>( pFont->data )->texture.Destroy();
	std::free( pFont->data );
	pFont->data = nullptr;
}

void Gwen::Renderer::OpenGL::LoadTexture(Gwen::Texture *pTexture)
{
	GLenum internalFormat = 0;
	gl::Texture::dimensions_t	dim{};
	gl::Texture::offsets_t pos{};
	
	const char* fileName = pTexture->name.Get().c_str();

	SDL_Surface* image = IMG_Load( fileName );
	if ( !image )
	{
		std:: cout << "Falied to open Image " << pTexture->name.Get().c_str() << std::endl;
		pTexture->failed = true;
		return;
	}
	
	// SDL requires a fliped image 
	//SDL_FlipSurface( image, SDL_FLIP_VERTICAL );

	switch ( image->format )
	{
	case SDL_PIXELFORMAT_RGB24:
	{
		internalFormat = GL_RGB8;
	} break;
	case SDL_PIXELFORMAT_BGR24:
	{
		internalFormat = GL_RGB8;
	} break;
	case SDL_PIXELFORMAT_ARGB8888:
	case SDL_PIXELFORMAT_RGBA32:
	{
		internalFormat = GL_RGBA8;
	} break;
	case SDL_PIXELFORMAT_ARGB32:
		{
		internalFormat = GL_RGBA8;
	} break;
	case SDL_PIXELFORMAT_RGB48:
	{
		internalFormat = GL_RGB16;
	} break;
	case SDL_PIXELFORMAT_BGR48:
		{
		internalFormat = GL_RGB16;
	} break;
	case SDL_PIXELFORMAT_RGBA64:
	{
		internalFormat = GL_RGBA16;
	} break;
	case SDL_PIXELFORMAT_ARGB64:
		{
		internalFormat = GL_RGBA16;
	} break;	
	default:
	{	
		pTexture->failed = true;
		return;

		SDL_DestroySurface( image );
		image = nullptr;
	}break;
	}

	// Create a little texture pointer..
	pTexture->data = new gl::Texture();
	
	// Sort out our GWEN texture
	pTexture->width = image->w;
	pTexture->height = image->h;

	// Create the opengl texture
	dim.depth = 1;
	dim.width = image->w;
	dim.height = image->h;

	gl::Texture::createInfo_t imageCI{};
	imageCI.target = GL_TEXTURE_2D;
	imageCI.levels = 0;
	imageCI.layers = 0;
	imageCI.samples = 1;
	imageCI.format = internalFormat;
	imageCI.dimensions = { image->w, image->h, 1 };

	reinterpret_cast<gl::Texture*>( pTexture->data )->Create( imageCI );
	reinterpret_cast<gl::Texture*>( pTexture->data )->SubImage( 0, { 0, 0, 0}, dim,  image->pixels, false );

	SDL_DestroySurface( image );
	image = nullptr;
	pTexture->failed = false;
}

void Gwen::Renderer::OpenGL::FreeTexture( Gwen::Texture* pTexture )
{
	gl::Texture* tex = static_cast<gl::Texture*>( pTexture->data );

	if ( !tex )
		return;

	tex->Destroy();
	delete tex;
	pTexture->data = nullptr;
}

void Gwen::Renderer::OpenGL::CreateFrameBuffer(Gwen::FrameBuffer *pFrameBuffer)
{
	if ( !pFrameBuffer )
		return;

	pFrameBuffer->data = new renderFrameBuffer_t();

	// create the frame buffer handle
	static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->frameBuffer.Create();

	// create a 2d image
	gl::Texture::createInfo_t imageCI{};
	imageCI.target = GL_TEXTURE_2D;
	imageCI.levels = 0;
	imageCI.layers = 0;
	imageCI.samples = 1;
	imageCI.format = GL_RGBA8;
	imageCI.dimensions = { pFrameBuffer->width, pFrameBuffer->height, 1 };

	// attach our texture to frame buffer
	gl::FrameBuffer::attachament_t attachament{};
	attachament.target = GL_TEXTURE_2D;
	attachament.attachament = GL_COLOR_ATTACHMENT0; 
	attachament.handle = static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->renderImage;

	// destroy if failed
	if( !static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->frameBuffer.Attach( &attachament, 0, 1 ) )
	{
		static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->frameBuffer.Destroy();
		static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->renderImage.Destroy();
		delete static_cast<renderFrameBuffer_t*>( pFrameBuffer->data );
	}
}

void Gwen::Renderer::OpenGL::FreeFrameBuffer(Gwen::FrameBuffer *pFrameBuffer)
{
	if ( !pFrameBuffer || !pFrameBuffer->data )
		return;

	static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->renderImage.Destroy();
	static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->frameBuffer.Destroy();
	delete static_cast<renderFrameBuffer_t*>( pFrameBuffer->data );
}

void Gwen::Renderer::OpenGL::BindFrameBuffer(Gwen::FrameBuffer *pFrameBuffer, Gwen::Rect renderRect )
{
	if ( !pFrameBuffer || !pFrameBuffer->data )
		return;
	
	// update the viewport
	glViewport( renderRect.x, renderRect.y, renderRect.w, renderRect.h );

	// bind the frame buffer 
	m_pContext->BindFrameBuffer( static_cast<renderFrameBuffer_t*>( pFrameBuffer->data )->frameBuffer );
	
	// black alpha 
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	// clear the buffer 
	glClear( GL_COLOR_BUFFER_BIT );
}

void Gwen::Renderer::OpenGL::DrawFrameBuffer( Gwen::FrameBuffer *pFrameBuffer, Gwen::Rect targetRect )
{	
	bounds_t pos{};
	bounds_t uv{};

	/// restore gui viewport size
	glViewport( 0, 0, m_width, m_heigth );

	/// restore defalt frame buffer
	m_pContext->BindFrameBuffer( 0 );

	if ( !pFrameBuffer || !pFrameBuffer->data )
	{
		DrawMissingImage( targetRect );
		return;
	}

	Translate( targetRect );

	renderFrameBuffer_t *fbro = static_cast<renderFrameBuffer_t*>( pFrameBuffer->data ); 
	
#if 0
	GLuint texture = fbro->renderImage;
	GLuint sample = m_sample;

	if ( m_mode != RECT_TEXTURED )
	{
		Flush();
		m_pContext->BindTextures( &texture, &sample, 0, 1 );
		m_mode = RECT_TEXTURED;
	}
	else
	{
		auto state = m_pContext->CurrentState();
		if ( state.textures.textures[0] != texture )
		{
			Flush();
			m_pContext->BindTextures( &texture, &sample, 0, 1 );
		}
	}

	// Missing image, not loaded properly?
	if ( texture == 0 )
		return DrawMissingImage( targetRect );

	// white base
	glm::vec4 rgba { 1.0f, 1.0f, 1.0f, 1.0f };
	m_uniformBuffer.Upload( glm::value_ptr( rgba ), 0, sizeof( glm::vec4 ) );


	pos.left = targetRect.x;
	pos.top = targetRect.y;
	pos.right = targetRect.x + targetRect.w;
	pos.bottom = targetRect.y + targetRect.h;	

	uv.left = 0;
	uv.right = 0;
	uv.top = 1;
	uv.bottom = 1;

	AddQuad( pos, uv );
#else
	glBlitNamedFramebuffer( 
		fbro->frameBuffer, 0, 
		0, 0, pFrameBuffer->width, pFrameBuffer->height,
		targetRect.x, targetRect.y, targetRect.x + targetRect.w, targetRect.y + targetRect.h,
		GL_COLOR_BUFFER_BIT, GL_NEAREST );
#endif

	/// restore defalt frame buffer
	m_pContext->BindFrameBuffer( 0 );

}

Gwen::Color Gwen::Renderer::OpenGL::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default )
{
	gl::Texture::dimensions_t dim{};
	gl::Texture::offsets_t	off{};
	gl::Texture* tex = static_cast<gl::Texture*>( pTexture->data );
	if ( !tex ) 
		return col_default;

	uint8_t pixel[4] { 0, 0, 0, 0 };
	unsigned int iPixelSize = sizeof( unsigned char ) * 4;
	dim.depth = 1;
	dim.width = 1;
	dim.height = 1;
	off.xoffset = x;
	off.yoffset = y;
	off.zoffset = 0;
	tex->GetSubImage( 0, off, dim, GL_UNSIGNED_BYTE, iPixelSize, pixel );
	
	// wait for retrieve image data 
	glFinish(); 

	Gwen::Color c;
	c.r = pixel[0];
	c.g = pixel[1];
	c.b = pixel[2];
	c.a = pixel[3];

	return c;
}

Gwen::Point Gwen::Renderer::OpenGL::MeasureText(Gwen::Font *pFont, const Gwen::UnicodeString &text)
{
	Gwen::Point p{ 0, 0 };

	if ( !pFont || !pFont->data )
		return Gwen::Point( 0, 0 );

	if ( !text.length() )
		return Gwen::Point( 0, 0 );

	renderFont_t* font = static_cast<renderFont_t*>( pFont->data );
	for ( wchar_t ch : text )
	{
		glyph_t g{};
	
		if ( ch < 0 || ch > 255 )
			g = font->glyphs[' '];
		else
			g = font->glyphs[ch];

		p.x += g.advance; 
	}

	p.y = font->h;
	return p;
}

void Gwen::Renderer::OpenGL::AddQuad(const bounds_t pos, const bounds_t uv)
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

void Gwen::Renderer::OpenGL::Flush( void )
{
	if ( !( m_ihead > m_itail ) )
		return;
	
	if ( m_mode == RECT_LINE )
		glDrawArrays( GL_LINE_LOOP, m_vtail, m_vhead - m_vtail );
	else
		glDrawElementsBaseVertex( GL_TRIANGLES, m_ihead - m_itail, GL_UNSIGNED_SHORT, reinterpret_cast<void*>( sizeof( GLushort ) * m_itail ), m_vtail );

	m_itail = m_ihead;
	m_vtail = m_vhead;
}

void Gwen::Renderer::OpenGL::InitShaders(void)
{
	int len = 0;
	char * sources = nullptr;
	gl::Shader* shaders[2] { nullptr, nullptr };

	///
	/// Load Vertex Shader
	///
	shaders[0] = new gl::Shader();
    len = ReadShaderSource( "./assets/shaders/draw_gui.vs", &sources );
    if ( len < 0 )
        throw std::runtime_error( "Failed To Load Vertex Shader" );
    
    if( !shaders[0]->Create( GL_VERTEX_SHADER, &sources, &len, 1 ) )
        throw std::runtime_error( "Failed to create vertex shader" );
    
    if ( sources != nullptr )
    {
        std::free( sources );
        sources = nullptr;
    }
    
	///
	/// Load Fragment shader
	///
    shaders[1] = new gl::Shader();
    len = ReadShaderSource( "./assets/shaders/draw_gui.fs", &sources );
    if ( len < 0 )
        throw std::runtime_error( "Failed To Load Fragmente Shader" );

    if( !shaders[1]->Create( GL_FRAGMENT_SHADER, &sources, &len, 1 ) )
        throw std::runtime_error( "Failed to create fragment shader" );
    
    if ( sources != nullptr )
    {
        std::free( sources );
        sources = nullptr;
    }

	///
	/// Create and Link Program 
	///
	if( !m_program.Create( const_cast<const gl::Shader**>(shaders), 2 ) )
		throw std::runtime_error("can't create program");
}

void Gwen::Renderer::OpenGL::InitBuffers(void)
{
	GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

	///
	/// Create Vertex Buffer
	m_vertexBuffer.Create( GL_ARRAY_BUFFER, k_VERTEX_SIZE * k_MAX_VERTICES_COUNT, nullptr, flags );
	m_vertexes = static_cast<GLfloat*>( m_vertexBuffer.Map( 0, k_VERTEX_SIZE * k_MAX_VERTICES_COUNT, flags ) );

	///
	/// Create Index Buffer 
	m_elementBuffer.Create( GL_ELEMENT_ARRAY_BUFFER, sizeof( uint16_t ) * k_MAX_ELEMENTS_COUNT, nullptr, flags );
	m_elements = static_cast<uint16_t*>( m_elementBuffer.Map( 0, sizeof( uint16_t ) * k_MAX_ELEMENTS_COUNT, flags ) );

	///
	/// Create Uniform Block Buffer
	m_uniformBuffer.Create( GL_UNIFORM_BUFFER, sizeof( float ) * 20, nullptr, GL_DYNAMIC_STORAGE_BIT );

}

void Gwen::Renderer::OpenGL::CreateSamplers(void)
{
	///
	/// Createa a white image whit size 16 x 16
	///
	uint16_t image[16 * 16 * 4];

	// make a full white image
	std::memset( image, 0xFF, 16 * 16 * 4 );

	gl::Texture::createInfo_t textureCI{};
	textureCI.target = GL_TEXTURE_2D;
	textureCI.levels = 1;
	textureCI.layers = 1;
	textureCI.samples = 1;
	textureCI.format = GL_RGBA8;
	textureCI.dimensions = { 16, 16, 0 };
	m_white.Create( &textureCI );

	gl::Texture::subImage_t wim{};
	wim.level = 0;
	wim.layer = 0;
	wim.imageSize = 16 * 16 * 4; 
	wim.offsets = { 0, 0, 0 };
	wim.dimension = { 16, 16, 1 };
	m_white.SubImage( &wim, image );

	m_sample.Create();
	m_sample.Parameteri( GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	m_sample.Parameteri( GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// we don't filter fonts 
	m_fontSample.Create();
	m_fontSample.Parameteri( GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	m_fontSample.Parameteri( GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void Gwen::Renderer::OpenGL::CreateVertexArray(void)
{
	///
	/// Create Vertex Array and configure vertex attributes 
	///
	gl::VertexArray::vertexAttrib_t	attrbs[2]
	{
		{ 0, 0, 2, GL_FLOAT, GL_FALSE, 0},
		{ 1, 0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2 }
	};

	m_vertexArray.Create( attrbs, 2 );
	
	gl::VertexArray::bufferBindingPoint_t binding{};
	binding.buffer = m_vertexBuffer;
	binding.offset = 0;
	binding.stride = k_VERTEX_SIZE;
	m_vertexArray.BindeVertexBuffers( &binding, 0, 1 );
	m_vertexArray.BindElementBuffer( m_elementBuffer );
}

bool Gwen::Renderer::OpenGL::InitializeContext( Gwen::WindowProvider* pWindow )
{
	int w = 0, h = 0;

	if ( !pWindow )
		return false;

#if 0
	m_pContext = new SDL3Context();
	m_pContext->Create( pWindow->GetWindow() );

	SDL_GetWindowSizeInPixels( static_cast<SDL_Window*>( pWindow->GetWindow() ), &w, &h );

	m_width = w;
	m_heigth = h;

	Init();
#endif
	
	/// set the ortographic projection matrix 
	glm::mat4 projection = glm::ortho( 0.0f, static_cast<float>( m_width ), static_cast<float>( m_heigth ), 0.0f, -1.0f, 1.0f );
	m_uniformBuffer.Upload( glm::value_ptr( projection ), sizeof( glm::vec4 ), sizeof( glm::mat4 ) );
	return true;
}

bool Gwen::Renderer::OpenGL::ShutdownContext( Gwen::WindowProvider* pWindow )
{
	Release();

	if( m_pContext )
	{
		m_pContext->Destroy();
		delete m_pContext;
	}

	return true;
}

bool Gwen::Renderer::OpenGL::PresentContext( Gwen::WindowProvider* pWindow )
{
	if ( !pWindow )
		return false;

	return SDL_GL_SwapWindow( static_cast<SDL_Window*>( pWindow->GetWindow() ) );
}

bool Gwen::Renderer::OpenGL::ResizedContext( Gwen::WindowProvider* pWindow, int w, int h )
{
	if ( pWindow )
	{
		SDL_GetWindowSizeInPixels( static_cast<SDL_Window*>( pWindow->GetWindow() ), &w, &h );
	}
	
	m_width = w;
	m_heigth = h;

	glm::mat4 projection = glm::ortho( 0.0f, static_cast<float>( m_width ), static_cast<float>( m_heigth ), 0.0f, -1.0f, 1.0f );
	m_uniformBuffer.Upload( glm::value_ptr( projection ), sizeof( glm::vec4 ), sizeof( glm::mat4 ) );
	return true;
}

bool Gwen::Renderer::OpenGL::BeginContext( Gwen::WindowProvider* pWindow )
{
	return true;
}

bool Gwen::Renderer::OpenGL::EndContext( Gwen::WindowProvider* pWindow )
{
	glFinish();
	return true;
}