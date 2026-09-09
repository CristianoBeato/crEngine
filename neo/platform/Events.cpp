
#include "Events.hpp"
#include "Platform.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_events.h>

// DG: those are needed for moving/resizing windows
extern idCVar r_windowX;
extern idCVar r_windowY;
extern idCVar r_windowWidth;
extern idCVar r_windowHeight;

/*
================
crEvents::PushConsoleEvent
================
*/
void crEvents::PushConsoleEvent( const char* s )
{
	char* b;
	size_t len;
	
	len = std::strlen( s ) + 1;
	b = ( char* )Mem_Alloc( len, TAG_EVENTS );
	std::strcpy( b, s );
	
	SDL_Event event;
	
	event.type = SDL_EVENT_USER;
	event.user.code = SE_CONSOLE;
	event.user.data1 = ( void* )len;
	event.user.data2 = b;
	
	SDL_PushEvent( &event );
}

/*
================
crEvents::StartUp
================
*/
void crEvents::StartUp(void)
{
    //Initialize Game Controller API
	if ( !SDL_WasInit( SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC ) )
	{
		if( !SDL_Init( SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC ) )
			common->Error( SDL_GetError() );
	}
	
	// Initialize Event Filter
	SDL_SetEventFilter( HandleSDLEvents, nullptr );
}

/*
================
crEvents::ShutDown
================
*/
void crEvents::ShutDown(void)
{
}

/*
================
crEvents::GenerateEvents
================
*/
void crEvents::GenerateEvents( void )
{
	const char* s = crConsole::Get()->ConsoleInput();
	
	if( s )
		PushConsoleEvent( s );
		
	SDL_PumpEvents();
}

/*
================
crEvents::QueEvent
================
*/
void crEvents::QueEvent(    const sysEventType_t type, 
                            const int value, 
                            const int value2, 
                            const size_t ptrLength, 
                            const void *ptr, 
                            const int inputDeviceNum )
{
	sysEvent_t eventData = { };
	eventData.evType = type;
	eventData.evValue = value;
	eventData.evValue2 = value2;
	eventData.evPtrLength = ptrLength;
	eventData.evPtr = ptr;
	eventData.inputDevice = inputDeviceNum;
	m_event_queue.Append( eventData );
}

/*
================
crEvents::GetEvent
================
*/
sysEvent_t crEvents::GetEvent( void )
{
	SDL_Event ev;
	sysEvent_t res = { };
	int eventNum = m_event_queue.Num();
	
	static const sysEvent_t res_none = { SE_NONE, 0, 0, 0, 0, nullptr };
	if(eventNum && m_eventHead < eventNum ) 
	{
		res = m_event_queue[m_eventHead];
		m_eventHead++;
		return res;
	} 
	else
		return res_none;
}

/*
================
crEvents::ClearEvents
================
*/
void crEvents::ClearEvents( void )
{
	SDL_Event ev;	
	while( SDL_PollEvent( &ev ) );
}

/*
================
crEvents::HandleSDLEvents
================
*/
bool SDLCALL crEvents::HandleSDLEvents( void *userdata, SDL_Event *event )
{
	sysEvent_t res = { };
	int key;
	static const sysEvent_t res_none = { SE_NONE, 0, 0, 0, 0, nullptr };
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
			idRenderSystem::Get()->UpdateRenderSize( event->window.data1, event->window.data2 );
			return false; 
		} break;
		case SDL_EVENT_WINDOW_MOVED:
		{
			int x = event->window.data1;
			int y = event->window.data2;
			r_windowX.SetInteger( x );
			r_windowY.SetInteger( y );
			return false;
		} break;

		case SDL_EVENT_KEY_DOWN:
		{
			if( event->key.key == SDLK_RETURN && ( event->key.mod & SDL_KMOD_ALT ) > 0 )
			{
				// DG: go to fullscreen on current display, instead of always first display
				int fullscreen = 0;
				if( ! idRenderSystem::Get()->IsFullScreen() )
				{
					// this will be handled as "fullscreen on current window"
					// r_fullscreen 1 means "fullscreen on first window" in d3 bfg
					fullscreen = -2;
				}
				cvarSystem->SetCVarInteger( "r_fullscreen", fullscreen );
				// DG end
				PushConsoleEvent( "vid_restart" );
				return false;
			}
			
			// DG: ctrl-g to un-grab mouse - yeah, left ctrl shoots, then just use right ctrl :)
			if( event->key.key == SDLK_G && ( event->key.mod & SDL_KMOD_CTRL ) > 0 )
			{
				bool grab = cvarSystem->GetCVarBool( "in_nograb" );
				grab = !grab;
				cvarSystem->SetCVarBool( "in_nograb", grab );
				return false;
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
						return false;
					}
				}
			}
			
			QueEvent( SE_KEY, key, event->key.down ? 1 : 0, 0, nullptr, 0 );
			kbd_polls.Append( kbd_poll_t( key, event->key.down ) );
			
			if( key == K_BACKSPACE && event->key.down ) 
			{
				//c = key;
				Sys_QueEvent( SE_CHAR, K_BACKSPACE, 0, 0, nullptr, 0 );
			}
			//Sys_QueEvent( SE_CHAR, c, 0, 0, nullptr, 0 );
			return false;

		}
		case SDL_EVENT_TEXT_EDITING:
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
					return false;
				}
			}
			
			return true;
		}
		case SDL_EVENT_MOUSE_MOTION:
		{
			// DG: return event with absolute mouse-coordinates when in menu
			// to fix cursor problems in windowed mode
			if( game && game->Shell_IsActive() )
				QueEvent( SE_MOUSE_ABSOLUTE, event->motion.x, event->motion.y, 0, nullptr, 0 );
			else // this is the old, default behavior
				QueEvent( SE_MOUSE, event->motion.xrel, event->motion.yrel, 0, nullptr, 0 );
			
			// DG end
			
			mouse_polls.Append( mouse_poll_t( M_DELTAX, event->motion.xrel ) );
			mouse_polls.Append( mouse_poll_t( M_DELTAY, event->motion.yrel ) );
			
			return false;
		}
		case SDL_EVENT_MOUSE_WHEEL:
			if( event->wheel.y > 0 )
			{
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, 1 ) );
				QueEvent( SE_KEY, K_MWHEELUP, 1, 0, nullptr, 0 );
				// Immediately Queue Not Pressed Event
				QueEvent( SE_KEY, K_MWHEELUP, 0, 0, nullptr, 0 );
			}
			else
			{
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, -1 ) );
				QueEvent( SE_KEY, K_MWHEELDOWN, 1, 0, nullptr, 0 );
				// Immediately Queue Not Pressed Event
				QueEvent( SE_KEY, K_MWHEELDOWN, 0, 0, nullptr, 0 );
			}
			return false;
			
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			switch( event->button.button )
			{
				case SDL_BUTTON_LEFT:
				{
					QueEvent( SE_KEY, K_MOUSE1, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION1, event->button.down ? 1 : 0 ) );
				}	break;
				case SDL_BUTTON_MIDDLE:
				{
					QueEvent( SE_KEY, K_MOUSE3, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION3, event->button.down ? 1 : 0 ) );
				} break;
				case SDL_BUTTON_RIGHT:
				{
					QueEvent( SE_KEY, K_MOUSE2, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION2, event->button.down ? 1 : 0 ) );
				} break;
			}
			return false;
		}

		case SDL_EVENT_GAMEPAD_ADDED:
		{
			Sys_JoystickConnect( event->gdevice.which );
	        return false;
		}
		case SDL_EVENT_GAMEPAD_REMOVED:
		{
			Sys_JoystickDisconnect( event->gdevice.which );
			return false;
		}
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
		{
			//Sys_QueEvent( SE_KEY, key, value, 0, nullptr, inputDeviceNum );
			return true;
		}
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		{
			//Sys_QueEvent( SE_JOYSTICK, axis, percent, 0, nullptr, inputDeviceNum );
	        return true;
		}

		//case SDL_JOYAXISMOTION:
		//case SDL_JOYBALLMOTION:          /**< Joystick trackball motion */
		//case SDL_JOYHATMOTION:           /**< Joystick hat position change */
		//case SDL_JOYBUTTONDOWN:          /**< Joystick button pressed */
		//case SDL_JOYBUTTONUP:            /**< Joystick button released */
		//case SDL_JOYDEVICEADDED:         /**< A new joystick has been inserted into the system */
		//case SDL_JOYDEVICEREMOVED:       /**< An opened joystick has been removed */
		//	// Always Pass these events on to SDL
	    //    return true;
		case SDL_EVENT_QUIT:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			PushConsoleEvent( "quit" );
			return false;
			
		// just to ignore this window events
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
		case SDL_EVENT_WINDOW_SHOWN:
		case SDL_EVENT_WINDOW_EXPOSED:
		case SDL_EVENT_AUDIO_DEVICE_ADDED:
		case SDL_EVENT_KEYMAP_CHANGED:
		case SDL_EVENT_CLIPBOARD_UPDATE:
			return true;

		// may we can pause and unpause game
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			return true;
	

		case SDL_EVENT_USER:
			switch( event->user.code )
			{
				case SE_CONSOLE:
					QueEvent( SE_CONSOLE, 0, 0, ( intptr_t )event->user.data1, event->user.data2, 0 );
					return false;
				default:
					common->Warning( "unknown user event %u", event->user.code );
					return true;
			}
		default:
			common->Warning( "unknown event %u", event->type );
			return true;
	}
	// Event Not Handled, Add to Main queue.
	return true;
}
