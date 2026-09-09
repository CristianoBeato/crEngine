
#ifndef __EVENTS_HPP__
#define __EVENTS_HPP__

enum sysEventType_t
{
	SE_NONE,				// evTime is still valid
	SE_KEY,					// evValue is a key code, evValue2 is the down flag
	SE_CHAR,				// evValue is an ascii char
	SE_MOUSE,				// evValue and evValue2 are reletive signed x / y moves
	SE_MOUSE_ABSOLUTE,		// evValue and evValue2 are absolute coordinates in the window's client area.
	SE_MOUSE_LEAVE,			// evValue and evValue2 are meaninless, this indicates the mouse has left the client area.
	SE_JOYSTICK,		// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE				// evPtr is a char*, from typing something at a non-game console
};

struct sysEvent_t
{
	sysEventType_t	evType;
	int				evValue;
	int				evValue2;
	int				inputDevice;
	size_t			evPtrLength;		// bytes of data pointed to by evPtr, for journaling
	void* 			evPtr;				// this must be manually freed if not nullptr

	inline const bool		IsKeyEvent( void ) const { return evType == SE_KEY; }
	inline const bool		IsMouseEvent( void ) const { return evType == SE_MOUSE; }
	inline const bool		IsCharEvent( void ) const { return evType == SE_CHAR; }
	inline const bool		IsJoystickEvent( void ) const { return evType == SE_JOYSTICK; }
	inline const bool		IsKeyDown( void ) const { return evValue2 != 0; }
	inline const keyNum_t	GetKey( void ) const { return static_cast< keyNum_t >( evValue ); }
	inline const int		GetXCoord( void ) const { return evValue; }
	inline const int	 	GetYCoord( void ) const { return evValue2; }
};

class crEvents
{
public:
	static crEvents*	Get( void );

	void		StartUp( void );
	void		ShutDown( void );

    void        PumpEvents( void );
    void        GenerateEvents( void );
    void        QueEvent( const sysEventType_t type, 
				const int value, 
				const int value2, 
				const size_t ptrLength, 
				const void *ptr, 
				const int inputDeviceNum );
    sysEvent_t  GetEvent( void );
    void        ClearEvents( void );

private:
	int					m_eventHead;
	idList<sysEvent_t>	m_event_queue;

	static void 		PushConsoleEvent( const char* s );
	static bool SDLCALL HandleSDLEvents( void *userdata, SDL_Event *event );
};

#endif //!__EVENTS_HPP__