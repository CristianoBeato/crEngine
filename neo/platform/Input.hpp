
#ifndef __INPUT_HPP__
#define __INPUT_HPP__

#include <SDL3/SDL_gamepad.h>

// mouse input polling
inline constexpr uint32_t MAX_MOUSE_EVENTS = 256;
inline constexpr uint32_t MAX_KEYBOARD_EVENTS = 512;
inline constexpr uint32_t MAX_JOYSTICKS = 4; // Limit for Most consoles is 4 Controllers 

inline constexpr uint32_t MAX_CONTROLLER_BUTTON_EVENTS = K_JOY_DPAD_RIGHT - K_JOY1 + 1;
inline constexpr uint32_t EVENTS_MAX_CONTROLLER_EVENTS = SDL_GAMEPAD_BUTTON_COUNT + SDL_GAMEPAD_AXIS_COUNT;

class crGamepad
{
public:
	crGamepad( void );
	~crGamepad( void );

	void	Init( );

	void	Release( void );
	void	ClearState( void );

private:
	int 	m_oldState[SDL_GAMEPAD_BUTTON_COUNT];
	int 	m_oldAxisState[SDL_GAMEPAD_AXIS_COUNT];
	bool	m_oldButtonStates[MAX_CONTROLLER_BUTTON_EVENTS];
	int 	m_activeEvents;
	struct
	{
		int event;
		int value;
	} 		m_events[ EVENTS_MAX_CONTROLLER_EVENTS ];

	SDL_JoystickID 	m_gamePadId;
	SDL_Joystick* 	m_gameJoyStick;
	SDL_Gamepad*	m_gamePad;
};

class crInputSystem
{
public:
	static crInputSystem*	Get( void );
	crInputSystem( void ) {};
	~crInputSystem( void ) {};

	// input is tied to windows, so it needs to be started up and shut down whenever
	// the main window is recreated
	void						Init( void );
	void						Shutdown( void );
	
	virtual const unsigned char*	GetScanTable( void );
	
	// keyboard input polling
	int							PollKeyboardInputEvents( void );
	int							ReturnKeyboardInputEvent( const int n, int& ch, bool& state );
	void						EndKeyboardInputEvents( void );
	
	// mouse polling
	int							PollMouseInputEvents( int mouseEvents[MAX_MOUSE_EVENTS][2] );
	sysEvent_t					GenerateMouseButtonEvent( const int button, const bool down );
	sysEvent_t 					GenerateMouseMoveEvent( const int32_t deltax, const int32_t deltay );

	// joystick input polling
	uint32_t					GamepadCount( void );
	void						SetRumble( const int device, uint16_t in_low, uint16_t in_hi );
	int							PollJoystickInputEvents( const int in_deviceNum );
	bool						ReturnJoystickInputEvent( const int n, int& in_action, int& in_value );
	void						EndJoystickInputEvents( void );

protected:
    friend class crEvents;

	// keyboard event storage structure
	struct kbd_poll_t
	{
		int		key;
		bool	state;
		
		kbd_poll_t( void )
		{
		}
		
		kbd_poll_t( const int k, const bool s )
		{
			key = k;
			state = s;
		}
	};

	// mouse event storage structure
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

	struct joystic_poll_t
	{
		joystic_poll_t( void )
		{
		}

		joystic_poll_t( const int b, const int v )
		{
			button = b;
			value = v;
		}

		int button;
		int value;
	};

	idStaticList<kbd_poll_t, MAX_KEYBOARD_EVENTS>		m_kbdPolls;
	idStaticList<mouse_poll_t, MAX_MOUSE_EVENTS>		m_mousePolls;
	idStaticList<joystic_poll_t, MAX_KEYBOARD_EVENTS>	m_joysticPolls[MAX_JOYSTICKS];

    void    AppendKeyboardEvent();
    void    AppendMouseEvents();
    inline void    AppendJoysticEvent( const uint32_t device, const int button, const int value )
	{
		m_joysticPolls[device].
	}
};

#endif //!__INPUT_HPP__