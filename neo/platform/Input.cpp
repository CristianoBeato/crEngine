
#include "Input.hpp"
#include "Platform.hpp"

#include "Joystick.hpp"

inline constexpr uint32_t MAX_CONTROLLER_BUTTON_EVENTS = K_JOY_DPAD_RIGHT - K_JOY1 + 1;
inline constexpr uint32_t EVENTS_MAX_CONTROLLER_EVENTS = SDL_GAMEPAD_BUTTON_COUNT + SDL_GAMEPAD_AXIS_COUNT;

// keyboard event storage structure
struct keyboardPoll_t
{
	int		key = 0;
	bool	state = false;
};

// mouse event storage structure
struct mousePoll_t
{
	int action = 0;
	int value = 0;
};

class crInputSystemSDL3 : public crInputSystem
{
public:
	crInputSystemSDL3( void );
	virtual ~crInputSystemSDL3( void );

	// input is tied to windows, so it needs to be started up and shut down whenever
	// the main window is recreated
	virtual void					Startup( void );
	virtual void					Shutdown( void );
	virtual const unsigned char*	GetScanTable( void );
	
	// keyboard input polling
	virtual uint32_t				PollKeyboardInputEvents( void ) const override;
	virtual bool					ReturnKeyboardInputEvent( const uint32_t in_event, int& out_ch, bool& out_state ) const override;
	virtual void					EndKeyboardInputEvents( void ) override;
	
	// mouse polling
	virtual uint32_t				PollMouseInputEvents( void ) const;
	virtual bool					ReturnMouseInputEvent( const uint32_t in_event, int &out_action, int &out_value );
	virtual void					EndMouseInputEvents( void );

	virtual sysEvent_t				GenerateMouseButtonEvent( const int in_button, const bool in_down );
	virtual sysEvent_t 				GenerateMouseMoveEvent( const int32_t deltax, const int32_t deltay );

	// joystick input polling
	virtual uint32_t				JoystickCount( void );
	virtual idJoystick*				Joystick( const uint32_t in_ID );
	
protected:
	friend class crEvents;

	virtual void    AppendKeyboardEvent( const int in_key, const bool in_state ) override
	{
		m_kbdPolls.Append( { in_key, in_state } );
	}

    virtual void    AppendMouseEvents( const int in_action, const int in_value ) override
	{
		m_mousePolls.Append({ in_action, in_value } );
	}

	virtual void	AppendMouseMotion( const int in_motionX, const int in_motionY )
	{
		m_mousePolls.Append({ M_DELTAX, in_motionX } );
		m_mousePolls.Append({ M_DELTAY, in_motionY } );
	}

private:
	idStaticList<keyboardPoll_t, MAX_KEYBOARD_EVENTS>	m_kbdPolls;
	idStaticList<mousePoll_t, MAX_MOUSE_EVENTS>			m_mousePolls;
	idJoystickSDL3										m_joysticks[MAX_JOYSTICKS];
};


crInputSystem* crInputSystem::Get( void ) 
{
	static crInputSystemSDL3 gInputSystemSDL3 = crInputSystemSDL3();
	return &gInputSystemSDL3;
}

/*
================
crInputSystemSDL3::crInputSystemSDL3
================
*/
crInputSystemSDL3::crInputSystemSDL3( void )
{
}

/*
================
crInputSystemSDL3::~crInputSystemSDL3
================
*/
crInputSystemSDL3::~crInputSystemSDL3( void )
{
}

/*
================
crInputSystemSDL3::Startup
================
*/
void crInputSystemSDL3::Startup(void)
{
}

/*
================
crInputSystemSDL3::Shutdown
================
*/
void crInputSystemSDL3::Shutdown(void)
{
}

/*
================
crInputSystemSDL3::GetScanTable
================
*/
const unsigned char *crInputSystemSDL3::GetScanTable(void)
{
    return nullptr;
}

/*
================
crInputSystem::PollKeyboardInputEvents
================
*/
uint32_t crInputSystemSDL3::PollKeyboardInputEvents( void ) const
{
	return m_kbdPolls.Num();
}

/*
================
crInputSystem::ReturnKeyboardInputEvent
================
*/
bool crInputSystemSDL3::ReturnKeyboardInputEvent( const uint32_t n, int& key, bool& state ) const
{
	if( n >= m_kbdPolls.Num() )
		return false;
		
	key = m_kbdPolls[n].key;
	state = m_kbdPolls[n].state;
	return true;
}

/*
================
crInputSystem::EndKeyboardInputEvents
================
*/
void crInputSystemSDL3::EndKeyboardInputEvents( void )
{
	m_kbdPolls.SetNum( 0 );
}

/*
================
crInputSystem::PollMouseInputEvents
================
*/
uint32_t crInputSystemSDL3::PollMouseInputEvents(void) const
{
    return m_mousePolls.Num();
}

/*
================
crInputSystem::ReturnMouseInputEvent
================
*/
bool crInputSystemSDL3::ReturnMouseInputEvent(const uint32_t in_event, int &out_action, int &out_value)
{
	if( in_event >= m_mousePolls.Num() )
		return false;

	const auto mpoll = m_mousePolls[in_event];
	out_action = mpoll.action;
	out_value = mpoll.value;

	return true;
}

/*
================
crInputSystem::EndMouseInputEvents
================
*/
void crInputSystemSDL3::EndMouseInputEvents(void)
{
	m_mousePolls.SetNum( 0 );
}

sysEvent_t crInputSystemSDL3::GenerateMouseButtonEvent(const int in_button, const bool in_down)
{
    return sysEvent_t();
}

sysEvent_t crInputSystemSDL3::GenerateMouseMoveEvent(const int32_t deltax, const int32_t deltay)
{
    return sysEvent_t();
}

/*
================
crInputSystemSDL3::GamepadCount
================
*/
uint32_t crInputSystemSDL3::JoystickCount(void)
{
	// Updates SDL3's internal hardware states before querying them
	SDL_UpdateJoysticks();

	/// Future 
    return 0;
}

/*
================
crInputSystemSDL3::Gamepad
================
*/
idJoystick *crInputSystemSDL3::Joystick(const uint32_t in_ID)
{
    return &m_joysticks[in_ID];
}
