
#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_gamepad.h>

/*
================================================
idJoystick is managed by each platform's local Sys implementation, and
provides full *Joy Pad* support (the most common device, these days).
================================================
*/
class idJoystick
{
public:
    idJoystick( void );
	~idJoystick( void );
	
	bool	    Init( void  );
	void	    Shutdown( void );
	void	    Deactivate( void );
	void	    SetRumble( const int rumbleLow, const int rumbleHigh );
	uint32_t    PollInputEvents( void );
	uint32_t    ReturnInputEvent( const int n, int& action, int& value );
	void        EndInputEvents( void );

private:
    int             m_deadZone;
    uint32_t        m_currentEventIndex;
    SDL_JoystickID  m_JoystickID;
    SDL_Gamepad*    m_gamepadHandle;
};

void    Sys_InitGamepads( void );
void    Sys_ShutdownGamepads( void );
bool    Sys_JoystickConnect( const SDL_JoystickID in_JoystickID );
void    Sys_JoystickDisconnect( const SDL_JoystickID in_JoystickID );

#endif //!__JOYSTICK_H__