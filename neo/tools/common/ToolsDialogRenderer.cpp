
#include "precompiled.h"
#include "ToolsDialogRenderer.hpp"

#include "OpenGL.hpp"

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
}

void crToolsDialogRenderer::End( void )
{
}

void crToolsDialogRenderer::StartClip(void)
{
    // Enable clipping
    gl::SetState( GL_SCISSOR_TEST, GL_TRUE );
}

void crToolsDialogRenderer::EndClip(void)
{
    // Disable clipping
    gl::SetState( GL_SCISSOR_TEST, GL_FALSE );
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
}

void crToolsDialogRenderer::Flush(void)
{
}

void crToolsDialogRenderer::InitShaders(void)
{
}

void crToolsDialogRenderer::InitBuffers(void)
{
}

void crToolsDialogRenderer::CreateSamplers(void)
{
}

void crToolsDialogRenderer::CreateVertexArray(void)
{
}
