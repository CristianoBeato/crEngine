/*
	GWEN
	Copyright (c) 2012 Facepunch Studios
	See license in Gwen.h
*/

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"
#include "Gwen/Structures.h"
#include "Gwen/Input/SDL3.h"

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_video.h>

void Gwen::Platform::Sleep( unsigned int iMS )
{
	SDL_Delay( iMS );
}

static SDL_Cursor* cursor = nullptr;



void Gwen::Platform::SetCursor( unsigned char iCursor )
{
	SDL_Cursor* old = nullptr;
	SDL_SystemCursor syscur = SDL_SYSTEM_CURSOR_DEFAULT;

	switch ( iCursor )
	{
	case CursorType::Normal:
		syscur = SDL_SYSTEM_CURSOR_DEFAULT;
		break;

	case CursorType::Beam:
		syscur = SDL_SYSTEM_CURSOR_TEXT;
		break;

	case CursorType::SizeNS:
		syscur = SDL_SYSTEM_CURSOR_NS_RESIZE;
		break;

	case CursorType::SizeWE:
		syscur = SDL_SYSTEM_CURSOR_EW_RESIZE;
		break;

	case CursorType::SizeNWSE:
		syscur = SDL_SYSTEM_CURSOR_NESW_RESIZE;
		break;

	case CursorType::SizeNESW:
		syscur = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
		break;

	case CursorType::SizeAll:
		syscur = SDL_SYSTEM_CURSOR_MOVE;
		break;

	case CursorType::No:
		syscur = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
		break;

	case CursorType::Wait:
		syscur = SDL_SYSTEM_CURSOR_WAIT;
		break;

	case CursorType::Finger:
		syscur = SDL_SYSTEM_CURSOR_POINTER;
		break;

	default:
		syscur = SDL_SYSTEM_CURSOR_DEFAULT;
		break;
	}

	// store current cursor
	old = cursor;

	// create a new pointer
	cursor = SDL_CreateSystemCursor( syscur );

	// set the cursor
	SDL_SetCursor( cursor );

	// release the previous cursor pointer
	if ( old != nullptr )
		SDL_DestroyCursor( old );
}

Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
	char* cpb = nullptr;
	Gwen::TextObject inString;
	if ( !SDL_HasClipboardText() )
		return Gwen::UnicodeString();

	// aquire the clipboard
	cpb = SDL_GetClipboardText();

	inString = cpb;

	SDL_free( cpb );

	return inString.GetUnicode();
}

bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString & str )
{
	Gwen::TextObject outString = str;
	
	// clear the clipboard before we set a new content 
	SDL_ClearClipboardData();

	return SDL_SetClipboardText( outString.c_str() );
}

float Gwen::Platform::GetTimeInSeconds()
{
	uint64_t time = SDL_GetTicks();
	return time / 1000;
}

bool Gwen::Platform::FileOpen( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
	return false;
}

bool Gwen::Platform::FileSave( const String & Name, const String & StartPath, const String & Extension, Gwen::Event::Handler* pHandler, Gwen::Event::Handler::FunctionWithInformation fnCallback )
{
	// No platform independent way to do this.
	// Ideally you would open a system dialog here
	return false;
}

bool Gwen::Platform::FolderOpen( const String & Name, const String & StartPath, Gwen::Event::Handler* pHandler, Event::Handler::FunctionWithInformation fnCallback )
{
	return false;
}

void* Gwen::Platform::CreatePlatformWindow( int x, int y, int w, int h, const Gwen::String & strWindowTitle )
{
	// create a OpenGL capable window
	SDL_Window* window = SDL_CreateWindow( strWindowTitle.c_str(), w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS );
	return reinterpret_cast<void*>( window );
}

void Gwen::Platform::DestroyPlatformWindow( void* pPtr )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	SDL_DestroyWindow( window );
}

void Gwen::Platform::MessagePump( void* pWindow, Gwen::Controls::Canvas* ptarget )
{
	SDL_Event evt;
	static Input::SDL3 GwenInput;

	GwenInput.Initialize( ptarget );

#if 1
	while ( SDL_PollEvent( &evt ) )
	{
		GwenInput.ProcessEvent( &evt );
	}
#else
	if( SDL_WaitEvent( &evt ) )
		GwenInput.ProcessEvent( &evt );
#endif
}

void Gwen::Platform::SetBoundsPlatformWindow( void* pPtr, int x, int y, int w, int h )
{	
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	SDL_SetWindowPosition( window, x, y );
	SDL_SetWindowSize( window, w, h );
}

void Gwen::Platform::SetWindowMaximized( void* pPtr, bool bMax, Gwen::Point & pNewPos, Gwen::Point & pNewSize )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	if ( bMax )
		SDL_MaximizeWindow( window );
	else
		SDL_RestoreWindow( window );

	// Wait for window to update
	SDL_SyncWindow( window );

	// Get new bound 
	SDL_GetWindowPosition( window, &pNewPos.x, &pNewPos.y );
	SDL_GetWindowSize( window, &pNewSize.x, &pNewSize.y );	
}

void Gwen::Platform::SetWindowMinimized( void* pPtr, bool bMinimized )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	if ( !window )
		return;

	if ( bMinimized )
		SDL_MinimizeWindow( window );
	else
		SDL_RestoreWindow( window );
}

bool Gwen::Platform::HasFocusPlatformWindow( void* pPtr )
{
	SDL_Window* window = static_cast<SDL_Window*>( pPtr );
	auto flags = SDL_GetWindowFlags( window );
	return ( flags & SDL_WINDOW_INPUT_FOCUS ) || ( flags & SDL_WINDOW_MOUSE_FOCUS );
}

void Gwen::Platform::GetDesktopSize( int & w, int & h )
{
	SDL_Rect rect{};
	if ( !SDL_GetDisplayUsableBounds( 0, &rect ) )
	{
		w = 1024;
		h = 768;
		return;
	}

	w = rect.w;
	h = rect.h;
}

void Gwen::Platform::GetCursorPos( Gwen::Point & po )
{
	float x = 0.0f;
	float y = 0.0f;
	SDL_GetGlobalMouseState( &x, &y );

}
