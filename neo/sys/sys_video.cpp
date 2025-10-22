
#include "precompiled.h"
#include "sys_video.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

static idCVar in_nograb( "in_nograb", "0", CVAR_SYSTEM | CVAR_NOCHEAT, "prevents input grabbing" );

crDisplaySDL::crDisplaySDL( void ) : 
    m_x( 0 ),
    m_y( 0 ),
    m_width( 0 ),
    m_height( 0 ),
    m_display( 0 )
{
}

crDisplaySDL::~crDisplaySDL( void )
{
    // TODO: check if are really cleared
    m_name.Clear();
    m_modes.Clear();   
}

bool crDisplaySDL::Init(const SDL_DisplayID in_displayID)
{
    m_display = in_displayID; 
    int numModes = 0;
    SDL_Rect bounds{};

    // Get Display Name
    m_name = SDL_GetDisplayName( m_display );

    // display bounds
    if( !SDL_GetDisplayUsableBounds( m_display, &bounds ) )
        return false;

    m_x = bounds.x;
    m_y = bounds.y;
    m_width = bounds.w;
    m_height = bounds.h;

    SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes( m_display, &numModes );
    if ( !modes )
        return false;

    m_modes.Resize( numModes );
    for ( uint32_t i = 0; i < numModes; i++)
    {
        m_modes[i].modeID = i;
        m_modes[i].displayID = m_display;
        m_modes[i].width = modes[i]->w;
        m_modes[i].height = modes[i]->h;
        m_modes[i].displayHz = modes[i]->refresh_rate;
        m_modes[i].format = modes[i]->format;
    }

    return true;
}

const char *crDisplaySDL::Name(void) const
{
    return m_name.c_str();
}

const vidMode_t *crDisplaySDL::Modes( uint32_t *in_count ) const
{
    if ( in_count != nullptr)
        *in_count = m_modes.Num();

    return m_modes.Ptr();
}

crVideoSDL3::crVideoSDL3( void ) : m_grabbed( false )
{
}

crVideoSDL3::~crVideoSDL3( void )
{
}

bool crVideoSDL3::StartUp( const uint32_t in_flags )
{
    int displayCount = 0;

    // we create a hiden window 
    SDL_WindowFlags windowflags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    
    if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		if( SDL_Init( SDL_INIT_VIDEO ) )
        {
			common->Error( "Error while initializing SDL: %s", SDL_GetError() );
            return false;
        }
	}

    m_vulkan = in_flags & 0 << 2; 
    if (  m_vulkan ) 
    {
        // we need load libraries before window creation
        windowflags |= SDL_WINDOW_VULKAN;
        if( !SDL_Vulkan_LoadLibrary( nullptr ) )
			common->Error( "SDL Error while loading Vulkan Library : %s", SDL_GetError() );
    }
    else
    {
        windowflags |= SDL_WINDOW_OPENGL;
        if( !SDL_GL_LoadLibrary( nullptr ) )
			common->Error( "SDL Error while loading OpenGL Library : %s", SDL_GetError() );
    }

    SDL_DisplayID * displays = SDL_GetDisplays( &displayCount );
    for ( uint32_t i = 0; i < displayCount; i++)
    {
        crDisplaySDL* display = new( TAG_VIDEO_SYS ) crDisplaySDL();
        if ( !display->Init( displays[i] ) )
            continue;

        m_displays.Append( display );
    }

    return true;
}

void crVideoSDL3::ShutDown(void)
{
    if ( m_mainWindow )
        m_mainWindow.Destroy();
    
    for ( uint32_t i = 0; i < m_displays.Num(); i++)
    {
        delete m_displays[i];
    }

    m_displays.Clear();

    if( m_vulkan )
        SDL_Vulkan_UnloadLibrary();
    else
        SDL_GL_UnloadLibrary();
}

void *crVideoSDL3::WindowHandler(void)
{
    return reinterpret_cast<void*>( m_mainWindow.GetHandle() );
}

void crVideoSDL3::GrabInput(const uint32_t in_flags)
{
    bool grab = in_flags & GRAB_ENABLE;
	
	if( grab && ( in_flags & GRAB_REENABLE ) )
		grab = false;
		
	if( in_flags & GRAB_SETSTATE )
		m_grabbed = grab;
		
	if( in_nograb.GetBool() )
		grab = false;
		
	if( !m_mainWindow )
	{
		common->Warning( "GLimp_GrabInput called without window" );
		return;
	}
	
    if ( grab )
    {
        m_mainWindow.SetMouseGrab( true );
        m_mainWindow.SetKeyboardGrab( true );
        SDL_SetWindowRelativeMouseMode( m_mainWindow, true );
    }
    else
    {
        m_mainWindow.SetMouseGrab( false );
        m_mainWindow.SetKeyboardGrab( false );
        SDL_SetWindowRelativeMouseMode( m_mainWindow, false );
    }
}

bool crVideoSDL3::SetMode(const vidMode_t in_mode, const videoMode_t in_fullScreen)
{    
    if ( in_fullScreen <= VIDEO_MODE_FULLSCREEN )
    {
        SDL_DisplayMode*    fsmode = nullptr;

        // we use a listed mode 
        if( in_fullScreen == VIDEO_MODE_DEDICATED )
        {
            SDL_DisplayMode** modes = nullptr; 
            modes = SDL_GetFullscreenDisplayModes( in_mode.displayID, nullptr );
            fsmode = modes[in_mode.modeID];
        }
        else // try a custom mode
        { 
            if( !SDL_GetClosestFullscreenDisplayMode( in_mode.displayID, in_mode.width, in_mode.height, in_mode.displayHz, true, fsmode ) )
            {
	    	    common->Warning( "Couldn't found compatible window mode for fullscreen, reason: %s", SDL_GetError() );
	    	    return false;
	        }
        }

        // set the fullscreen mode
        if( SDL_SetWindowFullscreenMode( m_mainWindow, fsmode ) )
	    {
	    	common->Warning( "Couldn't set window mode for fullscreen, reason: %s", SDL_GetError() );
	    	return false;
	    }

        // try enable full screen mode
        if( !( SDL_GetWindowFlags( m_mainWindow ) & SDL_WINDOW_FULLSCREEN ) )
        {
            if( !SDL_SetWindowFullscreen( m_mainWindow, true )  )
            {
                common->Warning( "Couldn't switch to fullscreen mode, reason: %s!", SDL_GetError() );
                return false;
            }
        }
    }
    else
    {
        // set windos size
        SDL_SetWindowSize( m_mainWindow, in_mode.width, in_mode.height );
	    SDL_SetWindowPosition( m_mainWindow, SDL_WINDOWPOS_CENTERED_DISPLAY( in_mode.displayID ), SDL_WINDOWPOS_CENTERED_DISPLAY( in_mode.displayID ) );
        if( SDL_GetWindowFlags( m_mainWindow ) & SDL_WINDOW_FULLSCREEN )
        {
            // disable full screen mode 
            if( !SDL_SetWindowFullscreen( m_mainWindow, false )  )
            {
                common->Warning( "Couldn't switch to windowed mode, reason: %s!", SDL_GetError() );
                return false;
            }
        }
    }

    return false;
}

void crVideoSDL3::SetGamma(uint16_t red[256], uint16_t green[256], uint16_t blue[256])
{
}

crDisplay* const* crVideoSDL3::Displays(uint32_t *in_count) const
{
    if ( in_count != nullptr )
        *in_count = m_displays.Num();

    return m_displays.Ptr();
}

void crVideoSDL3::ShowWindow(const bool in_show)
{
    if ( !m_mainWindow )
        return;

    if ( in_show )
        m_mainWindow.Show();
    else
        m_mainWindow.Hide();
}

bool crVideoSDL3::IsWindowVisible( void ) const
{
    if ( !m_mainWindow )
        return false;

    return !( m_mainWindow.GetFlags() & SDL_WINDOW_HIDDEN );
}
