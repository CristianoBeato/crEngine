
#include "Events.hpp"
#include "Platform.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_events.h>

// DG: those are needed for moving/resizing windows
extern idCVar r_windowX;
extern idCVar r_windowY;
extern idCVar r_windowWidth;
extern idCVar r_windowHeight;

const char* kbdNames[] =
{
	"english", "french", "german", "italian", "spanish", "turkish", "norwegian", NULL
};

static idCVar in_keyboard( "in_keyboard", "english", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT, "keyboard layout", kbdNames, idCmdSystem::ArgCompletion_String<kbdNames> );

// input subsystems
constexpr SDL_InitFlags K_EVENTS_INIT_FLAGS = SDL_INIT_EVENTS | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC;

class crEventsSDL3 : public crEvents
{
public:
	virtual void			StartUp( void ) override;
	virtual void			ShutDown( void ) override;
    virtual void        	PumpEvents( void ) override;
    virtual void        	GenerateEvents( void ) override;
	virtual sysEvent_t  	GetEvent( void ) override;
	virtual void        	ClearEvents( void ) override;
	virtual unsigned char	GetConsoleKey( const bool in_shifted );
    virtual void        	QueEvent( const sysEventType_t in_type, 
				const int in_value, 
				const int in_value2, 
				const size_t in_ptrLength, 
				const void* in_ptr, 
				const int in_inputDeviceNum ) override;

private:
	int					m_eventHead;
	idList<sysEvent_t>	m_event_queue;

	int 				SDLToDoom3Key( const SDL_Keycode &key, bool& isChar );
	static void 		PushConsoleEvent( const char* s );
	static bool SDLCALL HandleSDLEvents( void *userdata, SDL_Event *event );
};

/*
================
crEvents::Get
================
*/
static crEventsSDL3 gEventsSDL3 = crEventsSDL3();
crEvents* crEvents::Get( void )
{
	return &gEventsSDL3
}

/*
================
crEvents::PushConsoleEvent
================
*/
void crEventsSDL3::PushConsoleEvent( const char* s )
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
void crEventsSDL3::StartUp(void)
{
    //Initialize Game Controller API
	if ( !SDL_WasInit( K_EVENTS_INIT_FLAGS ) )
	{
		if( !SDL_InitSubSystem( K_EVENTS_INIT_FLAGS ) )
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
void crEventsSDL3::ShutDown(void)
{
	if( SDL_WasInit( K_EVENTS_INIT_FLAGS ) )
		SDL_QuitSubSystem( K_EVENTS_INIT_FLAGS );
}

/*
================
crEvents::GenerateEvents
================
*/
void crEventsSDL3::GenerateEvents( void )
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
void crEventsSDL3::QueEvent(    const sysEventType_t type, 
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
sysEvent_t crEventsSDL3::GetEvent( void )
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
void crEventsSDL3::ClearEvents( void )
{
	SDL_Event ev;	
	while( SDL_PollEvent( &ev ) );
}

/*
================
crEventsSDL3::GetConsoleKey
================
*/
unsigned char crEventsSDL3::GetConsoleKey(const bool in_shifted)
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
	
	return in_shifted ? keys[1] : keys[0];
}

/*
================
crEvents::HandleSDLEvents
================
*/
bool SDLCALL crEventsSDL3::HandleSDLEvents( void *userdata, SDL_Event *event )
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
				key = gEventsSDL3.SDLToDoom3Key( event->key.key, isChar );
				
				if( key == 0 )
				{
					unsigned char uc = event->key.scancode & 0xff;
					// check if its an unmapped console key
					if( uc == gEventsSDL3.GetConsoleKey( false ) || uc == gEventsSDL3.GetConsoleKey( true ) )
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
			
			gEventsSDL3.QueEvent( SE_KEY, key, event->key.down ? 1 : 0, 0, nullptr, 0 );
			crInputSystem::Get()->AppendKeyboardEvent( key, event->key.down ); // kbd_polls.Append( kbd_poll_t( key, event->key.down ) );
			
			if( key == K_BACKSPACE && event->key.down ) 
			{
				//c = key;
				gEventsSDL3.QueEvent( SE_CHAR, K_BACKSPACE, 0, 0, nullptr, 0 );
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
					gEventsSDL3.QueEvent( SE_CHAR, *event->text.text, 0, 0, nullptr, 0 );
				else 
				{
					char* s = nullptr;
					size_t s_pos = 0;
					s = std::strdup( event->text.text );
					while( s != nullptr )
					{
						gEventsSDL3.QueEvent( SE_CHAR, s[s_pos], 0, 0, nullptr, 0 );
						s_pos++;
						if( !s[s_pos] )
						{
							std::free( s );
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
				gEventsSDL3.QueEvent( SE_MOUSE_ABSOLUTE, event->motion.x, event->motion.y, 0, nullptr, 0 );
			else // this is the old, default behavior
				gEventsSDL3.QueEvent( SE_MOUSE, event->motion.xrel, event->motion.yrel, 0, nullptr, 0 );
			
			// DG end
			
			mouse_polls.Append( mouse_poll_t( M_DELTAX, event->motion.xrel ) );
			mouse_polls.Append( mouse_poll_t( M_DELTAY, event->motion.yrel ) );
			
			return false;
		}
		case SDL_EVENT_MOUSE_WHEEL:
			if( event->wheel.y > 0 )
			{
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, 1 ) );
				gEventsSDL3.QueEvent( SE_KEY, K_MWHEELUP, 1, 0, nullptr, 0 );
				// Immediately Queue Not Pressed Event
				gEventsSDL3.QueEvent( SE_KEY, K_MWHEELUP, 0, 0, nullptr, 0 );
			}
			else
			{
				mouse_polls.Append( mouse_poll_t( M_DELTAZ, -1 ) );
				gEventsSDL3.QueEvent( SE_KEY, K_MWHEELDOWN, 1, 0, nullptr, 0 );
				// Immediately Queue Not Pressed Event
				gEventsSDL3.QueEvent( SE_KEY, K_MWHEELDOWN, 0, 0, nullptr, 0 );
			}
			return false;
			
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			switch( event->button.button )
			{
				case SDL_BUTTON_LEFT:
				{
					gEventsSDL3.QueEvent( SE_KEY, K_MOUSE1, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION1, event->button.down ? 1 : 0 ) );
				}	break;
				case SDL_BUTTON_MIDDLE:
				{
					gEventsSDL3.QueEvent( SE_KEY, K_MOUSE3, event->button.down ? 1 : 0, 0, nullptr, 0 );
					mouse_polls.Append( mouse_poll_t( M_ACTION3, event->button.down ? 1 : 0 ) );
				} break;
				case SDL_BUTTON_RIGHT:
				{
					gEventsSDL3.QueEvent( SE_KEY, K_MOUSE2, event->button.down ? 1 : 0, 0, nullptr, 0 );
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
					gEventsSDL3.QueEvent( SE_CONSOLE, 0, 0, ( intptr_t )event->user.data1, event->user.data2, 0 );
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

int crEventsSDL3::SDLToDoom3Key( const SDL_Keycode &key, bool& isChar )
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