
#include "precompiled.h"
#include "ToolsDialogCommon.hpp"

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_clipboard.h>

// OpenGL Based render 
//#include "Gwen/Renderers/OpenGL.h"
#include "ToolsDialogRenderer.hpp"

// Simple colored skin
#include "Gwen/Skins/Simple.h"

static SDL_Cursor* cursor = nullptr;

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
    auto dialogRenderer = new crToolsDialogRenderer();
    dialogRenderer->Init();

    /// Use basic skit for the dialog rendering
    auto renderSkin = new Gwen::Skin::Simple( dialogRenderer );

    SetSkin( renderSkin );

    /// Set this Dialogs as main event destination
    m_eventManager.Initialize( this );
}

void crToolsDialogCommon::Destroy(void)
{  
    auto skin = GetSkin();
    auto renderer = skin->GetRender();
    
    /// Release skin
    if( skin )
    {
        delete skin;
        skin = nullptr;
    }

    /// Release renderer 
    if( renderer )
    {
        delete renderer;
        renderer = nullptr;
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

bool crToolsDialogCommon::Update( const SDL_Event *in_evt )
{
    /// Check if are a event from the script editor dialog window 
    if( in_evt->window.windowID != m_dialogWindowID || m_dialogWindow == nullptr )
        return false;

    /// Handle input event
    auto e = in_evt;
    return m_eventManager.ProcessEvent( &e );
}

/*
Gwen Platform
*/
void Gwen::Platform::Sleep( unsigned int iMS )
{
	SDL_Delay( iMS );
}

void Gwen::Platform::SetCursor( unsigned char iCursor )
{
	SDL_Cursor* old = nullptr;
	SDL_SystemCursor syscur = SDL_SYSTEM_CURSOR_DEFAULT;

	switch ( iCursor )
	{
	case CursorType::Normal:
		syscur = SDL_SYSTEM_CURSOR_DEFAULT;
		break;

	case CursorType::Beam:
		syscur = SDL_SYSTEM_CURSOR_TEXT;
		break;

	case CursorType::SizeNS:
		syscur = SDL_SYSTEM_CURSOR_NS_RESIZE;
		break;

	case CursorType::SizeWE:
		syscur = SDL_SYSTEM_CURSOR_EW_RESIZE;
		break;

	case CursorType::SizeNWSE:
		syscur = SDL_SYSTEM_CURSOR_NESW_RESIZE;
		break;

	case CursorType::SizeNESW:
		syscur = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
		break;

	case CursorType::SizeAll:
		syscur = SDL_SYSTEM_CURSOR_MOVE;
		break;

	case CursorType::No:
		syscur = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
		break;

	case CursorType::Wait:
		syscur = SDL_SYSTEM_CURSOR_WAIT;
		break;

	case CursorType::Finger:
		syscur = SDL_SYSTEM_CURSOR_POINTER;
		break;

	default:
		syscur = SDL_SYSTEM_CURSOR_DEFAULT;
		break;
	}

	// store current cursor
	old = cursor;

	// create a new pointer
	cursor = SDL_CreateSystemCursor( syscur );

	// set the cursor
	SDL_SetCursor( cursor );

	// release the previous cursor pointer
	if ( old != nullptr )
		SDL_DestroyCursor( old );
}

Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
	char* cpb = nullptr;
	Gwen::TextObject inString;
	if ( !SDL_HasClipboardText() )
		return Gwen::UnicodeString();

	// aquire the clipboard
	cpb = SDL_GetClipboardText();

	inString = cpb;

	SDL_free( cpb );

	return inString.GetUnicode();
}

bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString & str )
{
	Gwen::TextObject outString = str;
	
	// clear the clipboard before we set a new content 
	SDL_ClearClipboardData();

	return SDL_SetClipboardText( outString.c_str() );
}

float Gwen::Platform::GetTimeInSeconds()
{
	uint64_t time = SDL_GetTicks();
	return time / 1000;
}

bool Gwen::Platform::FileOpen( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
	return false;
}

bool Gwen::Platform::FileSave( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Gwen::Event::Handler::FunctionWithInformation fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
	return false;
}

bool Gwen::Platform::FolderOpen( const String & Name, const String & StartPath, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	return false;
}

void* Gwen::Platform::CreatePlatformWindow( int x, int y, int w, int h, const Gwen::String & strWindowTitle )
{
	// create a OpenGL capable window
	SDL_Window* window = SDL_CreateWindow( strWindowTitle.c_str(), w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS );
	return reinterpret_cast<void*>( window );
}

void Gwen::Platform::DestroyPlatformWindow( void* pPtr )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	SDL_DestroyWindow( window );
}

void Gwen::Platform::MessagePump( void* pWindow, Gwen::Controls::Canvas* ptarget )
{
	SDL_Event evt;
	static Input::SDL3 GwenInput;

	GwenInput.Initialize( ptarget );

#if 1
	while ( SDL_PollEvent( &evt ) )
	{
		GwenInput.ProcessEvent( &evt );
	}
#else
	if( SDL_WaitEvent( &evt ) )
		GwenInput.ProcessEvent( &evt );
#endif
}

void Gwen::Platform::SetBoundsPlatformWindow( void* pPtr, int x, int y, int w, int h )
{	
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	SDL_SetWindowPosition( window, x, y );
	SDL_SetWindowSize( window, w, h );
}

void Gwen::Platform::SetWindowMaximized( void* pPtr, bool bMax, Gwen::Point & pNewPos, Gwen::Point & pNewSize )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	if ( bMax )
		SDL_MaximizeWindow( window );
	else
		SDL_RestoreWindow( window );

	// Wait for window to update
	SDL_SyncWindow( window );

	// Get new bound 
	SDL_GetWindowPosition( window, &pNewPos.x, &pNewPos.y );
	SDL_GetWindowSize( window, &pNewSize.x, &pNewSize.y );	
}

void Gwen::Platform::SetWindowMinimized( void* pPtr, bool bMinimized )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	if ( bMinimized )
		SDL_MinimizeWindow( window );
	else
		SDL_RestoreWindow( window );
}

bool Gwen::Platform::HasFocusPlatformWindow( void* pPtr )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	auto flags = SDL_GetWindowFlags( window );
	return ( flags & SDL_WINDOW_INPUT_FOCUS ) || ( flags & SDL_WINDOW_MOUSE_FOCUS );
}

void Gwen::Platform::GetDesktopSize( int & w, int & h )
{
	SDL_Rect rect{};
	if ( !SDL_GetDisplayUsableBounds( 0, &rect ) )
	{
		w = 1024;
		h = 768;
		return;
	}

	w = rect.w;
	h = rect.h;
}

void Gwen::Platform::GetCursorPos( Gwen::Point & po )
{
	float x = 0.0f;
	float y = 0.0f;
	SDL_GetGlobalMouseState( &x, &y );
    po.x = static_cast<int>( x );
    po.y = static_cast<int>( y );
}