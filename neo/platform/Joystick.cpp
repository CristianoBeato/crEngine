#include "precompiled.h"
#include "sys/sys_local.h"
#include "sys/sys_public.h"
#include "Joystick.hpp"

#include <SDL3/SDL_init.h>
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
//	idJoystickSDL3 Input Handling
//=====================================================================================

idJoystickSDL3::idJoystickSDL3(void) :
    m_deadZone( 0 ),
    m_currentEventIndex( 0 ),
    m_JoystickID( 0 ),
    m_gamepadFeedback( 0 ),
    m_gamepadHandle( nullptr )
{
}

idJoystickSDL3::~idJoystickSDL3( void )
{
    Shutdown();
}

bool idJoystickSDL3::Init(void)
{
    // check if are a suported gamepad 
    if ( !SDL_IsGamepad( m_JoystickID ) )
    {
        // TODO: Print a warning ?
        return false;
    }

    m_gamepadHandle = SDL_OpenGamepad( m_JoystickID );
    if ( m_gamepadHandle )
    {
        idLib::Warning( "Failed to initialize gamepad!" );
        return false;
    }

    auto type = SDL_GetGamepadType( m_gamepadHandle );
    if( type == SDL_GAMEPAD_TYPE_PS3 || type == SDL_GAMEPAD_TYPE_PS4 || type == SDL_GAMEPAD_TYPE_PS5 )
    {
        // i will do latter
    }
    else if( type == SDL_GAMEPAD_TYPE_XBOX360 || type == SDL_GAMEPAD_TYPE_XBOXONE )
    {
        // latter 
    }
    else
    {
        // its generic
    }

#if 0
    m_gamepadFeedback = SDL_OpenHapticFromJoystick( m_JoystickID );

    // Initialize simple rumble
    if ( !SDL_InitHapticRumble( m_gamepadFeedback ) )
        SDL_CloseHaptic( m_gamepadFeedback );
 
    SDL_HapticEffect hapticEffect{};
#endif

    return true;
}

void idJoystickSDL3::Shutdown(void)
{
#if 0
    if( m_gamepadFeedback )
    {
        SDL_CloseHaptic( m_gamepadFeedback );
        m_gamepadFeedback = nullptr;
    }
#endif

	if( m_gamepadHandle )
	{
		SDL_CloseGamepad( m_gamepadHandle );
		m_gamepadHandle = nullptr;
	}
}

void idJoystickSDL3::Deactivate( void )
{
	// Se o jogo for minimizado, você pode zerar os estados se quiser
	m_pendingEvents.Clear();
	m_currentEventIndex = 0;
}

static inline uint16_t normalizeTo16Bits( const int32_t fullval )
{
	if( fullval <= 0 )
		return 0;

	double proportional = ( fullval / INT32_MAX) * UINT16_MAX; // Rule of three / Proportional normalization
	return static_cast<uint16_t>( proportional + 0.5 ); // Converts back to a 16-bit integer (with standard rounding)
}

void idJoystickSDL3::SetRumble( const int rumbleLow, const int rumbleHigh )
{
    if ( !m_gamepadHandle || !m_gamepadFeedback )
		return; 

#if 0
    SDL_PlayHapticRumble( m_gamepadFeedback, 0.5, 1000 );
#else
    // Simple conversion to the 16-bit scale required by SDL3 (0 to 65535)
    Uint16 low = normalizeTo16Bits( rumbleLow );
    Uint16 high = normalizeTo16Bits( rumbleHigh );
    SDL_RumbleGamepad( m_gamepadHandle, low, high, 16u );
#endif 
}

uint32_t idJoystickSDL3::PollInputEvents( void )
{
	m_pendingEvents.Clear(); // Make surre that we have cleared the states 
	m_currentEventIndex = 0;

	// Updates SDL3's internal hardware states before querying them
	SDL_UpdateJoysticks(); // Move, here we gola lose some events

	// SCANNING THE BUTTONS (Button Polling)
    // Map the standard buttons that idTech 4 expects. 
	static SDL_GamepadButton botoesParaVerificar[] = 
	{
            SDL_GAMEPAD_BUTTON_SOUTH,   // A / X
            SDL_GAMEPAD_BUTTON_EAST,    // B / O
            SDL_GAMEPAD_BUTTON_WEST,    // X / Quadrado
            SDL_GAMEPAD_BUTTON_NORTH,   // Y / Triângulo
            SDL_GAMEPAD_BUTTON_START,   // start/ right
            SDL_GAMEPAD_BUTTON_BACK     // select / back 
    };

	for ( auto btn : botoesParaVerificar ) 
	{
        // Pergunta o estado exato e atual do botão direto para a memória do SDL3
        bool isPressed = SDL_GetGamepadButton( m_gamepadHandle, btn);
            
        joystickAction_t action{};
        action.actionType = 1; // Substitua pelo ID correspondente de botão na idTech (ex: SE_KEY)
        action.index = btn;
        action.value = isPressed ? 1 : 0;
            
        m_pendingEvents.Append( action );
    }

    // 2. FAZENDO VARREDURA DOS ANALÓGICOS (Polling de Eixos)
    SDL_GamepadAxis eixosParaVerificar[] = 
	{
        SDL_GAMEPAD_AXIS_LEFTX,
        SDL_GAMEPAD_AXIS_LEFTY,
        SDL_GAMEPAD_AXIS_RIGHTX,
        SDL_GAMEPAD_AXIS_RIGHTY
    };

    for (auto axis : eixosParaVerificar) 
	{
        // Pergunta a posição exata atual do analógico
        Sint16 axisValue = SDL_GetGamepadAxis( m_gamepadHandle, axis);

        // Aplica filtro de deadzone direto no polling
        if ( axisValue < - m_deadZone || axisValue > m_deadZone ) 
		{
            joystickAction_t action{};
            action.actionType = 2; // Substitua pelo ID de eixo na idTech (ex: SE_AXIS)
            action.index = axis;
            action.value = axisValue;
            m_pendingEvents.Append( action );
        } 
		else 
		{
            // Envia 0 para indicar que o analógico voltou ao centro estabilizado
            joystickAction_t action{};
            action.actionType = 2;
            action.index = axis;
            action.value = 0;
            m_pendingEvents.Append( action );
        }
    }

    // Retorna a quantidade de estados capturados nesta rodada para o motor saber que há dados
	return m_pendingEvents.Num();
}

bool idJoystickSDL3::ReturnInputEvent( const uint32_t n, int &action, int &value )
{
    if ( m_currentEventIndex >= m_pendingEvents.Num())
		return false; // End read states 

    const auto& ev = m_pendingEvents[m_currentEventIndex];
    action = ev.actionType;

    // Dependendo de como a idTech mapeia, junte o tipo do botão/eixo aqui:
    // value = ev.value;
    
    m_currentEventIndex++;
    return true; // Indica que retornou com sucesso um dado de input
}

void idJoystickSDL3::EndInputEvents( void )
{
	m_pendingEvents.Clear();
}

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

	idStr ControllerPath = crPaths::Get()->DefaultBasePath();
	ControllerPath.Append("/base/gamecontrollerdb.txt");
	common->Printf( "Loading controller Mapping file \"%s\"\n",ControllerPath.c_str());
	SDL_AddGamepadMappingsFromFile( ControllerPath.c_str() );
	ControllerPath = crPaths::Get()->DefaultSavePath();
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
