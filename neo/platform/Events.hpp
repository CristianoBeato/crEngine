
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
	static crEvents*		Get( void );

	virtual void			StartUp( void ) = 0;
	virtual void			ShutDown( void ) = 0;
    virtual void        	PumpEvents( void ) = 0;
    virtual void        	GenerateEvents( void ) = 0;
    virtual sysEvent_t		GetEvent( void ) = 0;
    virtual void        	ClearEvents( void ) = 0;
	virtual unsigned char	GetConsoleKey( const bool in_shifted ) = 0;
    virtual void        	QueEvent( const sysEventType_t in_type, 
				const int in_value, 
				const int in_value2, 
				const size_t in_ptrLength, 
				const void* in_ptr, 
				const int in_inputDeviceNum ) = 0;
};

#endif //!__EVENTS_HPP__