#include "precompiled.h"
#include "sys/sys_local.h"
#include "sys/sys_public.h"
#include "joystick.h"

#include <SDL3/SDL_haptic.h>
#include <utility> // std::move

const uint32_t MAX_JOYSTICK_BUTTON_EVENTS = K_JOY_DPAD_RIGHT - K_JOY1 + 1;
const uint32_t MAX_JOYSTICK_EVENTS = SDL_GAMEPAD_BUTTON_COUNT + SDL_GAMEPAD_AXIS_COUNT;

constexpr int SDL3_TO_IDTECH_BUTTONS[SDL_GAMEPAD_BUTTON_COUNT] =	
{
    // SEE: SDL_GameControllerButton in SDL_gamecontroller.h 
    J_ACTION1,		// SDL_GAMEPAD_BUTTON_SOUTH ( A / CROSS )
	J_ACTION2,      // SDL_GAMEPAD_BUTTON_EAST ( B / CIRCLE
    J_ACTION3,		// SDL_GAMEPAD_BUTTON_WEST ( X / SQUARE )
	J_ACTION4,		// SDL_GAMEPAD_BUTTON_NORTH ( Y / TRIANGLE )
    J_ACTION10,		// SDL_GAMEPAD_BUTTON_BACK ( Back / SELECT )
	-1,				// SDL_GAMEPAD_BUTTON_GUIDE ( Unused ( Guide ) )
    J_ACTION9,		// SDL_GAMEPAD_BUTTON_START ( Start / OPTIONS )
    J_ACTION7,		// SDL_GAMEPAD_BUTTON_LEFT_STICK ( Left Stick Down / L3 ) 
	J_ACTION8,		// SDL_GAMEPAD_BUTTON_RIGHT_STICK ( Right Stick Down / R3 )		
    J_ACTION5,		// SDL_GAMEPAD_BUTTON_LEFT_SHOULDER ( Black (Left Shoulder / L1)
	J_ACTION6,		// SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER ( White ( Right Shoulder / R1 )
    J_DPAD_UP,		// SDL_GAMEPAD_BUTTON_DPAD_UP ( Up /  )
	J_DPAD_DOWN,	// SDL_GAMEPAD_BUTTON_DPAD_DOWN ( Down /  )
    J_DPAD_LEFT,	// SDL_GAMEPAD_BUTTON_DPAD_LEFT ( Left /  )
	J_DPAD_RIGHT,	// SDL_GAMEPAD_BUTTON_DPAD_RIGHT ( Right /  )
};

constexpr int SDL3_TO_IDTECH_AXIS[SDL_GAMEPAD_AXIS_COUNT] =
{
	J_AXIS_LEFT_X,		// SDL_GAMEPAD_AXIS_LEFTX 
	J_AXIS_LEFT_Y,		// SDL_GAMEPAD_AXIS_LEFTY 
	J_AXIS_RIGHT_X,		// SDL_GAMEPAD_AXIS_RIGHTX 		
	J_AXIS_RIGHT_Y,		// SDL_GAMEPAD_AXIS_RIGHTY 
	J_AXIS_LEFT_TRIG,	// SDL_GAMEPAD_AXIS_LEFT_TRIGGER ( LT / L2 )	
	J_AXIS_RIGHT_TRIG	// SDL_GAMEPAD_AXIS_RIGHT_TRIGGER ( RT R2 )
};


//=====================================================================================
//	Joystick Input Handling
//=====================================================================================

bool Sys_JoystickConnect( const SDL_JoystickID in_JoystickID )
{
	return true;
}

void Sys_JoystickDisconnect( const SDL_JoystickID in_JoystickID )
{
}

// ----------------------------------------------------------
// Inicialização
// ----------------------------------------------------------
void Sys_InitGamepads( void ) 
{
	int gamepadCount = 0;
    SDL_JoystickID * gamepadIds = nullptr;

    //if ( SDL_InitSubSystem(SDL_INIT_GAMEPAD) < 0) 
	//{
    //    common->Printf("SDL_INIT_GAMEPAD falhou: %s\n", SDL_GetError());
    //    return;
    //}

	idStr ControllerPath = Sys_DefaultBasePath();
	ControllerPath.Append("/base/gamecontrollerdb.txt");
	common->Printf( "Loading controller Mapping file \"%s\"\n",ControllerPath.c_str());
	SDL_AddGamepadMappingsFromFile( ControllerPath.c_str() );
	ControllerPath = Sys_DefaultSavePath();
	ControllerPath.Append("/gamecontrollerdb.txt");
	common->Printf( "Loading controller Mapping file \"%s\"\n",ControllerPath.c_str());
	SDL_AddGamepadMappingsFromFile(ControllerPath.c_str());

	// if we don't have any, don't lose time, just go out 
	if( !SDL_HasGamepad() )
		return;

    gamepadIds = SDL_GetGamepads( &gamepadCount );

    common->Printf("Detected Gamepads: %d\n", gamepadCount );

    for (int i = 0; i < gamepadCount && i < 4; i++) 
	{
		Sys_JoystickConnect( gamepadIds[i] );
    }
}

void Sys_ShutdownGamepads(void)
{
}


int Sys_PollJoystickInputEvents( int deviceNum )
{	
	return 0;
}

bool Sys_ReturnJoystickInputEvent( const int n, int& action, int& value )
{
	return false;
}

void Sys_EndJoystickInputEvents( void )
{
}

void Sys_SetRumble( int device, uint16_t low, uint16_t high )
{
}
