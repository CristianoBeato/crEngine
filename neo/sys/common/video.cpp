/*
===========================================================================

Beato idTech 4x Source Code
Copyright (C) 2016-2022 Cristiano B. Santos <cristianobeato_dm@hotmail.com>.

This file is part of the Beato idTech 4x  GPL Source Code (?Beato idTech 4  Source Code?).

Beato idTech 4  Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Beato idTech 4x  Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Beato idTech 4x  Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#include "precompiled.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_window.hpp>
#include <SDL3/SDL_vulkan.h>
#include "sys_public.h"

static idCVar in_nograb( "in_nograb", "0", CVAR_SYSTEM | CVAR_NOCHEAT, "prevents input grabbing" );

struct display_t
{
    int x = 0;
    int y = 0;
    int width = 0; 
    int height = 0;
    idList<vidMode_t>   modes;
};

static struct video_t
{
    bool                grabbed = true;
    idList<display_t>   displays;
    SDL::Window         window = nullptr;
} video;

void    Sys_VideoInit( const uint32_t in_flags )
{
    int numDisplays = 0;

    // we create a hiden window 
    SDL_WindowFlags windowflags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    
    if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		if( SDL_Init( SDL_INIT_VIDEO ) )
			common->Error( "Error while initializing SDL: %s", SDL_GetError() );
	}

    // list the available displays 
    SDL_DisplayID * displays = SDL_GetDisplays( &numDisplays );
    video.displays.Resize( numDisplays );
    for ( uint32_t i = 0; i < numDisplays; i++)
    {
        int numModes = 0;
        SDL_Rect bounds{};
        if( !SDL_GetDisplayUsableBounds( displays[i], &bounds ) )
            continue;

        video.displays[i].x = bounds.x;
        video.displays[i].y = bounds.x;
        video.displays[i].width = bounds.w;
        video.displays[i].height = bounds.h;

        SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes( displays[i], &numModes );
        video.displays[i].modes.Resize( numModes );
        for ( uint32_t j = 0; j < numModes; j++)
        {
            SDL_DisplayMode* mode = modes[j];
            video.displays[i].modes[j].modeID = j;
            video.displays[i].modes[j].displayID = i;
            video.displays[i].modes[j].width = mode->w;
            video.displays[i].modes[j].height = mode->h;
            video.displays[i].modes[j].displayHz = mode->refresh_rate;
            video.displays[i].modes[j].format = mode->format;
        }
    }
    
    if ( video.window != nullptr )
        video.window.Destroy();

    if ( in_flags & 0 << 2 ) 
    {
        windowflags |= SDL_WINDOW_VULKAN;
                 
        // we need load libraries before window creation
        if( !SDL_Vulkan_LoadLibrary( nullptr ) )
			common->Error( "SDL Error while loading Vulkan Library : %s", SDL_GetError() );
    }
    else
    {
        windowflags |= SDL_WINDOW_OPENGL;

        if( !SDL_GL_LoadLibrary( nullptr ) )
			common->Error( "SDL Error while loading Vulkan Library : %s", SDL_GetError() );
    }

    // SDL_WINDOW_BORDERLESS;
    // SDL_WINDOW_RESIZABLE

    if( !video.window.Create( GAME_NAME, 800, 600,  windowflags ) )
        common->Error( "Error while creating SDL window: %s", SDL_GetError() );
}

void    Sys_VideoFinish( void )
{
    // release window 
    if ( video.window != nullptr )
        video.window.Destroy();

    SDL_GL_UnloadLibrary();
    SDL_Vulkan_UnloadLibrary();
}

void *Sys_VideoWindowHandler( void )
{
    return reinterpret_cast<void*>( &video.window );
}

void Sys_VideoGrabInput(const uint32_t flags)
{
	bool grab = flags & GRAB_ENABLE;
	
	if( grab && ( flags & GRAB_REENABLE ) )
		grab = false;
		
	if( flags & GRAB_SETSTATE )
		video.grabbed = grab;
		
	if( in_nograb.GetBool() )
		grab = false;
		
	if( !video.window )
	{
		common->Warning( "GLimp_GrabInput called without window" );
		return;
	}
	

    if ( grab )
    {
        video.window.SetMouseGrab( true );
        video.window.SetKeyboardGrab( true );
        SDL_SetWindowRelativeMouseMode( video.window, true );
    }
    else
    {
        video.window.SetMouseGrab( false );
        video.window.SetKeyboardGrab( false );
        SDL_SetWindowRelativeMouseMode( video.window, false );
    }

}

void Sys_VideoSetGamma( uint16_t red[256], uint16_t green[256], uint16_t blue[256] )
{

}

vidMode_t *Sys_VideoGetModeListForDisplay( const uint32_t in_requestedDisplayNum, uint32_t &in_count)
{
    if ( video.displays.Num() < in_requestedDisplayNum )
    {
        in_count = video.displays[in_requestedDisplayNum].modes.Num(); 
        return video.displays[in_requestedDisplayNum].modes.Ptr();
    }
    
    in_count = 0;
    return nullptr;
}

bool Sys_VideoSetMode( const vidMode_t in_mode, const videoMode_t in_fullScreen )
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
        if( SDL_SetWindowFullscreenMode( video.window, fsmode ) )
	    {
	    	common->Warning( "Couldn't set window mode for fullscreen, reason: %s", SDL_GetError() );
	    	return false;
	    }

        // try enable full screen mode
        if( !( SDL_GetWindowFlags( video.window ) & SDL_WINDOW_FULLSCREEN ) )
        {
            if( !SDL_SetWindowFullscreen( video.window, true )  )
            {
                common->Warning( "Couldn't switch to fullscreen mode, reason: %s!", SDL_GetError() );
                return false;
            }
        }
    }
    else
    {
        // set windos size
        SDL_SetWindowSize( video.window, in_mode.width, in_mode.height );
	    SDL_SetWindowPosition( video.window, SDL_WINDOWPOS_CENTERED_DISPLAY( in_mode.displayID ), SDL_WINDOWPOS_CENTERED_DISPLAY( in_mode.displayID ) );
        if( SDL_GetWindowFlags( video.window ) & SDL_WINDOW_FULLSCREEN )
        {
            // disable full screen mode 
            if( !SDL_SetWindowFullscreen( video.window, false )  )
            {
                common->Warning( "Couldn't switch to windowed mode, reason: %s!", SDL_GetError() );
                return false;
            }
        }
    }

    return false;
}