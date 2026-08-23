
#include "Gwen/Precompiled.h"
#include "DrawPanel.h"

GWEN_CONTROL_CONSTRUCTOR( DrawPanel ),
    m_samples( 0 )
{
    SetBounds( 0, 0, 10000, 10000 );
    SetBackgroundColor( Color( 255, 255, 255, 255 ) );    
}

Gwen::Controls::DrawPanel::~DrawPanel(void)
{
}

void Gwen::Controls::DrawPanel::Initialize(void)
{
}

void Gwen::Controls::DrawPanel::Render(Skin::Base *skin)
{
    // draw to the framebuffe
    auto bounds = GetBounds();
    auto renderer = skin->GetRender();

#if 1
    renderer->BindFrameBuffer( &m_framebuffer, bounds );
    Draw();
    renderer->DrawFrameBuffer( &m_framebuffer, bounds );
#else
    renderer->DrawFilledRect( bounds );
#endif
}

void Gwen::Controls::DrawPanel::OnBoundsChanged( Gwen::Rect oldBounds )
{
    UpdateRenderBounds();

    auto bounds = GetRenderBounds();
    auto renderer = GetSkin()->GetRender();

    if ( m_framebuffer.failed == false )
    {
        // release old frame buffer
        renderer->FreeFrameBuffer( &m_framebuffer );   
    }
    
    // create a new frame buffer
    m_framebuffer.width = bounds.w;
    m_framebuffer.height = bounds.h;
    m_framebuffer.samples = m_samples;
    renderer->CreateFrameBuffer( &m_framebuffer );
}

void Gwen::Controls::DrawPanel::SetBackgroundColor(const Gwen::Color &color)
{
    m_backgroundColor = color;
}