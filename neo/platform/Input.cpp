
#include "Input.hpp"
#include "Platform.hpp"

#include <SDL3/SDL_gamepad.h>

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

struct joysticPoll_t
{
	int button = 0;
	int value = 0;
};

class crInputSystemSDL3 : public crInputSystem
{
public:
	crInputSystemSDL3( void );
	~crInputSystemSDL3( void );

	// input is tied to windows, so it needs to be started up and shut down whenever
	// the main window is recreated
	virtual void					Startup( void );
	virtual void					Shutdown( void );
	virtual const unsigned char*	GetScanTable( void );
	
	// keyboard input polling
	virtual uint32_t				PollKeyboardInputEvents( void ) const;
	virtual bool					ReturnKeyboardInputEvent( const uint32_t in_event, int& out_ch, bool& out_state ) const;
	virtual void					EndKeyboardInputEvents( void );
	
	// mouse polling
	virtual uint32_t				PollMouseInputEvents( void ) const;
	virtual bool					ReturnMouseInputEvent( const uint32_t in_event, int &out_action, int &out_value );
	virtual void					EndMouseInputEvents( void );

	virtual sysEvent_t				GenerateMouseButtonEvent( const int in_button, const bool in_down );
	virtual sysEvent_t 				GenerateMouseMoveEvent( const int32_t deltax, const int32_t deltay );

	// joystick input polling
	virtual uint32_t				PollJoystickInputEvents( const uint32_t in_deviceNum );
	virtual bool					ReturnJoystickInputEvent( const uint32_t in_deviceNum, const uint32_t in_event, int& out_action, int& out_value );
	virtual void					EndJoystickInputEvents( const uint32_t in_deviceNum );

	virtual uint32_t				GamepadCount( void ) = 0;
	virtual void					SetRumble( const int device, uint16_t in_low, uint16_t in_hi );
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

    virtual void    AppendJoysticEvent( const uint32_t in_device, const int in_button, const int in_value ) override
	{
		// TODO: clamp device
		m_joysticPolls[in_device].Append( { in_button, in_value } );
	}

private:
	idStaticList<keyboardPoll_t, MAX_KEYBOARD_EVENTS>	m_kbdPolls;
	idStaticList<mousePoll_t, MAX_MOUSE_EVENTS>			m_mousePolls;
	idStaticList<joysticPoll_t, MAX_KEYBOARD_EVENTS>	m_joysticPolls[MAX_JOYSTICKS];
};

crInputSystemSDL3::crInputSystemSDL3( void ) : crInputSystem()
{
}

crInputSystemSDL3::~crInputSystemSDL3( void )
{
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

/*
================
crInputSystem::PollJoystickInputEvents
================
*/
uint32_t crInputSystemSDL3::PollJoystickInputEvents( const uint32_t in_deviceNum )
{
	/// TODO check if device is available
    return m_joysticPolls[in_deviceNum].Num();
}

/*
================
crInputSystem::ReturnJoystickInputEvent
================
*/
bool crInputSystemSDL3::ReturnJoystickInputEvent(const uint32_t in_deviceNum, const uint32_t in_event, int &out_action, int &out_value)
{
	/// TODO: if not device available return false

	if( in_event >= m_joysticPolls[in_deviceNum].Num() )
		return false;

	const auto jpoll = m_joysticPolls[in_deviceNum][in_event];
	out_action = jpoll.button;
	out_value = jpoll.value;

    return true;
}

/*
================
crInputSystem::EndJoystickInputEvents
================
*/
void crInputSystemSDL3::EndJoystickInputEvents(const uint32_t in_deviceNum)
{
	/// TODO: yah yow know, just do it...

	/// Don't resize to don't reallocate memory 
	m_joysticPolls[in_deviceNum].SetNum( 0 );
}
