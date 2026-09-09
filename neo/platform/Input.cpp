
#include "Input.hpp"
#include "Platform.hpp"

void crGamepad::Init()
{
    
}

void crGamepad::Release(void)
{
    m_gamePadId = -1;
	m_gamePad = nullptr;
	m_activeEvents = 0;
	m_gameJoyStick = nullptr;


    ClearState();
}

void crGamepad::ClearState(void)
{
    // clear button states 
	for( auto i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; i++)
    {
		m_oldState[i] = 0;
    }

    // clear axis states
    for( auto i = 0;i < SDL_GAMEPAD_AXIS_COUNT; i++) 
    {
		m_oldAxisState[i] = 0;
	}

	for( auto i = 0; i < MAX_CONTROLLER_BUTTON_EVENTS;i++)
    {
        m_oldButtonStates[i] = false;
    }
}

/*
================
crInputSystem::PollKeyboardInputEvents
================
*/
int crInputSystem::PollKeyboardInputEvents( void )
{
	return m_kbdPolls.Num();
}

/*
================
crInputSystem::ReturnKeyboardInputEvent
================
*/
int crInputSystem::ReturnKeyboardInputEvent( const int n, int& key, bool& state )
{
	if( n >= m_kbdPolls.Num() )
		return 0;
		
	key = m_kbdPolls[n].key;
	state = m_kbdPolls[n].state;
	return 1;
}

/*
================
crInputSystem::EndKeyboardInputEvents
================
*/
void crInputSystem::EndKeyboardInputEvents( void )
{
	m_kbdPolls.SetNum( 0 );
}


/*
================
crInputSystem::PollMouseInputEvents
================
*/
int crInputSystem::PollMouseInputEvents( int mouseEvents[MAX_MOUSE_EVENTS][2] )
{
	int numEvents = m_mousePolls.Num();
	
	if( numEvents > MAX_MOUSE_EVENTS )
		numEvents = MAX_MOUSE_EVENTS;
	
	
	for( int i = 0; i < numEvents; i++ )
	{
		const mouse_poll_t& mp = m_mousePolls[i];
		
		mouseEvents[i][0] = mp.action;
		mouseEvents[i][1] = mp.value;
	}
	
	m_mousePolls.SetNum( 0 );
	
	return numEvents;
}