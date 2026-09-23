
#include "precompiled.h"
#include "ToolsDialogCommon.hpp"

crToolsDialogCommon::crToolsDialogCommon( void ) : Gwen::Controls::Canvas( nullptr )
{
}

crToolsDialogCommon::~crToolsDialogCommon( void )
{
}

void crToolsDialogCommon::Create( void )
{
    /// main egine window 
    SDL_Window* mwindow = static_cast<SDL_Window*>( crVideo::Get()->WindowHandler() );

    /// window properties
    SDL_WindowFlags wflags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_POPUP_MENU;

    /// Create script editor dialog window 
    m_dialogWindow = SDL_CreatePopupWindow( mwindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 420, wflags );
    if( !m_dialogWindow )
    {
        idLib::Error( "Failed to create Script Editor Window! (%s)\n", SDL_GetError() );
        Destroy();
        return;
    }

    // TODO: Create Window
    m_dialogWindowID = SDL_GetWindowID( m_dialogWindow );
    
    // Don't need multisample for tools 
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

    // Utilize LAST available OpenGL especification ( So sad Khronous, keep GL alive )
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 6 );

    m_renderContext = SDL_GL_CreateContext( m_dialogWindow );
    if( !m_renderContext )
    {
        idLib::Error( "Failed to initialize script editor! (%s)\n", SDL_GetError() );
        Destroy();
        return;
    }

    /// Initialize OpenGL Renderer
    m_dialogRenderer = new Gwen::Renderer::OpenGL();
    m_dialogRenderer->Init();

    /// Use basic skit for the dialog rendering
    m_renderSkin = new Gwen::Skin::Simple( m_dialogRenderer );

    /// Set this Dialogs as main event destination
    m_eventManager.Initialize( this );
}

void crToolsDialogCommon::Destroy(void)
{
    if( m_renderSkin )
    {
        delete m_renderSkin;
        m_renderSkin = nullptr;
    }

    if( m_dialogRenderer )
    {
        delete m_dialogRenderer;
        m_dialogRenderer = nullptr;
    }

    // Release script editor render context
    if( m_renderContext != nullptr )
    {
        SDL_GL_DestroyContext( m_renderContext );
        m_renderContext = nullptr;
    }

    // Release the script editor window 
    if( m_dialogWindow != nullptr )
    {   
        SDL_DestroyWindow( m_dialogWindow );
        m_dialogWindow = nullptr;
    }
}


bool crToolsDialogCommon::Update(const SDL_Event &in_evt)
{
    /// Check if are a event from the script editor dialog window 
    if( in_evt.window.windowID != m_dialogWindowID || m_dialogWindow == nullptr )
        return false;

    /// Handle input event
    auto e = in_evt;
    return m_eventManager.ProcessEvent( &e );
}
