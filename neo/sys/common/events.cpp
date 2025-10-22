/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 dhewg (dhewm3)
Copyright (C) 2012 Robert Beckebans
Copyright (C) 2013 Daniel Gibson
Copyright (C) 2025 Cristiano Beato

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.	If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "idlib/precompiled.h"

// DG: SDL.h somehow needs the following functions, so #undef those silly
//     "don't use" #defines from Str.h
#undef strncmp
#undef strcasecmp
#undef vsnprintf
// DG end

#if __PLATFORM_WINDOWS__
#include "sys/win32/win_local.h"
#elif __PLATFORM_LINUX__ || __PLATFORM_FBSD__ 
#include "sys/posix/posix_public.h" 
#endif

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keyboard.h>

#include "renderer/tr_local.h"
#include "joystick.h"

// DG: those are needed for moving/resizing windows
extern idCVar r_windowX;
extern idCVar r_windowY;
extern idCVar r_windowWidth;
extern idCVar r_windowHeight;
// DG end

const char* kbdNames[] =
{
	"english", "french", "german", "italian", "spanish", "turkish", "norwegian", NULL
};

idCVar in_keyboard( "in_keyboard", "english", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT, "keyboard layout", kbdNames, idCmdSystem::ArgCompletion_String<kbdNames> );

struct kbd_poll_t
{
	int key;
	bool state;
	
	kbd_poll_t()
	{
	}
	
	kbd_poll_t( int k, bool s )
	{
		key = k;
		state = s;
	}
};

struct mouse_poll_t
{
	int action;
	int value;
	
	mouse_poll_t()
	{
	}
	
	mouse_poll_t( int a, int v )
	{
		action = a;
		value = v;
	}
};

static idList<kbd_poll_t> kbd_polls;
static idList<mouse_poll_t> mouse_polls;
static idList<sysEvent_t> event_queue;

void Sys_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void *ptr, int inputDeviceNum );
static bool SDLCALL sys_HandleSDL_Events( void *userdata, SDL_Event *event );

int			eventHead = 0;

// RB begin
static int SDL_KeyToDoom3Key( SDL_Keycode key, bool& isChar )
{
	isChar = false;
	
	if( key >= SDLK_SPACE && key < SDLK_DELETE )
	{
		isChar = true;
		//return key;// & 0xff;
	}
	
	switch( key )
	{
		case SDLK_ESCAPE:
			return K_ESCAPE;
			
		case SDLK_SPACE:
			return K_SPACE;
			
			//case SDLK_EXCLAIM:
			/*
			SDLK_QUOTEDBL:
			SDLK_HASH:
			SDLK_DOLLAR:
			SDLK_AMPERSAND:
			SDLK_QUOTE		= 39,
			SDLK_LEFTPAREN		= 40,
			SDLK_RIGHTPAREN		= 41,
			SDLK_ASTERISK		= 42,
			SDLK_PLUS		= 43,
			SDLK_COMMA		= 44,
			SDLK_MINUS		= 45,
			SDLK_PERIOD		= 46,
			SDLK_SLASH		= 47,
			*/
		case SDLK_SLASH:
			return K_SLASH;// this is the '/' key on the keyboard
		case SDLK_APOSTROPHE:
			return K_APOSTROPHE; // This is the "'" key.
		case SDLK_0:
			return K_0;
			
		case SDLK_1:
			return K_1;
			
		case SDLK_2:
			return K_2;
			
		case SDLK_3:
			return K_3;
			
		case SDLK_4:
			return K_4;
			
		case SDLK_5:
			return K_5;
			
		case SDLK_6:
			return K_6;
			
		case SDLK_7:
			return K_7;
			
		case SDLK_8:
			return K_8;
			
		case SDLK_9:
			return K_9;
			
			// DG: add some missing keys..
		case SDLK_UNDERSCORE:
			return K_UNDERLINE;
			
		case SDLK_MINUS:
			return K_MINUS;
			
		case SDLK_COMMA:
			return K_COMMA;
			
		case SDLK_COLON:
			return K_COLON;
			
		case SDLK_SEMICOLON:
			return K_SEMICOLON;
			
		case SDLK_PERIOD:
			return K_PERIOD;
			
		case SDLK_AT:
			return K_AT;
			
		case SDLK_EQUALS:
			return K_EQUALS;
			// DG end
			
			/*
			SDLK_COLON		= 58,
			SDLK_SEMICOLON		= 59,
			SDLK_LESS		= 60,
			SDLK_EQUALS		= 61,
			SDLK_GREATER		= 62,
			SDLK_QUESTION		= 63,
			SDLK_AT			= 64,
			*/
			/*
			   Skip uppercase letters
			 */
			/*
			SDLK_LEFTBRACKET	= 91,
			SDLK_BACKSLASH		= 92,
			SDLK_RIGHTBRACKET	= 93,
			SDLK_CARET		= 94,
			SDLK_UNDERSCORE		= 95,
			SDLK_BACKQUOTE		= 96,
			*/
		case SDLK_RIGHTBRACKET:
			return K_RBRACKET;
		case SDLK_LEFTBRACKET:
			return K_LBRACKET;
		case SDLK_BACKSLASH:
			return K_BACKSLASH;
		case SDLK_A:
			return K_A;
			
		case SDLK_B:
			return K_B;
			
		case SDLK_C:
			return K_C;
			
		case SDLK_D:
			return K_D;
			
		case SDLK_E:
			return K_E;
			
		case SDLK_F:
			return K_F;
			
		case SDLK_G:
			return K_G;
			
		case SDLK_H:
			return K_H;
			
		case SDLK_I:
			return K_I;
			
		case SDLK_J:
			return K_J;
			
		case SDLK_K:
			return K_K;
			
		case SDLK_L:
			return K_L;
			
		case SDLK_M:
			return K_M;
			
		case SDLK_N:
			return K_N;
			
		case SDLK_O:
			return K_O;
			
		case SDLK_P:
			return K_P;
			
		case SDLK_Q:
			return K_Q;
			
		case SDLK_R:
			return K_R;
			
		case SDLK_S:
			return K_S;
			
		case SDLK_T:
			return K_T;
			
		case SDLK_U:
			return K_U;
			
		case SDLK_V:
			return K_V;
			
		case SDLK_W:
			return K_W;
			
		case SDLK_X:
			return K_X;
			
		case SDLK_Y:
			return K_Y;
			
		case SDLK_Z:
			return K_Z;
			
		case SDLK_RETURN:
			return K_ENTER;
			
		case SDLK_BACKSPACE:
			return K_BACKSPACE;
			
		case SDLK_PAUSE:
			return K_PAUSE;
			
			// DG: add tab key support
		case SDLK_TAB:
			return K_TAB;
			// DG end
			
			//case SDLK_APPLICATION:
			//	return K_COMMAND;
		case SDLK_CAPSLOCK:
			return K_CAPSLOCK;
			
		case SDLK_SCROLLLOCK:
			return K_SCROLL;
			
		case SDLK_POWER:
			return K_POWER;
			
		case SDLK_UP:
			return K_UPARROW;
			
		case SDLK_DOWN:
			return K_DOWNARROW;
			
		case SDLK_LEFT:
			return K_LEFTARROW;
			
		case SDLK_RIGHT:
			return K_RIGHTARROW;
			
		case SDLK_LGUI:
			return K_LWIN;
			
		case SDLK_RGUI:
			return K_RWIN;
			//case SDLK_MENU:
			//	return K_MENU;
			
		case SDLK_LALT:
			return K_LALT;
			
		case SDLK_RALT:
			return K_RALT;
			
		case SDLK_RCTRL:
			return K_RCTRL;
			
		case SDLK_LCTRL:
			return K_LCTRL;
			
		case SDLK_RSHIFT:
			return K_RSHIFT;
			
		case SDLK_LSHIFT:
			return K_LSHIFT;
			
		case SDLK_INSERT:
			return K_INS;
			
		case SDLK_DELETE:
			return K_DEL;
			
		case SDLK_PAGEDOWN:
			return K_PGDN;
			
		case SDLK_PAGEUP:
			return K_PGUP;
			
		case SDLK_HOME:
			return K_HOME;
			
		case SDLK_END:
			return K_END;
			
		case SDLK_F1:
			return K_F1;
			
		case SDLK_F2:
			return K_F2;
			
		case SDLK_F3:
			return K_F3;
			
		case SDLK_F4:
			return K_F4;
			
		case SDLK_F5:
			return K_F5;
			
		case SDLK_F6:
			return K_F6;
			
		case SDLK_F7:
			return K_F7;
			
		case SDLK_F8:
			return K_F8;
			
		case SDLK_F9:
			return K_F9;
			
		case SDLK_F10:
			return K_F10;
			
		case SDLK_F11:
			return K_F11;
			
		case SDLK_F12:
			return K_F12;
			// K_INVERTED_EXCLAMATION;
			
		case SDLK_F13:
			return K_F13;
			
		case SDLK_F14:
			return K_F14;
			
		case SDLK_F15:
			return K_F15;
			
		case SDLK_KP_7:
			return K_KP_7;
			
		case SDLK_KP_8:
			return K_KP_8;
			
		case SDLK_KP_9:
			return K_KP_9;
			
		case SDLK_KP_4:
			return K_KP_4;
			
		case SDLK_KP_5:
			return K_KP_5;
			
		case SDLK_KP_6:
			return K_KP_6;
			
		case SDLK_KP_1:
			return K_KP_1;
			
		case SDLK_KP_2:
			return K_KP_2;
			
		case SDLK_KP_3:
			return K_KP_3;
			
		case SDLK_KP_ENTER:
			return K_KP_ENTER;
			
		case SDLK_KP_0:
			return K_KP_0;
			
		case SDLK_KP_PERIOD:
			return K_KP_DOT;
			
		case SDLK_KP_DIVIDE:
			return K_KP_SLASH;
			// K_SUPERSCRIPT_TWO;


			
		case SDLK_KP_MINUS:
			return K_KP_MINUS;
			// K_ACUTE_ACCENT;
			
		case SDLK_KP_PLUS:
			return K_KP_PLUS;
			
		case SDLK_NUMLOCKCLEAR:
			return K_NUMLOCK;
			
		case SDLK_KP_MULTIPLY:
			return K_KP_STAR;
			
		case SDLK_KP_EQUALS:
			return K_KP_EQUALS;
			
			// K_MASCULINE_ORDINATOR;
			// K_GRAVE_A;
			// K_AUX1;
			// K_CEDILLA_C;
			// K_GRAVE_E;
			// K_AUX2;
			// K_AUX3;
			// K_AUX4;
			// K_GRAVE_I;
			// K_AUX5;
			// K_AUX6;
			// K_AUX7;
			// K_AUX8;
			// K_TILDE_N;
			// K_GRAVE_O;
			// K_AUX9;
			// K_AUX10;
			// K_AUX11;
			// K_AUX12;
			// K_AUX13;
			// K_AUX14;
			// K_GRAVE_U;
			// K_AUX15;
			// K_AUX16;
			
		case SDLK_PRINTSCREEN:
			return K_PRINTSCREEN;
			
		case SDLK_MODE:
			return K_RALT;
	}
	
	return 0;
}
// RB end

static void PushConsoleEvent( const char* s )
{
	char* b;
	size_t len;
	
	len = strlen( s ) + 1;
	b = ( char* )Mem_Alloc( len, TAG_EVENTS );
	strcpy( b, s );
	
	SDL_Event event;
	
	event.type = SDL_EVENT_USER;
	event.user.code = SE_CONSOLE;
	event.user.data1 = ( void* )len;
	event.user.data2 = b;
	
	SDL_PushEvent( &event );
}

/*
=================
Sys_InitInput
=================
*/
void Sys_InitInput( void )
{
	int	joysticCount = 0;
	char guid[64]{};

	common->Printf( "\n------- Input Initialization -------\n" );
	kbd_polls.SetGranularity( 64 );
	mouse_polls.SetGranularity( 64 );
	event_queue.SetGranularity( 256 );
	in_keyboard.SetModified();

	//Initialize Game Controller API
	if ( !SDL_WasInit( SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC ) )
	{
		if( !SDL_Init( SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC ) )
			common->Error( SDL_GetError() );
	}
	
	// Initialize Event Filter
	SDL_SetEventFilter( sys_HandleSDL_Events, nullptr );
	
	Sys_InitGamepads();
}

/*
=================
Sys_ShutdownInput
=================
*/
void Sys_ShutdownInput( void )
{
	Sys_ShutdownGamepads();
	kbd_polls.Clear();
	mouse_polls.Clear();
	event_queue.Clear();
}

/*
===========
Sys_InitScanTable
===========
*/
// Windows has its own version due to the tools
#ifndef _WIN32
void Sys_InitScanTable()
{
}
#endif

/*
===============
Sys_GetConsoleKey
===============
*/
unsigned char Sys_GetConsoleKey( bool shifted )
{
	static unsigned char keys[2] = { '`', '~' };
	
	if( in_keyboard.IsModified() )
	{
		idStr lang = in_keyboard.GetString();
		
		if( lang.Length() )
		{
			if( !lang.Icmp( "french" ) )
			{
				keys[0] = '<';
				keys[1] = '>';
			}
			else if( !lang.Icmp( "german" ) )
			{
				keys[0] = '^';
				keys[1] = 176; // °
			}
			else if( !lang.Icmp( "italian" ) )
			{
				keys[0] = '\\';
				keys[1] = '|';
			}
			else if( !lang.Icmp( "spanish" ) )
			{
				keys[0] = 186; // º
				keys[1] = 170; // ª
			}
			else if( !lang.Icmp( "turkish" ) )
			{
				keys[0] = '"';
				keys[1] = 233; // é
			}
			else if( !lang.Icmp( "norwegian" ) )
			{
				keys[0] = 124; // |
				keys[1] = 167; // §
			}
		}
		
		in_keyboard.ClearModified();
	}
	
	return shifted ? keys[1] : keys[0];
}

/*
===============
Sys_MapCharForKey
===============
*/
unsigned char Sys_MapCharForKey( int key )
{
	return key & 0xff;
}

/*
===============
Sys_GrabMouseCursor
===============
*/
void Sys_GrabMouseCursor( bool grabIt )
{
	int flags;
	
	if( grabIt )
	{
		// DG: disabling the cursor is now done once in GLimp_Init() because it should always be disabled
		flags = GRAB_ENABLE | GRAB_SETSTATE;
		// DG end
	}
	else
		flags = GRAB_SETSTATE;
	
	sys->GetVideoSystem()->GrabInput( flags );
}

/*
================

Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent( void )
{
	SDL_Event ev;
	sysEvent_t res = { };
	int eventNum = event_queue.Num();
	
	static const sysEvent_t res_none = { SE_NONE, 0, 0, 0, nullptr };
	if(eventNum && eventHead < eventNum ) 
	{
		res = event_queue[eventHead];
		eventHead++;
		return res;
	} 
	else
	{
		return res_none;
	}
		
}

void Sys_QueEvent( sysEventType_t type, int value, int value2, int ptrLength, void *ptr, int inputDeviceNum )
{
	sysEvent_t eventData = { };
	eventData.evType = type;
	eventData.evValue = value;
	eventData.evValue2 = value2;
	eventData.evPtrLength = ptrLength;
	eventData.evPtr = ptr;
	eventData.inputDevice = inputDeviceNum;
	event_queue.Append(eventData);
}

bool SDLCALL sys_HandleSDL_Events( void *userdata, SDL_Event *event )
{
	sysEvent_t res = { };
	int key;
	static const sysEvent_t res_none = { SE_NONE, 0, 0, 0, nullptr };
	switch( event->type )
	{
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		{
			// unset modifier, in case alt-tab was used to leave window and ALT is still set
			// as that can cause fullscreen-toggling when pressing enter...
			SDL_Keymod currentmod = SDL_GetModState();
			int newmod = SDL_KMOD_NONE;
			if( currentmod & SDL_KMOD_CAPS ) // preserve capslock
				newmod |= SDL_KMOD_CAPS;
				
			SDL_SetModState( ( SDL_Keymod )newmod );
			
			// DG: un-pause the game when focus is gained, that also re-grabs the input
			//     disabling the cursor is now done once in GLimp_Init() because it should always be disabled
			cvarSystem->SetCVarBool( "com_pause", false );
			// DG end
			break;
		}
		case SDL_EVENT_WINDOW_FOCUS_LOST:
		{
			// DG: pause the game when focus is lost, that also un-grabs the input
			cvarSystem->SetCVarBool( "com_pause", true );
			// DG end

		} break;
		case SDL_EVENT_WINDOW_RESIZED:
		{
			int w = event->window.data1;
			int h = event->window.data2;
			r_windowWidth.SetInteger( w );
			r_windowHeight.SetInteger( h );
					
			glConfig.nativeScreenWidth = w;
			glConfig.nativeScreenHeight = h;
		} break;
		case SDL_EVENT_WINDOW_MOVED:
		{
			int x = event->window.data1;
			int y = event->window.data2;
			r_windowX.SetInteger( x );
			r_windowY.SetInteger( y );
		} break;

		case SDL_EVENT_KEY_DOWN:
		{
			if( event->key.key == SDLK_RETURN && ( event->key.mod & SDL_KMOD_ALT ) > 0 )
			{
				// DG: go to fullscreen on current display, instead of always first display
				int fullscreen = 0;
				if( ! renderSystem->IsFullScreen() )
				{
					// this will be handled as "fullscreen on current window"
					// r_fullscreen 1 means "fullscreen on first window" in d3 bfg
					fullscreen = -2;
				}
				cvarSystem->SetCVarInteger( "r_fullscreen", fullscreen );
				// DG end
				PushConsoleEvent( "vid_restart" );
				return 0;
			}
			
			// DG: ctrl-g to un-grab mouse - yeah, left ctrl shoots, then just use right ctrl :)
			if( event->key.key == SDLK_G && ( event->key.mod & SDL_KMOD_CTRL ) > 0 )
			{
				bool grab = cvarSystem->GetCVarBool( "in_nograb" );
				grab = !grab;
				cvarSystem->SetCVarBool( "in_nograb", grab );
				return 0;
			}
			// DG end	
			// fall through
		}
		case SDL_EVENT_KEY_UP:
		{
			bool isChar;
			char c;
			// DG: special case for SDL_SCANCODE_GRAVE - the console key under Esc
			if( event->key.scancode == SDL_SCANCODE_GRAVE )
			{
				key = K_GRAVE;
				c = K_BACKSPACE; // bad hack to get empty console inputline..

			} // DG end, the original code is in the else case
			else
			{
				key = SDL_KeyToDoom3Key( event->key.key, isChar );
				
				if( key == 0 )
				{
					unsigned char uc = event->key.scancode & 0xff;
					// check if its an unmapped console key
					if( uc == Sys_GetConsoleKey( false ) || uc == Sys_GetConsoleKey( true ) )
					{
						key = K_GRAVE;
						c = K_BACKSPACE; // bad hack to get empty console inputline..
					}
					else
					{
						if( event->type == SDL_EVENT_KEY_DOWN ) // FIXME: don't complain if this was an ASCII char and the console is open?
							common->Warning( "unmapped SDL key %d (0x%x) scancode %d", event->key.key, event->	key.scancode, event->key.scancode );
						return 0;
					}
				}
			}
			
			Sys_QueEvent( SE_KEY, key, event->key.down ? 1 : 0, 0, nullptr, 0 );
			kbd_polls.Append( kbd_poll_t( key, event->key.down ) );
			
			if( key == K_BACKSPACE && event->key.down ) 
			{
				//c = key;
				Sys_QueEvent( SE_CHAR, K_BACKSPACE, 0, 0, nullptr, 0 );
			}
			//Sys_QueEvent( SE_CHAR, c, 0, 0, NULL, 0 );
			return 0;

		}
		case SDL_EVENT_TEXT_INPUT:
		{
			if( event->text.text && *event->text.text )
			{
				if( !event->text.text[1] ) 
					Sys_QueEvent( SE_CHAR, *event->text.text, 0, 0, nullptr, 0 );
				else 
				{
					char* s = nullptr;
					size_t s_pos = 0;
					s = strdup( event->text.text );
					while( s != nullptr )
					{
						Sys_QueEvent( SE_CHAR, s[s_pos], 0, 0, nullptr, 0 );
						s_pos++;
						if( !s[s_pos] )
						{
							free( s );
							s = nullptr;
							s_pos = 0;
						}
					}
					return 0;
				}
			}
			
			return 1;
		}
		case SDL_EVENT_MOUSE_MOTION:
		{
			// DG: return event with absolute mouse-coordinates when in menu
			// to fix cursor problems in windowed mode
			if( game && game->Shell_IsActive() )
			{
				Sys_QueEvent( SE_MOUSE_ABSOLUTE, event->motion.x, event->motion.y, 0, NULL, 0 );
			}
			else // this is the old, default behavior
			{
				Sys_QueEvent( SE_MOUSE, event->motion.xrel, event->motion.yrel, 0, NULL, 0 );
			}
			// DG end
			
			mouse_polls.Append( mouse_poll_t( M_DELTAX, event->motion.xrel ) );
			mouse_polls.Append( mouse_poll_t( M_DELTAY, event->motion.yrel ) );
			
			return 0;
		}
		case SDL_EVENT_MOUSE_WHEEL:
			if( event->wheel.y > 0 )
			{
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, 1 ) );
				Sys_QueEvent( SE_KEY, K_MWHEELUP, 1, 0, NULL, 0 );
				/* Immediately Queue Not Pressed Event */
				Sys_QueEvent( SE_KEY, K_MWHEELUP, 0, 0, NULL, 0 );
			}
			else
			{
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, -1 ) );
				Sys_QueEvent( SE_KEY, K_MWHEELDOWN, 1, 0, NULL, 0 );
				/* Immediately Queue Not Pressed Event */
				Sys_QueEvent( SE_KEY, K_MWHEELDOWN, 0, 0, NULL, 0 );
			}
			return 0;
			
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			switch( event->button.button )
			{
				case SDL_BUTTON_LEFT:
				{
					Sys_QueEvent( SE_KEY, K_MOUSE1, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION1, event->button.down ? 1 : 0 ) );
				}	break;
				case SDL_BUTTON_MIDDLE:
				{
					Sys_QueEvent( SE_KEY, K_MOUSE3, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION3, event->button.down ? 1 : 0 ) );
				} break;
				case SDL_BUTTON_RIGHT:
				{
					Sys_QueEvent( SE_KEY, K_MOUSE2, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION2, event->button.down ? 1 : 0 ) );
				} break;
			}
			return 0;
		}

		case SDL_EVENT_GAMEPAD_ADDED:
		{
			Sys_JoystickConnect( event->gdevice.which );
	        return 1;
		}
		case SDL_EVENT_GAMEPAD_REMOVED:
		{
			Sys_JoystickDisconnect( event->gdevice.which );
			return 1;
		}
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
		{
			//Sys_QueEvent( SE_KEY, key, value, 0, NULL, inputDeviceNum );
		}
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		{
			//Sys_QueEvent( SE_JOYSTICK, axis, percent, 0, nullptr, inputDeviceNum );
	        return 1;
		}

		//case SDL_JOYAXISMOTION:
		//case SDL_JOYBALLMOTION:          /**< Joystick trackball motion */
		//case SDL_JOYHATMOTION:           /**< Joystick hat position change */
		//case SDL_JOYBUTTONDOWN:          /**< Joystick button pressed */
		//case SDL_JOYBUTTONUP:            /**< Joystick button released */
		//case SDL_JOYDEVICEADDED:         /**< A new joystick has been inserted into the system */
		//case SDL_JOYDEVICEREMOVED:       /**< An opened joystick has been removed */
		//	// Always Pass these events on to SDL
	    //    return 1;
		case SDL_EVENT_QUIT:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			PushConsoleEvent( "quit" );
			return 0;
			
		case SDL_EVENT_USER:
			switch( event->user.code )
			{
				case SE_CONSOLE:
					//res.evType = SE_CONSOLE;
					//res.evPtrLength = ( intptr_t )event->user.data1;
					//res.evPtr = event->user.data2;
					Sys_QueEvent( SE_CONSOLE, 0, 0, ( intptr_t )event->user.data1, event->user.data2, 0 );
					return 0;
				default:
					common->Warning( "unknown user event %u", event->user.code );
					return 1;
			}
		default:
			common->Warning( "unknown event %u", event->type );
			return 1;
	}
	// Event Not Handled, Add to Main queue.
	return 1;
}

/*
================
Sys_ClearEvents
================
*/
void Sys_ClearEvents( void )
{
	SDL_Event ev;
	
	while( SDL_PollEvent( &ev ) )
		;
		
	kbd_polls.SetNum( 0 );
	mouse_polls.SetNum( 0 );
	event_queue.SetNum(0);
	eventHead = 0;
}

/*
================
Sys_GenerateEvents
================
*/
void Sys_GenerateEvents( void )
{
#if __PLATFORM_WINDOWS__
	char* s = Sys_ConsoleInput();
#elif __PLATFORM_LINUX__ || __PLATFORM_FBSD__
	char* s = Posix_ConsoleInput();
#endif
	
	if( s )
		PushConsoleEvent( s );
		
	SDL_PumpEvents();
}

/*
================
Sys_PollKeyboardInputEvents
================
*/
int Sys_PollKeyboardInputEvents()
{
	return kbd_polls.Num();
}

/*
================
Sys_ReturnKeyboardInputEvent
================
*/
int Sys_ReturnKeyboardInputEvent( const int n, int& key, bool& state )
{
	if( n >= kbd_polls.Num() )
		return 0;
		
	key = kbd_polls[n].key;
	state = kbd_polls[n].state;
	return 1;
}

/*
================
Sys_EndKeyboardInputEvents
================
*/
void Sys_EndKeyboardInputEvents()
{
	kbd_polls.SetNum( 0 );
}

/*
================
Sys_PollMouseInputEvents
================
*/
int Sys_PollMouseInputEvents( int mouseEvents[MAX_MOUSE_EVENTS][2] )
{
	int numEvents = mouse_polls.Num();
	
	if( numEvents > MAX_MOUSE_EVENTS )
	{
		numEvents = MAX_MOUSE_EVENTS;
	}
	
	for( int i = 0; i < numEvents; i++ )
	{
		const mouse_poll_t& mp = mouse_polls[i];
		
		mouseEvents[i][0] = mp.action;
		mouseEvents[i][1] = mp.value;
	}
	
	mouse_polls.SetNum( 0 );
	
	return numEvents;
}