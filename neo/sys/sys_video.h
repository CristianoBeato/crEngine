/*
===========================================================================

crEngine GPL Source Code
Copyright (C) 2025 Cristiano B. Santos.

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
along with crEngine Source Code.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <SDL3/SDL_window.hpp>

class crDisplaySDL3 : public crDisplay
{
public:
    crDisplaySDL3( void );
    virtual ~crDisplaySDL3( void );
    bool Init( const SDL_DisplayID in_displayID, const int in_displayIndex );
    virtual const char* Name( void ) const;
    virtual const vidMode_t* Modes( uint32_t *in_count ) const;
    SDL_DisplayID       ID( void ) const { return m_display; }
    SDL_DisplayMode**   FullScreenModes( void ) const { return m_videoModes; }
private:
    int32_t                             m_x; 
    int32_t                             m_y;
    uint32_t                            m_width; 
    uint32_t                            m_height;
    SDL_DisplayID                       m_display;
    idStr                               m_name;
    idList<vidMode_t, TAG_VIDEO_SYS>    m_modes;
    SDL_DisplayMode**                   m_videoModes;
};

class crVideoSDL3 : public crVideo
{    
public:
    crVideoSDL3( void );
    ~crVideoSDL3( void );
    virtual bool    StartUp( const uint32_t in_flags );
    virtual void    ShutDown( void );
	virtual void*	WindowHandler( void );
	virtual void	GrabInput( const uint32_t in_flags );
	virtual bool	SetMode( const vidMode_t in_mode, const videoMode_t in_fullScreen );
	virtual void	SetGamma( uint16_t red[256], uint16_t green[256], uint16_t blue[256] );
	virtual void	ShowWindow( const bool in_show );
    virtual void    TextInput( const bool in_enable );	
	virtual bool    IsWindowVisible( void ) const;
	virtual crDisplay* const* Displays( uint32_t* in_count ) const;

private:
    bool                                m_grabbed;
    bool                                m_vulkan;
    SDL::Window                         m_mainWindow;
    idList<crDisplay*, TAG_VIDEO_SYS>   m_displays;
};

#endif //!__VIDEO_H__