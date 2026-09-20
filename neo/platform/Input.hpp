
#ifndef __INPUT_HPP__
#define __INPUT_HPP__

// mouse input polling
inline constexpr uint32_t MAX_MOUSE_EVENTS = 256;
inline constexpr uint32_t MAX_KEYBOARD_EVENTS = 512;
inline constexpr uint32_t MAX_JOYSTICKS = 4; // Limit for Most consoles is 4 Controllers 

class crInputSystem
{
public:
	static crInputSystem*	Get( void );
	
	// input is tied to windows, so it needs to be started up and shut down whenever
	// the main window is recreated
	virtual void					Startup( void ) = 0;
	virtual void					Shutdown( void ) = 0;
	virtual const unsigned char*	GetScanTable( void ) = 0;
	
	// keyboard input polling

	/// @brief Retrieve the keyboard poll event count 
	/// @return 0 for none or error, and n for the count
	virtual uint32_t				PollKeyboardInputEvents( void ) const = 0;

	/// @brief Acess a event by index 
	/// @param in_event event index 
	/// @param out_ch the key id
	/// @param out_state the state of the key ( true pressed false released )
	/// @return true on success, false on error 
	virtual bool					ReturnKeyboardInputEvent( const uint32_t in_event, int& out_ch, bool& out_state ) const= 0;
	
	/// @brief Cleat the keyboard event queue 	
	virtual void					EndKeyboardInputEvents( void ) = 0;
	
	// mouse polling
	virtual uint32_t				PollMouseInputEvents( void ) const = 0;
	virtual bool					ReturnMouseInputEvent( const uint32_t in_event, int &out_action, int &out_value ) = 0;
	virtual void					EndMouseInputEvents( void ) = 0;

	virtual sysEvent_t				GenerateMouseButtonEvent( const int button, const bool down ) = 0;
	virtual sysEvent_t 				GenerateMouseMoveEvent( const int32_t deltax, const int32_t deltay ) = 0;

	// joystick input polling
	virtual uint32_t				PollJoystickInputEvents( const uint32_t in_deviceNum ) = 0;
	virtual bool					ReturnJoystickInputEvent( const uint32_t in_deviceNum, const uint32_t in_event, int& out_action, int& out_value ) = 0;
	virtual void					EndJoystickInputEvents( const uint32_t in_deviceNum ) = 0;    

	virtual uint32_t				GamepadCount( void ) = 0;
	virtual void					SetRumble( const int device, uint16_t in_low, uint16_t in_hi ) = 0;


protected:
	friend class crEventsSDL3;
	virtual void    AppendKeyboardEvent( const int in_key, const bool in_state ) = 0;
    virtual void    AppendMouseEvents( const int in_action, const int in_value ) = 0;
    virtual void	AppendMouseMotion( const int in_motionX, const int in_motionY ) = 0;
	virtual void    AppendJoysticEvent( const uint32_t in_device, const int in_button, const int in_value ) = 0;
};

#endif //!__INPUT_HPP__