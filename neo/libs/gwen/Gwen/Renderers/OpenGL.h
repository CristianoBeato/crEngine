/*
	GWEN
	Copyright (c) 2011 Facepunch Studios
	See license in Gwen.h
*/

#ifndef GWEN_RENDERERS_OPENGL_H
#define GWEN_RENDERERS_OPENGL_H

#include <SDL3/SDL_video.h>
#include "crglCore.hpp"
#include "crglContext.hpp"
#include "Gwen/BaseRender.h"

namespace Gwen
{
	namespace Renderer
	{
		class OpenGL : public Gwen::Renderer::Base
		{
			public:
				OpenGL( void );
				OpenGL( gl::Context* in_pContext );
				~OpenGL( void );

				virtual void Init( void ) override;
				virtual void Release( void ) override;

				virtual void Begin( void ) override;
				virtual void End( void ) override;

				virtual void StartClip( void ) override;
				virtual void EndClip( void ) override;
				
				virtual void SetDrawColor( Gwen::Color color ) override;

				//virtual void DrawShavedCornerRect( Gwen::Rect rect, bool bSlight = false );
				virtual void DrawPixel( int x, int y ) override;
				virtual void DrawLinedRect( Gwen::Rect rect ) override;
				virtual void DrawFilledRect( Gwen::Rect rect ) override;
				virtual void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f ) override;
				virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString & text );
				virtual void LoadFont( Gwen::Font* pFont ) override;
				virtual void FreeFont( Gwen::Font* pFont ) override;
				virtual void LoadTexture( Gwen::Texture* pTexture ) override;
				virtual void FreeTexture( Gwen::Texture* pTexture ) override;
				virtual void CreateFrameBuffer( Gwen::FrameBuffer* pFrameBuffer ) override;
				virtual void FreeFrameBuffer( Gwen::FrameBuffer* pFrameBuffer ) override;
				virtual void BindFrameBuffer( Gwen::FrameBuffer* pFrameBuffer, Gwen::Rect renderRect ) override;
				virtual void DrawFrameBuffer( Gwen::FrameBuffer* pFrameBuffer, Gwen::Rect targetRect ) override;

				Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color & col_default ) override;
				Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString & text );

			protected:
				enum drawMode_t
				{
					RECT_LINE,
					RECT_FILL,
					RECT_TEXTURED,
					RECT_FONT
				};
				
				struct bounds_t
				{
					float	left = 0.0f;
					float	top = 0.0f;
					float	right = 0.0f;
					float	bottom = 0.0f;
				};
				
				drawMode_t			m_mode;
				uint16_t			m_vhead;
				uint16_t			m_vtail;
				uint16_t			m_ihead;
				uint16_t			m_itail;
				GLsizei				m_width;
				GLsizei				m_heigth;
				gl::VertexArray		m_vertexArray;
				gl::Program			m_program;
				gl::Buffer			m_vertexBuffer;
				gl::Buffer			m_elementBuffer;
				gl::Buffer			m_uniformBuffer;
				gl::Texture			m_white;
				gl::Texture			m_fontImage;				
				gl::Sampler			m_sample;
				gl::Sampler			m_fontSample;
				GLushort*			m_elements;
				GLfloat*			m_vertexes;
				
				void	AddQuad( const bounds_t pos, const bounds_t uv );
				void	Flush( void );
				
				void	InitShaders( void );
				void	InitBuffers( void );
				void	CreateSamplers( void );
				void	CreateVertexArray( void );

			public:
				//
				// Self Initialization
				//
				virtual bool InitializeContext( Gwen::WindowProvider* pWindow ) override;
				virtual bool ShutdownContext( Gwen::WindowProvider* pWindow ) override;
				virtual bool PresentContext( Gwen::WindowProvider* pWindow ) override;
				virtual bool ResizedContext( Gwen::WindowProvider* pWindow, int w, int h ) override;
				virtual bool BeginContext( Gwen::WindowProvider* pWindow ) override;
				virtual bool EndContext( Gwen::WindowProvider* pWindow ) override;

				
				gl::Context*	m_pContext;
		};

	}
}
#endif
