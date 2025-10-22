
#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_haptic.h>

void    Sys_InitGamepads( void );
void    Sys_ShutdownGamepads( void );
bool    Sys_JoystickConnect( const SDL_JoystickID in_JoystickID );
void    Sys_JoystickDisconnect( const SDL_JoystickID in_JoystickID );

#endif //!__JOYSTICK_H__