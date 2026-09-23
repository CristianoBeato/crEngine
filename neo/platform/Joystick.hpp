
#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_haptic.h>

/*
================================================
idJoystick is managed by each platform's local Sys implementation, and
provides full *Joy Pad* support (the most common device, these days).
================================================
*/

// Estrutura interna que a idTech usa para propagar as ações para o resto do motor
struct joystickAction_t 
{
    int actionType; // Ex: ID_JOYSTICK_BUTTON, ID_JOYSTICK_AXIS
    int index;      // Qual botão ou qual eixo
    int value;      // Valor (0 ou 1 para botão, -32768 a 32767 para analógicos)
};

enum js_type_t
{
	JT_GENERIC = 0,
	JT_XBOX,			// A/B/X/Y/RB/LB/RT/LT
	JT_PLAY,			// TRIANGLE, SQUARE, CROSS, 
};

class idJoystickSDL3 : public idJoystick
{
public:
    idJoystickSDL3( void );
	~idJoystickSDL3( void );
	
	bool	    Init( void  ) override;
	void	    Shutdown( void ) override;
	void	    Deactivate( void ) override;
	void	    SetRumble( const int rumbleLow, const int rumbleHigh ) override;
	uint32_t    PollInputEvents( void ) override;
	bool	    ReturnInputEvent( const uint32_t n, int& action, int& value ) override;
	void        EndInputEvents( void ) override;

private:
    int             									m_deadZone;
    uint32_t        									m_currentEventIndex;
    SDL_JoystickID  									m_JoystickID;
	SDL_HapticID										m_gamepadFeedbackID;
	SDL_Haptic*											m_gamepadFeedback;
    SDL_Gamepad*    									m_gamepadHandle;
	idStaticList<joystickAction_t, MAX_KEYBOARD_EVENTS>	m_pendingEvents;
};

void    Sys_InitGamepads( void );
void    Sys_ShutdownGamepads( void );
void	Sys_UpdateGamepads( void );
bool    Sys_JoystickConnect( const SDL_JoystickID in_JoystickID );
void    Sys_JoystickDisconnect( const SDL_JoystickID in_JoystickID );

#endif //!__JOYSTICK_H__